#include "Materials/AGX_TerrainMaterialInstance.h"

#include "Engine/World.h"

#include "Materials/AGX_TerrainMaterialAsset.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"

UAGX_TerrainMaterialInstance::~UAGX_TerrainMaterialInstance()
{
}

// Surface properties.
void UAGX_TerrainMaterialInstance::SetFrictionEnabled(bool Enabled)
{
	if (HasShapeMaterialNative())
	{
		ShapeMaterialNativeBarrier->SetFrictionEnabled(Enabled);
	}

	Surface.bFrictionEnabled = Enabled;
}

bool UAGX_TerrainMaterialInstance::GetFrictionEnabled() const
{
	if (HasShapeMaterialNative())
	{
		return ShapeMaterialNativeBarrier->GetFrictionEnabled();
	}

	return Surface.bFrictionEnabled;
}

void UAGX_TerrainMaterialInstance::SetRoughness(float Roughness)
{
	if (HasShapeMaterialNative())
	{
		ShapeMaterialNativeBarrier->SetRoughness(static_cast<double>(Roughness));
	}

	Surface.Roughness = static_cast<double>(Roughness);
}

float UAGX_TerrainMaterialInstance::GetRoughness() const
{
	if (HasShapeMaterialNative())
	{
		return static_cast<float>(ShapeMaterialNativeBarrier->GetRoughness());
	}

	return static_cast<float>(Surface.Roughness);
}

void UAGX_TerrainMaterialInstance::SetSurfaceViscosity(float Viscosity)
{
	if (HasShapeMaterialNative())
	{
		ShapeMaterialNativeBarrier->SetSurfaceViscosity(static_cast<double>(Viscosity));
	}

	Surface.Viscosity = static_cast<double>(Viscosity);
}

float UAGX_TerrainMaterialInstance::GetSurfaceViscosity() const
{
	if (HasShapeMaterialNative())
	{
		return static_cast<float>(ShapeMaterialNativeBarrier->GetSurfaceViscosity());
	}

	return static_cast<float>(Surface.Viscosity);
}

void UAGX_TerrainMaterialInstance::SetAdhesion(float AdhesiveForce, float AdhesiveOverlap)
{
	if (HasShapeMaterialNative())
	{
		ShapeMaterialNativeBarrier->SetAdhesion(
			static_cast<double>(AdhesiveForce), static_cast<double>(AdhesiveOverlap));
	}

	Surface.AdhesiveForce = static_cast<double>(AdhesiveForce);
	Surface.AdhesiveOverlap = static_cast<double>(AdhesiveOverlap);
}

float UAGX_TerrainMaterialInstance::GetAdhesiveForce() const
{
	if (HasShapeMaterialNative())
	{
		return static_cast<float>(ShapeMaterialNativeBarrier->GetAdhesiveForce());
	}

	return static_cast<float>(Surface.AdhesiveForce);
}

float UAGX_TerrainMaterialInstance::GetAdhesiveOverlap() const
{
	if (HasShapeMaterialNative())
	{
		return static_cast<float>(ShapeMaterialNativeBarrier->GetAdhesiveOverlap());
	}

	return static_cast<float>(Surface.AdhesiveOverlap);
}


// Bulk properties.
void UAGX_TerrainMaterialInstance::SetAdhesionOverlapFactor(float AdhesionOverlapFactor)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetAdhesionOverlapFactor(static_cast<double>(AdhesionOverlapFactor));
	}

	TerrainBulk.AdhesionOverlapFactor = static_cast<double>(AdhesionOverlapFactor);
}

float UAGX_TerrainMaterialInstance::GetAdhesionOverlapFactor() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetAdhesionOverlapFactor());
	}

	return static_cast<float>(TerrainBulk.AdhesionOverlapFactor);
}

void UAGX_TerrainMaterialInstance::SetCohesion(float Cohesion)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetCohesion(
			static_cast<double>(Cohesion));
	}

	TerrainBulk.Cohesion = static_cast<double>(Cohesion);
}

float UAGX_TerrainMaterialInstance::GetCohesion() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetCohesion());
	}

	return static_cast<float>(TerrainBulk.Cohesion);
}

void UAGX_TerrainMaterialInstance::SetDensity(float Density)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetDensity(
			static_cast<double>(Density));
	}

	TerrainBulk.Density = static_cast<double>(Density);
}

float UAGX_TerrainMaterialInstance::GetDensity() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetDensity());
	}

	return static_cast<float>(TerrainBulk.Density);
}

