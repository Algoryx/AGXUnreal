// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum EAGX_ContactState
{
	/** There is no contact between the two shapes. */
	CS_NoContact = 0 UMETA(DisplayName = "No contact"),

	/** The two shapes have a relative impacting velocity higher than the threshold. */
	CS_Impact = 1 UMETA(DisplayName = "First contact."),

	/** The two shapes are in contact and not impacting. */
	CS_Contact = 2 UMETA(DisplayName = "Continuous contact."),

	/** The two shapes were in contact at the start of the time step, but have now separated. */
	CS_Separation = 3 UMETA(DisplayName = "Contact in the previous time step, now separated."),
};
