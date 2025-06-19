// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"
#include "Terrain/AGX_Terrain.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "CoreTypes.h"

#include "AGX_UpsamplingParticleRendererComponent.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class UAGX_ParticleUpsamplingDI;

UCLASS(
	ClassGroup = "AGX_Terrain_Particle_Rendering",
	meta = (BlueprintSpawnableComponent, ShortToolTip = "Uses the particle data from AGX_Terrain to upsample the rendered particles, significantly increasing particle count."))
class AGXUNREAL_API UAGX_UpsamplingParticleRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UAGX_UpsamplingParticleRendererComponent();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	bool bEnableParticleRendering = true;

	/**
	 * The desired upsampling factor which the renderer will try to achieve.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta =
			(EditCondition = "bEnableParticleRendering",
			 ClampMin = "1", UIMin = "1", UIMax = "5000"))
	int32 Upsampling = 100;

	/**
	 * Toggle between using the default voxel size or use user defined size.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta = (EditCondition = "bEnableParticleRendering"))
	bool bEnableVoxelSize = false;

	/**
	 * The size of how large a singe voxel will be when setting up the data grids.
	 *
	 * Really small voxel size will decrease performance singificantly.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta =
			(EditCondition =
				 "bEnableParticleRendering && bEnableVoxelSize",
			 ClampMin=10, UIMin = "10", UIMax = "1000"))
	double VoxelSize = 10.0;

	/**
	 * This controls how quickly the particles are eased in/out when spawned/destroyed.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta =
			(EditCondition = "bEnableParticleRendering", UIMin = "0.1",
			 UIMax = "1.0"))
	double EaseStepSize = 0.1;

	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	UNiagaraSystem* ParticleSystemAsset;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool SetEnableParticleRendering(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetEnableParticleRendering();

	/**
	 * If a Particle System Component has been spawned by the Renderer Component, this function will return it.
	 * Returns nullptr otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	UNiagaraComponent* GetParticleSystemComponent();
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	int32 SetUpsampling(int32 InUpsampling);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	int32 GetUpsampling();

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool SetEnableVoxelSize(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetEnableVoxelSize();
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double SetVoxelSize(double InVoxelSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetVoxelSize();

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double SetEaseStepSize(double InEaseStepSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetEaseStepSize();
	
protected:

	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif

private:

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	UNiagaraComponent* ParticleSystemComponent = nullptr;
	UAGX_ParticleUpsamplingDI* UpsamplingDataInterface = nullptr;

	float ElementSize = 0;

	UFUNCTION()
	void HandleParticleData(FDelegateParticleData data);

	/** 
	 * Appends the voxel indices that a coarse particle intersects with to the given array. 
	 */
	void AppendIfActiveVoxel(
		TSet<FIntVector>& ActiveVoxelIndices, FVector CPPosition, float CPRadius);

	/** 
	 * Converts the active voxels from the set to a TArray 
	 */
	TArray<FIntVector4> GetActiveVoxelsFromSet(TSet<FIntVector> VoxelSet);
};