void UAGX_TerrainMaterialInstance::SetDilatancyAngle(float DilatancyAngle)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetDilatancyAngle(
			static_cast<double>(DilatancyAngle));
	}

	TerrainBulk.DilatancyAngle = static_cast<double>(DilatancyAngle);
}

float UAGX_TerrainMaterialInstance::GetDilatancyAngle() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetDilatancyAngle());
	}

	return static_cast<float>(TerrainBulk.DilatancyAngle);
}

void UAGX_TerrainMaterialInstance::SetFrictionAngle(float FrictionAngle)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetFrictionAngle(
			static_cast<double>(FrictionAngle));
	}

	TerrainBulk.FrictionAngle = static_cast<double>(FrictionAngle);
}

float UAGX_TerrainMaterialInstance::GetFrictionAngle() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetFrictionAngle());
	}

	return static_cast<float>(TerrainBulk.FrictionAngle);
}

void UAGX_TerrainMaterialInstance::SetMaxDensity(float MaxDensity)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetMaximumDensity(
			static_cast<double>(MaxDensity));
	}

	TerrainBulk.MaxDensity = static_cast<double>(MaxDensity);
}

float UAGX_TerrainMaterialInstance::GetMaxDensity() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetMaximumDensity());
	}

	return static_cast<float>(TerrainBulk.MaxDensity);
}

void UAGX_TerrainMaterialInstance::SetPoissonsRatio(float PoissonsRatio)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetPoissonsRatio(
			static_cast<double>(PoissonsRatio));
	}

	TerrainBulk.PoissonsRatio = static_cast<double>(PoissonsRatio);
}

float UAGX_TerrainMaterialInstance::GetPoissonsRatio() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetPoissonsRatio());
	}

	return static_cast<float>(TerrainBulk.PoissonsRatio);
}

void UAGX_TerrainMaterialInstance::SetSwellFactor(float SwellFactor)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetSwellFactor(
			static_cast<double>(SwellFactor));
	}

	TerrainBulk.SwellFactor = static_cast<double>(SwellFactor);
}

float UAGX_TerrainMaterialInstance::GetSwellFactor() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetSwellFactor());
	}

	return static_cast<float>(TerrainBulk.SwellFactor);
}

void UAGX_TerrainMaterialInstance::SetYoungsModulus(float YoungsModulus)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetYoungsModulus(
			static_cast<double>(YoungsModulus));
	}

	TerrainBulk.YoungsModulus = static_cast<double>(YoungsModulus);
}

float UAGX_TerrainMaterialInstance::GetYoungsModulus() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetYoungsModulus());
	}

	return static_cast<float>(TerrainBulk.YoungsModulus);
}

// Compaction properties.
void UAGX_TerrainMaterialInstance::SetAngleOfReposeCompactionRate(float AngleOfReposeCompactionRate)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetAngleOfReposeCompactionRate(static_cast<double>(AngleOfReposeCompactionRate));
	}

	TerrainCompaction.AngleOfReposeCompactionRate = static_cast<double>(AngleOfReposeCompactionRate);
}

float UAGX_TerrainMaterialInstance::GetAngleOfReposeCompactionRate() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetAngleOfReposeCompactionRate());
	}

	return static_cast<float>(TerrainCompaction.AngleOfReposeCompactionRate);
}

void UAGX_TerrainMaterialInstance::SetBankStatePhi(float Phi0)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetBankStatePhi(static_cast<double>(Phi0));
	}

	TerrainCompaction.Phi0 = static_cast<double>(Phi0);
}

float UAGX_TerrainMaterialInstance::GetBankStatePhi() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetBankStatePhi());
	}

	return static_cast<float>(TerrainCompaction.Phi0);
}

void UAGX_TerrainMaterialInstance::SetCompactionTimeRelaxationConstant(float CompactionTimeRelaxationConstant)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetCompactionTimeRelaxationConstant(static_cast<double>(CompactionTimeRelaxationConstant));
	}

	TerrainCompaction.CompactionTimeRelaxationConstant = static_cast<double>(CompactionTimeRelaxationConstant);
}

float UAGX_TerrainMaterialInstance::GetCompactionTimeRelaxationConstant() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetCompactionTimeRelaxationConstant());
	}

	return static_cast<float>(TerrainCompaction.CompactionTimeRelaxationConstant);
}

void UAGX_TerrainMaterialInstance::SetCompressionIndex(float CompressionIndex)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetCompressionIndex(static_cast<double>(CompressionIndex));
	}

	TerrainCompaction.CompressionIndex = static_cast<double>(CompressionIndex);
}

