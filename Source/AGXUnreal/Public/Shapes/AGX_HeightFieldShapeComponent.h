// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/HeightFieldShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_HeightFieldShapeComponent.generated.h"

class ALandscape;
class UAGX_HeightFieldBoundsComponent;

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_HeightFieldShapeComponent : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_HeightFieldShapeComponent();

	UPROPERTY(Category = "AGX Shape", VisibleAnywhere)
	UAGX_HeightFieldBoundsComponent* HeightFieldBounds;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	ALandscape* SourceLandscape = nullptr;

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	/// Get the native AGX Dynamics representation of this HeightField. May return nullptr.
	FHeightFieldShapeBarrier* GetNativeHeightField();

	virtual void DestroyComponent(bool bPromoteChildren) override;

protected:
	// ~Begin UAGX_ShapeComponent interface.
	virtual FShapeBarrier* GetNativeBarrier() override;
	virtual const FShapeBarrier* GetNativeBarrier() const override;
	virtual void ReleaseNative() override;
	virtual void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;
	virtual void UpdateNativeGlobalTransform() override;
#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const override;
#endif
	// ~End UAGX_ShapeComponent interface.

private:
	/// Create the AGX Dynamics objects owned by the FBoxShapeBarrier.
	void CreateNative();
	virtual void Serialize(FArchive& Archive) override;

private:
	FHeightFieldShapeBarrier NativeBarrier;
};
