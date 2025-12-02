// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireControllerBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_WireController.generated.h"

class UAGX_ShapeComponent;
class UAGX_RigidBodyComponent;
class UAGX_WireComponent;

/**
 * The Wire Controller handles inter-wire and global wire settings such as wire-wire collision
 * detection and dynamic wire contacts.
 *
 * On the AGX Dynamics side Wire Controller is a singleton.
 */
UCLASS(Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_WireController : public UObject
{
	GENERATED_BODY()

public:
	UAGX_WireController();

	/**
	 * Get the Wire Controller.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Wire Controller",
		Meta = (DisplayName = "Get Wire Controller"))
	static UPARAM(DisplayName = "Wire Controller") UAGX_WireController* Get();

	//
	// Wire-wire collision functions.
	//

	/**
	 * Returns true if collision detection is enabled for at least one wire-wire pair.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Controller")
	bool IsWireWireActive() const;

	/**
	 * Enable or disable collision detection for the given wire pair.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Controller")
	bool SetCollisionsEnabled(UAGX_WireComponent* Wire1, UAGX_WireComponent* Wire2, bool bEnable);

	/**
	 * Returns true if collision detection is enabled for the given wire pair.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Controller")
	bool GetCollisionsEnabled(
		const UAGX_WireComponent* Wire1, const UAGX_WireComponent* Wire2) const;

	//
	// Dynamic wire contacts functions.
	//

	/**
	 * Enable or disable the use of dynamic wire contact model against the given Shape.
	 *
	 * Setting the global setting to true overrides a false here.
	 */
	bool SetDynamicWireContactsEnabled(UAGX_ShapeComponent* Shape, bool bEnable);

	/**
	 * Enable or disable the use of dynamic wire contact model against the given Shape.
	 *
	 * Setting the global setting to true overrides a false here.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Wire Controller",
		Meta = (DisplayName = "Set Dynamic Wire Contacts Enabled"))
	bool SetDynamicWireContactsEnabledShape(UAGX_ShapeComponent* Shape, bool bEnable)
	{
		return SetDynamicWireContactsEnabled(Shape, bEnable);
	}

	/**
	 * Enable, or disable, the use of dynamic wire contact model against Shapes currently in
	 * the given Rigid Body.
	 *
	 * Setting the global setting to true overrides a false here.
	 */
	bool SetDynamicWireContactsEnabled(UAGX_RigidBodyComponent* RigidBody, bool bEnable);

	/**
	 * Enable or disable the use of dynamic wire contact model against Shapes currently in
	 * the given Rigid Body.
	 *
	 * Setting the global setting to true overrides a false here.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Wire Controller",
		Meta = (DisplayName = "Set Dynamic Wire Contacts Enabled"))
	bool SetDynamicWireContactsEnabledRigidBody(UAGX_RigidBodyComponent* RigidBody, bool bEnable)
	{
		return SetDynamicWireContactsEnabled(RigidBody, bEnable);
	}

	/**
	 * Force all wire contacts to be dynamic.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Controller")
	bool SetDynamicWireContactsGloballyEnabled(bool bEnable);

	/**
	 * True if the dynamic wire contact model is enabled for the given Shape, either globally
	 * or for that Shape specifically.
	 */
	bool GetDynamicWireContactsEnabled(const UAGX_ShapeComponent* Shape) const;

	/**
	 * True if the dynamic wire contact model is enabled for the given Shape, either globally
	 * or for that Shape specifically.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Wire Controller",
		Meta = (DisplayName = "Get Dynamic Wire Contacts Enabled"))
	bool GetDynamicWireContactsEnabledShape(const UAGX_ShapeComponent* Shape) const
	{
		return GetDynamicWireContactsEnabled(Shape);
	}

	/**
	 * True if the dynamic wire contact model is enabled for all Shapes in the given Rigid Body,
	 * either globally or for those Shapes specifically.
	 */
	bool GetDynamicWireContactsEnabled(const UAGX_RigidBodyComponent* RigidBody) const;

	/**
	 * True if the dynamic wire contact model is enabled for all Shapes in the given Rigid Body,
	 * either globally or for those Shapes specifically.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Wire Controller",
		Meta = (DisplayName = "Get Dynamic Wire Contacts Enabled"))
	bool GetDynamicWireContactsEnabledRigidBody(const UAGX_RigidBodyComponent* RigidBody) const
	{
		return GetDynamicWireContactsEnabled(RigidBody);
	}

	/**
	 * True if all wire contacts are forced to be dynamic.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Controller")
	bool GetDynamicWireContactsGloballyEnabled() const;

	/**
	 * Returns true if this Wire Controller has a Native, which it always should have.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Controller")
	bool HasNative() const;

private:
	void EnsureNativeIsSet();

	FWireControllerBarrier NativeBarrier;
};