float UAGX_TerrainMaterialInstance::GetCompressionIndex() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetCompressionIndex());
	}

	return static_cast<float>(TerrainCompaction.CompressionIndex);
}

void UAGX_TerrainMaterialInstance::SetHardeningConstantKe(float K_e)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetHardeningConstantKE(static_cast<double>(K_e));
	}

	TerrainCompaction.K_e = static_cast<double>(K_e);
}

float UAGX_TerrainMaterialInstance::GetHardeningConstantKe() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetHardeningConstantKE());
	}

	return static_cast<float>(TerrainCompaction.K_e);
}

void UAGX_TerrainMaterialInstance::SetHardeningConstantNe(float N_e)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetHardeningConstantNE(static_cast<double>(N_e));
	}

	TerrainCompaction.N_e = static_cast<double>(N_e);
}

float UAGX_TerrainMaterialInstance::GetHardeningConstantNe() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetHardeningConstantNE());
	}

	return static_cast<float>(TerrainCompaction.N_e);
}

void UAGX_TerrainMaterialInstance::SetPreconsolidationStress(float PreconsolidationStress)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetPreconsolidationStress(static_cast<double>(PreconsolidationStress));
	}

	TerrainCompaction.PreconsolidationStress = static_cast<double>(PreconsolidationStress);
}

float UAGX_TerrainMaterialInstance::GetPreconsolidationStress() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetPreconsolidationStress());
	}

	return static_cast<float>(TerrainCompaction.PreconsolidationStress);
}

void UAGX_TerrainMaterialInstance::SetStressCutOffFraction(float StressCutOffFraction)
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetStressCutOffFraction(static_cast<double>(StressCutOffFraction));
	}

	TerrainCompaction.StressCutOffFraction = static_cast<double>(StressCutOffFraction);
}

float UAGX_TerrainMaterialInstance::GetStressCutOffFraction() const
{
	if (HasTerrainMaterialNative())
	{
		return static_cast<float>(TerrainMaterialNativeBarrier->GetStressCutOffFraction());
	}

	return static_cast<float>(TerrainCompaction.StressCutOffFraction);
}

FTerrainMaterialBarrier* UAGX_TerrainMaterialInstance::GetOrCreateTerrainMaterialNative(
	UWorld* PlayingWorld)
{
	if (!HasTerrainMaterialNative())
	{
		CreateTerrainMaterialNative(PlayingWorld);
	}
	return GetTerrainMaterialNative();
}

FShapeMaterialBarrier* UAGX_TerrainMaterialInstance::GetOrCreateShapeMaterialNative(
	UWorld* PlayingWorld)
{
	if (!HasShapeMaterialNative())
	{
		CreateShapeMaterialNative(PlayingWorld);
	}
	return GetShapeMaterialNative();
}

UAGX_MaterialBase* UAGX_TerrainMaterialInstance::GetOrCreateInstance(UWorld* PlayingWorld)
{
	return this;
}

UAGX_TerrainMaterialInstance* UAGX_TerrainMaterialInstance::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_TerrainMaterialAsset* Source)
{
	check(Source);
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	UObject* Outer = UAGX_Simulation::GetFrom(PlayingWorld);
	check(Outer);

	FString InstanceName = Source->GetName() + "_TerrainMaterialInstance";

	UE_LOG(
		LogAGX, Log,
		TEXT("UAGX_TerrainMaterialInstance::CreateFromAsset is creating an instance "
			 "named \"%s\" (from asset  %s)."),
		*InstanceName, *Source->GetName());

	UAGX_TerrainMaterialInstance* NewInstance = NewObject<UAGX_TerrainMaterialInstance>(
		Outer, UAGX_TerrainMaterialInstance::StaticClass(), *InstanceName, RF_Transient);

	// Copy the terrain material properties
	NewInstance->CopyTerrainMaterialProperties(Source);

	NewInstance->CreateTerrainMaterialNative(PlayingWorld);
	NewInstance->CreateShapeMaterialNative(PlayingWorld);

	return NewInstance;
}

void UAGX_TerrainMaterialInstance::CreateTerrainMaterialNative(UWorld* PlayingWorld)
{
	TerrainMaterialNativeBarrier.Reset(new FTerrainMaterialBarrier());

	TerrainMaterialNativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasTerrainMaterialNative());

	UpdateTerrainMaterialNativeProperties();
}

