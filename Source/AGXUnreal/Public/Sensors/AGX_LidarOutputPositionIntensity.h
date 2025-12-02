// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarOutputBase.h"
#include "Sensors/AGX_LidarOutputTypes.h"
#include "Sensors/LidarOutputPositionIntensityBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LidarOutputPositionIntensity.generated.h"

class FLidarOutputBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_LidarOutputPositionIntensity : public FAGX_LidarOutputBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarOutputPositionIntensity() = default;

	void Render(
		const TArray<FAGX_LidarOutputPositionIntensityData>& Data, UAGX_LidarSensorComponent* Lidar,
		float LifeTime, float ZeroDistanceSize, float IntensityScaleFactor);

	virtual bool HasNative() const override;
	virtual FLidarOutputBarrier* GetOrCreateNative() override;
	virtual const FLidarOutputBarrier* GetNative() const override;
	virtual FLidarOutputBarrier* GetNative() override;

	void GetData(TArray<FAGX_LidarOutputPositionIntensityData>& OutData);

	FAGX_LidarOutputPositionIntensity& operator=(const FAGX_LidarOutputPositionIntensity& Other);
	bool operator==(const FAGX_LidarOutputPositionIntensity& Other) const;

private:
	FLidarOutputPositionIntensityBarrier NativeBarrier;
	TArray<FVector> RenderPositions;
	TArray<FLinearColor> RenderColors;
};

/**
 * This class acts as an API that exposes functions of FAGX_LidarOutputPositionIntensity in
 * Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_LidarOutputPositionIntensity_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void AddTo(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Output, UAGX_LidarSensorComponent* Lidar)
	{
		Output.AddTo(Lidar);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void GetData(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Output,
		TArray<FAGX_LidarOutputPositionIntensityData>& OutData)
	{
		Output.GetData(OutData);
	}

	/**
	 * Render the data of this Lidar Output.
	 *
	 * LifeTime is how long each point is visible before disappearing [s].
	 *
	 * ZeroDistanceSize is the minimum apparent size of a point [cm].
	 *
	 * Intensity Scale Factor is a (non-phisical) scaling factor that is multiplied with all
	 * intensity values before calculating a color for the corresponding points. I.e. it changes the
	 * sentitivity of the intensity coloration (blue to red).
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void Render(
		UPARAM(ref) FAGX_LidarOutputPositionIntensity& Output,
		const TArray<FAGX_LidarOutputPositionIntensityData>& Data, UAGX_LidarSensorComponent* Lidar,
		float LifeTime = 0.12f, float ZeroDistanceSize = 4.f, float IntensityScaleFactor = 10.f)
	{
		Output.Render(Data, Lidar, LifeTime, ZeroDistanceSize, IntensityScaleFactor);
	}
};
