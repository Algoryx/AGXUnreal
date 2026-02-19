// Copyright 2026, Algoryx Simulation AB.

#include "MassPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Vec3.h>
#include "EndAGXIncludes.h"

FMassPropertiesBarrier::FMassPropertiesBarrier()
	: NativePtr {new FMassPropertiesPtr}
{
}

FMassPropertiesBarrier::FMassPropertiesBarrier(std::shared_ptr<FMassPropertiesPtr> Native)
	: NativePtr {Native}
{
}

void FMassPropertiesBarrier::SetMass(float MassUnreal)
{
	check(HasNative());
	agx::Real MassAgx = ConvertToAGX(MassUnreal);
	// Pass current auto-generate flag state because we do not want to accidentally change the mask.
	bool bAutoGenerate = NativePtr->Native->getAutoGenerateMask() & agx::MassProperties::MASS;
	NativePtr->Native->setMass(MassAgx, bAutoGenerate);
}

float FMassPropertiesBarrier::GetMass() const
{
	check(HasNative());
	agx::Real MassAgx = NativePtr->Native->getMass();
	float MassUnreal = ConvertToUnreal<float>(MassAgx);
	return MassUnreal;
}

void FMassPropertiesBarrier::SetPrincipalInertia(const FVector& InertiaUnreal)
{
	check(HasNative());
	agx::Vec3 InertiaAgx = Convert(InertiaUnreal);
	// Pass current auto-generate flag state because we do not want to accidentally change the mask.
	bool bAutoGenerate = NativePtr->Native->getAutoGenerateMask() & agx::MassProperties::INERTIA;
	NativePtr->Native->setInertiaTensor(InertiaAgx, bAutoGenerate);
}

FVector FMassPropertiesBarrier::GetPrincipalInertia() const
{
	check(HasNative());
	agx::Vec3 InertiaAgx = NativePtr->Native->getPrincipalInertiae();
	FVector InertiaUnreal = Convert(InertiaAgx);
	return InertiaUnreal;
}

namespace MassPropertiesBarrier_helpers
{
	void SetAutoGenerateFlag(agx::MassProperties& MassProperties, agx::Int32 Flag, bool bEnable)
	{
		agx::UInt32 Mask = MassProperties.getAutoGenerateMask();
		if (bEnable)
		{
			Mask |= Flag;
		}
		else
		{
			Mask &= ~Flag;
		}
		MassProperties.setAutoGenerateMask(Mask);
	}

	bool GetAutoGenerateFlag(const agx::MassProperties& MassProperties, agx::Int32 Flag)
	{
		const agx::UInt32 Mask = MassProperties.getAutoGenerateMask();
		return (Mask & Flag) != 0;
	}
}

void FMassPropertiesBarrier::SetAutoGenerateMass(bool bAuto)
{
	check(HasNative());
	MassPropertiesBarrier_helpers::SetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::MASS, bAuto);
}

bool FMassPropertiesBarrier::GetAutoGenerateMass() const
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::GetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::MASS);
}

void FMassPropertiesBarrier::SetAutoGenerateCenterOfMassOffset(bool bAuto)
{
	check(HasNative());
	MassPropertiesBarrier_helpers::SetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::CM_OFFSET, bAuto);
}

bool FMassPropertiesBarrier::GetAutoGenerateCenterOfMassOffset() const
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::GetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::CM_OFFSET);
}

void FMassPropertiesBarrier::SetAutoGeneratePrincipalInertia(bool bAuto)
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::SetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::INERTIA, bAuto);
}

bool FMassPropertiesBarrier::GetAutoGeneratePrincipalInertia() const
{
	check(HasNative());
	return MassPropertiesBarrier_helpers::GetAutoGenerateFlag(
		*NativePtr->Native, agx::MassProperties::INERTIA);
}

bool FMassPropertiesBarrier::HasNative() const
{
	return NativePtr->Native != nullptr;
}

FMassPropertiesPtr* FMassPropertiesBarrier::GetNative()
{
	check(HasNative());
	return NativePtr.get();
}

const FMassPropertiesPtr* FMassPropertiesBarrier::GetNative() const
{
	return NativePtr.get();
}

void FMassPropertiesBarrier::BindTo(FRigidBodyRef& RigidBody)
{
	if (RigidBody.Native == nullptr)
	{
		NativePtr->Native = nullptr;
		return;
	}

	NativePtr->Native = RigidBody.Native->getMassProperties();
}
