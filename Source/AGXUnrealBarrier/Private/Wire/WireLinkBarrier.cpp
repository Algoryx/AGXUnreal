// Copyright 2026, Algoryx Simulation AB.

#include "Wire/WireLinkBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Wire/WireLinkRef.h"
#include "BarrierOnly/Wire/WireRef.h"
#include "RigidBodyBarrier.h"
#include "Wire/WireBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxWire/Link.h>
#include <agxWire/Wire.h>
#include "EndAGXIncludes.h"

FWireLinkBarrier::FWireLinkBarrier()
	: NativeRef(new FWireLinkRef())
{
}

FWireLinkBarrier::FWireLinkBarrier(FWireLinkBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))
{
	Other.NativeRef.reset(new FWireLinkRef());
}

FWireLinkBarrier::~FWireLinkBarrier()
{
	// Must provide destructor in .cpp so std::unique_ptr can see the full FWireLinkRef definition.
}

void FWireLinkBarrier::AllocateNative(FRigidBodyBarrier& Body)
{
	check(!HasNative());
	check(Body.HasNative());
	NativeRef->Native = new agxWire::Link(Body.GetNative()->Native);
}

bool FWireLinkBarrier::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Native != nullptr;
}

void FWireLinkBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

FWireLinkRef* FWireLinkBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FWireLinkRef* FWireLinkBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FWireLinkBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FWireLinkBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
	}
	else
	{
		NativeRef->Native = reinterpret_cast<agxWire::Link*>(NativeAddress);
	}
}

void FWireLinkBarrier::Connect(FWireBarrier& Wire, const FVector& LocalPosition, bool bWireBegin)
{
	check(HasNative());
	check(Wire.HasNative());
	const agx::Vec3 LocalPositionAGX = ConvertDisplacement(LocalPosition);
	const agxWire::Link::ConnectionType ConnType =
		bWireBegin ? agxWire::Link::WIRE_BEGIN : agxWire::Link::WIRE_END;
	NativeRef->Native->connect(Wire.GetNative()->Native, LocalPositionAGX, ConnType);
}

void FWireLinkBarrier::AddToWireRoute(FWireBarrier& Wire)
{
	check(HasNative());
	check(Wire.HasNative());
	// wire->add(link) inserts a ConnectingNode into the wire's route.
	// Pass false so that the connection type already set via Connect() is respected.
	Wire.GetNative()->Native->add(NativeRef->Native, false);
}

void FWireLinkBarrier::SetConnectingNodeRadius(FWireBarrier& Wire, double RadiusCm)
{
	check(HasNative());
	check(Wire.HasNative());
	agxWire::ILinkNode* ConnNode =
		NativeRef->Native->getConnectingNode(Wire.GetNative()->Native);
	if (ConnNode != nullptr)
	{
		ConnNode->setRadius(ConvertDistanceToAGX(RadiusCm));
	}
}

FRigidBodyBarrier FWireLinkBarrier::GetRigidBody() const
{
	check(HasNative());
	agx::RigidBody* Body = NativeRef->Native->getRigidBody();
	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}

FGuid FWireLinkBarrier::GetGuid() const
{
	check(HasNative());
	const agx::Uuid UuidAGX = NativeRef->Native->getUuid();
	return Convert(UuidAGX);
}

