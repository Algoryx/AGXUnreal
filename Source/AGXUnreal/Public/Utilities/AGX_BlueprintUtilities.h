// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"

#if WITH_EDITOR

/**
 * A collection of utility functions for working with Blueprints.
 */
class AGXUNREAL_API FAGX_BlueprintUtilities
{
public:
	/**
	 * Convenience structure for representing a SCS Node search result.
	 */
	struct FAGX_BlueprintNodeSearchResult
	{
		FAGX_BlueprintNodeSearchResult(const UBlueprint& InBlueprint, USCS_Node* InNode)
			: Blueprint(InBlueprint)
			, FoundNode(InNode)
		{
		}
		const UBlueprint& Blueprint;
		USCS_Node* FoundNode = nullptr;
	};

	/**
	 * If the Component is inside a Blueprint, this function returns the SCS Node that has this
	 * component as its Component Template. If "SearchParentBlueprints" is
	 * set, any Blueprint parents will be searched as well. Returns the found SCS Node (if any) and
	 * the Blueprint it was found in, or the last Blueprint searched if no matching SCS Node
	 * was found.
	 */
	static FAGX_BlueprintNodeSearchResult GetSCSNodeFromComponent(
		const UBlueprint& Blueprint, const UActorComponent* Component, bool SearchParentBlueprints);

	/**
	 * Searches the Blueprint for an SCS Node of the given name. If "SearchParentBlueprints" is
	 * set, any Blueprint parents will be searched as well. Returns the found SCS Node (if any) and
	 * the Blueprint it was found in, or the last Blueprint searched if no matching SCS Node
	 * was found.
	 */
	static FAGX_BlueprintNodeSearchResult GetSCSNodeFromName(
		const UBlueprint& Blueprint, const FString& Name, bool SearchParentBlueprints);

	/**
	 * Check if the node name exists in the Blueprint.
	 */
	static bool NameExists(UBlueprint& Blueprint, const FString& Name);

	/**
	 * Returns the transform of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FTransform is returned.
	 */
	static FTransform GetTemplateComponentWorldTransform(const USceneComponent* Component);

	/**
	 * Sets the world transform of a template object in a Blueprint.
	 */
	static bool SetTemplateComponentWorldTransform(
		USceneComponent* Component, const FTransform& Transform,
		bool UpdateArchetypeInstances = true, bool ForceOverwriteInstances = false);

	/**
	 * Sets the relative transform of a template object in a Blueprint.
	 */
	static void SetTemplateComponentRelativeTransform(
		USceneComponent& Component, const FTransform& Transform,
		bool UpdateArchetypeInstances = true, bool ForceOverwriteInstances = false);

	/**
	 * Returns the location of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FVector is returned.
	 */
	static FVector GetTemplateComponentWorldLocation(USceneComponent* Component);

	/**
	 * Returns the rotation of a template component inside a Blueprint in relation to the root
	 * component of that Blueprint. If the passed component is not inside a Blueprint, or is
	 * nullptr, the identity FRotator is returned.
	 */
	static FRotator GetTemplateComponentWorldRotation(USceneComponent* Component);

	/**
	 * Returns a list of all template Components in a Blueprint. Does not include template
	 * Components in inherited Blueprints.
	 */
	static TArray<UActorComponent*> GetTemplateComponents(UBlueprint* Bp);
	static TArray<UActorComponent*> GetTemplateComponents(UBlueprintGeneratedClass* Bp);

	/**
	 * Get a list of all template Components, including from parent Blueprints, of the given type.
	 *
	 * @tparam ComponentT The type of Component to get templates for.
	 * @param Blueprint The Blueprint to get templates from.
	 * @return A list of template Components of the given type.
	 */
	template <typename ComponentT>
	static TArray<ComponentT*> GetAllTemplateComponents(UBlueprint& Blueprint);

	/**
	 * Returns the default template component name given a regular name.
	 */
	static FString ToTemplateComponentName(const FString& RegularName);

	/**
	 * Returns the regular name of a template component, i.e. the template component suffix is
	 * removed if present.
	 */
	static FString GetRegularNameFromTemplateComponentName(FString Name);

	/**
	 * Walks up the Blueprint inheritance chain one step and returns the immediate parent if it
	 * exists.
	 */
	static UBlueprint* GetParent(const UBlueprint& Child);

