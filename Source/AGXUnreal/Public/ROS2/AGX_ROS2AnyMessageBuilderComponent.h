// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/ROS2AnyMessageBuilderBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

struct FAGX_AgxMsgsAny;

#include "AGX_ROS2AnyMessageBuilderComponent.generated.h"

/**
 * Helper class for serializing custom data types into an agxMsg::Any message that canb e sent via
 * ROS2. The agxMsgs::Any message can then be de-serialized at the receiving side usging the
 * AnyMessageParser.
 */
UCLASS(
	ClassGroup = "AGX_ROS2", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2AnyMessageBuilderComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2AnyMessageBuilderComponent();

	bool HasNative() const;

	FROS2AnyMessageBuilderBarrier* GetNative();
	const FROS2AnyMessageBuilderBarrier* GetNative() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

	/**
	 * Must be called once each time a new message is to be built, before any of the Write
	 * functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* BeginMessage();

	/** The input is cast to int8_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt8(int32 Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt8(uint8 Data);

	/** The input is cast to int16_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt16(int32 Data);

	/** The input is cast to uint16_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt16(int32 Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt32(int32 Data);

	/** The input is cast to uint32_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt32(int64 Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt64(int64 Data);

	/** The input is cast to uint64_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt64(int64 Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteFloat32(float Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteDouble64(double Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteString(const FString& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteBool(bool Data);

	/** The input is cast to int8_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt8Sequence(const TArray<int32>& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt8Sequence(const TArray<uint8>& Data);

	/** The input is cast to int16_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt16Sequence(const TArray<int32>& Data);

	/** The input is cast to uint16_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt16Sequence(const TArray<int32>& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt32Sequence(const TArray<int32>& Data);

	/** The input is cast to uint32_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt32Sequence(const TArray<int64>& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteInt64Sequence(const TArray<int64>& Data);

	/** The input is cast to uint64_t internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteUInt64Sequence(const TArray<int64>& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteFloat32Sequence(const TArray<float>& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteDouble64Sequence(const TArray<double>& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteStringSequence(const TArray<FString>& Data);

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageBuilderComponent* WriteBoolSequence(const TArray<bool>& Data);

	/**
	 * Returns the message that has been built by this Component.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	FAGX_AgxMsgsAny GetMessage() const;

private:
	FROS2AnyMessageBuilderBarrier NativeBarrier;
};
