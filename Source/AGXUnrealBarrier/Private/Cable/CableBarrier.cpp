// Copyright 2025, Algoryx Simulation AB.

#include "Cable/CableBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/Cable/CableRef.h"
#include "Cable/AGX_CableNodeInfo.h"
#include "Cable/CableNodeBarrier.h"
#include "Cable/CablePropertiesBarrier.h"

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
	const double SegmentLengthSafe = std::max(0.01, SegmentLength);
	agx::Real ResolutionPerUnitLengthAGX = ConvertDistanceInvToAGX(1.0 / SegmentLengthSafe);
	NativeRef->Native = new agxCable::Cable(RadiusAGX, ResolutionPerUnitLengthAGX);
}

bool FCableBarrier::Add(FCableNodeBarrier& Node)
{
	check(HasNative());
	check(Node.HasNative());
	return NativeRef->Native->add(Node.GetNative()->Native);
}

void FCableBarrier::SetCableProperties(FCablePropertiesBarrier& Properties)
{
	check(HasNative());
	check(Properties.HasNative());
	NativeRef->Native->setCableProperties(Properties.GetNative()->Native);
}

FCablePropertiesBarrier FCableBarrier::GetCableProperties() const
{
	check(HasNative());
	return FCablePropertiesBarrier(
		std::make_shared<FCablePropertiesRef>(NativeRef->Native->getCableProperties()));
}

void FCableBarrier::SetCablePropertiesToDefault()
{
	check(HasNative());

	// Setting nullptr is not allowed in AGX.
	FCablePropertiesBarrier Default;
	Default.AllocateNative();
	NativeRef->Native->setCableProperties(Default.GetNative()->Native);
}

namespace CableBarrier_helpers
{
	FAGX_CableNodeInfo GetCableNodeInfoFrom(
		agxCable::CableIterator It, const agx::Vec3& AttachLocation)
	{
		FAGX_CableNodeInfo Info;
		Info.NodeType = EAGX_CableNodeType::Free;
		Info.LockRotationToBody = false;
		Info.WorldTransform.SetLocation(ConvertDisplacement(AttachLocation));
		for (agxCable::SegmentAttachment* Attachment : It->getAttachments())
		{
			agx::RigidBody* Body = Attachment->getRigidBody();
			agx::Constraint* Constraint = Attachment->getConstraint();
			if (Body == nullptr || Constraint == nullptr)
				continue;

			for (agx::UInt I = 0; I < 2; I++)
			{
				if (auto ConstraintAttachment = Constraint->getAttachment(I))
				{
					agx::Vec3 OutWorld;
					agx::Vec3 AttachmentWorld = Body->getFrame()->transformPointToWorld(
						ConstraintAttachment->getFrame()->getTranslate(), OutWorld);
					if (agx::equivalent(OutWorld, AttachLocation))
					{
						Info.NodeType = EAGX_CableNodeType::BodyFixed;
						Info.WorldTransform.SetRotation(
							Convert(It->getAttachments()[0]->getWorldMatrix().getRotate()));
						Info.BodyGuid = Convert(Body->getUuid());
						Info.LockRotationToBody =
							Attachment->is<agxCable::RigidSegmentAttachment>();
					}
				}
			}
		}

		return Info;
	}
}

TArray<FAGX_CableNodeInfo> FCableBarrier::GetNodeInfo() const
{
	using namespace CableBarrier_helpers;
	check(HasNative());
	TArray<FAGX_CableNodeInfo> Nodes;

	agxCable::CableIterator Iterator = NativeRef->Native->begin();
	while (!Iterator.isEnd())
	{
		if (Iterator == NativeRef->Native->begin())
			Nodes.Add(GetCableNodeInfoFrom(Iterator, Iterator->getBeginPosition()));

		Nodes.Add(GetCableNodeInfoFrom(Iterator, Iterator->getEndPosition()));
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

double FCableBarrier::GetSegmentLength() const
{
	check(HasNative());

	// The get resolution in AGX returns length per segment here.
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getResolution());
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

void FCableBarrier::AddCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Add collision group as (hashed) unsigned int.
	NativeRef->Native->addGroup(
		StringTo32BitFnvHash(GroupName.ToString()));
}

void FCableBarrier::AddCollisionGroups(const TArray<FName>& GroupNames)
{
	for (auto& GroupName : GroupNames)
	{
		AddCollisionGroup(GroupName);
	}
}

void FCableBarrier::RemoveCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Remove collision group as (hashed) unsigned int.
	NativeRef->Native->removeGroup(
		StringTo32BitFnvHash(GroupName.ToString()));
}

TArray<FName> FCableBarrier::GetCollisionGroups() const
{
	check(HasNative());

	const auto Collection = NativeRef->Native->findGroupIdCollection();
	TArray<FName> Result;
	for (const agx::Name& Name : Collection.getNames())
	{
		Result.Add(FName(*Convert(Name)));
	}

	for (const agx::UInt32 Id : Collection.getIds())
	{
		Result.Add(FName(*FString::FromInt(Id)));
	}

	return Result;
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
