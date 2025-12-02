// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Enums.h"

// Standard library includes.
#include <memory>

struct FAGX_ROS2Message;
struct FAGX_ROS2Qos;
struct FROS2Subscriber;

class AGXUNREALBARRIER_API FROS2SubscriberBarrier
{
public:
	FROS2SubscriberBarrier();
	~FROS2SubscriberBarrier();
	FROS2SubscriberBarrier(FROS2SubscriberBarrier&& Other) noexcept;
	FROS2SubscriberBarrier& operator=(FROS2SubscriberBarrier&& Other) noexcept;

	bool HasNative() const;

	void AllocateNative(
		EAGX_ROS2MessageType InMessageType, const FString& Topic, const FAGX_ROS2Qos& Qos,
		uint8 DomainID = 0);

	FROS2Subscriber* GetNative();
	const FROS2Subscriber* GetNative() const;

	void ReleaseNative();

	bool ReceiveMessage(FAGX_ROS2Message& OutMsg) const;

	EAGX_ROS2MessageType GetMessageType() const;

private:
	FROS2SubscriberBarrier(const FROS2SubscriberBarrier&) = delete;
	void operator=(const FROS2SubscriberBarrier&) = delete;

private:
	std::unique_ptr<FROS2Subscriber> Native;
	EAGX_ROS2MessageType MessageType {EAGX_ROS2MessageType::Invalid};
};
