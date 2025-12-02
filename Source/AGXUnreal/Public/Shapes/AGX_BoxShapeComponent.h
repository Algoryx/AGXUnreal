// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_AutoFitShape.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/BoxShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BoxShapeComponent.generated.h"

class AStaticMeshActor;

/**
 *
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BoxShapeComponent final : public UAGX_ShapeComponent,
												   public AGX_AutoFitShape
{
	GENERATED_BODY()

public:
	UAGX_BoxShapeComponent();

	/**
	 * The distance from the center of the box to its surface along the three cardinal axes [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	FVector HalfExtent;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetHalfExtent(FVector InHalfExtent);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	FVector GetHalfExtent() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Auto-fit")
	static UAGX_BoxShapeComponent* CreateFromMeshActors(
		AActor* Parent, TArray<AStaticMeshActor*> Meshes);

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	// ~Begin AGX_AutoFitShape interface.
	virtual bool AutoFitFromVertices(const TArray<FVector>& Vertices) override;
	// ~End AGX_AutoFitShape interface.

	/// Get the native AGX Dynamics representation of this Box. May return nullptr.
	FBoxShapeBarrier* GetNativeBox();

	/**
	 * Copy properties from the given AGX Dynamics box into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics box to copy from.
	 */
	virtual void CopyFrom(const FShapeBarrier& Barrier, FAGX_ImportContext* Context) override;

protected:
	// ~Begin UAGX_ShapeComponent interface.
	virtual FShapeBarrier* GetNativeBarrier() override;
	virtual const FShapeBarrier* GetNativeBarrier() const override;
	virtual void ReleaseNative() override;
	void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData) override;
	virtual bool SupportsShapeBodySetup() override;
	virtual void UpdateBodySetup() override;
	virtual void AddShapeBodySetupGeometry() override;
#if WITH_EDITOR
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const override;
#endif
	// ~End UAGX_ShapeComponent interface.

private:
	/// Create the AGX Dynamics objects owned by this Box Shape Component.
	void CreateNative();

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	// ~End UObject interface.

	void InitPropertyDispatcher();
#endif

private:
	FBoxShapeBarrier NativeBarrier;
};
