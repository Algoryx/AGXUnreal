// Copyright 2026, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintController.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Utilities/AGXUtilities.h"

// Unreal Engine includes.
#include "Serialization/Archive.h"
#include "UObject/UObjectGlobals.h"

FAGX_ConstraintController::FAGX_ConstraintController()
	: bEnable(false)
	, Compliance(ConstraintConstants::DefaultCompliance())
	, Attenuation(ConstraintConstants::DefaultAttenuation())
	, ForceRange(ConstraintConstants::DefaultForceRange())
	, NativeBarrier(nullptr)
	, SpookDamping_DEPRECATED(ConstraintConstants::DefaultSpookDamping())
	, Elasticity_DEPRECATED(ConstraintConstants::DefaultElasticity())
{
}

FAGX_ConstraintController::~FAGX_ConstraintController()
{
}

FAGX_ConstraintController& FAGX_ConstraintController::operator=(
	const FAGX_ConstraintController& Other)
{
	bEnable = Other.bEnable;
	Compliance = Other.Compliance;
	Attenuation = Other.Attenuation;
	ForceRange = Other.ForceRange;
	Name = Other.Name;
	return *this;
}

namespace FAGX_ConstraintController_helpers
{
	void PrintNoNativeConstraintWarning()
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ConstraintController without a native constraint used.\nThis happens when a "
				 "ConstraintController is stored in a Blueprint variable. Instead, store a "
				 "reference to the constraint and re-fetch the ConstraintController every time "
				 "it's needed."));
	}
}

void FAGX_ConstraintController::SetEnable(bool bInEnable)
{
	if (HasNative())
	{
		NativeBarrier->SetEnable(bInEnable);
	}
	bEnable = bInEnable;
}

bool FAGX_ConstraintController::GetEnable() const
{
	if (HasNative())
	{
		return NativeBarrier->GetEnable();
	}
	else
	{
		return bEnable;
	}
}

void FAGX_ConstraintController::SetCompliance(double InCompliance)
{
	if (HasNative())
	{
		NativeBarrier->SetCompliance(InCompliance);
	}
	Compliance = InCompliance;
}

double FAGX_ConstraintController::GetCompliance() const
{
	if (HasNative())
	{
		return NativeBarrier->GetCompliance();
	}
	else
	{
		return Compliance;
	}
}

void FAGX_ConstraintController::SetStiffness(double InStiffness)
{
	SetCompliance(FAGXUtilities::ConvertStiffnessToCompliance(InStiffness));
}

double FAGX_ConstraintController::GetStiffness() const
{
	return FAGXUtilities::ConvertComplianceToStiffness(GetCompliance());
}

void FAGX_ConstraintController::SetElasticity(double InElasticity)
{
	SetStiffness(InElasticity);
}

double FAGX_ConstraintController::GetElasticity() const
{
	return GetStiffness();
}

void FAGX_ConstraintController::SetSpookDamping(double InSpookDamping)
{
	SetAttenuation(FAGXUtilities::ConvertSpookDampingToAttenuation(
		InSpookDamping, GetDefault<UAGX_Simulation>()->TimeStep));
}

double FAGX_ConstraintController::GetSpookDamping() const
{
	return FAGXUtilities::ConvertAttenuationToSpookDamping(
		GetAttenuation(), GetDefault<UAGX_Simulation>()->TimeStep);
}

void FAGX_ConstraintController::SetAttenuation(double InAttenuation)
{
	if (HasNative())
	{
		NativeBarrier->SetAttenuation(InAttenuation);
	}
	Attenuation = InAttenuation;
}

double FAGX_ConstraintController::GetAttenuation() const
{
	if (HasNative())
	{
		return NativeBarrier->GetAttenuation();
	}
	else
	{
		return Attenuation;
	}
}

void FAGX_ConstraintController::SetForceRange(const FAGX_RealInterval& InForceRange)
{
	if (HasNative())
	{
		NativeBarrier->SetForceRange(InForceRange);
	}
	ForceRange = InForceRange;
}

void FAGX_ConstraintController::SetForceRange(double MinForce, double MaxForce)
{
	SetForceRange(FAGX_RealInterval(MinForce, MaxForce));
}

FAGX_RealInterval FAGX_ConstraintController::GetForceRange() const
{
	if (HasNative())
	{
		return NativeBarrier->GetForceRange();
	}
	else
	{
		return ForceRange;
	}
}

double FAGX_ConstraintController::GetForce()
{
	if (!HasNative())
	{
		FAGX_ConstraintController_helpers::PrintNoNativeConstraintWarning();
		return 0.0f;
	}
	return NativeBarrier->GetForce();
}

void FAGX_ConstraintController::Serialize(FArchive& Archive)
{
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::ConstraintsStoreComplianceInsteadOfElasticity))
	{
		Compliance = 1.0 / Elasticity_DEPRECATED;
	}
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::DampingToAttenuation))
	{
		Attenuation = FAGXUtilities::ConvertSpookDampingToAttenuation(
			SpookDamping_DEPRECATED, GetDefault<UAGX_Simulation>()->TimeStep);
	}
}

bool FAGX_ConstraintController::HasNative() const
{
	return NativeBarrier.IsValid() && NativeBarrier->HasNative();
}

FConstraintControllerBarrier* FAGX_ConstraintController::GetNative()
{
	return NativeBarrier.Get();
}

const FConstraintControllerBarrier* FAGX_ConstraintController::GetNative() const
{
	return NativeBarrier.Get();
}

void FAGX_ConstraintController::UpdateNativeProperties()
{
	if (!HasNative())
	{
		FAGX_ConstraintController_helpers::PrintNoNativeConstraintWarning();
		return;
	}
	NativeBarrier->SetEnable(bEnable);
	NativeBarrier->SetCompliance(Compliance);
	NativeBarrier->SetAttenuation(Attenuation);
	NativeBarrier->SetForceRange(ForceRange);
	NativeBarrier->SetName(Name);
	UpdateNativePropertiesImpl();
}

void FAGX_ConstraintController::CopyFrom(const FConstraintControllerBarrier& Source)
{
	bEnable = Source.GetEnable();
	Compliance = Source.GetCompliance();
	Attenuation = Source.GetAttenuation();
	ForceRange = Source.GetForceRange();
	Name = Source.GetName();
}
