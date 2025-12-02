// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_Frame.generated.h"

class USceneComponent;

/**
 * Specifies a transformation relative to a Scene Component.
 *
 * Used, for example, to specify where something attaches to something else, or where something is.
 *
 * Contains an FAGX_ComponentReference named Parent used to identify the Scene Component that this
 * frame is relative to. The Component Reference require some setup to function properly. See
 * AGX_ComponentReference for details.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_Frame
{
	GENERATED_BODY()

	FAGX_Frame();

	/**
	 * The Component that this Frame is relative to.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Frame")
	FAGX_ComponentReference Parent;

	/*
	 * Set the given Component to be the parent of this frame.
	 *
	 * Note that the raw pointer will not be stored, instead the owner of the Component and its name
	 * is stored which means that if the Component is renamed then the relationship is lost. It also
	 * means that the relationship will survive a Blueprint Reconstruction.
	 */
	void SetParentComponent(USceneComponent* Component);

	USceneComponent* GetParentComponent() const;

	/**
	 * The location of the origin of this Frame, specified in the Parent's local coordinate system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Frame")
	FVector LocalLocation {FVector::ZeroVector};

	/**
	 * The rotation of the origin of this Frame, specified in the Parent's local coordinate system.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Frame")
	FRotator LocalRotation {FRotator::ZeroRotator};

	/**
	 * Get the world location of this Frame.
	 *
	 * That is, the result of transforming this Frame's Local Location with the Parent's world
	 * transform.
	 */
	FVector GetWorldLocation() const;

	/**
	 * Same as Get World Location, but use Fallback Parent if Parent is not set.
	 */
	FVector GetWorldLocation(const USceneComponent& FallbackParent) const;

	/**
	 * Set LocalLocation so that this Frame becomes located at the given world location.
	 *
	 * The world-to-local computation is done using the Fallback Parent's transform if Parent is not
	 * set.
	 */
	void SetWorldLocation(const FVector& InLocation, const USceneComponent& FallbackParent);

	/**
	 * Get the world rotation of this Frame.
	 *
	 * That is, the result of transforming this Frame's Local Rotation with the Parent's world
	 * transform.
	 */
	FRotator GetWorldRotation() const;

	/**
	 * Same as Get World Rotation, but use Fallback Parent if Parent is not set.
	 */
	FRotator GetWorldRotation(const USceneComponent& FallbackParent) const;

	void GetWorldLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const;

	/**
	 * Get the location of this Frame relative to the given Scene Component.
	 */
	FVector GetLocationRelativeTo(const USceneComponent& Component) const;

	/**
	 * Same as Get Location Relative To, but use Fallback Parent if Parent is not set.
	 */
	FVector GetLocationRelativeTo(
		const USceneComponent& Component, const USceneComponent& FallbackParent) const;

	/**
	 * Get the location of this Frame relative to the given Scene Component.
	 *
	 * If Parent is not set then FallbackTransform is used as the parent transform instead.
	 */
	FVector GetLocationRelativeTo(
		const USceneComponent& Component, const FTransform& FallbackTransform) const;

	/**
	 * Get the rotation of this Frame relative to the given Scene Component.
	 */
	FRotator GetRotationRelativeTo(const USceneComponent& Component) const;

	/**
	 * Same as Get Rotation Relative To, but use Fallback Parent is Parent is not set.
	 */
	FRotator GetRotationRelativeTo(
		const USceneComponent& Component, const USceneComponent& FallbackParent) const;

	void GetRelativeTo(
		const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation) const;

	void GetRelativeTo(
		const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation,
		const USceneComponent& FallbackParent) const;

	const FTransform& GetParentTransform(const USceneComponent& FallbackParent) const;
	const FTransform& GetParentTransform(const FTransform& FallbackTransform) const;
};

UCLASS()
class AGXUNREAL_API UAGX_Frame_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "AGX Frame|Parent")
	static UPARAM(Ref) FAGX_Frame& SetName(UPARAM(Ref) FAGX_Frame& Frame, FName Name)
	{
		Frame.Parent.Name = Name;
		return Frame;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame|Parent")
	static FName GetName(UPARAM(Ref) FAGX_Frame& Frame)
	{
		return Frame.Parent.Name;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame")
	static USceneComponent* GetParentComponent(const FAGX_Frame& Frame)
	{
		return Frame.GetParentComponent();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame")
	static FVector GetWorldLocation(const FAGX_Frame& Frame)
	{
		return Frame.GetWorldLocation();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame")
	static FRotator GetWorldRotation(const FAGX_Frame& Frame)
	{
		return Frame.GetWorldRotation();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame")
	void GetWorldLocationAndRotation(
		const FAGX_Frame& Frame, FVector& OutLocation, FRotator& OutRotation)
	{
		Frame.GetWorldLocationAndRotation(OutLocation, OutRotation);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame")
	FVector GetLocationRelativeTo(const FAGX_Frame& Frame, USceneComponent* Component)
	{
		if (Component == nullptr)
		{
			return Frame.GetWorldLocation();
		}
		return Frame.GetLocationRelativeTo(*Component);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame")
	FRotator GetRotationRelativeTo(const FAGX_Frame& Frame, USceneComponent* Component)
	{
		if (Component == nullptr)
		{
			return Frame.GetWorldRotation();
		}
		return Frame.GetRotationRelativeTo(*Component);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Frame")
	void GetRelativeTo(
		const FAGX_Frame& Frame, USceneComponent* Component, FVector& OutLocation,
		FRotator& OutRotation)
	{
		if (Component == nullptr)
		{
			OutLocation = Frame.GetWorldLocation();
			OutRotation = Frame.GetWorldRotation();
			return;
		}
		Frame.GetRelativeTo(*Component, OutLocation, OutRotation);
	}
};
