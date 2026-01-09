// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Cable/AGX_CableNodeInfo.h"
#include "Cable/AGX_CableRouteNode.h"
#include "Cable/CableBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_CableComponent.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInterface;

/**
 * A frame attached to a RigidBody with an optional relative transform.
 * During runtime, it is possible to get its position, velocity, angular velocity, acceleration
 * etc.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CableComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_CableComponent();

	/**
	 * The radius of the Cable [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Cable",
		Meta = (ClampMin = "0", UIMin = "0"))
	double Radius {3.0};

	/**
	 * Scale to apply to the radius when rendering the Cable.
	 * By increasing the Rander Radius Scale it is possible to make the Cable larger on-screen
	 * without affecting the simulation behavior.
	 *
	 * This setting affects rendering only, it does not change the simulation behavior or collision
	 * shape of the Cable.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Cable")
	double RenderRadiusScale {1.0};

	/**
	 * The resolution of the Cable, i.e. the number of lumped elements per centimeter.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Cable",
		Meta = (ClampMin = "0", UIMin = "0"))
	double ResolutionPerUnitLength {0.01};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Cable")
	UMaterialInterface* RenderMaterial {nullptr};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetRenderMaterial(UMaterialInterface* Material);

	/**
	 * An array of route nodes that are used to initialize the Cable.
	 * At BeginPlay these nodes are used to create simulation nodes, after that these nodes are not
	 * used and will not be updated during Play.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Cable Route")
	TArray<FAGX_CableRouteNode> RouteNodes;

	/*
	 * The import name of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Name")
	FString ImportName;

	/**
	 * Returns an array of Node Info structs containing information about each Node in the Cable.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	TArray<FAGX_CableNodeInfo> GetNodeInfo() const;

	FCableBarrier* GetNative();
	const FCableBarrier* GetNative() const;

	FCableBarrier* GetOrCreateNative();

	//~ Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeObject interface.

	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent interface

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void OnRegister() override;
	// ~End UObject interface.
#endif

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
	bool DoesPropertyAffectVisuals(const FName& MemberPropertyName) const;
#endif
	void UpdateNativeProperties();
	void CreateNative();
	void CreateVisuals();
	bool ShouldRenderSelf() const;
	void UpdateVisuals();
	void SetVisualsInstanceCount(int32 NumCylinders, int32 NumSpheres);
	void RenderSelf();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	TArray<FTransform> VisualCylinderTransformsPrev;
	TArray<FTransform> VisualSphereTransformsPrev;

	TObjectPtr<UInstancedStaticMeshComponent> VisualCylinders;
	TObjectPtr<UInstancedStaticMeshComponent> VisualSpheres;
	FCableBarrier NativeBarrier;
};
