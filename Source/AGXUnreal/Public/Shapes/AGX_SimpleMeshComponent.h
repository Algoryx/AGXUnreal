// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_UE4Compatibility.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"

#include "AGX_SimpleMeshComponent.generated.h"

class FPrimitiveSceneProxy;

struct AGXUNREAL_API FAGX_SimpleMeshData
{
	TArray<FVector3f> Vertices; // mandatory
	TArray<FVector2f> TexCoords;
	TArray<FVector3f> Normals; // mandatory
	TArray<FVector3f> Tangents;
	TArray<uint32> Indices;
};

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(
	hidecategories = (Object, LOD, Physics, Collision), Abstract, editinlinenew,
	Meta = (BlueprintSpawnableComponent), ClassGroup = Rendering)
class AGXUNREAL_API UAGX_SimpleMeshComponent : public UMeshComponent
{
	GENERATED_UCLASS_BODY()

	/**
	 * Set the geometry to use on this triangle mesh.
	 * Vertex positions and normals are mandatory!
	 */
	bool SetMeshData(const TSharedPtr<FAGX_SimpleMeshData>& Data);

	const FAGX_SimpleMeshData* GetMeshData() const;

	/** Removes all geometry from this triangle mesh. */
	void ClearMeshData();

	virtual bool LineTraceMesh(FHitResult& OutHit, FVector Start, FVector Stop);

	static bool LineTraceMeshes(
		FHitResult& OutHit, FVector Start, FVector Stop,
		const TArray<UAGX_SimpleMeshComponent*>& SimpleMeshComponents);

protected:
	TSharedPtr<FAGX_SimpleMeshData> MeshData;

private:
	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	//~ Begin UMeshComponent Interface.
	virtual int32 GetNumMaterials() const override;
	//~ End UMeshComponent Interface.

	//~ Begin USceneComponent Interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;
	//~ Begin USceneComponent Interface.

	friend class FAGX_SimpleMeshSceneProxy;
};
