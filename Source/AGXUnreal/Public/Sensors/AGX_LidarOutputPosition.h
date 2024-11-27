// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarOutputBase.h"
#include "Sensors/AGX_LidarOutputTypes.h"
#include "Sensors/LidarOutputPositionBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LidarOutputPosition.generated.h"

class FLidarOutputBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_LidarOutputPosition : public FAGX_LidarOutputBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_LidarOutputPosition() = default;

	void Render(
		const TArray<FAGX_LidarOutputPositionData>& InData, UAGX_LidarSensorComponent* Lidar,
		float LifeTime, float ZeroDistanceSize);

	virtual bool HasNative() const override;
	virtual FLidarOutputBarrier* GetOrCreateNative() override;
	virtual const FLidarOutputBarrier* GetNative() const override;
	virtual FLidarOutputBarrier* GetNative() override;

	void GetData(TArray<FAGX_LidarOutputPositionData>& OutData);

	FAGX_LidarOutputPosition& operator=(const FAGX_LidarOutputPosition& Other);
	bool operator==(const FAGX_LidarOutputPosition& Other) const;

private:
	FLidarOutputPositionBarrier NativeBarrier;
	TArray<FVector> RenderPositions;
};

/**
 * This class acts as an API that exposes functions of FAGX_LidarOutputPosition in
 * Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_LidarOutputPosition_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void AddTo(
		UPARAM(ref) FAGX_LidarOutputPosition& Output, UAGX_LidarSensorComponent* Lidar)
	{
		Output.AddTo(Lidar);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void GetData(
		UPARAM(ref) FAGX_LidarOutputPosition& Output,
		TArray<FAGX_LidarOutputPositionData>& OutData)
	{
		Output.GetData(OutData);
	}

	/**
	 * Render the data of this Lidar Output.
	 *
	 * LifeTime is how long each point is visible before disappearing [s].
	 *
	 * ZeroDistanceSize is the minimum apparent size of a point [cm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	static void Render(
		UPARAM(ref) FAGX_LidarOutputPosition& Output,
		const TArray<FAGX_LidarOutputPositionData>& Data, UAGX_LidarSensorComponent* Lidar,
		float LifeTime = 0.12f, float ZeroDistanceSize = 4.f)
	{
		Output.Render(Data, Lidar, LifeTime, ZeroDistanceSize);
	}
};
