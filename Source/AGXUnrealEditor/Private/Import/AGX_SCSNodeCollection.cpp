// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "Import/AGX_SCSNodeCollection.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
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
		else if (auto Sph = Cast<UAGX_SphereShapeComponent>(Component))
		{
			AGX_CHECK(!SphereShapes.Contains(Sph->ImportGuid));
			if (Sph->ImportGuid.IsValid())
				SphereShapes.Add(Sph->ImportGuid, Node);
		}
		else if (auto Box = Cast<UAGX_BoxShapeComponent>(Component))
		{
			AGX_CHECK(!BoxShapes.Contains(Box->ImportGuid));
			if (Box->ImportGuid.IsValid())
				BoxShapes.Add(Box->ImportGuid, Node);
		}
		else if (auto Cyl = Cast<UAGX_CylinderShapeComponent>(Component))
		{
			AGX_CHECK(!CylinderShapes.Contains(Cyl->ImportGuid));
			if (Cyl->ImportGuid.IsValid())
				CylinderShapes.Add(Cyl->ImportGuid, Node);
		}
		else if (auto Cap = Cast<UAGX_CapsuleShapeComponent>(Component))
		{
			AGX_CHECK(!CapsuleShapes.Contains(Cap->ImportGuid));
			if (Cap->ImportGuid.IsValid())
				CapsuleShapes.Add(Cap->ImportGuid, Node);
		}
		else if (auto Tri = Cast<UAGX_TrimeshShapeComponent>(Component))
		{
			AGX_CHECK(!TrimeshShapes.Contains(Tri->ImportGuid));
			if (Tri->ImportGuid.IsValid())
				TrimeshShapes.Add(Tri->ImportGuid, Node);
		}
		else if (auto Co = Cast<UAGX_ConstraintComponent>(Component))
		{
			if (auto Hi = Cast<UAGX_HingeConstraintComponent>(Component))
			{
				AGX_CHECK(!HingeConstraints.Contains(Hi->ImportGuid));
				if (Hi->ImportGuid.IsValid())
					HingeConstraints.Add(Hi->ImportGuid, Node);
			}
			else if (auto Pr = Cast<UAGX_PrismaticConstraintComponent>(Component))
			{
				AGX_CHECK(!PrismaticConstraints.Contains(Pr->ImportGuid));
				if (Pr->ImportGuid.IsValid())
					PrismaticConstraints.Add(Pr->ImportGuid, Node);
			}
			else if (auto Ba = Cast<UAGX_BallConstraintComponent>(Component))
			{
				AGX_CHECK(!BallConstraints.Contains(Ba->ImportGuid));
				if (Ba->ImportGuid.IsValid())
					BallConstraints.Add(Ba->ImportGuid, Node);
			}
			else if (auto Cy = Cast<UAGX_CylindricalConstraintComponent>(Component))
			{
				AGX_CHECK(!CylindricalConstraints.Contains(Cy->ImportGuid));
				if (Cy->ImportGuid.IsValid())
					CylindricalConstraints.Add(Cy->ImportGuid, Node);
			}
			else if (auto Di = Cast<UAGX_DistanceConstraintComponent>(Component))
			{
				AGX_CHECK(!DistanceConstraints.Contains(Di->ImportGuid));
				if (Di->ImportGuid.IsValid())
					DistanceConstraints.Add(Di->ImportGuid, Node);
			}
			else if (auto Lo = Cast<UAGX_LockConstraintComponent>(Component))
			{
				AGX_CHECK(!LockConstraints.Contains(Lo->ImportGuid));
				if (Lo->ImportGuid.IsValid())
					LockConstraints.Add(Lo->ImportGuid, Node);
			}
			else
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("FAGX_SCSNodeCollection found constraint node: '%s' with unsupported "
						 "type %s."),
					*Node->GetName(), *Component->GetClass()->GetName());
				AGX_CHECK(false);
			}
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
		else if (auto Wi = Cast<UAGX_WireComponent>(Component))
		{
			// Not supported, will be ignored.
		}
		else if (auto Tr = Cast<UAGX_TrackComponent>(Component))
		{
			// Not supported, will be ignored.
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
