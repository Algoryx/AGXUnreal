// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/ROS2AnyMessageParserBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

struct FAGX_AgxMsgsAny;

#include "AGX_ROS2AnyMessageParserComponent.generated.h"

/**
 * Helper class for serializing custom data types into an agxMsg::Any message that canb e sent via
 * ROS2. The agxMsgs::Any message can then be de-serialized at the receiving side usging the
 * AnyMessageParser.
 */
UCLASS(
	ClassGroup = "AGX_ROS2", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2AnyMessageParserComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2AnyMessageParserComponent();

	bool HasNative() const;

	FROS2AnyMessageParserBarrier* GetNative();
	const FROS2AnyMessageParserBarrier* GetNative() const;

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

	/**
	 * Must be called once each time a new message is to be parsed, before any of the Read
	 * functions.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	UAGX_ROS2AnyMessageParserComponent* BeginParse(const FAGX_AgxMsgsAny& Message);

	/** The result is cast to int32 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int32 ReadInt8();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	uint8 ReadUInt8();

	/** The result is cast to int32 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int32 ReadInt16();

	/** The result is cast to int32 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int32 ReadUInt16();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int32 ReadInt32();

	/** The result is cast to int64 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int64 ReadUInt32();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int64 ReadInt64();

	/** The result is cast to int64 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	int64 ReadUInt64();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	float ReadFloat32();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	double ReadDouble64();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	FString ReadString();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	bool ReadBool();

	/** The result is cast to int32 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<int32> ReadInt8Sequence();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<uint8> ReadUInt8Sequence();

	/** The result is cast to int32 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<int32> ReadInt16Sequence();

	/** The result is cast to int32 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<int32> ReadUInt16Sequence();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<int32> ReadInt32Sequence();

	/** The result is cast to int64 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<int64> ReadUInt32Sequence();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<int64> ReadInt64Sequence();

	/** The result is cast to int64 internally. */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<int64> ReadUInt64Sequence();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<float> ReadFloat32Sequence();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<double> ReadDouble64Sequence();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<FString> ReadStringSequence();

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2 Any Message")
	TArray<bool> ReadBoolSequence();

private:
	FROS2AnyMessageParserBarrier NativeBarrier;
};