void UAGX_TerrainMaterialInstance::CreateShapeMaterialNative(UWorld* PlayingWorld)
{
	ShapeMaterialNativeBarrier.Reset(new FShapeMaterialBarrier());

	ShapeMaterialNativeBarrier->AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasShapeMaterialNative());

	UpdateShapeMaterialNativeProperties();
}

bool UAGX_TerrainMaterialInstance::HasTerrainMaterialNative() const
{
	return TerrainMaterialNativeBarrier && TerrainMaterialNativeBarrier->HasNative();
}

bool UAGX_TerrainMaterialInstance::HasShapeMaterialNative() const
{
	return ShapeMaterialNativeBarrier && ShapeMaterialNativeBarrier->HasNative();
}

FTerrainMaterialBarrier* UAGX_TerrainMaterialInstance::GetTerrainMaterialNative()
{
	if (TerrainMaterialNativeBarrier)
	{
		return TerrainMaterialNativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

FShapeMaterialBarrier* UAGX_TerrainMaterialInstance::GetShapeMaterialNative()
{
	if (ShapeMaterialNativeBarrier)
	{
		return ShapeMaterialNativeBarrier.Get();
	}
	else
	{
		return nullptr;
	}
}

void UAGX_TerrainMaterialInstance::UpdateTerrainMaterialNativeProperties()
{
	if (HasTerrainMaterialNative())
	{
		TerrainMaterialNativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));

		// Set Bulk properties.
		TerrainMaterialNativeBarrier->SetAdhesionOverlapFactor(TerrainBulk.AdhesionOverlapFactor);
		TerrainMaterialNativeBarrier->SetCohesion(TerrainBulk.Cohesion);
		TerrainMaterialNativeBarrier->SetDensity(TerrainBulk.Density);
		TerrainMaterialNativeBarrier->SetDilatancyAngle(TerrainBulk.DilatancyAngle);
		TerrainMaterialNativeBarrier->SetFrictionAngle(TerrainBulk.FrictionAngle);
		TerrainMaterialNativeBarrier->SetMaximumDensity(TerrainBulk.MaxDensity);
		TerrainMaterialNativeBarrier->SetPoissonsRatio(TerrainBulk.PoissonsRatio);
		TerrainMaterialNativeBarrier->SetSwellFactor(TerrainBulk.SwellFactor);
		TerrainMaterialNativeBarrier->SetYoungsModulus(TerrainBulk.YoungsModulus);

		// Set Compaction properties.
		TerrainMaterialNativeBarrier->SetAngleOfReposeCompactionRate(
			TerrainCompaction.AngleOfReposeCompactionRate);
		TerrainMaterialNativeBarrier->SetBankStatePhi(TerrainCompaction.Phi0);
		TerrainMaterialNativeBarrier->SetCompactionTimeRelaxationConstant(
			TerrainCompaction.CompactionTimeRelaxationConstant);
		TerrainMaterialNativeBarrier->SetCompressionIndex(TerrainCompaction.CompressionIndex);
		TerrainMaterialNativeBarrier->SetHardeningConstantKE(TerrainCompaction.K_e);
		TerrainMaterialNativeBarrier->SetHardeningConstantNE(TerrainCompaction.N_e);
		TerrainMaterialNativeBarrier->SetPreconsolidationStress(
			TerrainCompaction.PreconsolidationStress);
		TerrainMaterialNativeBarrier->SetStressCutOffFraction(
			TerrainCompaction.StressCutOffFraction);
	}
}

void UAGX_TerrainMaterialInstance::UpdateShapeMaterialNativeProperties()
{
	if (HasShapeMaterialNative())
	{
		ShapeMaterialNativeBarrier->SetName(TCHAR_TO_UTF8(*GetName()));

		ShapeMaterialNativeBarrier->SetDensity(Bulk.Density);
		ShapeMaterialNativeBarrier->SetYoungsModulus(Bulk.YoungsModulus);
		ShapeMaterialNativeBarrier->SetBulkViscosity(Bulk.Viscosity);
		ShapeMaterialNativeBarrier->SetDamping(Bulk.Damping);
		ShapeMaterialNativeBarrier->SetMinMaxElasticRestLength(
			Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);

		ShapeMaterialNativeBarrier->SetFrictionEnabled(Surface.bFrictionEnabled);
		ShapeMaterialNativeBarrier->SetRoughness(Surface.Roughness);
		ShapeMaterialNativeBarrier->SetSurfaceViscosity(Surface.Viscosity);
		ShapeMaterialNativeBarrier->SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
	}
}
