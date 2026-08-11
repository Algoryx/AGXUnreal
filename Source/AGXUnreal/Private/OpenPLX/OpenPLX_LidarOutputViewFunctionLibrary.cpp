// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_LidarOutputViewFunctionLibrary.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

// Standard library includes.
#include <limits>

bool UOpenPLX_LidarOutputView::Render(
	FOpenPLXLidarOutputView& View, UAGX_LidarSensorComponent* Lidar, float LifeTime,
	float ZeroDistanceSize, float IntensityScaleFactor)
{
	if (Lidar == nullptr)
		return false;

	if (!Lidar->bEnableRendering)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UOpenPLX_LidarOutputView::Render called but the given Lidar does not have "
				 "bEnableRendering set to true. Doing nothing."));
		return false;
	}

	UNiagaraComponent* Nc = Lidar->GetSpawnedNiagaraSystemComponent();
	if (Nc == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UOpenPLX_LidarOutputView::Render called but the given Lidar does not have a "
				 "spawned Niagara Component. Doing nothing."));
		return false;
	}

	TArray<FVector> RenderPositions;
	if (Lidar->GetEnabled() &&
		!View.ReadPositionsTransformed(Lidar->GetComponentTransform(), RenderPositions))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UOpenPLX_LidarOutputView::Render could not read positions from the Lidar output "
				 "view. Doing nothing."));
		return false;
	}

	TArray<float> Intensities;
	const bool bHasIntensities = Lidar->GetEnabled() && View.HasIntensities();
	if (bHasIntensities && !View.ReadIntensities(Intensities))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UOpenPLX_LidarOutputView::Render could not read intensities from the Lidar "
				 "output view. Rendering positions without intensity colors."));
		Intensities.Reset();
	}

	if (Intensities.Num() != RenderPositions.Num())
	{
		if (Intensities.Num() > 0)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("UOpenPLX_LidarOutputView::Render read %d intensities for %d positions. "
					 "Rendering positions without intensity colors."),
				Intensities.Num(), RenderPositions.Num());
		}
		Intensities.Reset();
	}

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	Nc->SetNiagaraVariableInt("User.NumPoints", RenderPositions.Num());
	Nc->SetNiagaraVariableFloat("User.Lifetime", LifeTime);
	Nc->SetNiagaraVariableFloat("User.ZeroDistanceSize", ZeroDistanceSize);
#else
	Nc->SetVariableInt(FName("User.NumPoints"), RenderPositions.Num());
	Nc->SetVariableFloat(FName("User.Lifetime"), LifeTime);
	Nc->SetVariableFloat(FName("User.ZeroDistanceSize"), ZeroDistanceSize);
#endif

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(
		Nc, "Positions", RenderPositions);

	if (Intensities.Num() > 0)
	{
		TArray<FLinearColor> RenderColors;
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		RenderColors.SetNum(0, false);
#else
		RenderColors.SetNum(0, EAllowShrinking::No);
#endif
		RenderColors.Reserve(Intensities.Num());

		for (float IntensityValue : Intensities)
		{
			const uint8 Intensity = static_cast<uint8>(
				FMath::Clamp(IntensityValue * IntensityScaleFactor, 0.0f, 1.0f) *
				static_cast<float>(std::numeric_limits<uint8>::max()));

			RenderColors.Add(FLinearColor::FromSRGBColor(FColor(
				Intensity, 0, std::numeric_limits<uint8>::max() - Intensity,
				std::numeric_limits<uint8>::max())));
		}

		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayColor(Nc, "Colors", RenderColors);
	}

	return true;
}
