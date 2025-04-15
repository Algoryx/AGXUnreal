// Copyright 2024, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Materials/TerrainMaterialBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

// Bulk properties.

void UAGX_TerrainMaterial::SetAdhesionOverlapFactor(double AdhesionOverlapFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.AdhesionOverlapFactor, AdhesionOverlapFactor, SetAdhesionOverlapFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetAdhesionOverlapFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.AdhesionOverlapFactor, GetAdhesionOverlapFactor, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetCohesion(double Cohesion)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Cohesion, Cohesion, SetCohesion, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetCohesion() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Cohesion, GetCohesion, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDensity(double Density)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Density, Density, SetDensity, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetDensity() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.Density, GetDensity, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDilatancyAngle(double DilatancyAngle)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.DilatancyAngle, DilatancyAngle, SetDilatancyAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetDilatancyAngle() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.DilatancyAngle, GetDilatancyAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetFrictionAngle(double FrictionAngle)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.FrictionAngle, FrictionAngle, SetFrictionAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetFrictionAngle() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.FrictionAngle, GetFrictionAngle, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetMaxDensity(double MaxDensity)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.MaxDensity, MaxDensity, SetMaxDensity, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetMaxDensity() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.MaxDensity, GetMaxDensity, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetPoissonsRatio(double PoissonsRatio)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.PoissonsRatio, PoissonsRatio, SetPoissonsRatio, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetPoissonsRatio() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.PoissonsRatio, GetPoissonsRatio, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetSwellFactor(double SwellFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.SwellFactor, SwellFactor, SetSwellFactor, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetSwellFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.SwellFactor, GetSwellFactor, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetYoungsModulus(double YoungsModulus)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.YoungsModulus, YoungsModulus, SetYoungsModulus, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetYoungsModulus() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainBulk.YoungsModulus, GetYoungsModulus, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

// Compaction properties.
void UAGX_TerrainMaterial::SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.AngleOfReposeCompactionRate, AngleOfReposeCompactionRate,
		SetAngleOfReposeCompactionRate, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetAngleOfReposeCompactionRate() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.AngleOfReposeCompactionRate, GetAngleOfReposeCompactionRate,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetBankStatePhi(double Phi0)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.BankStatePhi0, Phi0, SetBankStatePhi, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetBankStatePhi() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.BankStatePhi0, GetBankStatePhi, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetCompactionTimeRelaxationConstant(
	double CompactionTimeRelaxationConstant)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompactionTimeRelaxationConstant, CompactionTimeRelaxationConstant,
		SetCompactionTimeRelaxationConstant, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetCompactionTimeRelaxationConstant() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompactionTimeRelaxationConstant, GetCompactionTimeRelaxationConstant,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetCompressionIndex(double CompressionIndex)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompressionIndex, CompressionIndex, SetCompressionIndex,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetCompressionIndex() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.CompressionIndex, GetCompressionIndex, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetHardeningConstantKe(double Ke)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantKe, Ke, SetHardeningConstantKe, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetHardeningConstantKe() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantKe, GetHardeningConstantKe, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetHardeningConstantNe(double Ne)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantNe, Ne, SetHardeningConstantNe, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetHardeningConstantNe() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.HardeningConstantNe, GetHardeningConstantNe, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetPreconsolidationStress(double PreconsolidationStress)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.PreconsolidationStress, PreconsolidationStress, SetPreconsolidationStress,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetPreconsolidationStress() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.PreconsolidationStress, GetPreconsolidationStress,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetStressCutOffFraction(double StressCutOffFraction)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.StressCutOffFraction, StressCutOffFraction, SetStressCutOffFraction,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetStressCutOffFraction() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.StressCutOffFraction, GetStressCutOffFraction, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDilatancyAngleScalingFactor(double DilatancyAngleScalingFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.DilatancyAngleScalingFactor, DilatancyAngleScalingFactor,
		SetDilatancyAngleScalingFactor, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetDilatancyAngleScalingFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainCompaction.DilatancyAngleScalingFactor, GetDilatancyAngleScalingFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

// Particle properties.
void UAGX_TerrainMaterial::SetParticleAdhesionOverlapFactor(double ParticleAdhesionOverlapFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.AdhesionOverlapFactor, ParticleAdhesionOverlapFactor,
		SetParticleAdhesionOverlapFactor, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleAdhesionOverlapFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.AdhesionOverlapFactor, GetParticleAdhesionOverlapFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleCohesion(double ParticleCohesion)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleCohesion, ParticleCohesion, SetParticleCohesion,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleCohesion() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleCohesion, GetParticleCohesion, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleRestitution(double ParticleRestitution)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleRestitution, ParticleRestitution, SetParticleRestitution,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleRestitution() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleRestitution, GetParticleRestitution, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleRollingResistance(double ParticleRollingResistance)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleRollingResistance, ParticleRollingResistance,
		SetParticleRollingResistance, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleRollingResistance() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleRollingResistance, GetParticleRollingResistance,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleSurfaceFriction(double ParticleSurfaceFriction)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleSurfaceFriction, ParticleSurfaceFriction,
		SetParticleSurfaceFriction, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleSurfaceFriction() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleSurfaceFriction, GetParticleSurfaceFriction,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleTerrainCohesion(double ParticleTerrainCohesion)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainCohesion, ParticleTerrainCohesion,
		SetParticleTerrainCohesion, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleTerrainCohesion() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainCohesion, GetParticleTerrainCohesion,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleTerrainRestitution(double ParticleTerrainRestitution)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainRestitution, ParticleTerrainRestitution,
		SetParticleTerrainRestitution, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleTerrainRestitution() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainRestitution, GetParticleTerrainRestitution,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleTerrainRollingResistance(
	double ParticleTerrainRollingResistance)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainRollingResistance, ParticleTerrainRollingResistance,
		SetParticleTerrainRollingResistance, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleTerrainRollingResistance() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainRollingResistance, GetParticleTerrainRollingResistance,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleTerrainSurfaceFriction(double ParticleTerrainSurfaceFriction)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainSurfaceFriction, ParticleTerrainSurfaceFriction,
		SetParticleTerrainSurfaceFriction, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleTerrainSurfaceFriction() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainSurfaceFriction, GetParticleTerrainSurfaceFriction,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleTerrainYoungsModulus(double ParticleTerrainYoungsModulus)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainYoungsModulus, ParticleTerrainYoungsModulus,
		SetParticleTerrainYoungsModulus, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleTerrainYoungsModulus() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleTerrainYoungsModulus, GetParticleTerrainYoungsModulus,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetParticleYoungsModulus(double ParticleYoungsModulus)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleYoungsModulus, ParticleYoungsModulus, SetParticleYoungsModulus,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetParticleYoungsModulus() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainParticles.ParticleYoungsModulus, GetParticleYoungsModulus, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

// Excavation contact properties.
void UAGX_TerrainMaterial::SetAggregateStiffnessMultiplier(double AggregateStiffnessMultiplier)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.AggregateStiffnessMultiplier, AggregateStiffnessMultiplier,
		SetAggregateStiffnessMultiplier, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetAggregateStiffnessMultiplier() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.AggregateStiffnessMultiplier, GetAggregateStiffnessMultiplier,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetExcavationStiffnessMultiplier(double ExcavationStiffnessMultiplier)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.ExcavationStiffnessMultiplier, ExcavationStiffnessMultiplier,
		SetExcavationStiffnessMultiplier, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetExcavationStiffnessMultiplier() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.ExcavationStiffnessMultiplier, GetExcavationStiffnessMultiplier,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDepthDecayFactor(double DepthDecayFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.DepthDecayFactor, DepthDecayFactor, SetDepthDecayFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetDepthDecayFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.DepthDecayFactor, GetDepthDecayFactor, HasTerrainMaterialNative,
		TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetDepthIncreaseFactor(double DepthIncreaseFactor)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.DepthIncreaseFactor, DepthIncreaseFactor, SetDepthIncreaseFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetDepthIncreaseFactor() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.DepthIncreaseFactor, GetDepthIncreaseFactor,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetMaximumAggregateNormalForce(double MaximumAggregateNormalForce)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.MaximumAggregateNormalForce, MaximumAggregateNormalForce,
		SetMaximumAggregateNormalForce, HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetMaximumAggregateNormalForce() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.MaximumAggregateNormalForce, GetMaximumAggregateNormalForce,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::SetMaximumContactDepth(double MaximumContactDepth)
{
	AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.MaximumContactDepth, MaximumContactDepth, SetMaximumContactDepth,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

double UAGX_TerrainMaterial::GetMaximumContactDepth() const
{
	AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(
		TerrainExcavationContact.MaximumContactDepth, GetMaximumContactDepth,
		HasTerrainMaterialNative, TerrainMaterialNativeBarrier);
}

void UAGX_TerrainMaterial::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	TerrainCompaction.Serialize(Archive);
}

#if WITH_EDITOR
void UAGX_TerrainMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TerrainMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_TerrainMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	// Bulk properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, AdhesionOverlapFactor),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainBulk.AdhesionOverlapFactor, SetAdhesionOverlapFactor)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Cohesion),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.Cohesion, SetCohesion) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, Density),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.Density, SetDensity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, DilatancyAngle),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.DilatancyAngle, SetDilatancyAngle) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, FrictionAngle),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.FrictionAngle, SetFrictionAngle) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, MaxDensity),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.MaxDensity, SetMaxDensity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, PoissonsRatio),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.PoissonsRatio, SetPoissonsRatio) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, SwellFactor),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.SwellFactor, SetSwellFactor) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainBulk),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainBulkProperties, YoungsModulus),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainBulk.YoungsModulus, SetYoungsModulus) });

	// Compaction properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, AngleOfReposeCompactionRate),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.AngleOfReposeCompactionRate, SetAngleOfReposeCompactionRate)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, BankStatePhi0),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainCompaction.BankStatePhi0, SetBankStatePhi) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompactionTimeRelaxationConstant),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.CompactionTimeRelaxationConstant,
				SetCompactionTimeRelaxationConstant)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, CompressionIndex),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.CompressionIndex, SetCompressionIndex)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, HardeningConstantKe),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.HardeningConstantKe, SetHardeningConstantKe)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, HardeningConstantNe),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.HardeningConstantNe, SetHardeningConstantNe)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, PreconsolidationStress),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.PreconsolidationStress, SetPreconsolidationStress)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, StressCutOffFraction),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.StressCutOffFraction, SetStressCutOffFraction)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainCompaction),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainCompactionProperties, DilatancyAngleScalingFactor),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainCompaction.DilatancyAngleScalingFactor, SetDilatancyAngleScalingFactor)
		});

	// Particle properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, AdhesionOverlapFactor),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.AdhesionOverlapFactor, SetParticleAdhesionOverlapFactor)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleCohesion),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(TerrainParticles.ParticleCohesion, SetParticleCohesion)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleRestitution),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleRestitution, SetParticleRestitution)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleRollingResistance),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleRollingResistance, SetParticleRollingResistance)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleSurfaceFriction),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleSurfaceFriction, SetParticleSurfaceFriction)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleTerrainCohesion),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleTerrainCohesion, SetParticleTerrainCohesion)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleTerrainRestitution),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleTerrainRestitution, SetParticleTerrainRestitution)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleTerrainRollingResistance),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleTerrainRollingResistance,
				SetParticleTerrainRollingResistance)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleTerrainSurfaceFriction),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleTerrainSurfaceFriction, SetParticleTerrainSurfaceFriction)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleTerrainYoungsModulus),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleTerrainYoungsModulus, SetParticleTerrainYoungsModulus)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainParticles),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainParticleProperties, ParticleYoungsModulus),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainParticles.ParticleYoungsModulus, SetParticleYoungsModulus)
		});

	// Excavation contact properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainExcavationContact),
		GET_MEMBER_NAME_CHECKED(
			FAGX_TerrainExcavationContactProperties, AggregateStiffnessMultiplier),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainExcavationContact.AggregateStiffnessMultiplier,
				SetAggregateStiffnessMultiplier)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainExcavationContact),
		GET_MEMBER_NAME_CHECKED(
			FAGX_TerrainExcavationContactProperties, ExcavationStiffnessMultiplier),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainExcavationContact.ExcavationStiffnessMultiplier,
				SetExcavationStiffnessMultiplier)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainExcavationContact),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainExcavationContactProperties, DepthDecayFactor),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainExcavationContact.DepthDecayFactor, SetDepthDecayFactor)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainExcavationContact),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainExcavationContactProperties, DepthIncreaseFactor),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainExcavationContact.DepthIncreaseFactor, SetDepthIncreaseFactor)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainExcavationContact),
		GET_MEMBER_NAME_CHECKED(
			FAGX_TerrainExcavationContactProperties, MaximumAggregateNormalForce),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainExcavationContact.MaximumAggregateNormalForce,
				SetMaximumAggregateNormalForce)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainMaterial, TerrainExcavationContact),
		GET_MEMBER_NAME_CHECKED(FAGX_TerrainExcavationContactProperties, MaximumContactDepth),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TerrainExcavationContact.MaximumContactDepth, SetMaximumContactDepth)
		});
}
#endif

