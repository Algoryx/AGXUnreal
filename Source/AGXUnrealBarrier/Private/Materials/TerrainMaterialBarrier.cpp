// Copyright 2025, Algoryx Simulation AB.

#include "Materials/TerrainMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/version.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Math/UnrealMathUtility.h"
#include "Misc/AssertionMacros.h"

FTerrainMaterialBarrier::FTerrainMaterialBarrier()
	: NativeRef {new FTerrainMaterialRef}
{
}

FTerrainMaterialBarrier::FTerrainMaterialBarrier(FTerrainMaterialBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FTerrainMaterialBarrier::FTerrainMaterialBarrier(std::unique_ptr<FTerrainMaterialRef> Native)
	: NativeRef {std::move(Native)}
{
}

FTerrainMaterialBarrier::~FTerrainMaterialBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTerrainMaterialRef.
}

bool FTerrainMaterialBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTerrainMaterialRef* FTerrainMaterialBarrier::GetNative()
{
	return NativeRef.get();
}

const FTerrainMaterialRef* FTerrainMaterialBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTerrainMaterialBarrier::AllocateNative(const FString& Name)
{
	check(!HasNative());
	NativeRef->Native = new agxTerrain::TerrainMaterial(Convert(Name));
}

void FTerrainMaterialBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FTerrainMaterialBarrier::SetName(const FString& Name)
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	NativeRef->Native->setDescription(Convert(Name));
#else
	NativeRef->Native->setName(Convert(Name));
#endif
}

FString FTerrainMaterialBarrier::GetName() const
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	return Convert(NativeRef->Native->getDescription());
#else
	return Convert(NativeRef->Native->getName());
#endif
}

void FTerrainMaterialBarrier::SetAdhesionOverlapFactor(double AdhesionOverlapFactor)
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	NativeRef->Native->getParticleProperties()->setAdhesionOverlapFactor(AdhesionOverlapFactor);
#else
	NativeRef->Native->getBulkProperties()->setAdhesionOverlapFactor(AdhesionOverlapFactor);
#endif
}

double FTerrainMaterialBarrier::GetAdhesionOverlapFactor() const
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 29, 0, 0)
	return NativeRef->Native->getParticleProperties()->getAdhesionOverlapFactor();
#else
	return NativeRef->Native->getBulkProperties()->getAdhesionOverlapFactor();
#endif
}

void FTerrainMaterialBarrier::SetCohesion(double Cohesion)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setCohesion(Cohesion);
}

double FTerrainMaterialBarrier::GetCohesion() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getCohesion();
}

void FTerrainMaterialBarrier::SetDensity(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setDensity(Density);
}

double FTerrainMaterialBarrier::GetDensity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getDensity();
}

void FTerrainMaterialBarrier::SetDilatancyAngle(double DilatancyAngle)
{
	check(HasNative());
	double DilatancyAngleRad = FMath::DegreesToRadians(DilatancyAngle);
	NativeRef->Native->getBulkProperties()->setDilatancyAngle(DilatancyAngleRad);
}

double FTerrainMaterialBarrier::GetDilatancyAngle() const
{
	check(HasNative());
	double DilatancyAngleRad = NativeRef->Native->getBulkProperties()->getDilatancyAngle();
	return FMath::RadiansToDegrees(DilatancyAngleRad);
}

void FTerrainMaterialBarrier::SetFrictionAngle(double FrictionAngle)
{
	check(HasNative());
	double FricAngRad = FMath::DegreesToRadians(FrictionAngle);
	NativeRef->Native->getBulkProperties()->setFrictionAngle(FricAngRad);
}

double FTerrainMaterialBarrier::GetFrictionAngle() const
{
	check(HasNative());
	double FricAngRad = NativeRef->Native->getBulkProperties()->getFrictionAngle();
	return FMath::RadiansToDegrees(FricAngRad);
}

void FTerrainMaterialBarrier::SetMaxDensity(double MaxDensity)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setMaximumDensity(MaxDensity);
}

double FTerrainMaterialBarrier::GetMaxDensity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getMaximumDensity();
}

void FTerrainMaterialBarrier::SetPoissonsRatio(double PoissonsRatio)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setPoissonsRatio(PoissonsRatio);
}

double FTerrainMaterialBarrier::GetPoissonsRatio() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getPoissonsRatio();
}

void FTerrainMaterialBarrier::SetSwellFactor(double SwellFactor)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setSwellFactor(SwellFactor);
}

double FTerrainMaterialBarrier::GetSwellFactor() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getSwellFactor();
}

void FTerrainMaterialBarrier::SetYoungsModulus(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->getBulkProperties()->setYoungsModulus(YoungsModulus);
}

double FTerrainMaterialBarrier::GetYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getBulkProperties()->getYoungsModulus();
}

