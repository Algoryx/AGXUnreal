// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_SCSNodeCollection.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "OpenPLX/PLX_SignalHandlerComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"

FAGX_SCSNodeCollection::FAGX_SCSNodeCollection(const UBlueprint& Bp)
{
	if (Bp.SimpleConstructionScript == nullptr)
	{
		return;
	}

	for (USCS_Node* Node : Bp.SimpleConstructionScript->GetAllNodes())
	{
		if (Node == nullptr)
		{
			continue;
		}

		UActorComponent* Component = Node->ComponentTemplate;
		if (Component == nullptr)
		{
			continue;
		}

		if (Component == Bp.SimpleConstructionScript->GetDefaultSceneRootNode()->ComponentTemplate)
		{
			AGX_CHECK(RootComponent == nullptr);
			RootComponent = Node;
		}
		else if (auto Ri = Cast<UAGX_RigidBodyComponent>(Component))
		{
			AGX_CHECK(!RigidBodies.Contains(Ri->ImportGuid));
			if (Ri->ImportGuid.IsValid())
				RigidBodies.Add(Ri->ImportGuid, Node);
		}
		else if (auto Shape = Cast<UAGX_ShapeComponent>(Component))
		{
			AGX_CHECK(!Shapes.Contains(Shape->ImportGuid));
			if (Shape->ImportGuid.IsValid())
				Shapes.Add(Shape->ImportGuid, Node);
		}
		else if (auto Co = Cast<UAGX_ConstraintComponent>(Component))
		{
			AGX_CHECK(!Constraints.Contains(Co->ImportGuid));
			if (Co->ImportGuid.IsValid())
				Constraints.Add(Co->ImportGuid, Node);
		}
		else if (auto Re = Cast<UAGX_ModelSourceComponent>(Component))
		{
			AGX_CHECK(ModelSourceComponent == nullptr);
			ModelSourceComponent = Node;
			for (const auto& SMCTuple : Re->StaticMeshComponentToOwningTrimesh)
			{
				if (USCS_Node* StaticMeshComponentNode =
						Bp.SimpleConstructionScript->FindSCSNode(FName(SMCTuple.Key)))
				{
					const FGuid Guid = SMCTuple.Value;
					if (!Guid.IsValid())
						continue;

					AGX_CHECK(!CollisionStaticMeshComponents.Contains(Guid));
					CollisionStaticMeshComponents.Add(Guid, StaticMeshComponentNode);
				}
			}

			for (const auto& [MeshName, ShapeGuid] : Re->StaticMeshComponentToOwningShape)
			{
				if (!ShapeGuid.IsValid())
					continue;

				USCS_Node* MeshNode = Bp.SimpleConstructionScript->FindSCSNode(FName(MeshName));
				if (MeshNode == nullptr)
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("  Model Source Component knows of a Static Mesh Component "
							 "named '%s' that there is no SCS Node for."),
						*MeshName);
					continue;
				}

				AGX_CHECK(!RenderStaticMeshComponents.Contains(ShapeGuid));
				RenderStaticMeshComponents.Add(ShapeGuid, MeshNode);
			}
		}
		else if (auto St = Cast<UStaticMeshComponent>(Component))
		{
			// Handled by gathering information from the ModelSourceComponent since a Static
			// Mesh Component does not have an Import Guid.
		}
		else if (auto Con = Cast<UAGX_ContactMaterialRegistrarComponent>(Component))
		{
			AGX_CHECK(ContactMaterialRegistrarComponent == nullptr);
			ContactMaterialRegistrarComponent = Node;
		}
		else if (auto Col = Cast<UAGX_CollisionGroupDisablerComponent>(Component))
		{
			AGX_CHECK(CollisionGroupDisablerComponent == nullptr);
			CollisionGroupDisablerComponent = Node;
		}
		else if (auto Tw = Cast<UAGX_TwoBodyTireComponent>(Component))
		{
			AGX_CHECK(!TwoBodyTires.Contains(Tw->ImportGuid));
			if (Tw->ImportGuid.IsValid())
				TwoBodyTires.Add(Tw->ImportGuid, Node);
		}
		else if (auto Ob = Cast<UAGX_ObserverFrameComponent>(Component))
		{
			AGX_CHECK(!ObserverFrames.Contains(Ob->ImportGuid));
			if (Ob->ImportGuid.IsValid())
				ObserverFrames.Add(Ob->ImportGuid, Node);
		}
		else if (auto Shovel = Cast<UAGX_ShovelComponent>(Component))
		{
			AGX_CHECK(!Shovels.Contains(Shovel->ImportGuid))
			if (Shovel->ImportGuid.IsValid())
				Shovels.Add(Shovel->ImportGuid, Node);
		}
		else if (auto Wire = Cast<UAGX_WireComponent>(Component))
		{
			AGX_CHECK(!Wires.Contains(Wire->ImportGuid))
			if (Wire->ImportGuid.IsValid())
				Wires.Add(Wire->ImportGuid, Node);
		}
		else if (auto Track = Cast<UAGX_TrackComponent>(Component))
		{
			AGX_CHECK(!Tracks.Contains(Track->ImportGuid))
			if (Track->ImportGuid.IsValid())
				Tracks.Add(Track->ImportGuid, Node);
		}
		else if (auto Sh = Cast<UPLX_SignalHandlerComponent>(Component))
		{
			AGX_CHECK(SignalHandler == nullptr);
			SignalHandler = Node;
		}
		else
		{
			// We should never encounter a Component type that does not match any of the
			// above.
			UE_LOG(
				LogAGX, Warning,
				TEXT("FAGX_SCSNodeCollection found node '%s' with unsupported type %s."),
				*Node->GetName(), *Component->GetClass()->GetName());
			AGX_CHECK(false);
		}
	}
}