FTerrainMaterialBarrier* UAGX_TerrainMaterial::GetOrCreateTerrainMaterialNative(
	UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateTerrainMaterialNative was called on UAGX_TerrainMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called "
					 "prior to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateTerrainMaterialNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasTerrainMaterialNative())
	{
		CreateTerrainMaterialNative();
	}
	return GetTerrainMaterialNative();
}

FTerrainMaterialBarrier* UAGX_TerrainMaterial::GetTerrainMaterialNative()
{
	return HasTerrainMaterialNative() ? &TerrainMaterialNativeBarrier : nullptr;
}

UAGX_TerrainMaterial* UAGX_TerrainMaterial::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_TerrainMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_TerrainMaterial::CreateFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

bool UAGX_TerrainMaterial::HasTerrainMaterialNative() const
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			return false;
		}

		return Instance->HasTerrainMaterialNative();
	}

	AGX_CHECK(IsInstance());
	return TerrainMaterialNativeBarrier.HasNative();
}

UAGX_TerrainMaterial* UAGX_TerrainMaterial::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_TerrainMaterial* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	FString InstanceName = Source->GetName() + "_Instance";

	UAGX_TerrainMaterial* NewInstance = NewObject<UAGX_TerrainMaterial>(
		GetTransientPackage(), UAGX_TerrainMaterial::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;

	// Copy the terrain material properties
	NewInstance->CopyTerrainMaterialProperties(Source);

	NewInstance->CreateTerrainMaterialNative();

	return NewInstance;
}

