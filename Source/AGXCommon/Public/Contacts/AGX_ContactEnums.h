// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ContactEnums.generated.h"

UENUM(BlueprintType)
enum class EAGX_ContactForceComponents : uint8
{
	/// Normal force. Orthogonal to both tangential directions.
	NormalForce,

	/// Tangential force U, orthogonal to the normal force and V.
	TangentialForceU,

	/// Tangential force V, orthogonal to the normal force and V.
	TangentialForceV
};

/**
 * Defines the event states for which a listener will be activated, a mask can be bitwise OR:ed from
 * these members.
 */
UENUM(BlueprintType)
enum class EAGX_ContactListenerActivationMask : uint8
{
	Invalid UMETA(Hidden),

	/// At First contact between two geometries.
	Impact = 1 << 0,

	/// After Impact, before separation, continuous contact.
	Contact = 1 << 1,

	/// At separation between two Geometries.
	Separation = 1 << 2,

	/// Before StepEventListener post are invoked.
	Post = 1 << 3,

	/// The default activation mask, enabling Impact and Separation.
	Default = Impact | Separation,
	All = Impact | Contact | Separation | Post
};

/**
 * Listing of all possible return values from Impact and Contact callbacks to Contact Event
 * Listener.
 */
UENUM(BlueprintType)
enum class EAGX_KeepContactPolicy : uint8
{
	/**
	 * Keep the contact.
	 *
	 * This is overriden by any Contact Event Listener specifying any Remove Contact.
	 */
	KeepContact,

	/**
	 * Remove the contact AFTER all collision handlers have been executed, but BEFORE the solver get
	 * the contacts.
	 */
	RemoveContact,

	/**
	 * Remove the contact immediately AFTER THIS collision handler returns. No contact handler
	 * executed after this will get this contact.
	 */
	RemoveContactImmediately
};

USTRUCT(BlueprintType, Meta = (DisplayName = "AGX Keep Contact Policy"))
struct AGXCOMMON_API FAGX_KeepContactPolicyHandle
{
	GENERATED_BODY()

	// Accessed via the Keep Contact Policy Blueprint Function Library.
	EAGX_KeepContactPolicy* Policy;
};

UCLASS(BlueprintType)
class AGXCOMMON_API UAGX_KeepContactPolicy_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Keep Contact Policy")
	static void SetPolicy(
		UPARAM(Ref) FAGX_KeepContactPolicyHandle& KeepContactPolicy, EAGX_KeepContactPolicy Policy)
	{
		// Higher-valued values override lower-valued ones. So Keep Contact is overriden by Remove
		// Contact, which is overriden by Remove Contact Immediately.
		*KeepContactPolicy.Policy = FMath::Max(*KeepContactPolicy.Policy, Policy);
	}
};
