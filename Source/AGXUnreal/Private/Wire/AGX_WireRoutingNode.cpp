// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireRoutingNode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_RigidBodyComponent.h"

FWireRoutingNode InvalidRoutingNode(EWireNodeType::NUM_NODE_TYPES);

void FWireRoutingNode::SetBody(UAGX_RigidBodyComponent* Body)
{
	RigidBody.SetComponent(Body);
}

bool FWireRoutingNode::IsValid() const
{
	return NodeType < EWireNodeType::NUM_NODE_TYPES;
}

bool FWireRoutingNode::Serialize(FArchive& Archive)
{
	// Serialize the normal UPROPERTY data.
	//
	// In what cases is this needed and when is it not? If I don't have this here then the Nodes
	// array become empty after load and Unreal Engine prints an error about too few bytes being
	// read. So it makes sense that it is needed. But FAGX_TerrainCompactionProperties::Serialize
	// doesn't call SerializeTaggedProperties. How are this and that struct different? Is it
	// because this Serialize is called because there is a TStructOpsTypeTraits for
	// FWireRoutingNode, causing the default serialization to not happen at all, as opposed to
	// FAGX_TerrainCompactionProperties which have the Serialize function called manually by
	// UAGX_TerrainMaterial after the default serialization has completed?
	//
	// We must serialize also during IsModifyingWeakAndStrongReferences because this struct
	// contains a pointer to a garbage collected object, the Owning Actor in the Rigid Body
	// Reference, so deserializing an instance may cause a new strong reference to the pointed-to
	// Actor to appear. See Unreal Developer Network question at
	// https://udn.unrealengine.com/s/question/0D5QP000008jPSZ0A2/uproperty-aactor-target-set-to-nullptr-by-garbage-collector-after-compile-of-targets-blueprint-class
	if (Archive.IsLoading() || Archive.IsSaving() || Archive.IsModifyingWeakAndStrongReferences())
	{
		UScriptStruct* Struct = FWireRoutingNode::StaticStruct();
		Struct->SerializeTaggedProperties(Archive, reinterpret_cast<uint8*>(this), Struct, nullptr);
	}

	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::WireRouteNodeFrame))
	{
		Frame.LocalLocation = Location_DEPRECATED;
	}

	return true;
}