void UAGX_TerrainMaterial::CopyTerrainMaterialProperties(const UAGX_TerrainMaterial* Source)
{
	if (Source)
	{
		TerrainBulk = Source->TerrainBulk;
		TerrainCompaction = Source->TerrainCompaction;
		TerrainParticles = Source->TerrainParticles;
		TerrainExcavationContact = Source->TerrainExcavationContact;

		// The rest of the properties are legacy and should not be used. Still copied to maintain
		// consistency, and to keep our unit tests passing.
		Bulk = Source->Bulk;
		Surface = Source->Surface;
		Wire = Source->Wire;
	}
}

void UAGX_TerrainMaterial::UpdateTerrainMaterialNativeProperties()
{
	if (HasTerrainMaterialNative())
	{
		AGX_CHECK(IsInstance());
		TerrainMaterialNativeBarrier.SetName(TCHAR_TO_UTF8(*GetName()));

		// Set Bulk properties.
		TerrainMaterialNativeBarrier.SetAdhesionOverlapFactor(TerrainBulk.AdhesionOverlapFactor);
		TerrainMaterialNativeBarrier.SetCohesion(TerrainBulk.Cohesion);
		TerrainMaterialNativeBarrier.SetDensity(TerrainBulk.Density);
		TerrainMaterialNativeBarrier.SetDilatancyAngle(TerrainBulk.DilatancyAngle);
		TerrainMaterialNativeBarrier.SetFrictionAngle(TerrainBulk.FrictionAngle);
		TerrainMaterialNativeBarrier.SetMaxDensity(TerrainBulk.MaxDensity);
		TerrainMaterialNativeBarrier.SetPoissonsRatio(TerrainBulk.PoissonsRatio);
		TerrainMaterialNativeBarrier.SetSwellFactor(TerrainBulk.SwellFactor);
		TerrainMaterialNativeBarrier.SetYoungsModulus(TerrainBulk.YoungsModulus);

		// Set Compaction properties.
		TerrainMaterialNativeBarrier.SetAngleOfReposeCompactionRate(
			TerrainCompaction.AngleOfReposeCompactionRate);
		TerrainMaterialNativeBarrier.SetBankStatePhi(TerrainCompaction.BankStatePhi0);
		TerrainMaterialNativeBarrier.SetCompactionTimeRelaxationConstant(
			TerrainCompaction.CompactionTimeRelaxationConstant);
		TerrainMaterialNativeBarrier.SetCompressionIndex(TerrainCompaction.CompressionIndex);
		TerrainMaterialNativeBarrier.SetHardeningConstantKe(TerrainCompaction.HardeningConstantKe);
		TerrainMaterialNativeBarrier.SetHardeningConstantNe(TerrainCompaction.HardeningConstantNe);
		TerrainMaterialNativeBarrier.SetPreconsolidationStress(
			TerrainCompaction.PreconsolidationStress);
		TerrainMaterialNativeBarrier.SetStressCutOffFraction(
			TerrainCompaction.StressCutOffFraction);
		TerrainMaterialNativeBarrier.SetDilatancyAngleScalingFactor(
			TerrainCompaction.DilatancyAngleScalingFactor);

		// Set Particle properties.
		TerrainMaterialNativeBarrier.SetParticleAdhesionOverlapFactor(
			TerrainParticles.AdhesionOverlapFactor);
		TerrainMaterialNativeBarrier.SetParticleCohesion(TerrainParticles.ParticleCohesion);
		TerrainMaterialNativeBarrier.SetParticleRestitution(TerrainParticles.ParticleRestitution);
		TerrainMaterialNativeBarrier.SetParticleRollingResistance(
			TerrainParticles.ParticleRollingResistance);
		TerrainMaterialNativeBarrier.SetParticleSurfaceFriction(
			TerrainParticles.ParticleSurfaceFriction);
		TerrainMaterialNativeBarrier.SetParticleTerrainCohesion(
			TerrainParticles.ParticleTerrainCohesion);
		TerrainMaterialNativeBarrier.SetParticleTerrainRestitution(
			TerrainParticles.ParticleTerrainRestitution);
		TerrainMaterialNativeBarrier.SetParticleTerrainRollingResistance(
			TerrainParticles.ParticleTerrainRollingResistance);
		TerrainMaterialNativeBarrier.SetParticleTerrainSurfaceFriction(
			TerrainParticles.ParticleTerrainSurfaceFriction);
		TerrainMaterialNativeBarrier.SetParticleTerrainYoungsModulus(
			TerrainParticles.ParticleTerrainYoungsModulus);
		TerrainMaterialNativeBarrier.SetParticleYoungsModulus(
			TerrainParticles.ParticleYoungsModulus);

		// Set Excavation contact properties.
		TerrainMaterialNativeBarrier.SetAggregateStiffnessMultiplier(
			TerrainExcavationContact.AggregateStiffnessMultiplier);
		TerrainMaterialNativeBarrier.SetExcavationStiffnessMultiplier(
			TerrainExcavationContact.ExcavationStiffnessMultiplier);
		TerrainMaterialNativeBarrier.SetDepthDecayFactor(TerrainExcavationContact.DepthDecayFactor);
		TerrainMaterialNativeBarrier.SetDepthIncreaseFactor(
			TerrainExcavationContact.DepthIncreaseFactor);
		TerrainMaterialNativeBarrier.SetMaximumAggregateNormalForce(
			TerrainExcavationContact.MaximumAggregateNormalForce);
		TerrainMaterialNativeBarrier.SetMaximumContactDepth(
			TerrainExcavationContact.MaximumContactDepth);
	}
}

