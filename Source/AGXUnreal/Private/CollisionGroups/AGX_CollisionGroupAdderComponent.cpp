// Copyright 2025, Algoryx Simulation AB.

#include "CollisionGroups/AGX_CollisionGroupAdderComponent.h"

#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "AGX_LogCategory.h"

#define LOCTEXT_NAMESPACE "UAGX_CollisionGroupAdderComponent"

UAGX_CollisionGroupAdderComponent::UAGX_CollisionGroupAdderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_CollisionGroupAdderComponent::ForceRefreshChildShapes()
{
	UE_LOG(LogAGX, Log, TEXT("Force refresh shapes called."));

	AActor* Parent = GetOwner();
	if (Parent == nullptr)
	{
		return;
	}

	TArray<AActor*> AllActors;
	FAGX_ObjectUtilities::GetChildActorsOfActor(Parent, AllActors);

	// The Parent must be processed as well.
	AllActors.Add(Parent);

	for (AActor* Actor : AllActors)
	{
		TArray<UAGX_ShapeComponent*> ChildrenShapeComponents;
		Actor->GetComponents(ChildrenShapeComponents, true);

		for (UAGX_ShapeComponent* ShapeComponent : ChildrenShapeComponents)
		{
			for (FName CollisionGroup : CollisionGroups)
			{
				// Note: duplicates will be ignored.
				ShapeComponent->AddCollisionGroup(CollisionGroup);
			}
		}
	}
}

#if WITH_EDITOR
void UAGX_CollisionGroupAdderComponent::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupAdderComponent, CollisionGroups)))
	{
		ApplyCollisionGroupChanges(PropertyChangedEvent);
	}
}
#endif

void UAGX_CollisionGroupAdderComponent::ApplyCollisionGroupChanges(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName =
		GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupAdderComponent, CollisionGroups);
	int32 ChangedArrayIndex = PropertyChangedEvent.GetArrayIndex(PropertyName.ToString());
	EPropertyChangeType::Type ChangeType = PropertyChangedEvent.ChangeType;

	AActor* Parent = GetOwner();
	if (Parent == nullptr)
	{
		return;
	}

	TArray<AActor*> AllActors;
	FAGX_ObjectUtilities::GetChildActorsOfActor(Parent, AllActors);

	// The Parent must be processed as well.
	AllActors.Add(Parent);

	for (AActor* Actor : AllActors)
	{
		TArray<UAGX_ShapeComponent*> ChildrenShapeComponents;
		Actor->GetComponents(ChildrenShapeComponents, true);

		for (UAGX_ShapeComponent* ShapeComponent : ChildrenShapeComponents)
		{
			ApplyChangesToChildShapes(ShapeComponent, ChangeType, ChangedArrayIndex);
		}
	}

	CollisionGroupsLastChange = CollisionGroups;
}

void UAGX_CollisionGroupAdderComponent::ApplyChangesToChildShapes(
	UAGX_ShapeComponent* ShapeComponent, EPropertyChangeType::Type ChangeType, int32 ChangeIndex)
{
	switch (ChangeType)
	{
		case EPropertyChangeType::ArrayAdd:
			ShapeComponent->AddCollisionGroup(CollisionGroups[ChangeIndex]);
			break;
		case EPropertyChangeType::ArrayRemove:
			ShapeComponent->RemoveCollisionGroupIfExists(CollisionGroupsLastChange[ChangeIndex]);
			break;
		case EPropertyChangeType::ArrayClear:
		{
			for (int i = 0; i < CollisionGroupsLastChange.Num(); i++)
			{
				ShapeComponent->RemoveCollisionGroupIfExists(CollisionGroupsLastChange[i]);
			}

			break;
		}
		case EPropertyChangeType::ValueSet: // Value changed.
		{
			// Remove old collision group and add new collision group.
			// @todo This check can be removed once we figure out how to either have proper support
			// for this component in BluePrints, or make it impossible to create this component
			// within a BluePrint. This check is done only so that we don't get a hard crash when
			// using this component in a Blueprint.
			if (CollisionGroupsLastChange.Num() > ChangeIndex)
			{
				ShapeComponent->RemoveCollisionGroupIfExists(
					CollisionGroupsLastChange[ChangeIndex]);
			}

			ShapeComponent->AddCollisionGroup(CollisionGroups[ChangeIndex]);

			break;
		}
		default:
			// Non implemented change type, do nothing.
			break;
	}
}

#undef LOCTEXT_NAMESPACE
