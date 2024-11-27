// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorLineTraceComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ROS2Utilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Async/ParallelFor.h"
#include "CollisionQueryParams.h"
#include "Components/PrimitiveComponent.h"
#include "Containers/ArrayView.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

// Standard library includes.
#include <algorithm>
#include <tuple>

UAGX_LidarSensorLineTraceComponent::UAGX_LidarSensorLineTraceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace AGX_LidarSensorLineTraceComponent_helpers
{
	void DrawDebugPoints(
		TArrayView<FAGX_LidarScanPoint> Points, UWorld* World, const FTransform& LocalFrame,
		float Size, float Lifetime)
	{
		if (World == nullptr)
			return;

		for (const auto& P : Points)
		{
			if (!P.bIsValid)
				continue;

			const FVector PGlobal = LocalFrame.TransformPositionNoScale(P.Position);
			DrawDebugPoint(World, PGlobal, Size, FColor::Red, false, Lifetime);
		}
	}

	double ApproximateIntensity(
		const FHitResult& HitResult, const FVector_NetQuantizeNormal& Direction,
		double BeamExitRadius, double BeamDivergenceRad)
	{
		// Intensity based on angle of incident.
		double Intensity = std::max(0.0, -Direction.Dot(HitResult.Normal));

		// Take material Roughness into account.
		if (HitResult.Component != nullptr)
		{
			UMaterialInterface* MaterialInterf = HitResult.Component->GetMaterial(0);
			if (MaterialInterf != nullptr)
			{
				FMaterialParameterInfo Info;
				Info.Name = TEXT("Roughness");
				float Roughness;
				if (MaterialInterf->GetScalarParameterValue(Info, Roughness))
				{
					Intensity *= (1.0 - FMath::Clamp(static_cast<double>(Roughness), 0.0, 1.0));
				}
			}
		}

		// Take beam divergence (drop off over distance) into account.
		// The beam is shaped like a cone. Here we simply take the beam area as it exists the Lidar
		// Sensor in relation to the beam area at the target.
		// So Fraction = AreaExit / AreaTarget.
		// AreaExit = pi * ExitRadius^2
		// AreaTarget = pi * (ExitRadius + 2 * Sin(Divergence / 2) * Distance)^2
		// We approximate 2 * Sin(Divergence / 2) to Sin(Divergence) since we have really small
		// angles.
		// We can rewrite this to:
		Intensity *= FMath::Pow(
			1.0 / (1.0 + FMath::Sin(BeamDivergenceRad) * HitResult.Distance / BeamExitRadius), 2.0);

		return Intensity;
	}

	struct LidarScanRequestParams
	{
		LidarScanRequestParams() = default;

		LidarScanRequestParams(
			const FTransform& InOrigin, const FVector2D& InFOV, const FVector2D& InResolution)
			: Origin(InOrigin)
			, FOV(InFOV)
			, Resolution(InResolution)
		{
		}

		FTransform Origin {FTransform::Identity};
		FVector2D FOV {FVector2D::ZeroVector};
		FVector2D Resolution {FVector2D::ZeroVector};
		FVector2D FOVWindowX {FVector2D::ZeroVector};
		FVector2D FOVWindowY {FVector2D::ZeroVector};
		double TimeStamp {0.0};
		double FractionStart {0.0};
		double FractionEnd {0.0};
		double Range {0.0};
		double BeamExitRadius {0.0};
		double BeamDivergenceRad {0.0}; // In radians.
		EAGX_LidarLineTraceScanPattern ScanPattern {
			EAGX_LidarLineTraceScanPattern::HorizontalSweep};
		bool bCalculateIntensity {true};
	};

	TArrayView<FAGX_LidarScanPoint> PerformPartialScanCPU(
		UWorld* World, const LidarScanRequestParams& Params, TArray<FAGX_LidarScanPoint>& OutData)
	{
		if (World == nullptr || Params.FractionEnd <= Params.FractionStart || Params.Range <= 0.0)
			return {};

		// The scan pattern implemented below is row-wise linear sweep.

		const int32 NumRaysCycleX = static_cast<int32>(Params.FOV.X / Params.Resolution.X);
		const int32 NumRaysCycleY = static_cast<int32>(Params.FOV.Y / Params.Resolution.Y);
		if (NumRaysCycleX <= 0 || NumRaysCycleY <= 0)
			return {};

		const int32 NumRaysCycle = NumRaysCycleX * NumRaysCycleY;
		const double NumRaysCycled = static_cast<double>(NumRaysCycle);
		const int32 FirstRay =
			std::max(static_cast<int32>(NumRaysCycled * Params.FractionStart), 0);
		const int32 LastRay =
			std::min(FMath::RoundToInt32(NumRaysCycled * Params.FractionEnd), NumRaysCycle - 1);

		const FVector StartGlobal = Params.Origin.GetLocation();
		const int32 NumPointsPreAppend = OutData.Num();
		const int32 NumRays = LastRay - FirstRay + 1;
		if (NumRays <= 0)
			return {};

		// Make room for the new points.
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutData.SetNumUninitialized(NumPointsPreAppend + NumRays, false);
#else
		OutData.SetNumUninitialized(NumPointsPreAppend + NumRays, EAllowShrinking::No);
#endif

		// This number is somewhat arbitrary, but a good starting point.
		// The cost of starting several threads will at some point make single threaded execution
		// a better option.
		static constexpr int32 MinRaysForMultithread = 300;
		const bool RunMultithreaded =
			FPlatformProcess::SupportsMultithreading() && NumRays >= MinRaysForMultithread;
		const int32 NumThreads =
			RunMultithreaded ? FPlatformMisc::NumberOfWorkerThreadsToSpawn() : 1;
		const int32 NumRaysPerThread = NumRays / NumThreads;
		FCollisionQueryParams CollParams;
		CollParams.bTraceComplex = true;

		auto RayToAngles = [&](int32 Ray) -> std::tuple<double, double>
		{
			switch (Params.ScanPattern)
			{
				case EAGX_LidarLineTraceScanPattern::HorizontalSweep:
				{
					const int32 IndexX = Ray / NumRaysCycleY;
					const int32 IndexY = Ray % NumRaysCycleY;
					return {
						-Params.FOV.X / 2.0 + Params.Resolution.X * static_cast<double>(IndexX),
						-Params.FOV.Y / 2.0 + Params.Resolution.Y * static_cast<double>(IndexY)};
				}
				case EAGX_LidarLineTraceScanPattern::VerticalSweep:
				{
					const int32 IndexX = Ray % NumRaysCycleX;
					const int32 IndexY = Ray / NumRaysCycleX;
					return {
						-Params.FOV.X / 2.0 + Params.Resolution.X * static_cast<double>(IndexX),
						-Params.FOV.Y / 2.0 + Params.Resolution.Y * static_cast<double>(IndexY)};
				}
			}

			UE_LOG(LogAGX, Warning, TEXT("Unknown Scan Pattern used in Lidar Sensor."));
			return {0.0, 0.0};
		};

		auto MultiThreadRayCasts = [&](int32 Thread)
		{
			const int32 ThreadFirstRay = FirstRay + Thread * NumRaysPerThread;
			int32 ThreadLastRayPlusOne = FirstRay + (Thread + 1) * NumRaysPerThread;

			// The last thread takes any extra rays that couldn't be evenly distributed over all
			// thread.
			if (Thread == NumThreads - 1)
				ThreadLastRayPlusOne = LastRay + 1;

			FHitResult HitResult;
			for (int32 Ray = ThreadFirstRay; Ray < ThreadLastRayPlusOne; Ray++)
			{
				const int32 OutDataIndex = Ray - FirstRay + NumPointsPreAppend;
				const auto [AngX, AngY] = RayToAngles(Ray);
				if (AngX < Params.FOVWindowX.X || AngX > Params.FOVWindowX.Y ||
					AngY < Params.FOVWindowY.X || AngY > Params.FOVWindowY.Y)
				{
					// Outside the FOVWindow, no need to scan this direction.
					OutData[OutDataIndex] = FAGX_LidarScanPoint(false);
					continue;
				}

				const double AngRadX = FMath::DegreesToRadians(AngX);
				const double AngRadY = FMath::DegreesToRadians(AngY);

				// Dir is the normalized vector following the current laser ray. Here X is
				// horizontal and Y vertical.
				const FVector Dir(
					FMath::Sin(AngRadX) * FMath::Cos(AngRadY), FMath::Sin(AngRadY),
					FMath::Cos(AngRadX) * FMath::Cos(AngRadY));

				// Lidar sensors local coordinate system is x forwards, y to the right and z up.
				const FVector EndLocal(
					Dir.Z * Params.Range, Dir.X * Params.Range, -Dir.Y * Params.Range);
				const FVector EndGlobal = Params.Origin.TransformPositionNoScale(EndLocal);
				if (!World->LineTraceSingleByChannel(
						HitResult, StartGlobal, EndGlobal, ECC_Visibility, CollParams))
				{
					// Line trace miss.
					OutData[OutDataIndex] = FAGX_LidarScanPoint(false);
					continue;
				}

				const FVector LocalPoint =
					Params.Origin.InverseTransformPositionNoScale(HitResult.Location);
				double Intensity = 0.0;
				if (Params.bCalculateIntensity)
				{
					const FVector_NetQuantizeNormal DirGlobal(
						(EndGlobal - StartGlobal).GetSafeNormal());
					Intensity = ApproximateIntensity(
						HitResult, DirGlobal, Params.BeamExitRadius, Params.BeamDivergenceRad);
				}
				OutData[OutDataIndex] = FAGX_LidarScanPoint(
					FVector(LocalPoint.X, LocalPoint.Y, LocalPoint.Z), Params.TimeStamp, Intensity,
					true);
			}
		};

		ParallelFor(NumThreads, MultiThreadRayCasts);
		return MakeArrayView(&OutData[NumPointsPreAppend], NumRays);
	}
}

