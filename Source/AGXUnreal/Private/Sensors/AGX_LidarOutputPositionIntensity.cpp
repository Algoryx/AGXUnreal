// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_LidarOutputPositionIntensity.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

void FAGX_LidarOutputPositionIntensity::Render(
	const TArray<FAGX_LidarOutputPositionIntensityData>& InData, UAGX_LidarSensorComponent* Lidar,
	float LifeTime, float ZeroDistanceSize, float IntensityScaleFactor)
{
	if (Lidar == nullptr)
		return;

	if (!Lidar->bEnableRendering)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPositionIntensity::Render called but the given Lidar does "
				 "not have bEnableRendering set to true. Doing nothing."));
		return;
	}

	UNiagaraComponent* Nc = Lidar->GetSpawnedNiagaraSystemComponent();
	if (Nc == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPositionIntensity::Render called but the given Lidar does "
				 "not have a spawned Niagara Component. Doing nothing."));
		return;
	}

	// Lidar in AGX Dynamics has output data left in the buffers even after being disabled.
	// Therefore, we check for this explicitly to not render old data after Lidar->setEnable(false).
	const TArray<FAGX_LidarOutputPositionIntensityData> NoData;
	const TArray<FAGX_LidarOutputPositionIntensityData>& DataToRender =
		Lidar->GetEnabled() ? InData : NoData;

#if UE_VERSION_OLDER_THAN(5, 5, 0)
	RenderPositions.SetNum(0, false);
	RenderColors.SetNum(0, false);
#else
	RenderPositions.SetNum(0, EAllowShrinking::No);
	RenderColors.SetNum(0, EAllowShrinking::No);
#endif
	

	const FTransform& Transform = Lidar->GetComponentTransform();

	for (const auto& Datum : DataToRender)
	{
		const uint8 Intensity = static_cast<uint8>(
			FMath::Clamp(Datum.Intensity * IntensityScaleFactor, 0.0, 1.0) *
			(double) std::numeric_limits<uint8>::max());

		const FLinearColor Color = FLinearColor::FromSRGBColor(
			FColor(Intensity, 0.0, std::numeric_limits<uint8>::max() - Intensity, 255));

		RenderPositions.Add(Transform.TransformPositionNoScale(FVector(Datum.Position)));
		RenderColors.Add(Color);
	}

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	Nc->SetNiagaraVariableInt("User.NumPoints", DataToRender.Num());
	Nc->SetNiagaraVariableFloat("User.Lifetime", LifeTime);
	Nc->SetNiagaraVariableFloat("User.ZeroDistanceSize", ZeroDistanceSize);
#else
	Nc->SetVariableInt(FName("User.NumPoints"), DataToRender.Num());
	Nc->SetVariableFloat(FName("User.Lifetime"), LifeTime);
	Nc->SetVariableFloat(FName("User.ZeroDistanceSize"), ZeroDistanceSize);
#endif

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(
		Nc, "Positions", RenderPositions);

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayColor(Nc, "Colors", RenderColors);
}

bool FAGX_LidarOutputPositionIntensity::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarOutputBarrier* FAGX_LidarOutputPositionIntensity::GetOrCreateNative()
{
	if (!HasNative())
	{
		NativeBarrier.AllocateNative();
	}

	return GetNative();
}

const FLidarOutputBarrier* FAGX_LidarOutputPositionIntensity::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FLidarOutputBarrier* FAGX_LidarOutputPositionIntensity::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FAGX_LidarOutputPositionIntensity& FAGX_LidarOutputPositionIntensity::operator=(
	const FAGX_LidarOutputPositionIntensity& Other)
{
	// This operator is needed to be able to declare e.g. TArray's containing this struct.
	// It is assumed not to be called during play, therefore no members are copied here.
	return *this;
}

bool FAGX_LidarOutputPositionIntensity::operator==(
	const FAGX_LidarOutputPositionIntensity& Other) const
{
	return FAGX_LidarOutputBase::operator==(Other);
}

void FAGX_LidarOutputPositionIntensity::GetData(
	TArray<FAGX_LidarOutputPositionIntensityData>& OutData)
{
	if (HasNative())
		NativeBarrier.GetData(OutData);
}
