// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_AutoFitShape.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/CapsuleShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CapsuleShapeComponent.generated.h"

class AStaticMeshActor;

UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CapsuleShapeComponent final : public UAGX_ShapeComponent,
													   public AGX_AutoFitShape
{
	GENERATED_BODY()

public:
	UAGX_CapsuleShapeComponent();

	/**
	 * The distance from the centers of the capsule's half-spheres at each end [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Height;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetHeight(float InHeight);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetHeight() const;

	/**
	 * The distance from the center of any the two half-spheres at each end to their surface [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Radius;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetRadius(float InRadius);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetRadius() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Auto-fit")
	static UAGX_CapsuleShapeComponent* CreateFromMeshActors(
		AActor* Parent, TArray<AStaticMeshActor*> Meshes);

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(
		struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif
	// ~End UObject interface.

	// ~Begin AGX_AutoFitShape interface.
	virtual bool AutoFitFromVertices(const TArray<FVector>& Vertices) override;
	// ~End AGX_AutoFitShape interface.

	/// Get the native AGX Dynamics representation of this Capsule. May return nullptr.
	FCapsuleShapeBarrier* GetNativeCapsule();

	/**
	 * Copy properties from the given AGX Dynamics Capsule into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics Capsule to copy from.
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
	/// Create the AGX Dynamics object owned by this Capsule Shape Component.
	void CreateNative();

private:
	FCapsuleShapeBarrier NativeBarrier;
};
