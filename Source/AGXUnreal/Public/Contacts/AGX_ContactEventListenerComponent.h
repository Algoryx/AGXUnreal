// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/AGX_ContactEnums.h"
#include "Contacts/AGX_ShapeContact.h"

// Unreal Engine includes
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_ContactEventListenerComponent.generated.h"

class UAGX_ShapeComponent;

/**
 * Provides access to AGX Dynamics Shape Contacts before contact pruning and Contact Constraint
 * generation. Makes it possible to manipulate and disable contacts.
 *
 * Listen to contacts either by creating a Blueprint class inheriting from AGX Contact Event
 * Listener Component and overriding the Impact, Contact, and/or Separation functions; or by adding
 * an instance of the C++ Component to an Actor and bind to the Impact, Contact, and/or Separation
 * events.
 *
 * An alternative to the Contact Event Listener Component is to bind to the event in Simulation.
 */
UCLASS(
	BlueprintType, Blueprintable, Category = "AGX", ClassGroup = "AGX",
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_ContactEventListenerComponent : public UActorComponent
{
	GENERATED_BODY()

public: // Delegates.
	// The Keep Contact Policy parameter emulates a return value.
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
		FOnImpact, double, TimeStamp, const FAGX_ShapeContact&, ShapeContact,
		const FAGX_KeepContactPolicyHandle&, KeepContactPolicy);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
		FOnContact, double, TimeStamp, const FAGX_ShapeContact&, ShapeContact,
		const FAGX_KeepContactPolicyHandle&, KeepContactPolicy);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
		FOnSeparation, double, TimeStamp, UAGX_ShapeComponent*, FirstShape, UAGX_ShapeComponent*,
		SecondShape);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
		FOnSeparationBarrier, double, TimeStamp, const FAnyShapeBarrier&, FirstShape, const FAnyShapeBarrier&,
		SecondShape);

	UPROPERTY(BlueprintAssignable, Category = "AGX Contact Event Listener")
	FOnImpact OnImpact;

	UPROPERTY(BlueprintAssignable, Category = "AGX Contact Event Listener")
	FOnImpact OnContact;

	UPROPERTY(BlueprintAssignable, Category = "AGX Contact Event Listener")
	FOnSeparation OnSeparation;

	UPROPERTY(BlueprintAssignable, Category = "AGX Contact Event Listener")
	FOnSeparationBarrier OnSeparationBarrier;

public: // Blueprint Native Events.
	/**
	 * Callback that is called when AGX Dynamics detects an impact between two Shapes.
	 *
	 * @param TimeStamp The current time stamp.
	 * @param ShapeContact The Shape Contact that was reported.
	 * @return What to do with the Shape Contact.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "AGX Contact Event Listener")
	EAGX_KeepContactPolicy Impact(double TimeStamp, const FAGX_ShapeContact& ShapeContact);

	/**
	 * Callback that is called when AGX Dynamics detects a contact between two Shapes.
	 *
	 * @param TimeStamp The current time stamp.
	 * @param ShapeContact The Shape Contact that was reported.
	 * @return What to do with the Shape Contact.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "AGX Contact Event Listener")
	EAGX_KeepContactPolicy Contact(double TimeStamp, const FAGX_ShapeContact& ShapeContact);

	/**
	 * Callback that is called when AGX Dynamics detects that two Shapes are no longer in contact.
	 *
	 * @param TimeStamp The current AGX Dynamics time stamp.
	 * @param FirstShape The Shape that is no longer in contact with Second Shape.
	 * @param SecondShape The Shape that is no longer in contact with the First Shape.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "AGX Contact Event Listener")
	void Separation(
		double TimeStamp, const UAGX_ShapeComponent* FirstShape, UAGX_ShapeComponent* SecondShape);

	UFUNCTION(BlueprintNativeEvent, Category = "AGX Contact Event Listener")
	void SeparationBarrier(
		double TimeStamp, const FAnyShapeBarrier& FirstShape, const FAnyShapeBarrier& SecondShape);

public: // Member function overrides.
	//~ Begin UActorComponent interface.
	virtual void BeginPlay() override;
	//~ End UActorComponent interface.

private: // Internal callbacks. These are passed to the AGX Dynamics Contact Event Listener.
	EAGX_KeepContactPolicy ImpactCallback(double TimeStamp, FShapeContactBarrier& ShapeContact);
	EAGX_KeepContactPolicy ContactCallback(double TimeStamp, FShapeContactBarrier& ShapeContact);
	void SeparationCallback(
		double TimeStamp, FAnyShapeBarrier& FirstShape, FAnyShapeBarrier& SecondShape);
};
