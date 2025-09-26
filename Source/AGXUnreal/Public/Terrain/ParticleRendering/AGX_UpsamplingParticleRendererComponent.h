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

#define VOLUME_MOD 1.611992

class UNiagaraComponent;
class UNiagaraSystem;
class UAGX_ParticleUpsamplingDI;

/**
 * AGXUpsamplingParticleRendererComponent provides real-time visualization of AGX Terrain particles
 * using an upsampling method. Unlike AGXSoilParticleRendererComponent, this component performs
 * an additional particle simulation on top of the existing AGX Terrain particles. This process
 * generates a higher number of finer particles to improve visual resolution, allowing for
 * more detailed rendering of materials like fine sand or dust.
 *
 * The core approach involves sampling attributes from the base AGX Terrain particles to build
 * a velocity grid. The finer particles then interpolate their motion linearly based on this grid,
 * resulting in smoother and more realistic particle movement.
 */
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
		Meta = (ClampMin = "1", UIMin = "1", UIMax = "5000"))
	int32 Upsampling = 100;

	/**
	 * Toggle between using the default voxel size or use user defined size.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	bool bOverrideVoxelSize = false;

	/**
	 * The size of how large a singe voxel will be when setting up the data grids [cm].
	 *
	 * Really small voxel size will decrease performance singificantly.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta = (EditCondition = "bOverrideVoxelSize", UIMin = "10", UIMax = "1000"))
	float VoxelSize = 10.0;

	/**
	 * This controls how quickly the particles are eased in/out when spawned/destroyed.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Upsampling Particle Rendering",
		Meta = (UIMin = "0.1", UIMax = "1.0"))
	double EaseStepSize = 0.1;

	UPROPERTY(EditAnywhere, Category = "AGX Upsampling Particle Rendering")
	UNiagaraSystem* ParticleSystemAsset;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetEnableParticleRendering(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetEnableParticleRendering() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	UNiagaraComponent* GetParticleSystemComponent();

	const UNiagaraComponent* GetParticleSystemComponent() const;
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetUpsampling(int32 InUpsampling);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	int32 GetUpsampling() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetOverrideVoxelSize(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	bool GetOverrideVoxelSize() const;
	
	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetVoxelSize(double InVoxelSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetVoxelSize() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	void SetEaseStepSize(double InEaseStepSize);

	UFUNCTION(BlueprintCallable, Category = "AGX Upsampling Particle Rendering")
	double GetEaseStepSize() const;
	
protected:

	// ~Begin UActorComponent interface.
	virtual void BeginPlay() override;
	// ~End UActorComponent interface.

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	// ~End UObject interface.


private:

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	UNiagaraComponent* ParticleSystemComponent = nullptr;
	UAGX_ParticleUpsamplingDI* UpsamplingDataInterface = nullptr;

	float ElementSize = 0;

	UFUNCTION()
	void HandleParticleData(FDelegateParticleData& data);

	/** 
	 * Appends the voxel indices that a coarse particle intersects with to the given array.
	 * The Position and Radius refers to the position and radius of a coarse particle in the
	 * simulation.
	 */
	void AppendIfActiveVoxel(
		TSet<FIntVector>& ActiveVoxelIndices, FVector Position, float Radius, float SizeOfVoxel);

	/** 
	 * Converts the active voxels from the set to a TArray 
	 */
	TArray<FIntVector4> GetActiveVoxelsFromSet(TSet<FIntVector> VoxelSet);
};