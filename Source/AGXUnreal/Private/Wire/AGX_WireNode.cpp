// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireNode.h"

// AGX Dynamics for Unreal includes.
#include "RigidBodyBarrier.h"

FAGX_WireNode::FAGX_WireNode(const FAGX_WireNode& InOther)
	: Barrier(InOther.Barrier)
{
}

FAGX_WireNode::FAGX_WireNode(FWireNodeBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
}

bool FAGX_WireNode::HasNative() const
{
	return Barrier.HasNative();
}

FWireNodeBarrier* FAGX_WireNode::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &Barrier;
}

const FWireNodeBarrier* FAGX_WireNode::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &Barrier;
}

FAGX_WireNode& FAGX_WireNode::operator=(const FAGX_WireNode& InOther)
{
	Barrier = InOther.Barrier;
	return *this;
}

FVector FAGX_WireNode::GetWorldLocation() const
{
	return Barrier.GetWorldLocation();
}

FVector FAGX_WireNode::GetLocalLocation() const
{
	return Barrier.GetTranslate();
}

EWireNodeType FAGX_WireNode::GetType() const
{
	return Barrier.GetType();
}

FRigidBodyBarrier FAGX_WireNode::GetRigidBody() const
{
	return Barrier.GetRigidBody();
}

/* Start of Blueprint Function Library. */

FVector UAGX_WireNode_FL::GetWorldLocation(UPARAM(Ref) FAGX_WireNode& Node)
{
	return Node.GetWorldLocation();
}

EWireNodeType UAGX_WireNode_FL::GetType(UPARAM(Ref) FAGX_WireNode& Node)
{
	return Node.GetType();
}
