// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/SphereShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_SphereShapeComponent.generated.h"

UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_SphereShapeComponent final : public UAGX_ShapeComponent
{
	GENERATED_BODY()

public:
	UAGX_SphereShapeComponent();

	/**
	 * The distance from the center of the sphere to its surface [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Radius;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetRadius(float InRadius);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetRadius() const;

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

	FSphereShapeBarrier* GetNativeSphere();

	/**
	 * Copy properties from the given AGX Dynamics sphere into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 *
	 * @param Barrier The AGX Dynamics sphere to copy from.
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
	/// Create the AGX Dynamics object owned by this Sphere Shape Component.
	void CreateNative();

private:
	FSphereShapeBarrier NativeBarrier;
};
