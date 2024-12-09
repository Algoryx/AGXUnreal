// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_BlueprintUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"

#if WITH_EDITOR
namespace AGX_BlueprintUtilities_helpers
{
	UBlueprintGeneratedClass* GetBlueprintGeneratedClass(const UActorComponent* Component)
	{
		if (Component == nullptr)
		{
			return nullptr;
		}
		return Cast<UBlueprintGeneratedClass>(Component->GetOuter());
	}

	template <typename T>
	TArray<UActorComponent*> GetTemplateComponents(T* Bp)
	{
		TArray<UActorComponent*> Components;
		if (Bp == nullptr)
		{
			return Components;
		}

		for (USCS_Node* Node : Bp->SimpleConstructionScript->GetAllNodes())
		{
			if (UActorComponent* Component = Node->ComponentTemplate)
			{
				Components.Add(Component);
			}
		}

		return Components;
	}
}

FAGX_BlueprintUtilities::FAGX_BlueprintNodeSearchResult
FAGX_BlueprintUtilities::GetSCSNodeFromComponent(
	const UBlueprint& Blueprint, const UActorComponent* Component, bool SearchParentBlueprints)
{
	using namespace AGX_BlueprintUtilities_helpers;
	if (Component == nullptr)
	{
		return FAGX_BlueprintNodeSearchResult(Blueprint, nullptr);
	}

	TArray<USCS_Node*> Nodes = Blueprint.SimpleConstructionScript->GetAllNodes();
	USCS_Node** ComponentNode = Nodes.FindByPredicate(
		[Component](USCS_Node* Node) { return Node->ComponentTemplate == Component; });
	if (ComponentNode != nullptr)
	{
		return FAGX_BlueprintNodeSearchResult(Blueprint, *ComponentNode);
	}

	if (!SearchParentBlueprints)
	{
		// Nothing found, we are done.
		return FAGX_BlueprintNodeSearchResult(Blueprint, nullptr);
	}

	// Search in parent Blueprints, by name.
	return GetSCSNodeFromName(
		Blueprint, GetRegularNameFromTemplateComponentName(Component->GetName()), true);
}

FAGX_BlueprintUtilities::FAGX_BlueprintNodeSearchResult FAGX_BlueprintUtilities::GetSCSNodeFromName(
	const UBlueprint& Blueprint, const FString& Name, bool SearchParentBlueprints)
{
	USCS_Node* Node = Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name));
	if (Node != nullptr)
	{
		return FAGX_BlueprintNodeSearchResult(Blueprint, Node);
	}

	if (!SearchParentBlueprints)
	{
		// Nothing found, we are done.
		return FAGX_BlueprintUtilities::FAGX_BlueprintNodeSearchResult(Blueprint, nullptr);
	}

	// Try with the next parent Blueprint.
	if (UBlueprint* Parent = GetParent(Blueprint))
	{
		return GetSCSNodeFromName(*Parent, Name, SearchParentBlueprints);
	}

	return FAGX_BlueprintNodeSearchResult(Blueprint, nullptr);
}

bool FAGX_BlueprintUtilities::NameExists(UBlueprint& Blueprint, const FString& Name)
{
	return Blueprint.SimpleConstructionScript->FindSCSNode(FName(Name)) != nullptr;
}

