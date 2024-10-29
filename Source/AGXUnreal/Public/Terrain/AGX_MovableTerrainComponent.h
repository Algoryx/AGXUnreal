#pragma once

// AGX Dynamics for Unreal include.s
#include "AGX_NativeOwner.h"
#include "Terrain/TerrainBarrier.h"
#include "AGX_ShovelReference.h"

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "AGX_MovableTerrainComponent.generated.h"


class UAGX_ShapeComponent;
class UAGX_TerrainMaterial;
class UAGX_ShovelComponent;
class UNiagaraSystem;
class UNiagaraComponent;


/**
 *
 */
UCLASS(
	ClassGroup = "AGX_Terrain", meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, Input, LOD, Physics, Replication))
class AGXUNREAL_API UAGX_MovableTerrainComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()
public:
	UAGX_MovableTerrainComponent(const FObjectInitializer& ObjectInitializer);

	bool HasNative() const;
	void CreateNative();
	FTerrainBarrier* GetNative();
	const FTerrainBarrier* GetNative() const;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction);

protected:
	UPROPERTY(EditAnywhere, Meta = (ClampMin = "1", UIMin = "1", ClampMax = "256", UIMax = "256"))
	int Resolution = 20;

	UPROPERTY(EditAnywhere)
	FVector2D Size = FVector2D(200.0f, 200.0f);

	UPROPERTY(EditAnywhere)
	float StartHeight = 0.0f;

	UPROPERTY(EditAnywhere)
	float NoiseHeight = 50.0f;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (GetOptions = "GetBedGeometryOptions"))
	TArray<FName> BedGeometries;
	UFUNCTION(CallInEditor)
	TArray<FString> GetBedGeometryOptions() const;
	TArray<UAGX_ShapeComponent*> GetBedGeometries() const;

	UPROPERTY(EditAnywhere)
	double BedOffset = -0.1;

	virtual void PostInitProperties() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& event) override;

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void UpdateInEditorMesh();

	void AutoFitToBed();

	void RebuildHeightMesh(
		const FVector2D& MeshSize, const int ResX, const int ResY,
		const TArray<float>& HeightArray);

	void SetupHeights(
		TArray<float>& InitialHeights, TArray<float>& MinimumHeights, int ResX, int ResY,
		double ElementSize, bool FlipYAxis) const;

	void AddBedHeights(
		TArray<float>& Heights, int ResX, int ResY, double ElementSize, bool FlipYAxis) const;

	void AddNoiseHeights(
		TArray<float>& Heights, int ResX, int ResY, double ElementSize, bool FlipYAxis) const;

private:
	FTerrainBarrier NativeBarrier;
	TArray<float> CurrentHeights;
	FDelegateHandle PostStepForwardHandle;

	/*
	--- AGX_Terrain Implementation
	------------------------------
	*/
protected:
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCanCollide {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetCanCollide(bool bInCanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetCanCollide() const;

	/** Whether the native terrain should generate particles or not during shovel interactions. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	bool bCreateParticles = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetCreateParticles(bool CreateParticles);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetCreateParticles() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain")
	bool bDeleteParticlesOutsideBounds = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool GetDeleteParticlesOutsideBounds() const;

	/**
	 * Scales the penetration force with the shovel velocity squared in the cutting
	 * direction according to: ( 1.0 + C * v^2 ).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	double PenetrationForceVelocityScaling = 0.0f;

	void SetPenetrationForceVelocityScaling(double InPenetrationForceVelocityScaling);

	double GetPenetrationForceVelocityScaling() const;

	/**
	 * Sets the maximum volume of active zone wedges that should wake particles [cm^3].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	double MaximumParticleActivationVolume = std::numeric_limits<double>::infinity();

	void SetMaximumParticleActivationVolume(double InMaximumParticleActivationVolume);

	double GetMaximumParticleActivationVolume() const;

	/** The physical bulk, compaction, particle and surface properties of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_TerrainMaterial* TerrainMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial);
	bool UpdateNativeTerrainMaterial();

	/** Defines physical properties of the surface of the Terrain. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain")
	UAGX_ShapeMaterial* ShapeMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);
	bool UpdateNativeShapeMaterial();

	
	/**
	 * List of collision groups that this Terrain is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FName> CollisionGroups;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	void RemoveCollisionGroupIfExists(FName GroupName);
	
	UPROPERTY(EditAnywhere, Category = "AGX Terrain")
	TArray<FAGX_ShovelReference> ShovelComponents;
	void CreateNativeShovels();

	/** Whether soil particles should be rendered or not. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Rendering")
	bool bEnableParticleRendering = true;

	/**
	 * Rough estimation of number of particles that will exist at once. Should not be too low,
	 * or some particles might not be rendered.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta =
			(EditCondition = "bEnableParticleRendering", ClampMin = "1", UIMin = "1",
			 UIMax = "4096"))
	int32 MaxNumRenderParticles = 2048;

	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Rendering",
		Meta = (EditCondition = "bEnableParticleRendering"))
	UNiagaraSystem* ParticleSystemAsset;
	UNiagaraComponent* ParticleSystemComponent = nullptr;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	UNiagaraComponent* GetSpawnedParticleSystemComponent();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain")
	int32 GetNumParticles() const;

	
	bool InitializeParticles();
	void UpdateParticles();
};
