// Copyright 2026, Algoryx Simulation AB.

#include "AMOR/MergeSplitPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/Wire/WireRef.h"
#include "Constraints/ConstraintBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Wire/WireBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/RigidBody.h>
#include <agxCollide/Geometry.h>
#include <agxSDK/MergeSplitHandler.h>
#include <agxWire/Wire.h>
#include "EndAGXIncludes.h"

FMergeSplitPropertiesBarrier::FMergeSplitPropertiesBarrier()
	: NativePtr {new FMergeSplitPropertiesPtr}
{
}

FMergeSplitPropertiesBarrier::FMergeSplitPropertiesBarrier(
	std::shared_ptr<FMergeSplitPropertiesPtr> Native)
	: NativePtr {(Native)}
{
}


namespace MergeSplitProperties_helpers
{
	agx::RigidBody* GetFrom(FRigidBodyBarrier& Body)
	{
		AGX_CHECK(Body.HasNative());
		return Body.GetNative()->Native;
	}

	agxCollide::Geometry* GetFrom(FShapeBarrier& Shape)
	{
		AGX_CHECK(Shape.HasNativeGeometry());
		return Shape.GetNative()->NativeGeometry;
	}

	agx::Constraint* GetFrom(FConstraintBarrier& Constraint)
	{
		AGX_CHECK(Constraint.HasNative());
		return Constraint.GetNative()->Native;
	}

	agxWire::Wire* GetFrom(FWireBarrier& Wire)
	{
		AGX_CHECK(Wire.HasNative());
		return Wire.GetNative()->Native;
	}
}

template <typename T>
FMergeSplitPropertiesBarrier FMergeSplitPropertiesBarrier::CreateFrom(T& Barrier)
{
	using namespace MergeSplitProperties_helpers;
	if (!Barrier.HasNative())
	{
		return FMergeSplitPropertiesBarrier();
	}

	auto Msp = agxSDK::MergeSplitHandler::getProperties(GetFrom(Barrier));
	if (Msp == nullptr)
	{
		return FMergeSplitPropertiesBarrier();
	}

	return FMergeSplitPropertiesBarrier(std::make_unique<FMergeSplitPropertiesPtr>(Msp));
}

template AGXUNREALBARRIER_API FMergeSplitPropertiesBarrier
FMergeSplitPropertiesBarrier::CreateFrom<FRigidBodyBarrier>(FRigidBodyBarrier&);

template AGXUNREALBARRIER_API FMergeSplitPropertiesBarrier
FMergeSplitPropertiesBarrier::CreateFrom<FShapeBarrier>(FShapeBarrier&);

template AGXUNREALBARRIER_API FMergeSplitPropertiesBarrier
FMergeSplitPropertiesBarrier::CreateFrom<FConstraintBarrier>(FConstraintBarrier&);

template AGXUNREALBARRIER_API FMergeSplitPropertiesBarrier
FMergeSplitPropertiesBarrier::CreateFrom<FWireBarrier>(FWireBarrier&);

bool FMergeSplitPropertiesBarrier::HasNative() const
{
	return NativePtr->Native != nullptr;
}

template <typename T>
void FMergeSplitPropertiesBarrier::AllocateNative(T& Owner)
{
	check(!HasNative());
	check(Owner.HasNative());
	NativePtr->Native = agxSDK::MergeSplitHandler::getOrCreateProperties(Owner.GetNative()->Native);
}

template <>
AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FShapeBarrier>(
	FShapeBarrier& Owner)
{
	check(!HasNative());
	check(Owner.HasNative());
	NativePtr->Native =
		agxSDK::MergeSplitHandler::getOrCreateProperties(Owner.GetNative()->NativeGeometry);
}

template AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FRigidBodyBarrier>(
	FRigidBodyBarrier&);
template AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FConstraintBarrier>(
	FConstraintBarrier&);
template AGXUNREALBARRIER_API void FMergeSplitPropertiesBarrier::AllocateNative<FWireBarrier>(
	FWireBarrier&);

void FMergeSplitPropertiesBarrier::ReleaseNative()
{
	check(HasNative());
	NativePtr->Native = nullptr;
}

FMergeSplitPropertiesPtr* FMergeSplitPropertiesBarrier::GetNative()
{
	check(HasNative());
	return NativePtr.get();
}

const FMergeSplitPropertiesPtr* FMergeSplitPropertiesBarrier::GetNative() const
{
	check(HasNative());
	return NativePtr.get();
}

void FMergeSplitPropertiesBarrier::SetEnableMerge(bool bEnable)
{
	check(HasNative());
	NativePtr->Native->setEnableMerge(bEnable);
}

bool FMergeSplitPropertiesBarrier::GetEnableMerge() const
{
	check(HasNative());
	return NativePtr->Native->getEnableMerge();
}

