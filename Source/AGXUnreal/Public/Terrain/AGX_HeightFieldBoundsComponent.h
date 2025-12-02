// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_HeightFieldBoundsComponent.generated.h"

class ALandscape;

UCLASS(ClassGroup = "AGX", Category = "AGX", NotPlaceable)
class AGXUNREAL_API UAGX_HeightFieldBoundsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	struct FHeightFieldBoundsInfo
	{
		FTransform Transform;
		FVector HalfExtent;
	};

	UAGX_HeightFieldBoundsComponent();

	/**
	 * The distance from the center of the Height Field to its edges [cm].
	 * The Z component has no effect on the Simulation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Height Field Bounds",
		Meta = (EditCondition = "!bInfiniteBounds"))
	FVector HalfExtent {2000.0, 2000.0, 100.0};

	/**
	 * If set to true, the Height Field Bounds will be as large as any selected Landscape.
	 * Notice that using smaller bounds may increase runtime performance.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Height Field Bounds")
	bool bInfiniteBounds = false;

	/**
	 * Get Bounds as defined by the user, i.e. using exactly the HalfExtent UPROPERTY.
	 * Only valid if the owner of this Component is an AGX_Terrain or AGX_HeightFieldShape and a
	 * SourceLandscape is set in that owner.
	 */
	TOptional<FHeightFieldBoundsInfo> GetUserSetBounds() const;

	/**
	 * Get Bounds that are adjusted to align with the Landscape quads.
	 * Each edge of this Bound will align with the nearest Landscape grid line inside the
	 * UserSetBounds. Only valid if the owner of this Component is an AGX_Terrain or
	 * AGX_HeightFieldShape and a SourceLandscape is set in that owner.
	 */
	TOptional<FHeightFieldBoundsInfo> GetLandscapeAdjustedBounds() const;

private:
	struct FTransformAndLandscape
	{
		FTransformAndLandscape(const ALandscape& InLandscape, const FTransform& InTransform)
			: Landscape(InLandscape)
			, Transform(InTransform)
		{
		}
		const ALandscape& Landscape;
		FTransform Transform;
	};

	/**
	 * Returns the transform of the Owner of this Component and any Landscape owned by that owner if
	 * possible. Only Terrain and HeightField owners are currently supported.
	 */
	TOptional<FTransformAndLandscape> GetLandscapeAndTransformFromOwner() const;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
};