FTransform FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(
	const USceneComponent* Component)
{
	using namespace AGX_BlueprintUtilities_helpers;
	if (Component == nullptr)
	{
		return FTransform::Identity;
	}

	UBlueprintGeneratedClass* BlueprintGC = GetBlueprintGeneratedClass(Component);
	if (BlueprintGC == nullptr)
	{
		return FTransform::Identity;
	}

	const UBlueprint* Blueprint = BlueprintGC->SimpleConstructionScript->GetBlueprint();

	USCS_Node* ComponentNode = GetSCSNodeFromComponent(*Blueprint, Component, false).FoundNode;
	if (ComponentNode == nullptr)
	{
		// We could not find a SCS Node matching the passed Component. This may be because we are in
		// a child Blueprint which does not hold any SCS Nodes itself, but it resides in a parent
		// Blueprint. Attempt to get the SCS Node via name and look through parent Blueprints.
		const FString ComponentName = GetRegularNameFromTemplateComponentName(Component->GetName());

		FAGX_BlueprintNodeSearchResult Result = GetSCSNodeFromName(*Blueprint, ComponentName, true);
		ComponentNode = Result.FoundNode;
		Blueprint = &Result.Blueprint;
	}

	if (ComponentNode == nullptr)
	{
		return FTransform::Identity;
	}
	else if (ComponentNode->IsRootNode())
	{
		return FTransform::Identity;
	}

	// Build a chain of USCS_Nodes starting from root and going down to the Component's
	// USCS_Node.
	TArray<USCS_Node*> RootToComponentChain;
	USCS_Node* CurrentNode = ComponentNode;
	RootToComponentChain.Insert(CurrentNode, 0);
	while (USCS_Node* Parent = GetParentSCSNode(CurrentNode, true))
	{
		RootToComponentChain.Insert(Parent, 0);
		CurrentNode = Parent;
	}

	FTransform WorldTransform = FTransform::Identity;
	for (USCS_Node* Node : RootToComponentChain)
	{
		if (Node == nullptr || Node->ComponentTemplate == nullptr)
		{
			continue;
		}

		UActorComponent* ComponentTemplate = FAGX_ObjectUtilities::GetMatchedInstance(
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			Node->ComponentTemplate,
#else
			Node->ComponentTemplate.Get(),
#endif
			Component->GetOuter());
		if (USceneComponent* SceneComponent = Cast<USceneComponent>(ComponentTemplate))
		{
			const FTransform RelativeTransform = SceneComponent->GetRelativeTransform();
			FTransform::Multiply(&WorldTransform, &RelativeTransform, &WorldTransform);
		}
	}

	return WorldTransform;
}

