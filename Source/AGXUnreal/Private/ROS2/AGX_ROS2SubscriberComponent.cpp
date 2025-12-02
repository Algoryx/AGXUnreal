// Copyright 2025, Algoryx Simulation AB.

#include "ROS2/AGX_ROS2SubscriberComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_ROS2SubscriberComponent::UAGX_ROS2SubscriberComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//
// AgxMsgs
//

bool UAGX_ROS2SubscriberComponent::ReceiveAgxMsgsAny(
	FAGX_AgxMsgsAny& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::AgxMsgsAny, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveAgxMsgsAnySequence(
	FAGX_AgxMsgsAnySequence& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::AgxMsgsAnySequence, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

//
// BuiltinInterfaces
//

bool UAGX_ROS2SubscriberComponent::ReceiveBuiltinInterfacesTime(
	FAGX_BuiltinInterfacesTime& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::BuiltinInterfacesTime, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveBuiltinInterfacesDuration(
	FAGX_BuiltinInterfacesDuration& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::BuiltinInterfacesDuration, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

//
// RosgraphMsgs
//

bool UAGX_ROS2SubscriberComponent::ReceiveRosgraphMsgsClock(
	FAGX_RosgraphMsgsClock& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::RosgraphMsgsClock, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

//
// StdMsgs
//

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsBool(
	FAGX_StdMsgsBool& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsBool, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsByte(
	FAGX_StdMsgsByte& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsByte, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsByteMultiArray(
	FAGX_StdMsgsByteMultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsByteMultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsChar(
	FAGX_StdMsgsChar& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsChar, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsColorRGBA(
	FAGX_StdMsgsColorRGBA& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsColorRGBA, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsEmpty(
	FAGX_StdMsgsEmpty& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsEmpty, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsFloat32(
	FAGX_StdMsgsFloat32& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat32, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsFloat32MultiArray(
	FAGX_StdMsgsFloat32MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat32MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsFloat64(
	FAGX_StdMsgsFloat64& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat64, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsFloat64MultiArray(
	FAGX_StdMsgsFloat64MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat64MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt16(
	FAGX_StdMsgsInt16& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt16, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt16MultiArray(
	FAGX_StdMsgsInt16MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt16MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt32(
	FAGX_StdMsgsInt32& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt32, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt32MultiArray(
	FAGX_StdMsgsInt32MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt32MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt64(
	FAGX_StdMsgsInt64& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt64, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt64MultiArray(
	FAGX_StdMsgsInt64MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt64MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt8(
	FAGX_StdMsgsInt8& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt8, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsInt8MultiArray(
	FAGX_StdMsgsInt8MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt8MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsString(
	FAGX_StdMsgsString& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsString, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt16(
	FAGX_StdMsgsUInt16& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt16, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt16MultiArray(
	FAGX_StdMsgsUInt16MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt16MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt32(
	FAGX_StdMsgsUInt32& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt32, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt32MultiArray(
	FAGX_StdMsgsUInt32MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt32MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt64(
	FAGX_StdMsgsUInt64& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt64, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt64MultiArray(
	FAGX_StdMsgsUInt64MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt64MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt8(
	FAGX_StdMsgsUInt8& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt8, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsUInt8MultiArray(
	FAGX_StdMsgsUInt8MultiArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt8MultiArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveStdMsgsHeader(
	FAGX_StdMsgsHeader& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsHeader, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

//
// GeometryMsgs
//

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsVector3(
	FAGX_GeometryMsgsVector3& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsVector3, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsQuaternion(
	FAGX_GeometryMsgsQuaternion& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsQuaternion, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsAccel(
	FAGX_GeometryMsgsAccel& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccel, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsAccelStamped(
	FAGX_GeometryMsgsAccelStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccelStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsAccelWithCovariance(
	FAGX_GeometryMsgsAccelWithCovariance& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccelWithCovariance, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsAccelWithCovarianceStamped(
	FAGX_GeometryMsgsAccelWithCovarianceStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccelWithCovarianceStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsInertia(
	FAGX_GeometryMsgsInertia& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsInertia, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsInertiaStamped(
	FAGX_GeometryMsgsInertiaStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsInertiaStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPoint(
	FAGX_GeometryMsgsPoint& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoint, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPoint32(
	FAGX_GeometryMsgsPoint32& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoint32, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPointStamped(
	FAGX_GeometryMsgsPointStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPointStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPolygon(
	FAGX_GeometryMsgsPolygon& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPolygon, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPolygonStamped(
	FAGX_GeometryMsgsPolygonStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPolygonStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPose(
	FAGX_GeometryMsgsPose& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPose, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPose2D(
	FAGX_GeometryMsgsPose2D& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPose2D, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPoseArray(
	FAGX_GeometryMsgsPoseArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPoseStamped(
	FAGX_GeometryMsgsPoseStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPoseWithCovariance(
	FAGX_GeometryMsgsPoseWithCovariance& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseWithCovariance, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsPoseWithCovarianceStamped(
	FAGX_GeometryMsgsPoseWithCovarianceStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseWithCovarianceStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsQuaternionStamped(
	FAGX_GeometryMsgsQuaternionStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsQuaternionStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsTransform(
	FAGX_GeometryMsgsTransform& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTransform, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsTransformStamped(
	FAGX_GeometryMsgsTransformStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTransformStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsTwist(
	FAGX_GeometryMsgsTwist& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwist, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsTwistStamped(
	FAGX_GeometryMsgsTwistStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwistStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsTwistWithCovariance(
	FAGX_GeometryMsgsTwistWithCovariance& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwistWithCovariance, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsTwistWithCovarianceStamped(
	FAGX_GeometryMsgsTwistWithCovarianceStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwistWithCovarianceStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsVector3Stamped(
	FAGX_GeometryMsgsVector3Stamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsVector3Stamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsWrench(
	FAGX_GeometryMsgsWrench& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsWrench, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveGeometryMsgsWrenchStamped(
	FAGX_GeometryMsgsWrenchStamped& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsWrenchStamped, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

//
// SensorMsgs
//

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsBatteryState(
	FAGX_SensorMsgsBatteryState& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsBatteryState, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsChannelFloat32(
	FAGX_SensorMsgsChannelFloat32& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsChannelFloat32, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsCompressedImage(
	FAGX_SensorMsgsCompressedImage& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsCompressedImage, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsFluidPressure(
	FAGX_SensorMsgsFluidPressure& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsFluidPressure, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsIlluminance(
	FAGX_SensorMsgsIlluminance& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsIlluminance, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsImage(
	FAGX_SensorMsgsImage& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsImage, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsImu(
	FAGX_SensorMsgsImu& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsImu, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsJointState(
	FAGX_SensorMsgsJointState& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJointState, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsJoy(
	FAGX_SensorMsgsJoy& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJoy, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsJoyFeedback(
	FAGX_SensorMsgsJoyFeedback& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJoyFeedback, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsJoyFeedbackArray(
	FAGX_SensorMsgsJoyFeedbackArray& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJoyFeedbackArray, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsLaserEcho(
	FAGX_SensorMsgsLaserEcho& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsLaserEcho, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsLaserScan(
	FAGX_SensorMsgsLaserScan& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsLaserScan, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsMagneticField(
	FAGX_SensorMsgsMagneticField& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsMagneticField, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsMultiDOFJointState(
	FAGX_SensorMsgsMultiDOFJointState& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsMultiDOFJointState, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsMultiEchoLaserScan(
	FAGX_SensorMsgsMultiEchoLaserScan& OutMessage, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsMultiEchoLaserScan, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsNavSatStatus(
	FAGX_SensorMsgsNavSatStatus& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsNavSatStatus, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsNavSatFix(
	FAGX_SensorMsgsNavSatFix& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsNavSatFix, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsPointCloud(
	FAGX_SensorMsgsPointCloud& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsPointCloud, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsPointField(
	FAGX_SensorMsgsPointField& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsPointField, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsPointCloud2(
	FAGX_SensorMsgsPointCloud2& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsPointCloud2, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsRange(
	FAGX_SensorMsgsRange& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsRange, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsRegionOfInterest(
	FAGX_SensorMsgsRegionOfInterest& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsRegionOfInterest, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsCameraInfo(
	FAGX_SensorMsgsCameraInfo& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsCameraInfo, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsRelativeHumidity(
	FAGX_SensorMsgsRelativeHumidity& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsRelativeHumidity, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsTemperature(
	FAGX_SensorMsgsTemperature& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsTemperature, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

bool UAGX_ROS2SubscriberComponent::ReceiveSensorMsgsTimeReference(
	FAGX_SensorMsgsTimeReference& OutMessage, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsTimeReference, Topic))
		return Barrier->ReceiveMessage(OutMessage);
	return false;
}

FROS2SubscriberBarrier* UAGX_ROS2SubscriberComponent::GetOrCreateBarrier(
	EAGX_ROS2MessageType Type, const FString& Topic)
{
	FROS2SubscriberBarrier* Barrier = NativeBarriers.Find(Topic);
	if (Barrier == nullptr)
	{
		if (Topic.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"GetOrCreateBarrier was called on ROS2 Subscriber Component '%s' in Actor '%s' "
					"with an empty Topic String. Ensure a Topic has been set."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		bool bIsPlaying = GetWorld() != nullptr && GetWorld()->IsGameWorld();
		if (!bIsPlaying)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"GetOrCreateBarrier was called on ROS2 Subscriber Component '%s' in Actor '%s' "
					"when not in Play. Only call this function during Play."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		Barrier = &NativeBarriers.Add(Topic, FROS2SubscriberBarrier());
		Barrier->AllocateNative(Type, Topic, Qos);
	}
	else if (Barrier->GetMessageType() != Type)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Existing Native ROS2 Subscriber with different message type found in "
				 "UAGX_ROS2Subscriber::GetOrCreateBarrier for Topic: '%s', Publisher Compoent '%s' "
				 "in Actor '%s'. Ensure only single message types are used for a specific Topic."),
			*Topic, *GetName(), *GetLabelSafe(GetOwner()));
		return nullptr;
	}

	AGX_CHECK(Barrier->HasNative());
	return Barrier;
}

#if WITH_EDITOR
bool UAGX_ROS2SubscriberComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty->GetFName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_ROS2SubscriberComponent, Qos)) ||
		InProperty->GetFName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_ROS2SubscriberComponent, DomainID)))
	{
		UWorld* World = GetWorld();
		return World == nullptr || !World->IsGameWorld();
	}

	return SuperCanEditChange;
}
#endif
