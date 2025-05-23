// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_ScrewController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


namespace
{
	FScrewControllerBarrier* GetScrewBarrier(FAGX_ConstraintScrewController& Controller)
	{
		// See comment in GetElectricMotorBarrier in AGX_GetElectricMotorController.cpp
		return static_cast<FScrewControllerBarrier*>(Controller.GetNative());
	}

	const FScrewControllerBarrier* GetScrewBarrier(const FAGX_ConstraintScrewController& Controller)
	{
		// See comment in GetElectricMotorBarrier in AGX_GetElectricMotorController.cpp
		return static_cast<const FScrewControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintScrewController::SetLead(double InLead)
{
	Lead = InLead;
	if (HasNative())
	{
		GetScrewBarrier(*this)->SetLead(Lead);
	}
}

double FAGX_ConstraintScrewController::GetLead() const
{
	if (HasNative())
	{
		return GetScrewBarrier(*this)->GetLead();
	}
	else
	{
		return Lead;
	}
}

void FAGX_ConstraintScrewController::InitializeBarrier(TUniquePtr<FScrewControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
}

void FAGX_ConstraintScrewController::UpdateNativePropertiesImpl()
{
	FScrewControllerBarrier* Barrier = GetScrewBarrier(*this);
	check(Barrier);
	Barrier->SetLead(Lead);
}

void FAGX_ConstraintScrewController::CopyFrom(const FScrewControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	Lead = Source.GetLead();
}