	/**
	 * Walks up the Blueprint inheritance chain and returns the "root" or outermost parent if it
	 * exists. If Child is the outermost parent, the Child itself is returned.
	 */
	static UBlueprint* GetOutermostParent(UBlueprint* Child);

	/**
	 * Returns the Blueprint that the template Component resides in.
	 */
	static UBlueprint* GetBlueprintFrom(const UActorComponent& Component);

	/**
	 * Returns the parent SCS Node if it was found. Will search in parent Blueprints if
	 * bSearchParentBlueprint is set to true.
	 */
	static USCS_Node* GetParentSCSNode(USCS_Node* Node, bool bSearchParentBlueprints = true);

	template <typename ParentComponentT>
	static USCS_Node* GetParentSCSNode(USCS_Node* Node, bool bSearchParentBlueprints = true);
	/**
	 * Finds the first parent Component that resided in the same Blueprint as the given
	 * ComponentTemplate. This function may search in parent Blueprints for SCS Nodes, but will walk
	 * down the archetype instance hierarchy to return the instance with the same outer as the given
	 * ComponentTemplate.
	 */
	static UActorComponent* GetTemplateComponentAttachParent(UActorComponent* ComponentTemplate);

	/**
	 * Makes Node a child of NewParent. If PreserveWorldTransform is set to true and the Node's
	 * TemplateComponent is a USceneComponent, it's world transform will be preserved, i.e. it's
	 * relative transform may change.
	 */
	static void ReParentNode(
		UBlueprint& Blueprint, USCS_Node& Node, USCS_Node& NewParent,
		bool PreserveWorldTransform = true);

	/**
	 * Searches through the node tree and returns the first Template Component matching the given
	 * type.
	 */
	template <typename T>
	static T* GetFirstComponentOfType(const UBlueprint* Blueprint, bool SkipSceneRoot = false);
};

template <typename ComponentT>
TArray<ComponentT*> FAGX_BlueprintUtilities::GetAllTemplateComponents(UBlueprint& Blueprint)
{
	TArray<ComponentT*> Templates;

	// Iterate over the inheritance chain, starting at the given Blueprint.
	for (UBlueprint* Parent = &Blueprint; Parent != nullptr; Parent = GetParent(*Parent))
	{
		// Iterate over all SCS nodes in the Blueprint.
		for (auto Node : Parent->SimpleConstructionScript->GetAllNodes())
		{
			if (Node == nullptr)
				continue;

			// Only interested in Components or a particular type.
			ComponentT* Component = Cast<ComponentT>(Node->ComponentTemplate);
			if (Component == nullptr)
				continue;

			// We want the template owned by the given Blueprint, not the one owned by a parent
			// Blueprint.
			Component =
				FAGX_ObjectUtilities::GetMatchedInstance(Component, Blueprint.GeneratedClass);
			if (Component == nullptr)
				continue;

			Templates.Add(Component);
		}
	}

	return Templates;
}

template <typename ParentComponentT>
USCS_Node* FAGX_BlueprintUtilities::GetParentSCSNode(USCS_Node* Node, bool bSearchParentBlueprints)
{
	USCS_Node* It = Node;
	while (It != nullptr)
	{
		USCS_Node* ParentNode = GetParentSCSNode(It, bSearchParentBlueprints);
		if (ParentNode != nullptr &&
			Cast<ParentComponentT>(ParentNode->ComponentTemplate) != nullptr)
		{
			return ParentNode;
		}
		It = ParentNode;
	}
	return nullptr;
}

template <typename T>
T* FAGX_BlueprintUtilities::GetFirstComponentOfType(const UBlueprint* Blueprint, bool SkipSceneRoot)
{
	if (Blueprint == nullptr || Blueprint->SimpleConstructionScript == nullptr)
	{
		return nullptr;
	}

	for (auto Node : Blueprint->SimpleConstructionScript->GetAllNodes())
	{
		if (T* Component = Cast<T>(Node->ComponentTemplate))
		{
			if (SkipSceneRoot)
			{
				if (Node != Blueprint->SimpleConstructionScript->GetDefaultSceneRootNode())
				{
					return Component;
				}
			}
			else
			{
				return Component;
			}
		}
	}

	return nullptr;
}
#endif // WITH_EDITOR
