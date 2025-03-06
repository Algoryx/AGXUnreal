// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintController.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintController::FAGX_ConstraintController()
	// This 'false' here doesn't really make much sense, but we must have a
	// default constructor and the default constructor must provide some value.
	// Should never be called for an actual ConstraintController.
	: FAGX_ConstraintController(false)
{
}

FAGX_ConstraintController::FAGX_ConstraintController(bool bInRotational)
	: bEnable(false)
	, Compliance(ConstraintConstants::DefaultCompliance())
	, SpookDamping(ConstraintConstants::DefaultSpookDamping())
	, ForceRange(ConstraintConstants::DefaultForceRange())
	, bRotational(bInRotational)
	, NativeBarrier(nullptr)
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
	SpookDamping = Other.SpookDamping;
	ForceRange = Other.ForceRange;
	bRotational = Other.bRotational;
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

void FAGX_ConstraintController::SetElasticity(double InElasticity)
{
	SetCompliance(1.0 / InElasticity);
}

double FAGX_ConstraintController::GetElasticity() const
{
	return 1.0 / GetCompliance();
}

void FAGX_ConstraintController::SetSpookDamping(double InSpookDamping)
{
	if (HasNative())
	{
		NativeBarrier->SetSpookDamping(InSpookDamping);
	}
	SpookDamping = InSpookDamping;
}

double FAGX_ConstraintController::GetSpookDamping() const
{
	if (HasNative())
	{
		return NativeBarrier->GetSpookDamping();
	}
	else
	{
		return SpookDamping;
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
}

bool FAGX_ConstraintController::HasNative() const
{
	return NativeBarrier.IsValid() && NativeBarrier->HasNative();
}

FConstraintControllerBarrier* FAGX_ConstraintController::GetNative()
{
	check(HasNative());
	return NativeBarrier.Get();
}

const FConstraintControllerBarrier* FAGX_ConstraintController::GetNative() const
{
	check(HasNative());
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
	NativeBarrier->SetSpookDamping(SpookDamping);
	NativeBarrier->SetForceRange(ForceRange);
	UpdateNativePropertiesImpl();
}

void FAGX_ConstraintController::CopyFrom(const FConstraintControllerBarrier& Source)
{
	bEnable = Source.GetEnable();
	Compliance = Source.GetCompliance();
	SpookDamping = Source.GetSpookDamping();
	ForceRange = Source.GetForceRange();
}
