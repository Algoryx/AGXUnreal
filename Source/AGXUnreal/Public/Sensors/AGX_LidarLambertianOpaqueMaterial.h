// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSurfaceMaterial.h"
#include "Sensors/RtLambertianOpaqueMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarLambertianOpaqueMaterial.generated.h"

/**
 * Lidar Surface Material that can be assigned to objects the Level. Determines the
 * Lidar laser ray interaction (e.g. intensity calculation) with such objects.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXUNREAL_API UAGX_LidarLambertianOpaqueMaterial : public UAGX_LidarSurfaceMaterial
{
	GENERATED_BODY()

public:
	/**
	 * Reflectivity, between 0 and 1 where 0 means no laser light is reflected, and 1 means perfect
	 * reflection.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lidar", Meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Reflectivity {0.8f};

	UFUNCTION(BlueprintCallable, Category = "AGX Lidar")
	void SetReflectivity(float InReflectivity);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	float GetReflectivity() const;

	bool HasNative() const;
	FRtLambertianOpaqueMaterialBarrier* GetNative();
	const FRtLambertianOpaqueMaterialBarrier* GetNative() const;

	void CommitToAsset();

	static UAGX_LidarLambertianOpaqueMaterial* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_LidarLambertianOpaqueMaterial& Source);

	virtual UAGX_LidarSurfaceMaterial* GetOrCreateInstance(UWorld* PlayingWorld) override;
	FRtLambertianOpaqueMaterialBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	bool IsInstance() const;

	void CopyFrom(const FRtLambertianOpaqueMaterialBarrier& Source);
	void CopyProperties(const UAGX_LidarLambertianOpaqueMaterial& Source);

private:
	void CreateNative();

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

private:
	TWeakObjectPtr<UAGX_LidarLambertianOpaqueMaterial> Asset;
	TWeakObjectPtr<UAGX_LidarLambertianOpaqueMaterial> Instance;
	FRtLambertianOpaqueMaterialBarrier NativeBarrier;
};