void FTerrainMaterialBarrier::SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setAngleOfReposeCompactionRate(
		AngleOfReposeCompactionRate);
}

double FTerrainMaterialBarrier::GetAngleOfReposeCompactionRate() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getAngleOfReposeCompactionRate();
}

void FTerrainMaterialBarrier::SetBankStatePhi(double Phi0)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setBankStatePhi(Phi0);
}

double FTerrainMaterialBarrier::GetBankStatePhi() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getBankStatePhi();
}

void FTerrainMaterialBarrier::SetCompactionTimeRelaxationConstant(
	double CompactionTimeRelaxationConstant)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setCompactionTimeRelaxationConstant(
		CompactionTimeRelaxationConstant);
}

double FTerrainMaterialBarrier::GetCompactionTimeRelaxationConstant() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getCompactionTimeRelaxationConstant();
}

void FTerrainMaterialBarrier::SetCompressionIndex(double CompressionIndex)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setCompressionIndex(CompressionIndex);
}

double FTerrainMaterialBarrier::GetCompressionIndex() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getCompressionIndex();
}

void FTerrainMaterialBarrier::SetHardeningConstantKe(double K_e)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setHardeningConstantKE(K_e);
}

double FTerrainMaterialBarrier::GetHardeningConstantKe() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getHardeningConstantKE();
}

void FTerrainMaterialBarrier::SetHardeningConstantNe(double N_e)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setHardeningConstantNE(N_e);
}

double FTerrainMaterialBarrier::GetHardeningConstantNe() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getHardeningConstantNE();
}

void FTerrainMaterialBarrier::SetPreconsolidationStress(double PreconsolidationStress)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setPreconsolidationStress(PreconsolidationStress);
}

double FTerrainMaterialBarrier::GetPreconsolidationStress() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getPreconsolidationStress();
}

void FTerrainMaterialBarrier::SetStressCutOffFraction(double StressCutOffFraction)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setStressCutOffFraction(StressCutOffFraction);
}

double FTerrainMaterialBarrier::GetStressCutOffFraction() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getStressCutOffFraction();
}

void FTerrainMaterialBarrier::SetDilatancyAngleScalingFactor(double DilatancyAngleScalingFactor)
{
	check(HasNative());
	NativeRef->Native->getCompactionProperties()->setDilatancyAngleScalingFactor(
		DilatancyAngleScalingFactor);
}

double FTerrainMaterialBarrier::GetDilatancyAngleScalingFactor() const
{
	check(HasNative());
	return NativeRef->Native->getCompactionProperties()->getDilatancyAngleScalingFactor();
}

// Particle properties.
void FTerrainMaterialBarrier::SetParticleAdhesionOverlapFactor(double ParticleAdhesionOverlapFactor)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setAdhesionOverlapFactor(
		ParticleAdhesionOverlapFactor);
}

double FTerrainMaterialBarrier::GetParticleAdhesionOverlapFactor() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getAdhesionOverlapFactor();
}

void FTerrainMaterialBarrier::SetParticleCohesion(double ParticleCohesion)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleCohesion(ParticleCohesion);
}

double FTerrainMaterialBarrier::GetParticleCohesion() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleCohesion();
}

void FTerrainMaterialBarrier::SetParticleRestitution(double ParticleRestitution)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleRestitution(ParticleRestitution);
}

double FTerrainMaterialBarrier::GetParticleRestitution() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleRestitution();
}

void FTerrainMaterialBarrier::SetParticleRollingResistance(double ParticleRollingResistance)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleRollingResistance(
		ParticleRollingResistance);
}

double FTerrainMaterialBarrier::GetParticleRollingResistance() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleRollingResistance();
}

void FTerrainMaterialBarrier::SetParticleSurfaceFriction(double ParticleSurfaceFriction)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleSurfaceFriction(ParticleSurfaceFriction);
}

double FTerrainMaterialBarrier::GetParticleSurfaceFriction() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleSurfaceFriction();
}

void FTerrainMaterialBarrier::SetParticleTerrainCohesion(double ParticleTerrainCohesion)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleTerrainCohesion(ParticleTerrainCohesion);
}

double FTerrainMaterialBarrier::GetParticleTerrainCohesion() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleTerrainCohesion();
}

void FTerrainMaterialBarrier::SetParticleTerrainRestitution(double ParticleTerrainRestitution)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleTerrainRestitution(
		ParticleTerrainRestitution);
}

double FTerrainMaterialBarrier::GetParticleTerrainRestitution() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleTerrainRestitution();
}

void FTerrainMaterialBarrier::SetParticleTerrainRollingResistance(
	double ParticleTerrainRollingResistance)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleTerrainRollingResistance(
		ParticleTerrainRollingResistance);
}

double FTerrainMaterialBarrier::GetParticleTerrainRollingResistance() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleTerrainRollingResistance();
}

