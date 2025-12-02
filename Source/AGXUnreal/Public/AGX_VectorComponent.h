// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Components/PrimitiveComponent.h"
#include "Math/Color.h"
#include "Math/TwoVectors.h"

#include "AGX_VectorComponent.generated.h"

/**
 * A VectorComponent is used to represent a direction or an edge in the world.
 * It is used by the terrain shovel to specify cutting edges and such. It comes
 * with editor integration for easier setup.
 *
 * This class is heavily influenced by the Unreal Engine ArrowComponent.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API UAGX_VectorComponent : public UPrimitiveComponent
{
	GENERATED_BODY()
public:
	/**
	 * The length of the arrow [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Vector")
	float ArrowSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Vector")
	FColor ArrowColor;

	/**
	 * Returns the world-space direction (i.e. x-axis) of this component,
	 * scaled by the size and transform scale.
	 */
	FVector GetVectorDirection() const;

	/**
	 * Returns the normalized world-space direction (i.e. x-axis) of this component.
	 */
	FVector GetVectorDirectionNormalized() const;

	/**
	 * Returns the world-space position.
	 */
	FVector GetVectorOrigin() const;

	/**
	 * Returns sum of world-space position and world space direction.
	 */
	FVector GetVectorTarget() const;

	/**
	 * Convert the world-space origin and target positions to two positions in
	 * the local space defined by the WorldToLocal transformation.
	 */
	FTwoVectors GetInLocal(FTransform const& WorldToLocal) const;

	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
};
