// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_MergeSplitPropertiesBase.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ShapeContactMergeSplitProperties.generated.h"

class FRigidBodyBarrier;
class FShapeBarrier;
class UAGX_RigidBodyComponent;
class UAGX_ShapeComponent;

/*
 * Defines the AMOR (merge split) properties for Rigid Bodies and Shapes. For this to take affect,
 * AMOR has to be enabled globally in the AGX Dynamics for Unreal project settings.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ShapeContactMergeSplitProperties : public FAGX_MergeSplitPropertiesBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX AMOR")
	UAGX_ShapeContactMergeSplitThresholds* Thresholds = nullptr;

	/**
	 * Must be called by the owning object at begin play (after the owning object has allocated a
	 * native AGX Dynamics object).
	 */
	void OnBeginPlay(UAGX_RigidBodyComponent& Owner);
	void OnBeginPlay(UAGX_ShapeComponent& Owner);

#if WITH_EDITOR
	/**
	 * Must be called by the owning object from PostEditChangeProperty or
	 * PostEditChangeChainProperty.
	 */
	void OnPostEditChangeProperty(UAGX_RigidBodyComponent& Owner);
	void OnPostEditChangeProperty(UAGX_ShapeComponent& Owner);
#endif

	void CreateNative(UAGX_RigidBodyComponent& Owner);
	void CreateNative(UAGX_ShapeComponent& Owner);

	virtual void CopyFrom(
		const FMergeSplitPropertiesBarrier& Barrier, FAGX_AGXToUeContext* Context) override;

	/*
	 * This struct is typically a member (UPROPERTY) of its owner, which is a
	 * UAGX_RigidBodyComponent or UAGX_ShapeComponent.
	 * The barrier of this struct has a Native which is a pointer to a AGX
	 * Dynamics agxSDK::MergeSplitProperties. The owner of that pointer on the AGX Dynamics side is
	 * the agx::RigidBody or agxCollide::Geometry pointed to by the owning Body or Shape's barrier's
	 * native.
	 *
	 * This function will update this struct's native to point to the agxSDK::MergeSplitProperties
	 * of the new Owner.
	 */
	void BindBarrierToOwner(FRigidBodyBarrier& NewOwner);
	void BindBarrierToOwner(FShapeBarrier& NewOwner);

	virtual UAGX_MergeSplitThresholdsBase* GetThresholds() override;

private:
	void UpdateNativeProperties();
	void CreateNativeThresholds(UWorld* PlayingWorld);
	void UpdateNativeThresholds();

	template <typename T>
	void OnBeginPlayInternal(T& Owner);

	template <typename T>
	void OnPostEditChangePropertyInternal(T& Owner);

	template <typename T>
	void CreateNativeInternal(T& Owner);
};

/**
 * This class acts as an API that exposes functions of FAGX_ShapeContactMergeSplitProperties in
 * Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ShapeContactMergeSplitProperties_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static void SetEnableMerge(
		UPARAM(ref) FAGX_ShapeContactMergeSplitProperties& Properties, bool bEnable)
	{
		Properties.SetEnableMerge(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static bool GetEnableMerge(UPARAM(ref) const FAGX_ShapeContactMergeSplitProperties& Properties)
	{
		return Properties.GetEnableMerge();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static void SetEnableSplit(
		UPARAM(ref) FAGX_ShapeContactMergeSplitProperties& Properties, bool bEnable)
	{
		Properties.SetEnableSplit(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static bool GetEnableSplit(UPARAM(ref) const FAGX_ShapeContactMergeSplitProperties& Properties)
	{
		return Properties.GetEnableSplit();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static UAGX_ShapeContactMergeSplitThresholds* GetThresholds(
		UPARAM(ref) const FAGX_ShapeContactMergeSplitProperties& Properties)
	{
		return Properties.Thresholds;
	}
};
