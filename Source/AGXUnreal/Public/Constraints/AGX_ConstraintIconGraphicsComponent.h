// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeshComponent.h"
#include "AGX_ConstraintIconGraphicsComponent.generated.h"

class AAGX_Constraint;
class FPrimitiveSceneProxy;


/**
 * A component that shows a dynamic constraint-specific selectable 'icon' in the level editor viewport.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", NotPlaceable)
class AGXUNREAL_API UAGX_ConstraintIconGraphicsComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

public:

	UPROPERTY()
	AAGX_Constraint* Constraint;
	
	UMaterialInterface* GetOuterShellMaterial() const;
	UMaterialInterface* GetInnerShellMaterial() const;
	UMaterialInterface* GetInnerDisksMaterial() const;

	void OnBecameSelected();

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual int32 GetNumMaterials() const override;
	virtual UMaterialInterface* GetMaterial(int32 ElementIndex) const override;
	virtual void GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials = false) const override;
	FMatrix GetRenderMatrix() const;
	//~ End UPrimitiveComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ End USceneComponent Interface.

	//~ Begin UActorComponent Interface.
	virtual void SendRenderDynamicData_Concurrent() override;
	virtual bool ShouldCollideWhenPlacing() const { return true; }
	//~ End UActorComponent Interface.

private:

	int32 OuterShellMaterialIndex;
	int32 InnerShellMaterialIndex;
	int32 InnerDisksMaterialIndex;

};