bool FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
	USceneComponent* Component, const FTransform& Transform, bool UpdateArchetypeInstances,
	bool ForceOverwriteInstances)
{
	using namespace AGX_BlueprintUtilities_helpers;
	if (Component == nullptr)
	{
		return false;
	}

	UBlueprintGeneratedClass* BlueprintGC = GetBlueprintGeneratedClass(Component);
	if (BlueprintGC == nullptr)
	{
		return false;
	}

	const UBlueprint* Blueprint = BlueprintGC->SimpleConstructionScript->GetBlueprint();
	if (Blueprint == nullptr)
	{
		return false;
	}

	USCS_Node* ComponentNode = GetSCSNodeFromComponent(*Blueprint, Component, false).FoundNode;
	if (ComponentNode == nullptr)
	{
		// We could not find a SCS Node matching the passed Component. This may be because we are in
		// a child Blueprint which does not hold any SCS Nodes itself, but it resides in a parent
		// Blueprint. Attempt to get the SCS Node via name and look through parent Blueprints.
		const FString ComponentName = GetRegularNameFromTemplateComponentName(Component->GetName());

		FAGX_BlueprintNodeSearchResult Result = GetSCSNodeFromName(*Blueprint, ComponentName, true);
		ComponentNode = Result.FoundNode;
		Blueprint = &Result.Blueprint;
	}

	if (ComponentNode == nullptr)
	{
		return false;
	}

	USCS_Node* ParentNode = GetParentSCSNode(ComponentNode);
	if (ParentNode == nullptr || ParentNode->ComponentTemplate == nullptr)
	{
		return false;
	}

	const FTransform ParentWorldTransform = [&]()
	{
		if (GetParentSCSNode(ParentNode) == nullptr)
		{
			return FTransform::Identity;
		}

		USceneComponent* ParentComponent =
			Cast<USceneComponent>(FAGX_ObjectUtilities::GetMatchedInstance<UActorComponent>(
				ParentNode->ComponentTemplate, Component->GetOuter()));

		if (ParentComponent == nullptr)
		{
			return FTransform::Identity;
		}

		return GetTemplateComponentWorldTransform(ParentComponent);
	}();

	const FVector OrigRelLocation = Component->GetRelativeLocation();
	const FRotator OrigRelRotation = Component->GetRelativeRotation();

	const FTransform NewRelTransform = Transform.GetRelativeTransform(ParentWorldTransform);
	Component->Modify();

	// SetRelativeTransform does not always work for Component templates. Probably due to
	// parent/child hierarchies not being setup and SetRelativeTransform does some world transform
	// calculations internally. Therefore we use the SetRelativeLocation/Rotation explicitly here.
	Component->SetRelativeLocation(NewRelTransform.GetLocation());
	Component->SetRelativeRotation(NewRelTransform.GetRotation());

	if (!UpdateArchetypeInstances)
	{
		// We are done.
		return true;
	}

	// Update any archetype instances that are "in sync" with the template component.
	// Note: Using SetWorldTransform fails here because that function internally uses the
	// transform of the attach-parent to calculate a new relative transform and sets that. When
	// dealing with objects inside a Blueprint, the attach-parent is not set. Therefore we must
	// stick to using only RelativeLocation/Rotation.
	for (USceneComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
	{
		// Only write to the Archetype Instances if they are currently in sync with this
		// template.
		if (ForceOverwriteInstances || (Instance->GetRelativeLocation() == OrigRelLocation &&
										Instance->GetRelativeRotation() == OrigRelRotation))
		{
			Instance->Modify();
			Instance->SetRelativeLocation(Component->GetRelativeLocation());
			Instance->SetRelativeRotation(Component->GetRelativeRotation());

			// The purpose of this function is to make sure the Instances get exactly the same
			// relative transform as the Archetype. However, SetRelativeLocation/Rotation does
			// some transformation calculations internally which in some cases result in (small)
			// rounding errors, which is enough to break the state in the Blueprint. We call the
			// Set..._Direct functions here to ensure that the RelativeLocation/Rotation matches
			// exactly with the archetype. The above calls are still needed because those make
			// sure the component is updated in the viewport without the need to recompile the
			// Blueprint.
			Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
			Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
		}
	}

	return true;
}

void FAGX_BlueprintUtilities::SetTemplateComponentRelativeTransform(
	USceneComponent& Component, const FTransform& Transform, bool UpdateArchetypeInstances,
	bool ForceOverwriteInstances)
{
	const FVector OrigRelLocation = Component.GetRelativeLocation();
	const FRotator OrigRelRotation = Component.GetRelativeRotation();

	Component.Modify();

	// SetRelativeTransform does not always work for Component templates. Probably due to
	// parent/child hierarchies not being setup and SetRelativeTransform does some world transform
	// calculations internally. Therefore we use the SetRelativeLocation/Rotation explicitly here.
	Component.SetRelativeLocation(Transform.GetLocation());
	Component.SetRelativeRotation(Transform.GetRotation());

	if (!UpdateArchetypeInstances)
	{
		// We are done.
		return;
	}

	// Update any archetype instances that are "in sync" with the template component.
	for (USceneComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))
	{
		// Only write to the Archetype Instances if they are currently in sync with this
		// template.
		if (ForceOverwriteInstances || (Instance->GetRelativeLocation() == OrigRelLocation &&
										Instance->GetRelativeRotation() == OrigRelRotation))
		{
			Instance->Modify();
			Instance->SetRelativeLocation(Component.GetRelativeLocation());
			Instance->SetRelativeRotation(Component.GetRelativeRotation());

			// The purpose of this function is to make sure the Instances get exactly the same
			// relative transform as the Archetype. However, SetRelativeLocation/Rotation does
			// some transformation calculations internally which in some cases result in (small)
			// rounding errors, which is enough to break the state in the Blueprint. We call the
			// Set..._Direct functions here to ensure that the RelativeLocation/Rotation matches
			// exactly with the archetype. The above calls are still needed because those make
			// sure the component is updated in the viewport without the need to recompile the
			// Blueprint.
			Instance->SetRelativeLocation_Direct(Component.GetRelativeLocation());
			Instance->SetRelativeRotation_Direct(Component.GetRelativeRotation());
		}
	}
}

TArray<UActorComponent*> FAGX_BlueprintUtilities::GetTemplateComponents(
	UBlueprintGeneratedClass* Bp)
{
	return AGX_BlueprintUtilities_helpers::GetTemplateComponents(Bp);
}

TArray<UActorComponent*> FAGX_BlueprintUtilities::GetTemplateComponents(UBlueprint* Bp)
{
	return AGX_BlueprintUtilities_helpers::GetTemplateComponents(Bp);
}

FString FAGX_BlueprintUtilities::ToTemplateComponentName(const FString& RegularName)
{
	return RegularName + UActorComponent::ComponentTemplateNameSuffix;
}

FString FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(FString Name)
{
	Name.RemoveFromEnd(UActorComponent::ComponentTemplateNameSuffix);
	return Name;
}

FVector FAGX_BlueprintUtilities::GetTemplateComponentWorldLocation(USceneComponent* Component)
{
	return GetTemplateComponentWorldTransform(Component).GetLocation();
}

