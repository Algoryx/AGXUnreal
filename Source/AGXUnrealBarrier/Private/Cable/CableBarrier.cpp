// Copyright 2025, Algoryx Simulation AB.

#include "Cable/CableBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Cable/CableRef.h"
#include "Cable/AGX_CableNodeInfo.h"
#include "Cable/CableNodeBarrier.h"

// Standard library inludes.
#include <algorithm>

FCableBarrier::FCableBarrier()
	: NativeRef {new FCableRef}
{
}

FCableBarrier::FCableBarrier(std::shared_ptr<FCableRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

void FCableBarrier::AllocateNative(double Radius, double SegmentLength)
{
	check(!HasNative());
	agx::Real RadiusAGX = ConvertDistanceToAGX(Radius);
	const double SegmentLengthSafe = std::max(0.1, SegmentLength);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInvToAGX(1.0 / SegmentLengthSafe);
	NativeRef->Native = new agxCable::Cable(RadiusAGX, ResolutionPerUnitLengthAGX);
}

bool FCableBarrier::Add(FCableNodeBarrier& Node)
{
	check(HasNative());
	check(Node.HasNative());
	return NativeRef->Native->add(Node.GetNative()->Native);
}

namespace CableBarrier_helpers
{
	struct CableAttachmenInfo
	{
		FQuat Orientation;
		EAGX_CableNodeType Type {EAGX_CableNodeType::Free};
		bool LockRotationToBody {false};
	};
}

TArray<FAGX_CableNodeInfo> FCableBarrier::GetNodeInfo() const
{
	using namespace CableBarrier_helpers;
	check(HasNative());
	TArray<FAGX_CableNodeInfo> Nodes;
	agxCable::CableIterator Iterator = NativeRef->Native->begin();

	auto GetAttachmentInfo = [](agxCable::CableIterator It) -> CableAttachmenInfo
	{
		CableAttachmenInfo Info;
		Info.Type =
			It->is<agxCable::FreeNode>() ? EAGX_CableNodeType::Free : EAGX_CableNodeType::BodyFixed;

		if (Info.Type == EAGX_CableNodeType::BodyFixed && It->getAttachments().size() > 0 &&
			It->getAttachments()[0]->is<agxCable::RigidSegmentAttachment>())
		{
			Info.LockRotationToBody = true;
			Info.Orientation = Convert(It->getAttachments()[0]->getWorldMatrix().getRotate());
		}
		else
		{
			Info.LockRotationToBody = false;
			Info.Orientation = FQuat::Identity;
		}

		return Info;
	};

	while (!Iterator.isEnd())
	{
		if (Iterator == NativeRef->Native->begin())
		{
			CableAttachmenInfo AttachInfo = GetAttachmentInfo(Iterator);
			const FTransform Trans(
				AttachInfo.Orientation, ConvertDisplacement(Iterator->getBeginPosition()));
			Nodes.Add(FAGX_CableNodeInfo(AttachInfo.Type, Trans, AttachInfo.LockRotationToBody));
		}

		CableAttachmenInfo AttachInfo = GetAttachmentInfo(Iterator);
		const FTransform Trans(
			AttachInfo.Orientation, ConvertDisplacement(Iterator->getEndPosition()));
		Nodes.Add(FAGX_CableNodeInfo(AttachInfo.Type, Trans, AttachInfo.LockRotationToBody));
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