void UAGX_TerrainMaterial::CopyFrom(const FTerrainMaterialBarrier& Source)
{
	// Copy Bulk properties.
	TerrainBulk = FAGX_TerrainBulkProperties();
	TerrainBulk.AdhesionOverlapFactor = Source.GetAdhesionOverlapFactor();
	TerrainBulk.Cohesion = Source.GetCohesion();
	TerrainBulk.Density = Source.GetDensity();
	TerrainBulk.DilatancyAngle = Source.GetDilatancyAngle();
	TerrainBulk.FrictionAngle = Source.GetFrictionAngle();
	TerrainBulk.MaxDensity = Source.GetMaxDensity();
	TerrainBulk.PoissonsRatio = Source.GetPoissonsRatio();
	TerrainBulk.SwellFactor = Source.GetSwellFactor();
	TerrainBulk.YoungsModulus = Source.GetYoungsModulus();

	// Copy Compaction properties.
	TerrainCompaction = FAGX_TerrainCompactionProperties();
	TerrainCompaction.AngleOfReposeCompactionRate = Source.GetAngleOfReposeCompactionRate();
	TerrainCompaction.BankStatePhi0 = Source.GetBankStatePhi();
	TerrainCompaction.CompactionTimeRelaxationConstant =
		Source.GetCompactionTimeRelaxationConstant();
	TerrainCompaction.CompressionIndex = Source.GetCompressionIndex();
	TerrainCompaction.HardeningConstantKe = Source.GetHardeningConstantKe();
	TerrainCompaction.HardeningConstantNe = Source.GetHardeningConstantNe();
	TerrainCompaction.PreconsolidationStress = Source.GetPreconsolidationStress();
	TerrainCompaction.StressCutOffFraction = Source.GetStressCutOffFraction();
	TerrainCompaction.DilatancyAngleScalingFactor = Source.GetDilatancyAngleScalingFactor();

	// Copy Particle properties.
	TerrainParticles = FAGX_TerrainParticleProperties();
	TerrainParticles.AdhesionOverlapFactor = Source.GetParticleAdhesionOverlapFactor();
	TerrainParticles.ParticleCohesion = Source.GetParticleCohesion();
	TerrainParticles.ParticleRestitution = Source.GetParticleRestitution();
	TerrainParticles.ParticleRollingResistance = Source.GetParticleRollingResistance();
	TerrainParticles.ParticleSurfaceFriction = Source.GetParticleSurfaceFriction();
	TerrainParticles.ParticleTerrainCohesion = Source.GetParticleTerrainCohesion();
	TerrainParticles.ParticleTerrainRestitution = Source.GetParticleTerrainRestitution();
	TerrainParticles.ParticleTerrainRollingResistance =
		Source.GetParticleTerrainRollingResistance();
	TerrainParticles.ParticleTerrainSurfaceFriction = Source.GetParticleTerrainSurfaceFriction();
	TerrainParticles.ParticleTerrainYoungsModulus = Source.GetParticleTerrainYoungsModulus();
	TerrainParticles.ParticleYoungsModulus = Source.GetParticleYoungsModulus();

	// Copy Excavation properties.
	TerrainExcavationContact = FAGX_TerrainExcavationContactProperties();
	TerrainExcavationContact.AggregateStiffnessMultiplier =
		Source.GetAggregateStiffnessMultiplier();
	TerrainExcavationContact.ExcavationStiffnessMultiplier =
		Source.GetExcavationStiffnessMultiplier();
	TerrainExcavationContact.DepthDecayFactor = Source.GetDepthDecayFactor();
	TerrainExcavationContact.DepthIncreaseFactor = Source.GetDepthIncreaseFactor();
	TerrainExcavationContact.MaximumAggregateNormalForce = Source.GetMaximumAggregateNormalForce();
	TerrainExcavationContact.MaximumContactDepth = Source.GetMaximumContactDepth();
}

