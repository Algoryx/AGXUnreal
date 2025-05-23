// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/TrimeshShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#include "AGX_TrimeshShapeComponent.generated.h"

/**
 * Uses triangle data from a Static Mesh to generate an AGX Triangle Collision Mesh.
 *
 * The Static Mesh source can be either the parent or child Static Mesh Component,
 * or a specific Static Mesh Asset.
 */
UCLASS(
	ClassGroup = "AGX_Shape", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = (HLOD, Lighting, LOD, Materials, MaterialParameters, Rendering))
class AGXUNREAL_API UAGX_TrimeshShapeComponent final : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_TrimeshShapeComponent();

	/**
	 * Specifies from where should the Static Mesh triangle data be read.
	 *
	 * Only used during initialization, changing this value after Begin Play has no effect.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape", Meta=(ExposeOnSpawn))
	TEnumAsByte<EAGX_StaticMeshSourceLocation> MeshSourceLocation;

	/**
	 * Only used if Mesh Source Location is set to Static Mesh Asset. Specifies
	 * which Static Mesh Asset to use.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Shape",
		Meta =
			(EditCondition =
				"MeshSourceLocation == EAGX_StaticMeshSourceLocation::TSL_STATIC_MESH_ASSET",
			ExposeOnSpawn))
	UStaticMesh* MeshSourceAsset;

	/**
	 * Whether to explicitly set LOD Level to read triangle data from here
	 * or to use the setting that already exists on the Static Mesh source.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape", AdvancedDisplay)
	bool bOverrideMeshSourceLodIndex;

	/**
	 * Only used if Override Mesh Source LOD Index is enabled. Specifies which LOD Level
	 * of the Static Mesh source to read triangle data from. Zero is the most detailed level.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Shape", AdvancedDisplay,
		Meta = (EditCondition = "bOverrideMeshSourceLodIndex"))
	uint32 MeshSourceLodIndex;

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	/// Get the native AGX Dynamics representation of this Trimesh. May return nullptr.
	FTrimeshShapeBarrier* GetNativeTrimesh();

	/**
	 * Copy properties from the given AGX Dynamics trimesh into this component.
	 * @param Barrier The AGX Dynamics trimesh to copy from.
	 */
	virtual void CopyFrom(const FShapeBarrier& Barrier, FAGX_ImportContext* Context) override;

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditChange(FProperty* PropertyThatWillChange) override;
	virtual bool CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
		const UProperty* InProperty
#else
		const FProperty* InProperty
#endif
	) const override;
#endif
	// ~End UObject interface.

protected:
	// ~Begin UAGX_ShapeComponent interface.
	virtual FShapeBarrier* GetNativeBarrier() override;
	virtual const FShapeBarrier* GetNativeBarrier() const override;
	virtual void ReleaseNative() override;
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;
#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const override;
#endif
	// ~End UAGX_ShapeComponent interface.

private:
	/// Create the AGX Dynamics object owned by this Trimesh Shape Component.
	void CreateNative();

	bool GetStaticMeshCollisionData(
		TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices) const;

	UMeshComponent* FindMeshComponent(
		TEnumAsByte<EAGX_StaticMeshSourceLocation> MeshSourceLocation) const;

private:
	FTrimeshShapeBarrier NativeBarrier;
};
