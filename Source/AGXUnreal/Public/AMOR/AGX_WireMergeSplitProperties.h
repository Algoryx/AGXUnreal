// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_MergeSplitPropertiesBase.h"
#include "AMOR/AGX_WireMergeSplitThresholds.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireMergeSplitProperties.generated.h"

class FWireBarrier;
class UAGX_WireComponent;
struct FAGX_ImportContext;

/*
 * Defines the AMOR (merge split) properties for Wires. For this to take affect, AMOR has to
 * be enabled globally in the AGX Dynamics for Unreal project settings.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_WireMergeSplitProperties : public FAGX_MergeSplitPropertiesBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "AGX AMOR")
	UAGX_WireMergeSplitThresholds* Thresholds = nullptr;

	/**
	 * Must be called by the owning object at begin play (after the owning object has allocated a
	 * native AGX Dynamics object).
	 */
	void OnBeginPlay(UAGX_WireComponent& Owner);

#if WITH_EDITOR
	/**
	 * Must be called by the owning object from PostEditChangeProperty or
	 * PostEditChangeChainProperty.
	 */
	void OnPostEditChangeProperty(UAGX_WireComponent& Owner);
#endif

	void CreateNative(UAGX_WireComponent& Owner);

	/*
	 * This struct is typically a member (UPROPERTY) of its owner, which is a
	 * UAGX_WireComponent.
	 * The barrier of this struct has a Native which is a pointer to a AGX
	 * Dynamics agxSDK::MergeSplitProperties. The owner of that pointer on the AGX Dynamics side is
	 * the agxWire::Wire pointed to by the owning UAGX_WireComponent's barrier's native.
	 *
	 * This function will update this struct's native to point to the agxSDK::MergeSplitProperties
	 * of the new Owner.
	 */
	void BindBarrierToOwner(FWireBarrier& NewOwner);

	virtual UAGX_MergeSplitThresholdsBase* GetThresholds() override;

	virtual void CopyFrom(
		const FMergeSplitPropertiesBarrier& Barrier, FAGX_ImportContext* Context) override;

private:
	void UpdateNativeProperties();
	void CreateNativeThresholds(UWorld* PlayingWorld);
	void UpdateNativeThresholds();
};

/**
 * This class acts as an API that exposes functions of FAGX_WireMergeSplitProperties in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_WireMergeSplitProperties_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static void SetEnableMerge(UPARAM(ref) FAGX_WireMergeSplitProperties& Properties, bool bEnable)
	{
		Properties.SetEnableMerge(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static bool GetEnableMerge(UPARAM(ref) const FAGX_WireMergeSplitProperties& Properties)
	{
		return Properties.GetEnableMerge();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static void SetEnableSplit(UPARAM(ref) FAGX_WireMergeSplitProperties& Properties, bool bEnable)
	{
		Properties.SetEnableSplit(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static bool GetEnableSplit(UPARAM(ref) const FAGX_WireMergeSplitProperties& Properties)
	{
		return Properties.GetEnableSplit();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	static UAGX_WireMergeSplitThresholds* GetThresholds(
		UPARAM(ref) const FAGX_WireMergeSplitProperties& Properties)
	{
		return Properties.Thresholds;
	}
};