bool UAGX_TerrainMaterial::IsInstance() const
{   
    // This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// Cannot use a negated return value from IsAsset because sometimes we create runtime instances
	// that we want to use as-if they are assets without actually creating real on-drive assets,
	// and difficult to fool the IsAsset function into believing that something is an asset when it
	// actually is not.
	return Asset != nullptr;
}

const FAGX_ShapeMaterialBulkProperties& UAGX_TerrainMaterial::GetShapeMaterialBulkProperties()
{
	return Bulk;
}

const FAGX_ShapeMaterialSurfaceProperties& UAGX_TerrainMaterial::GetShapeMaterialSurfaceProperties()
{
	return Surface;
}

const FAGX_ShapeMaterialWireProperties& UAGX_TerrainMaterial::GetShapeMaterialWireProperties()
{
	return Wire;
}

void UAGX_TerrainMaterial::CreateTerrainMaterialNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateTerrainMaterialNative was called on UAGX_TerrainMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called "
					 "prior to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateTerrainMaterialNative();
		return;
	}

	AGX_CHECK(IsInstance());
	if (TerrainMaterialNativeBarrier.HasNative())
	{
		TerrainMaterialNativeBarrier.ReleaseNative();
	}

	TerrainMaterialNativeBarrier.AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasTerrainMaterialNative());

	UpdateTerrainMaterialNativeProperties();
}