void UAGX_LidarSensorLineTraceComponent::RequestManualScan(
	double FractionStart, double FractionEnd, FVector2D FOVWindowX, FVector2D FOVWindowY)
{
	using namespace AGX_LidarSensorLineTraceComponent_helpers;
	if (!bIsValid || !bEnabled)
		return;

	if (ExecutionMode != EAGX_LidarLineTraceExecutonMode::Manual)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("RequestManualScan was called on Lidar Sensor '%s' in Actor '%s' but the "
				 "ExecutionMode is not set to Manual. Set ExecutionMode to Manual before calling "
				 "this function."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FractionStart = FMath::Clamp(FractionStart, 0.0, 1.0);
	FractionEnd = FMath::Clamp(FractionEnd, 0.0, 1.0);
	if (FractionEnd <= FractionStart)
		return;

	if (FOVWindowX.IsNearlyZero())
		FOVWindowX = {-FOV.X / 2.0, FOV.X / 2.0};
	if (FOVWindowY.IsNearlyZero())
		FOVWindowY = {-FOV.Y / 2.0, FOV.Y / 2.0};

	LidarScanRequestParams Params(GetComponentTransform(), FOV, Resolution);
	{
		Params.FOVWindowX = FOVWindowX;
		Params.FOVWindowY = FOVWindowY;
		Params.TimeStamp = LidarState.ElapsedTime;
		Params.FractionStart = FractionStart;
		Params.FractionEnd = FractionEnd;
		Params.Range = Range;
		Params.BeamExitRadius = BeamExitDiameter / 2.0;
		Params.BeamDivergenceRad = FMath::DegreesToRadians(BeamDivergence);
		Params.ScanPattern = ScanPattern;
		Params.bCalculateIntensity = bCalculateIntensity;
	}

	AGX_CHECK(Buffer.Num() == 0);
	auto NewPoints = PerformPartialScanCPU(GetWorld(), Params, Buffer);

	if (bDebugRenderPoints)
	{
		DrawDebugPoints(
			NewPoints, GetWorld(), GetComponentTransform(), DebugDrawPointSize,
			DebugDrawPointLifetime);
	}

	PointCloudDataOutput.Broadcast(Buffer);

#if UE_VERSION_OLDER_THAN(5, 5, 0)
	Buffer.SetNum(0, false);
#else
	Buffer.SetNum(0, EAllowShrinking::No);
#endif
}

void UAGX_LidarSensorLineTraceComponent::BeginPlay()
{
	Super::BeginPlay();
	bIsValid = CheckValid();

	if (!bIsValid)
		return;

	LidarState.ScanCycleDuration = 1.0 / ScanFrequency;
	LidarState.OutputCycleDuration = 1.0 / OutputFrequency;

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda([this](double TimeStamp) { OnStepForward(TimeStamp); });
	}
}