FRotator FAGX_BlueprintUtilities::GetTemplateComponentWorldRotation(USceneComponent* Component)
{
	return GetTemplateComponentWorldTransform(Component).Rotator();
}

UBlueprint* FAGX_BlueprintUtilities::GetParent(const UBlueprint& Child)
{
	TArray<UBlueprint*> Parents;
	UBlueprint::GetBlueprintHierarchyFromClass(Child.GeneratedClass, Parents);
	if (Parents.Num() <= 1)
	{
		return nullptr;
	}

	return Parents[1];
}

UBlueprint* FAGX_BlueprintUtilities::GetOutermostParent(UBlueprint* Child)
{
	if (Child == nullptr)
	{
		return nullptr;
	}

	TArray<UBlueprint*> Parents;
	UBlueprint::GetBlueprintHierarchyFromClass(Child->GeneratedClass, Parents);
	return Parents.Last();
}

UBlueprint* FAGX_BlueprintUtilities::GetBlueprintFrom(const UActorComponent& Component)
{
	using namespace AGX_BlueprintUtilities_helpers;
	UBlueprintGeneratedClass* Bpgc = GetBlueprintGeneratedClass(&Component);
	if (Bpgc == nullptr || Bpgc->SimpleConstructionScript == nullptr)
	{
		return nullptr;
	}

	return Bpgc->SimpleConstructionScript->GetBlueprint();
}

USCS_Node* FAGX_BlueprintUtilities::GetParentSCSNode(USCS_Node* Node, bool bSearchParentBlueprints)
{
	if (Node == nullptr)
		return nullptr;

	UBlueprint* Blueprint = GetBlueprintFrom(*Node->ComponentTemplate);
	if (Blueprint == nullptr)
		return nullptr;

	if (USCS_Node* Parent = Blueprint->SimpleConstructionScript->FindParentNode(Node))
		return Parent;

	if (!bSearchParentBlueprints)
		return nullptr;

	const FName ParentName = Node->ParentComponentOrVariableName;
	if (ParentName.IsNone())
		return nullptr;

	return GetSCSNodeFromName(*Blueprint, ParentName.ToString(), true).FoundNode;
}

UActorComponent* FAGX_BlueprintUtilities::GetTemplateComponentAttachParent(
	UActorComponent* ComponentTemplate)
{
	if (ComponentTemplate == nullptr)
		return nullptr;

	UBlueprint* Blueprint = GetBlueprintFrom(*ComponentTemplate);
	if (Blueprint == nullptr)
		return nullptr;

	USCS_Node* Node = GetSCSNodeFromComponent(*Blueprint, ComponentTemplate, true).FoundNode;
	if (Node == nullptr)
		return nullptr;

	USCS_Node* Parent = GetParentSCSNode(Node, true);
	if (Parent == nullptr)
		return nullptr;

	return FAGX_ObjectUtilities::GetMatchedInstance<UActorComponent>(
		Parent->ComponentTemplate, ComponentTemplate->GetOuter());
}

void FAGX_BlueprintUtilities::ReParentNode(
	UBlueprint& Blueprint, USCS_Node& Node, USCS_Node& NewParent, bool PreserveWorldTransform)
{
	if (Blueprint.SimpleConstructionScript == nullptr)
		return;

	USCS_Node* OldParent = GetParentSCSNode(&Node, true);
	if (OldParent == &NewParent)
	{
		return; // The parent is already correct. We are done.
	}

	USceneComponent* Component =
		PreserveWorldTransform ? Cast<USceneComponent>(Node.ComponentTemplate) : nullptr;

	if (PreserveWorldTransform && Component == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ReParentNode was called with PreserveWorldTransform for SCS Node '%s', but the "
				 "ComponentTemplate owned by the SCS Node was not a USceneComponent. The world "
				 "transform will not be preserved."),
			*Node.GetVariableName().ToString());
	}

	const FTransform OrigTransform = [PreserveWorldTransform, Component]()
	{
		if (PreserveWorldTransform && Component)
			return FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(Component);
		return FTransform::Identity;
	}();

	if (OldParent != nullptr)
	{
		OldParent->RemoveChildNode(&Node);
	}

	NewParent.AddChildNode(&Node);

	if (PreserveWorldTransform && Component != nullptr)
	{
		SetTemplateComponentWorldTransform(Component, OrigTransform, true);
	}
}

#endif // WITH_EDITOR
