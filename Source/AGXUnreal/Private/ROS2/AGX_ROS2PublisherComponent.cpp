// Copyright 2025, Algoryx Simulation AB.

#include "ROS2/AGX_ROS2PublisherComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

UAGX_ROS2PublisherComponent::UAGX_ROS2PublisherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

//
// AgxMsgs
//

bool UAGX_ROS2PublisherComponent::SendAgxMsgsAny(const FAGX_AgxMsgsAny& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::AgxMsgsAny, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendAgxMsgsAnySequence(
	const FAGX_AgxMsgsAnySequence& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::AgxMsgsAnySequence, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

//
// BuiltinInterfaces
//

bool UAGX_ROS2PublisherComponent::SendBuiltinInterfacesTime(
	const FAGX_BuiltinInterfacesTime& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::BuiltinInterfacesTime, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendBuiltinInterfacesDuration(
	const FAGX_BuiltinInterfacesDuration& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::BuiltinInterfacesDuration, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

//
// RosgraphMsgs
//

bool UAGX_ROS2PublisherComponent::SendRosgraphMsgsClock(
	const FAGX_RosgraphMsgsClock& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::RosgraphMsgsClock, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

//
// StdMsgs
//

bool UAGX_ROS2PublisherComponent::SendStdMsgsBool(const FAGX_StdMsgsBool& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsBool, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsByte(const FAGX_StdMsgsByte& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsByte, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsByteMultiArray(
	const FAGX_StdMsgsByteMultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsByteMultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsChar(const FAGX_StdMsgsChar& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsChar, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsColorRGBA(
	const FAGX_StdMsgsColorRGBA& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsColorRGBA, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsEmpty(
	const FAGX_StdMsgsEmpty& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsEmpty, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsFloat32(
	const FAGX_StdMsgsFloat32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat32, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsFloat32MultiArray(
	const FAGX_StdMsgsFloat32MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat32MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsFloat64(
	const FAGX_StdMsgsFloat64& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat64, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsFloat64MultiArray(
	const FAGX_StdMsgsFloat64MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsFloat64MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt16(
	const FAGX_StdMsgsInt16& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt16, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt16MultiArray(
	const FAGX_StdMsgsInt16MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt16MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt32(
	const FAGX_StdMsgsInt32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt32, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt32MultiArray(
	const FAGX_StdMsgsInt32MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt32MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt64(
	const FAGX_StdMsgsInt64& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt64, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt64MultiArray(
	const FAGX_StdMsgsInt64MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt64MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt8(const FAGX_StdMsgsInt8& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt8, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsInt8MultiArray(
	const FAGX_StdMsgsInt8MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsInt8MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsString(
	const FAGX_StdMsgsString& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsString, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt16(
	const FAGX_StdMsgsUInt16& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt16, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt16MultiArray(
	const FAGX_StdMsgsUInt16MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt16MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt32(
	const FAGX_StdMsgsUInt32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt32, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt32MultiArray(
	const FAGX_StdMsgsUInt32MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt32MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt64(
	const FAGX_StdMsgsUInt64& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt64, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt64MultiArray(
	const FAGX_StdMsgsUInt64MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt64MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt8(
	const FAGX_StdMsgsUInt8& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt8, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsUInt8MultiArray(
	const FAGX_StdMsgsUInt8MultiArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsUInt8MultiArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendStdMsgsHeader(
	const FAGX_StdMsgsHeader& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::StdMsgsHeader, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

//
// GeometryMsgs
//

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsVector3(
	const FAGX_GeometryMsgsVector3& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsVector3, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsQuaternion(
	const FAGX_GeometryMsgsQuaternion& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsQuaternion, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsAccel(
	const FAGX_GeometryMsgsAccel& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccel, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsAccelStamped(
	const FAGX_GeometryMsgsAccelStamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccelStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsAccelWithCovariance(
	const FAGX_GeometryMsgsAccelWithCovariance& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccelWithCovariance, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsAccelWithCovarianceStamped(
	const FAGX_GeometryMsgsAccelWithCovarianceStamped& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsAccelWithCovarianceStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsInertia(
	const FAGX_GeometryMsgsInertia& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsInertia, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsInertiaStamped(
	const FAGX_GeometryMsgsInertiaStamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsInertiaStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPoint(
	const FAGX_GeometryMsgsPoint& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoint, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPoint32(
	const FAGX_GeometryMsgsPoint32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoint32, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPointStamped(
	const FAGX_GeometryMsgsPointStamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPointStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPolygon(
	const FAGX_GeometryMsgsPolygon& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPolygon, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPolygonStamped(
	const FAGX_GeometryMsgsPolygonStamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPolygonStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPose(
	const FAGX_GeometryMsgsPose& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPose, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPose2D(
	const FAGX_GeometryMsgsPose2D& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPose2D, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPoseArray(
	const FAGX_GeometryMsgsPoseArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPoseStamped(
	const FAGX_GeometryMsgsPoseStamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPoseWithCovariance(
	const FAGX_GeometryMsgsPoseWithCovariance& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseWithCovariance, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsPoseWithCovarianceStamped(
	const FAGX_GeometryMsgsPoseWithCovarianceStamped& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsPoseWithCovarianceStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsQuaternionStamped(
	const FAGX_GeometryMsgsQuaternionStamped& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsQuaternionStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsTransform(
	const FAGX_GeometryMsgsTransform& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTransform, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsTransformStamped(
	const FAGX_GeometryMsgsTransformStamped& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTransformStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsTwist(
	const FAGX_GeometryMsgsTwist& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwist, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsTwistStamped(
	const FAGX_GeometryMsgsTwistStamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwistStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsTwistWithCovariance(
	const FAGX_GeometryMsgsTwistWithCovariance& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwistWithCovariance, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsTwistWithCovarianceStamped(
	const FAGX_GeometryMsgsTwistWithCovarianceStamped& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsTwistWithCovarianceStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsVector3Stamped(
	const FAGX_GeometryMsgsVector3Stamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsVector3Stamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsWrench(
	const FAGX_GeometryMsgsWrench& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsWrench, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendGeometryMsgsWrenchStamped(
	const FAGX_GeometryMsgsWrenchStamped& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::GeometryMsgsWrenchStamped, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

//
// SensorMsgs
//

bool UAGX_ROS2PublisherComponent::SendSensorMsgsBatteryState(
	const FAGX_SensorMsgsBatteryState& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsBatteryState, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsChannelFloat32(
	const FAGX_SensorMsgsChannelFloat32& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsChannelFloat32, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsCompressedImage(
	const FAGX_SensorMsgsCompressedImage& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsCompressedImage, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsFluidPressure(
	const FAGX_SensorMsgsFluidPressure& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsFluidPressure, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsIlluminance(
	const FAGX_SensorMsgsIlluminance& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsIlluminance, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsImage(
	const FAGX_SensorMsgsImage& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsImage, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsImu(
	const FAGX_SensorMsgsImu& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsImu, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsJointState(
	const FAGX_SensorMsgsJointState& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJointState, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsJoy(
	const FAGX_SensorMsgsJoy& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJoy, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsJoyFeedback(
	const FAGX_SensorMsgsJoyFeedback& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJoyFeedback, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsJoyFeedbackArray(
	const FAGX_SensorMsgsJoyFeedbackArray& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsJoyFeedbackArray, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsLaserEcho(
	const FAGX_SensorMsgsLaserEcho& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsLaserEcho, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsLaserScan(
	const FAGX_SensorMsgsLaserScan& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsLaserScan, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsMagneticField(
	const FAGX_SensorMsgsMagneticField& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsMagneticField, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsMultiDOFJointState(
	const FAGX_SensorMsgsMultiDOFJointState& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsMultiDOFJointState, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsMultiEchoLaserScan(
	const FAGX_SensorMsgsMultiEchoLaserScan& Msg, const FString& Topic)
{
	if (auto Barrier =
			GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsMultiEchoLaserScan, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsNavSatStatus(
	const FAGX_SensorMsgsNavSatStatus& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsNavSatStatus, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsNavSatFix(
	const FAGX_SensorMsgsNavSatFix& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsNavSatFix, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsPointCloud(
	const FAGX_SensorMsgsPointCloud& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsPointCloud, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsPointField(
	const FAGX_SensorMsgsPointField& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsPointField, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsPointCloud2(
	const FAGX_SensorMsgsPointCloud2& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsPointCloud2, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsRange(
	const FAGX_SensorMsgsRange& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsRange, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsRegionOfInterest(
	const FAGX_SensorMsgsRegionOfInterest& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsRegionOfInterest, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsCameraInfo(
	const FAGX_SensorMsgsCameraInfo& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsCameraInfo, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsRelativeHumidity(
	const FAGX_SensorMsgsRelativeHumidity& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsRelativeHumidity, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsTemperature(
	const FAGX_SensorMsgsTemperature& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsTemperature, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

bool UAGX_ROS2PublisherComponent::SendSensorMsgsTimeReference(
	const FAGX_SensorMsgsTimeReference& Msg, const FString& Topic)
{
	if (auto Barrier = GetOrCreateBarrier(EAGX_ROS2MessageType::SensorMsgsTimeReference, Topic))
		return Barrier->SendMsg(Msg);
	return false;
}

FROS2PublisherBarrier* UAGX_ROS2PublisherComponent::GetOrCreateBarrier(
	EAGX_ROS2MessageType Type, const FString& Topic)
{
	FROS2PublisherBarrier* Barrier = NativeBarriers.Find(Topic);
	if (Barrier == nullptr)
	{
		if (Topic.IsEmpty())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateBarrier was called on ROS2 Publisher Component '%s' in Actor '%s' "
					 "whith an empty Topic String. Ensure a Topic has been set."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		bool bIsPlaying = GetWorld() != nullptr && GetWorld()->IsGameWorld();
		if (!bIsPlaying)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateBarrier was called on ROS2 Publisher Component '%s' in Actor '%s' "
					 "when not inPlay. Only call this function during Play."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		Barrier = &NativeBarriers.Add(Topic, FROS2PublisherBarrier());
		Barrier->AllocateNative(Type, Topic, Qos);
	}
	else if (Barrier->GetMessageType() != Type)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Existing Native ROS2 Publisher with different message type found in "
				 "UAGX_ROS2Publisher::GetOrCreateBarrier for Topic: '%s', Publisher Component '%s' "
				 "in Actor '%s'. Ensure only single message types are used for a specific Topic."),
			*Topic, *GetName(), *GetLabelSafe(GetOwner()));
		return nullptr;
	}

	AGX_CHECK(Barrier->HasNative());
	return Barrier;
}

#if WITH_EDITOR
bool UAGX_ROS2PublisherComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty->GetFName().IsEqual(GET_MEMBER_NAME_CHECKED(UAGX_ROS2PublisherComponent, Qos)) ||
		InProperty->GetFName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_ROS2PublisherComponent, DomainID)))
	{
		UWorld* World = GetWorld();
		return World == nullptr || !World->IsGameWorld();
	}

	return SuperCanEditChange;
}
#endif
