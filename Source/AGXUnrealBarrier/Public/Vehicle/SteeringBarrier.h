// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard libarryincludes.
#include <memory>

#include "SteeringBarrier.generated.h"

class FWheelJointBarrier;

struct FAGX_SteeringParametersData;
struct FSteeringRef;

UENUM()
enum class EAGX_SteeringType
{
	Invalid,
	Ackermann,
	BellCrank,
	Davis,
	RackPinion
};

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FSteeringBarrier
{
	GENERATED_BODY()

	FSteeringBarrier();
	FSteeringBarrier(std::shared_ptr<FSteeringRef> Native);
	virtual ~FSteeringBarrier() = default;

	void AllocateAckermann(
		FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
		const FAGX_SteeringParametersData& Params);

	void AllocateBellCrank(
		FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
		const FAGX_SteeringParametersData& Params);

	void AllocateRackPinion(
		FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
		const FAGX_SteeringParametersData& Params);

	void AllocateDavis(
		FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
		const FAGX_SteeringParametersData& Params);

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetSteeringAngle(double Angle);
	double GetSteeringAngle() const;

	void SetName(const FString& NewName);
	FString GetName() const;

	FGuid GetGuid() const;

	EAGX_SteeringType GetType() const;

	bool HasNative() const;
	FSteeringRef* GetNative();
	const FSteeringRef* GetNative() const;

	void SetNativeAddress(uintptr_t NativeAddress);
	uintptr_t GetNativeAddress() const;

	void ReleaseNative();

private:
	std::shared_ptr<FSteeringRef> NativeRef;
};
