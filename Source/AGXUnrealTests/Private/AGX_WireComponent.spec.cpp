// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(FAGX_WireComponentSpec, "AGXUnreal.Spec.WireComponent", AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_WireComponentSpec)

void FAGX_WireComponentSpec::Define()
{
	Describe(
		"When add on one-past-end index",
		[this]()
		{
			It("should add and return a valid node",
				[this]()
				{
					UAGX_WireComponent* Wire = NewObject<UAGX_WireComponent>();
					TestEqual("Initial number of routing nodes.", Wire->RouteNodes.Num(), 2);

					const FVector Location(0.0, 1.0, 2.0);
					const FWireRoutingNode& Node = Wire->AddNodeAtLocationAtIndex(Location, 2);
					TestTrue("Node is valid.", Node.IsValid());
					TestEqual("Number of routing nodes after add.", Wire->RouteNodes.Num(), 3);
					TestEqual("Node location.", Node.Frame.LocalLocation, Location);
					TestEqual("Node type.", Node.NodeType, EWireNodeType::Free);

					Wire->MarkAsGarbage();
				});
		});

	Describe(
		"When adding on index past one-past-end",
		[this]()
		{
			It("should not add a new and return an invalid node",
				[this]()
				{
					UAGX_WireComponent* Wire = NewObject<UAGX_WireComponent>();
					TestEqual("Initial number of routing nodes.", Wire->RouteNodes.Num(), 2);

					const FVector Location(0.0, 1.0, 2.0);
					const FWireRoutingNode& Node = Wire->AddNodeAtLocationAtIndex(Location, 3);
					TestFalse("Node is invalid.", Node.IsValid());
					TestEqual("Number of routing node is unchanged.", Wire->RouteNodes.Num(), 2);

					Wire->MarkAsGarbage();
				});
		});
}
