// Copyright 2025, Algoryx Simulation AB.

/*
 * Deprecated: see internal issue 739.
 */

#pragma once

// AGX Dynamics for Unreal includes.
#include "Components/MeshComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintDofGraphicsComponent.generated.h"

class UAGX_ConstraintComponent;
class FPrimitiveSceneProxy;

/**
 * A component that visualizes the constraint's degrees of freedom directly in the viewport.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", NotPlaceable)
class AGXUNREAL_API UAGX_ConstraintDofGraphicsComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY()
	UAGX_ConstraintComponent* Constraint;

	UMaterialInterface* GetFreeTranslationMaterial() const;
	UMaterialInterface* GetFreeRotationMaterial() const;
	UMaterialInterface* GetLockedTranslationMaterial() const;
	UMaterialInterface* GetLockedRotationMaterial() const;
	UMaterialInterface* GetViolatedMaterial() const;

	void OnBecameSelected();

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override;
	virtual void GetUsedMaterials(
		TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	FMatrix GetRenderMatrix() const;
	//~ End UPrimitiveComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface.

	//~ Begin UActorComponent Interface.
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual bool ShouldCollideWhenPlacing() const
	{
		return true;
	}
	//~ End UActorComponent Interface.

	int32 AttachmentId;

private:
	int32 FreeTranslationMaterialIndex;
	int32 FreeRotationMaterialIndex;
	int32 LockedTranslationMaterialIndex;
	int32 LockedRotationMaterialIndex;
	int32 ViolatedMaterialIndex;
};