void UAGX_LidarSensorLineTraceComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	PointCloudDataOutput.Clear();

	if (Reason != EEndPlayReason::EndPlayInEditor && Reason != EEndPlayReason::Quit &&
		Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.Remove(PostStepForwardHandle);
		}
	}
}

#if WITH_EDITOR
bool UAGX_LidarSensorLineTraceComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, ExecutionMode),
			GET_MEMBER_NAME_CHECKED(ThisClass, ScanFrequency),
			GET_MEMBER_NAME_CHECKED(ThisClass, OutputFrequency),
			GET_MEMBER_NAME_CHECKED(ThisClass, FOV /*clang-format padding*/),
			GET_MEMBER_NAME_CHECKED(ThisClass, ScanPattern),
			GET_MEMBER_NAME_CHECKED(ThisClass, Resolution)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}

	return SuperCanEditChange;
}
#endif

bool UAGX_LidarSensorLineTraceComponent::CheckValid() const
{
	if (ScanFrequency <= 0.0 || OutputFrequency <= 0.0)
	{
		const FString Msg = FString::Printf(
			TEXT("Lidar Sensor '%s' in Actor '%s' has a non-positive Scan Frequency or Output "
				 "Frequency. Update these so that they are larger than zero."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (OutputFrequency < ScanFrequency)
	{
		const FString Msg = FString::Printf(
			TEXT("Lidar Sensor '%s' in Actor '%s' has an Output Frequency that is lower than the "
				 "Scan Frequency. Set the Output Frequency so that it is at least ar high as the "
				 "Scan Frequency."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	auto IsInRange = [](const FVector2D& V, double Min, double Max)
	{ return FMath::IsWithinInclusive(V.X, Min, Max) && FMath::IsWithinInclusive(V.Y, Min, Max); };

	if (!IsInRange(FOV, 0.0, 360.0) || !IsInRange(Resolution, 0.0, 180))
	{
		const FString Msg = FString::Printf(
			TEXT("Lidar Sensor '%s' in Actor '%s' has a FOV or Resolution outside of the valid "
				 "range. The x and y component of the FOV must both be within [0..360] and for "
				 "Resolution they must be within [0..180]."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	return true;
}

void UAGX_LidarSensorLineTraceComponent::OnStepForward(double TimeStamp)
{
	if (!bIsValid || !bEnabled)
		return;

	UpdateElapsedTime(TimeStamp);

	if (ExecutionMode != EAGX_LidarLineTraceExecutonMode::Auto)
		return;

	ScanAutoCPU();

	OutputPointCloudDataIfReady();
}

void UAGX_LidarSensorLineTraceComponent::UpdateElapsedTime(double TimeStamp)
{
	LidarState.ElapsedTimePrev = LidarState.ElapsedTime;

	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		LidarState.ElapsedTime = Sim->GetTimeStamp();
}

void UAGX_LidarSensorLineTraceComponent::ScanAutoCPU()
{
	using namespace AGX_LidarSensorLineTraceComponent_helpers;
	AGX_CHECK(bIsValid);

	if (LidarState.ElapsedTime == LidarState.ElapsedTimePrev)
		return;

	const double ScanCycleTimeElapsedPrev =
		LidarState.ElapsedTimePrev - LidarState.CurrentScanCycleStartTime;
	const double ScanCycleFractionPrev = ScanCycleTimeElapsedPrev / LidarState.ScanCycleDuration;
	AGX_CHECK(ScanCycleFractionPrev < 1.0);

	const double ScanCycleTimeElapsed =
		LidarState.ElapsedTime - LidarState.CurrentScanCycleStartTime;
	const double ScanCycleFraction = ScanCycleTimeElapsed / LidarState.ScanCycleDuration;

	AGX_CHECK(ScanCycleFraction > ScanCycleFractionPrev);

	LidarScanRequestParams Params(GetComponentTransform(), FOV, Resolution);
	{
		Params.FOVWindowX = {-FOV.X / 2.0, FOV.X / 2.0};
		Params.FOVWindowY = {-FOV.Y / 2.0, FOV.Y / 2.0};
		Params.TimeStamp = LidarState.ElapsedTime;
		Params.FractionStart = ScanCycleFractionPrev;

		// We scan at most up until the end of the cycle.
		Params.FractionEnd = std::min(ScanCycleFraction, 1.0);
		Params.Range = Range;
		Params.BeamExitRadius = BeamExitDiameter / 2.0;
		Params.BeamDivergenceRad = FMath::DegreesToRadians(BeamDivergence);
		Params.ScanPattern = ScanPattern;
		Params.bCalculateIntensity = bCalculateIntensity;
	}

	auto NewPoints = PerformPartialScanCPU(GetWorld(), Params, Buffer);
	if (bDebugRenderPoints)
	{
		DrawDebugPoints(
			NewPoints, GetWorld(), GetComponentTransform(), DebugDrawPointSize,
			DebugDrawPointLifetime);
	}

	if (ScanCycleFraction >= 1.0)
	{
		// Set the state as to prepare the next cycle.
		LidarState.CurrentScanCycleStartTime = LidarState.ElapsedTime;
	}
}

void UAGX_LidarSensorLineTraceComponent::OutputPointCloudDataIfReady()
{
	AGX_CHECK(bIsValid);

	const double OutputCycleTimeElapsed =
		LidarState.ElapsedTime - LidarState.CurrentOutputCycleStartTime;

	if (OutputCycleTimeElapsed >= LidarState.OutputCycleDuration)
	{
		PointCloudDataOutput.Broadcast(Buffer);
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		Buffer.SetNum(0, false);
#else
		Buffer.SetNum(0, EAllowShrinking::No);
#endif
		LidarState.CurrentOutputCycleStartTime = LidarState.ElapsedTime;
	}
}
