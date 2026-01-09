// Copyright 2025, Algoryx Simulation AB.

#include "Cable/CableBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Cable/CableRef.h"
#include "Cable/AGX_CableNodeInfo.h"
#include "Cable/CableNodeBarrier.h"

FCableBarrier::FCableBarrier()
	: NativeRef {new FCableRef}
{
}

FCableBarrier::FCableBarrier(std::shared_ptr<FCableRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

void FCableBarrier::AllocateNative(double Radius, double ResolutionPerUnitLength)
{
	check(!HasNative());
	agx::Real RadiusAGX = ConvertDistanceToAGX(Radius);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInvToAGX(ResolutionPerUnitLength);
	NativeRef->Native = new agxCable::Cable(RadiusAGX, ResolutionPerUnitLengthAGX);
}

bool FCableBarrier::Add(FCableNodeBarrier& Node)
{
	check(HasNative());
	check(Node.HasNative());
	return NativeRef->Native->add(Node.GetNative()->Native);
}

TArray<FAGX_CableNodeInfo> FCableBarrier::GetNodeInfo() const
{
	check(HasNative());
	TArray<FAGX_CableNodeInfo> Nodes;
	agxCable::CableIterator Iterator = NativeRef->Native->begin();

	auto GetNodeType = [](agxCable::CableIterator It)
	{
		return It->is<agxCable::FreeNode>() ? EAGX_CableNodeType::Free
											: EAGX_CableNodeType::BodyFixed;
	};

	while (!Iterator.isEnd())
	{
		if (Iterator == NativeRef->Native->begin())
		{
			// TODO: figure out locked or not.
			Nodes.Add(FAGX_CableNodeInfo(
				GetNodeType(Iterator), ConvertDisplacement(Iterator->getBeginPosition()), false));
		}

		// TODO: figure out locked or not.
		Nodes.Add(FAGX_CableNodeInfo(
			GetNodeType(Iterator), ConvertDisplacement(Iterator->getEndPosition()), false));
		Iterator++;
	}

	return Nodes;
}

double FCableBarrier::GetRadius() const
{
	check(HasNative());
	const agx::Real RadiusAGX = NativeRef->Native->getRadius();
	const double Radius = ConvertDistanceToUnreal<double>(RadiusAGX);
	return Radius;
}

FGuid FCableBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

void FCableBarrier::SetName(const FString& NameUnreal)
{
	check(HasNative());
	agx::String NameAGX = Convert(NameUnreal);
	NativeRef->Native->setName(NameAGX);
}

FString FCableBarrier::GetName() const
{
	check(HasNative());
	FString NameUnreal(Convert(NativeRef->Native->getName()));
	return NameUnreal;
}

bool FCableBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FCableRef* FCableBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FCableRef* FCableBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FCableBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FCableBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
		return;

	if (HasNative())
		ReleaseNative();

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		return;
	}

	NativeRef->Native = reinterpret_cast<agxCable::Cable*>(NativeAddress);
}

void FCableBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
