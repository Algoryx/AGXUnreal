#include "CollisionGroups/AGX_CollisionGroupsComponent.h"

#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "AGX_LogCategory.h"

#define LOCTEXT_NAMESPACE "UAGX_CollisionGroupsComponent"

UAGX_CollisionGroupsComponent::UAGX_CollisionGroupsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_CollisionGroupsComponent::ForceRefreshChildShapes()
{
	UE_LOG(LogAGX, Log, TEXT("Force refresh shapes called."));

	AActor* Parent = GetOwner();
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

void UAGX_CollisionGroupsComponent::PostEditChangeProperty(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupsComponent, CollisionGroups)))
	{
		ApplyCollisionGroupChanges(PropertyChangedEvent);
	}
}

void UAGX_CollisionGroupsComponent::ApplyCollisionGroupChanges(
	FPropertyChangedEvent& PropertyChangedEvent)
{
	FName PropertyName = GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupsComponent, CollisionGroups);
	int32 ChangedArrayIndex = PropertyChangedEvent.GetArrayIndex(PropertyName.ToString());
	EPropertyChangeType::Type ChangeType = PropertyChangedEvent.ChangeType;

	AActor* Parent = GetOwner();
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

void UAGX_CollisionGroupsComponent::ApplyChangesToChildShapes(
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
			ShapeComponent->RemoveCollisionGroupIfExists(CollisionGroupsLastChange[ChangeIndex]);

			ShapeComponent->AddCollisionGroup(CollisionGroups[ChangeIndex]);

			break;
		}
		default:
			// Non implemented change type, do nothing.
			break;
	}
}

#undef LOCTEXT_NAMESPACE