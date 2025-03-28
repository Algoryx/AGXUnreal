// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_AutoFitShape.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/CylinderShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CylinderShapeComponent.generated.h"

class AStaticMeshActor;

/**
 * A cylindrical collision shape. Can be used to give a Rigid Body a collision shape by
 * attaching the Cylinder as a child to the Rigid Body.
 *
 * It is common that wires are routed around cylinders. To make such constructions more stable
 * one can mark the cylinder as being either a Pulley or a Gypsy. This simulates a groove along
 * the Cylinder's perimeter that the wire can't slide off of.
 */
UCLASS(ClassGroup = "AGX_Shape", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CylinderShapeComponent final : public UAGX_ShapeComponent,
														public AGX_AutoFitShape
{
	GENERATED_BODY()

public:
	UAGX_CylinderShapeComponent();

	/**
	 * The distance from the the surface of one of its end disks to the other [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Height;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetHeight(float InHeight);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetHeight() const;

	/**
	 * The distance from the center of the cylinder to the cylindrical surface [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape")
	float Radius;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetRadius(float InRadius);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	float GetRadius() const;

	/**
	 * Set to true to enable the Pulley property on this cylinder.
	 *
	 * When enabled contact points with wires will only be created on the center line of the
	 * cylinder perimeter, preventing the wire from slipping off the cylinder.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Wire", Meta = (EditCondition = "!bGypsy"))
	bool bPulley = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetPulley(bool bInPulley);

	/**
	 * Set to true to enable the Gypsy property on this cylinder.
	 *
	 * When enabled contact points with wires will only be created on the center line of the
	 * cylinder perimeter, preventing the wire from slipping off of the Cylinder.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Wire", Meta = (EditCondition = "!bPulley"))
	bool bGypsy = false;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetGypsy(bool bInGypsy);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Auto-fit")
	static UAGX_CylinderShapeComponent* CreateFromMeshActors(
		AActor* Parent, TArray<AStaticMeshActor*> Meshes);

	// ~Begin UAGX_ShapeComponent interface.
	FShapeBarrier* GetNative() override;
	const FShapeBarrier* GetNative() const override;
	FShapeBarrier* GetOrCreateNative() override;
	virtual void UpdateNativeProperties() override;
	// ~End UAGX_ShapeComponent interface.

	// ~Begin UActorComponent interface.
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	// ~End UActorComponent interface.

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event);
	// ~End UObject interface.
#endif

	// ~Begin AGX_AutoFitShape interface.
	virtual bool AutoFitFromVertices(const TArray<FVector>& Vertices) override;
	// ~End AGX_AutoFitShape interface.

	/// Get the native AGX Dynamics representation of this Cylinder. May return nullptr.
	FCylinderShapeBarrier* GetNativeCylinder();

	/**
	 * Copy properties from the given AGX Dynamics cylinder into this component.
	 * Will also copy properties inherited from UAGX_ShapeComponent.
	 * @param Barrier The AGX Dynamics cylinder to copy from.
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
	/// Create the AGX Dynamics object owned by this Cylinder Shape Component.
	void CreateNative();

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	FCylinderShapeBarrier NativeBarrier;
};