void FMergeSplitPropertiesBarrier::SetEnableSplit(bool bEnable)
{
	check(HasNative());
	NativePtr->Native->setEnableSplit(bEnable);
}

bool FMergeSplitPropertiesBarrier::GetEnableSplit() const
{
	check(HasNative());
	return NativePtr->Native->getEnableSplit();
}

void FMergeSplitPropertiesBarrier::SetShapeContactMergeSplitThresholds(
	FShapeContactMergeSplitThresholdsBarrier* Thresholds)
{
	if (Thresholds == nullptr)
	{
		// The default thresholds will be used.
		NativePtr->Native->setContactThresholds(nullptr);
	}
	else
	{
		check(Thresholds->HasNative());
		auto ThresholdsAGX = dynamic_cast<agxSDK::GeometryContactMergeSplitThresholds*>(
			Thresholds->GetNative()->Native.get());
		check(ThresholdsAGX);
		NativePtr->Native->setContactThresholds(ThresholdsAGX);
	}
}

FShapeContactMergeSplitThresholdsBarrier
FMergeSplitPropertiesBarrier::GetShapeContactMergeSplitThresholds() const
{
	if (!HasNative() || NativePtr->Native->getContactThresholds() == nullptr)
	{
		return FShapeContactMergeSplitThresholdsBarrier();
	}

	return FShapeContactMergeSplitThresholdsBarrier(
		std::make_unique<FMergeSplitThresholdsRef>(NativePtr->Native->getContactThresholds()));
}

void FMergeSplitPropertiesBarrier::SetConstraintMergeSplitThresholds(
	FConstraintMergeSplitThresholdsBarrier* Thresholds)
{
	if (Thresholds == nullptr)
	{
		// The default thresholds will be used.
		NativePtr->Native->setConstraintThresholds(nullptr);
	}
	else
	{
		check(Thresholds->HasNative());
		auto ThresholdsAGX = dynamic_cast<agxSDK::ConstraintMergeSplitThresholds*>(
			Thresholds->GetNative()->Native.get());
		check(ThresholdsAGX);
		NativePtr->Native->setConstraintThresholds(ThresholdsAGX);
	}
}

FConstraintMergeSplitThresholdsBarrier
FMergeSplitPropertiesBarrier::GetConstraintMergeSplitThresholds() const
{
	if (!HasNative() || NativePtr->Native->getConstraintThresholds() == nullptr)
	{
		return FConstraintMergeSplitThresholdsBarrier();
	}

	return FConstraintMergeSplitThresholdsBarrier(
		std::make_unique<FMergeSplitThresholdsRef>(NativePtr->Native->getConstraintThresholds()));
}

void FMergeSplitPropertiesBarrier::SetWireMergeSplitThresholds(
	FWireMergeSplitThresholdsBarrier* Thresholds)
{
	if (Thresholds == nullptr)
	{
		// The default thresholds will be used.
		NativePtr->Native->setWireThresholds(nullptr);
	}
	else
	{
		check(Thresholds->HasNative());
		auto ThresholdsAGX =
			dynamic_cast<agxSDK::WireMergeSplitThresholds*>(Thresholds->GetNative()->Native.get());
		check(ThresholdsAGX);
		NativePtr->Native->setWireThresholds(ThresholdsAGX);
	}
}

FWireMergeSplitThresholdsBarrier FMergeSplitPropertiesBarrier::GetWireMergeSplitThresholds() const
{
	if (!HasNative() || NativePtr->Native->getWireThresholds() == nullptr)
	{
		return FWireMergeSplitThresholdsBarrier();
	}

	return FWireMergeSplitThresholdsBarrier(
		std::make_unique<FMergeSplitThresholdsRef>(NativePtr->Native->getWireThresholds()));
}

template <typename T>
void FMergeSplitPropertiesBarrier::BindToNewOwnerImpl(T& NewOwner)
{
	using namespace MergeSplitProperties_helpers;
	AGX_CHECK(NewOwner.HasNative());
	AGX_CHECK(NativePtr != nullptr);

	// Setting nullptr here is valid.
	NativePtr->Native = agxSDK::MergeSplitHandler::getProperties(GetFrom(NewOwner));
}

void FMergeSplitPropertiesBarrier::BindToNewOwner(FRigidBodyBarrier& NewOwner)
{
	BindToNewOwnerImpl(NewOwner);
}

void FMergeSplitPropertiesBarrier::BindToNewOwner(FShapeBarrier& NewOwner)
{
	BindToNewOwnerImpl(NewOwner);
}

void FMergeSplitPropertiesBarrier::BindToNewOwner(FWireBarrier& NewOwner)
{
	BindToNewOwnerImpl(NewOwner);
}

void FMergeSplitPropertiesBarrier::BindToNewOwner(FConstraintBarrier& NewOwner)
{
	BindToNewOwnerImpl(NewOwner);
}
