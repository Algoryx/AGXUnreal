// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"

#if WITH_EDITOR

enum struct EAGX_Inherited
{
	Include,
	Exclude
};

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
	 * Returns a list of all template Components in a Blueprint,  optionally including inherited
	 * Components, optionally with type filtering.
	 *
	 * If ComponentType is provided then only Component of that type, and subclasses, are included.
	 *
	 * @param Blueprint The Blueprint to get Components from.
	 * @param Inherited Whether to search up the Blueprint inheritance tree.
	 * @param ComponentType Optional. If provided only include templates of that type.
	 */
	static TArray<UActorComponent*> GetTemplateComponents(
		UBlueprint& Blueprint, EAGX_Inherited Inherited, const UClass* ComponentType = nullptr);

	/**
	 * Returns a list of all template Components in the Blueprint that are of, or inherits from, the
	 * given Component type.
	 *
	 * @tparam ComponentT The type of Component to get.
	 * @param Blueprint The Blueprint to get Components from.
	 * @param Inherited Whether to search up the Blueprint inheritance tree.
	 * @return List of template Components with the requested type.
	 */
	template <typename ComponentT>
	static TArray<ComponentT*> GetTemplateComponents(
		UBlueprint& Blueprint, EAGX_Inherited Inherited);

	/**
	 * Returns a list of a SCS Nodes in a Blueprint, optionally including inherited SCS Nodes,
	 * optionally with type filtering.
	 *
	 * If ComponentType is provided then only SCS Nodes for Components of that type, and subclasses,
	 * are included.
	 *
	 * @param Blueprint The Blueprint to get SCS Nodes from.
	 * @param Inherited Whether to search up the Blueprint inheritance tree.
	 * @param ComponentType Optional. If provided only include SCS Nodes for that type.
	 * @return A list of SCS Nodes.
	 */
	static TArray<USCS_Node*> GetSCSNodes(
		UBlueprint& Blueprint, EAGX_Inherited Inherited, const UClass* ComponentType = nullptr);

	/**
	 * Returns a list of SCS Nodes in a Blueprint that represent Components of the given type,
	 * optionally including inherited SCS Nodes.
	 *
	 * @tparam ComponentT The type of Component get get SCS Nodes for.
	 * @param Blueprint The Blueprint to get SCS Nodes from.
	 * @param Inherited Whether to search up the Blueprint inheritance tree.
	 * @return List of SCS Nodes.
	 */
	template <typename ComponentT>
	static TArray<USCS_Node*> GetSCSNodes(UBlueprint& Blueprint, EAGX_Inherited Inherited);

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
	 * Get a list of Blueprints to operate on.
	 *
	 * If inherited are excluded then only Child is returned. If inherited are included then Child
	 * and all ancestor Blueprints are returned.
	 *
	 * @param Child The Blueprint to optionally get the ancestors of.
	 * @param Inherited Whether to include or exclude inheritance.
	 * @return List of Blueprints. Always with Child, possibly including inherited ones as well.
	 */
	static TArray<UBlueprint*> GetSelfOrAncestors(UBlueprint& Child, EAGX_Inherited Inherited);

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
	static T* GetFirstComponentOfType(UBlueprint* Blueprint, bool SkipSceneRoot = false);
};

template <typename ComponentT>
TArray<ComponentT*> FAGX_BlueprintUtilities::GetTemplateComponents(
	UBlueprint& Blueprint, EAGX_Inherited Inherited)
{
	UClass* Class = ComponentT::StaticClass();
	TArray<ComponentT*> Components;
	for (UActorComponent* Component : GetTemplateComponents(Blueprint, Inherited, Class))
	{
		Components.Add(Cast<ComponentT>(Component));
	}
	return Components;
}

// Fast path for the base class case.
template <>
inline TArray<UActorComponent*> FAGX_BlueprintUtilities::GetTemplateComponents(
	UBlueprint& Blueprint, EAGX_Inherited Inherited)
{
	return GetTemplateComponents(Blueprint, Inherited, nullptr);
}

template <typename ComponentT>
TArray<USCS_Node*> FAGX_BlueprintUtilities::GetSCSNodes(
	UBlueprint& Blueprint, EAGX_Inherited Inherited)
{
	UClass* Class = ComponentT::StaticClass();
	return GetSCSNodes(Blueprint, Inherited, Class);
}

// Fast path for the base class case.
template <>
inline TArray<USCS_Node*> FAGX_BlueprintUtilities::GetSCSNodes<UActorComponent>(
	UBlueprint& Blueprint, EAGX_Inherited Inherited)
{
	return GetSCSNodes(Blueprint, Inherited, nullptr);
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
T* FAGX_BlueprintUtilities::GetFirstComponentOfType(UBlueprint* Blueprint, bool SkipSceneRoot)
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
