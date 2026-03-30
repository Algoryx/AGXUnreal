// Copyright 2026, Algoryx Simulation AB.

#include "AGX_ComponentReference.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"


FAGX_ComponentReference::FAGX_ComponentReference()
	: FAGX_ComponentReference(UActorComponent::StaticClass())
{
}

FAGX_ComponentReference::FAGX_ComponentReference(TSubclassOf<UActorComponent> InComponentType)
	: bSearchChildActors(false)
	, ComponentType(InComponentType)
{
}

void FAGX_ComponentReference::SetLocalScope(AActor* InLocalScope)
{
	// This is a workaround for the case where the Component that owns this Component Reference
	// is part of a Child Actor and a Blueprint instance[1]. In that case, the Child Actor does not
	// behave the same as a regular Blueprint Actor instance; it will be destroyed shortly after
	// this code runs. This means that this Component Reference's Local Scope will be invalid or
	// nullptr at the time of visualizing the component, or even at the time when creating the
	// native AGX Dynamics representation of the Component in some cases.
	//
	// Therefore, we set the Local Scope to the owner of the Child Actor containing us, and
	// recursively up until we find an Actor that isn't a Child Actor. This is a workaround, and
	// better solutions may exist, though I have not found any. Limitations: name collisions
	// for Components in any Child Actor and the root Actor are not handled.
	//
	// Should we set bSearchChildActors to true here? If we don't then the user may be confused
	// as to why a Component Reference doesn't find a Component the user expects it to find
	// because we changed the Local Scope. If we do set bSearchChildActors then we might
	// inadvertently make this Component Reference not-equal to the template Component
	// Reference, causing issues with serialization and property changed propagation. Since the
	// former case can be handled by the user, simply tick Search Child Actors on the template,
	// and the user can do nothing about the latter, failing property changed propagation, the
	// safer option is to not modify bSearchChildActors.
	//
	// [1] Doesn't actually test for being a Blueprint instance, since traversing the Parent Actor
	// chain is valid either way, and it is preferable to limit the number of special cases as much
	// as possible.
	LocalScope = FAGX_ObjectUtilities::GetRootParentActor(InLocalScope);
}

AActor* FAGX_ComponentReference::GetScope() const
{
	return IsValid(OwningActor) ? OwningActor : LocalScope;
}

namespace FAGX_ComponentReference_helpers
{
	TArray<UActorComponent*> GetCompatibleComponents(
		TSubclassOf<UActorComponent> ComponentType, const AActor* const Scope,
		bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components;
		if (Scope == nullptr)
		{
			return Components;
		}
		Scope->GetComponents(ComponentType, Components, bSearchChildActors);
		return Components;
	}

	UActorComponent* FindComponent(
		TSubclassOf<UActorComponent> ComponentType, const AActor* const Scope, const FName& Name,
		bool bSearchChildActors)
	{
		TArray<UActorComponent*> Components =
			GetCompatibleComponents(ComponentType, Scope, bSearchChildActors);
		UActorComponent** It = Components.FindByPredicate(
			[&Name](UActorComponent* Component) { return Component->GetFName() == Name; });
		return It != nullptr ? *It : nullptr;
	}
}

void FAGX_ComponentReference::SetComponent(UActorComponent* Component)
{
	if (Component == nullptr)
	{
		OwningActor = nullptr;
		Name = NAME_None;
		return;
	}

	OwningActor = Component->GetOwner();
	Name = Component->GetFName();
}

UActorComponent* FAGX_ComponentReference::GetComponent() const
{
	const AActor* const Scope = GetScope();
	return FAGX_ComponentReference_helpers::FindComponent(
		ComponentType, Scope, Name, bSearchChildActors);
}

TArray<UActorComponent*> FAGX_ComponentReference::GetCompatibleComponents() const
{
	const AActor* const Scope = GetScope();
	return FAGX_ComponentReference_helpers::GetCompatibleComponents(
		ComponentType, Scope, bSearchChildActors);
}

void UAGX_ComponentReference_FL::SetComponent(
	FAGX_ComponentReference& Reference, UActorComponent* Component)
{
	Reference.SetComponent(Component);
}

UActorComponent* UAGX_ComponentReference_FL::GetComponent(FAGX_ComponentReference& Reference)
{
	return Reference.GetComponent();
}