void FTerrainMaterialBarrier::SetParticleTerrainSurfaceFriction(
	double ParticleTerrainSurfaceFriction)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleTerrainSurfaceFriction(
		ParticleTerrainSurfaceFriction);
}

double FTerrainMaterialBarrier::GetParticleTerrainSurfaceFriction() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleTerrainSurfaceFriction();
}

void FTerrainMaterialBarrier::SetParticleTerrainYoungsModulus(double ParticleTerrainYoungsModulus)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleTerrainYoungsModulus(
		ParticleTerrainYoungsModulus);
}

double FTerrainMaterialBarrier::GetParticleTerrainYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleTerrainYoungsModulus();
}

void FTerrainMaterialBarrier::SetParticleYoungsModulus(double ParticleYoungsModulus)
{
	check(HasNative());
	NativeRef->Native->getParticleProperties()->setParticleYoungsModulus(ParticleYoungsModulus);
}

double FTerrainMaterialBarrier::GetParticleYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getParticleProperties()->getParticleYoungsModulus();
}

// Excavation contact properties.
void FTerrainMaterialBarrier::SetAggregateStiffnessMultiplier(double AggregateStiffnessMultiplier)
{
	check(HasNative());
	NativeRef->Native->getExcavationContactProperties()->setAggregateStiffnessMultiplier(
		AggregateStiffnessMultiplier);
}

double FTerrainMaterialBarrier::GetAggregateStiffnessMultiplier() const
{
	check(HasNative());
	return NativeRef->Native->getExcavationContactProperties()->getAggregateStiffnessMultiplier();
}

void FTerrainMaterialBarrier::SetExcavationStiffnessMultiplier(double ExcavationStiffnessMultiplier)
{
	check(HasNative());
	NativeRef->Native->getExcavationContactProperties()->setExcavationStiffnessMultiplier(
		ExcavationStiffnessMultiplier);
}

double FTerrainMaterialBarrier::GetExcavationStiffnessMultiplier() const
{
	check(HasNative());
	return NativeRef->Native->getExcavationContactProperties()->getExcavationStiffnessMultiplier();
}

void FTerrainMaterialBarrier::SetDepthDecayFactor(double DepthDecayFactor)
{
	check(HasNative());
	NativeRef->Native->getExcavationContactProperties()->setDepthDecayFactor(DepthDecayFactor);
}

double FTerrainMaterialBarrier::GetDepthDecayFactor() const
{
	check(HasNative());
	return NativeRef->Native->getExcavationContactProperties()->getDepthDecayFactor();
}

void FTerrainMaterialBarrier::SetDepthIncreaseFactor(double DepthIncreaseFactor)
{
	check(HasNative());
	NativeRef->Native->getExcavationContactProperties()->setDepthIncreaseFactor(
		DepthIncreaseFactor);
}

double FTerrainMaterialBarrier::GetDepthIncreaseFactor() const
{
	check(HasNative());
	return NativeRef->Native->getExcavationContactProperties()->getDepthIncreaseFactor();
}

void FTerrainMaterialBarrier::SetDepthAngleThreshold(double DepthAngleThreshold)
{
	check(HasNative());
	NativeRef->Native->getExcavationContactProperties()->setDepthAngleThreshold(
		ConvertAngleToAGX(DepthAngleThreshold));
}

double FTerrainMaterialBarrier::GetDepthAngleThreshold() const
{
	check(HasNative());
	return ConvertAngleToUnreal<double>(
		NativeRef->Native->getExcavationContactProperties()->getDepthAngleThreshold());
}

void FTerrainMaterialBarrier::SetMaximumAggregateNormalForce(double MaximumAggregateNormalForce)
{
	check(HasNative());
	NativeRef->Native->getExcavationContactProperties()->setMaximumAggregateNormalForce(
		MaximumAggregateNormalForce);
}

double FTerrainMaterialBarrier::GetMaximumAggregateNormalForce() const
{
	check(HasNative());
	return NativeRef->Native->getExcavationContactProperties()->getMaximumAggregateNormalForce();
}

void FTerrainMaterialBarrier::SetMaximumContactDepth(double MaximumContactDepth)
{
	check(HasNative());
	NativeRef->Native->getExcavationContactProperties()->setMaximumContactDepth(
		ConvertDistanceToAGX(MaximumContactDepth));
}

double FTerrainMaterialBarrier::GetMaximumContactDepth() const
{
	check(HasNative());
#if AGX_VERSION_GREATER_OR_EQUAL(2, 36, 0, 0)
	return ConvertDistanceToUnreal<double>(
		NativeRef->Native->getExcavationContactProperties()->getMaximumContactDepth());
#else
	return ConvertDistanceToUnreal<double>(
		NativeRef->Native->getExcavationContactProperties()->getMaximumDepth());
#endif
}
