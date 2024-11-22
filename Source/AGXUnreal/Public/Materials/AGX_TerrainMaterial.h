// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_ShapeMaterialBulkProperties.h"
#include "Materials/AGX_ShapeMaterialSurfaceProperties.h"
#include "Materials/AGX_ShapeMaterialWireProperties.h"
#include "Materials/AGX_TerrainBulkProperties.h"
#include "Materials/AGX_TerrainCompactionProperties.h"
#include "Materials/AGX_TerrainExcavationContactProperties.h"
#include "Materials/AGX_TerrainParticleProperties.h"
#include "Materials/TerrainMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainMaterial.generated.h"

class UAGX_ShapeMaterial;

/**
 * Defines the material for a terrain.
 *
 * Terrain Materials are created by the user in-Editor by creating a UAGX_TerrainMaterial asset.
 * In-Editor they are treated as assets and can be referenced by Terrains.
 *
 * When game begins playing, one UAGX_TerrainMaterial instace will be created for each
 * UAGX_TerrainMaterial asset that is referenced by an in-game Terrain. The
 * UAGX_TerrainMaterial will create the actual native AGX terrain material. The in-game
 * Terrain that referenced the UAGX_TerrainMaterial asset will swap its
 * reference to the in-game created instance instead. This means that ultimately only
 * UAGX_TerrainMaterials will be referenced in-game. When play stops the in-Editor state
 * will be restored.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_TerrainMaterial : public UObject
{
public:
	GENERATED_BODY()

	// Terrain Bulk properties.

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material")
	FAGX_TerrainBulkProperties TerrainBulk;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetAdhesionOverlapFactor(double AdhesionOverlapFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetAdhesionOverlapFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetCohesion(double Cohesion);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetCohesion() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetDensity(double Density);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetDilatancyAngle(double DilatancyAngle);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetDilatancyAngle() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetFrictionAngle(double FrictionAngle);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetFrictionAngle() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetMaxDensity(double MaxDensity);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetMaxDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetPoissonsRatio(double PoissonsRatio);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetPoissonsRatio() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetSwellFactor(double SwellFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetSwellFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	void SetYoungsModulus(double YoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Bulk")
	double GetYoungsModulus() const;

	// Compaction properties.

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material")
	FAGX_TerrainCompactionProperties TerrainCompaction;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetAngleOfReposeCompactionRate() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetBankStatePhi(double Phi0);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetBankStatePhi() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetCompactionTimeRelaxationConstant(double CompactionTimeRelaxationConstant);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetCompactionTimeRelaxationConstant() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetCompressionIndex(double CompressionIndex);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetCompressionIndex() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetHardeningConstantKe(double Ke);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetHardeningConstantKe() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetHardeningConstantNe(double Ne);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetHardeningConstantNe() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetPreconsolidationStress(double PreconsolidationStress);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetPreconsolidationStress() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetStressCutOffFraction(double StressCutOffFraction);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetStressCutOffFraction() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	void SetDilatancyAngleScalingFactor(double DilatancyAngleScalingFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Compaction")
	double GetDilatancyAngleScalingFactor() const;

	// Particle properties.
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material")
	FAGX_TerrainParticleProperties TerrainParticles;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleAdhesionOverlapFactor(double ParticleAdhesionOverlapFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleAdhesionOverlapFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleCohesion(double ParticleCohesion);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleCohesion() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleRestitution(double ParticleRestitution);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleRestitution() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleRollingResistance(double ParticleRollingResistance);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleRollingResistance() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleSurfaceFriction(double ParticleSurfaceFriction);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleSurfaceFriction() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleTerrainCohesion(double ParticleTerrainCohesion);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleTerrainCohesion() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleTerrainRestitution(double ParticleTerrainRestitution);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleTerrainRestitution() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleTerrainRollingResistance(double ParticleTerrainRollingResistance);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleTerrainRollingResistance() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleTerrainSurfaceFriction(double ParticleTerrainSurfaceFriction);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleTerrainSurfaceFriction() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleTerrainYoungsModulus(double ParticleTerrainYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleTerrainYoungsModulus() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	void SetParticleYoungsModulus(double ParticleYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Particle")
	double GetParticleYoungsModulus() const;

	// Excavation contact properties.
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material")
	FAGX_TerrainExcavationContactProperties TerrainExcavationContact;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	void SetAggregateStiffnessMultiplier(double AggregateStiffnessMultiplier);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	double GetAggregateStiffnessMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	void SetExcavationStiffnessMultiplier(double ExcavationStiffnessMultiplier);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	double GetExcavationStiffnessMultiplier() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	void SetDepthDecayFactor(double DepthDecayFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	double GetDepthDecayFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	void SetDepthIncreaseFactor(double DepthIncreaseFactor);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	double GetDepthIncreaseFactor() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	void SetMaximumAggregateNormalForce(double MaximumAggregateNormalForce);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	double GetMaximumAggregateNormalForce() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	void SetMaximumContactDepth(double MaximumContactDepth);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Excavation Contact")
	double GetMaximumContactDepth() const;

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	virtual void Serialize(FArchive& Archive) override;

	void CopyFrom(const FTerrainMaterialBarrier& Source);

	FTerrainMaterialBarrier* GetOrCreateTerrainMaterialNative(UWorld* PlayingWorld);
	UAGX_TerrainMaterial* GetOrCreateInstance(UWorld* PlayingWorld);
	static UAGX_TerrainMaterial* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_TerrainMaterial* Source);

	void CopyTerrainMaterialProperties(const UAGX_TerrainMaterial* Source);

	bool IsInstance() const;

	const FAGX_ShapeMaterialBulkProperties& GetShapeMaterialBulkProperties();
	const FAGX_ShapeMaterialSurfaceProperties& GetShapeMaterialSurfaceProperties();
	const FAGX_ShapeMaterialWireProperties& GetShapeMaterialWireProperties();

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void CreateTerrainMaterialNative();
	bool HasTerrainMaterialNative() const;
	FTerrainMaterialBarrier* GetTerrainMaterialNative();
	void UpdateTerrainMaterialNativeProperties();

	TWeakObjectPtr<UAGX_TerrainMaterial> Asset;
	TWeakObjectPtr<UAGX_TerrainMaterial> Instance;
	FTerrainMaterialBarrier TerrainMaterialNativeBarrier;

	// These Shape Material properties are here because in older versions of AGXUnreal, the Terrain
	// Material contained these. We can still retrieve this data and put it into a new Shape
	// Material using the helper button in the Details Panel. These can be removed once that
	// helper-button is removed in a future release. See TerrainMaterialShapeMaterialSplit.
	UPROPERTY()
	FAGX_ShapeMaterialBulkProperties Bulk;

	UPROPERTY()
	FAGX_ShapeMaterialSurfaceProperties Surface;

	UPROPERTY()
	FAGX_ShapeMaterialWireProperties Wire;
};
