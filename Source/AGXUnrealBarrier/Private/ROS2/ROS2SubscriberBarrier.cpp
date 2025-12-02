// Copyright 2025, Algoryx Simulation AB.

#include "ROS2/ROS2SubscriberBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"
#include "TypeConversions.h"
#include "Utilities/ROS2Utilities.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxROS2/ROS2Util.h>
#include "EndAGXIncludes.h"

// Helper macros to minimize amount of code needed in large switch-statement.
#define AGX_RECEIVE_ROS2_MSGS(SubType, MsgTypeUnreal, MsgTypeROS2)                              \
	{                                                                                           \
		if (auto Sub = dynamic_cast<const SubType*>(Native.get()))                              \
		{                                                                                       \
			MsgTypeROS2 MsgAGX;                                                                 \
			if (Sub->Native->receiveMessage(MsgAGX))                                            \
			{                                                                                   \
				*static_cast<MsgTypeUnreal*>(&OutMsg) = Convert(MsgAGX);                        \
				agxROS2::freeContainerMemory(MsgAGX);                                           \
				return true;                                                                    \
			}                                                                                   \
		}                                                                                       \
		else                                                                                    \
		{                                                                                       \
			UE_LOG(                                                                             \
				LogAGX, Error,                                                                  \
				TEXT("Unexpected internal error: unable to downcast to the correct Subscriber " \
					 "type "                                                                    \
					 "in FROS2SubscriberBarrier::ReceiveMessage. No message could not be "      \
					 "received."));                                                             \
		}                                                                                       \
		return false;                                                                           \
	}

#define AGX_ASSIGN_ROS2_NATIVE(SubTypeUnreal, SubTypeROS2)            \
	{                                                                 \
		Native = std::make_unique<SubTypeUnreal>(                     \
			new SubTypeROS2(Convert(Topic), Convert(Qos), DomainID)); \
		return;                                                       \
	}

FROS2SubscriberBarrier::FROS2SubscriberBarrier()
{
}

FROS2SubscriberBarrier::~FROS2SubscriberBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration, of FROS2Subscriber.
}

FROS2SubscriberBarrier::FROS2SubscriberBarrier(FROS2SubscriberBarrier&& Other) noexcept
{
	*this = std::move(Other);
}

FROS2SubscriberBarrier& FROS2SubscriberBarrier::operator=(FROS2SubscriberBarrier&& Other) noexcept
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	MessageType = Other.MessageType;
	return *this;
}

bool FROS2SubscriberBarrier::HasNative() const
{
	return Native != nullptr;
}

void FROS2SubscriberBarrier::AllocateNative(
	EAGX_ROS2MessageType InMessageType, const FString& Topic, const FAGX_ROS2Qos& Qos,
	uint8 DomainID)
{
	using namespace agxROS2::agxMsgs;
	using namespace agxROS2::builtinInterfaces;
	using namespace agxROS2::rosgraphMsgs;
	using namespace agxROS2::stdMsgs;
	using namespace agxROS2::geometryMsgs;
	using namespace agxROS2::sensorMsgs;

	MessageType = InMessageType;
	switch (InMessageType)
	{
		case EAGX_ROS2MessageType::Invalid:
			break; // Log error after this switch statement.
		case EAGX_ROS2MessageType::AgxMsgsAny:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberAny, SubscriberAny)
		case EAGX_ROS2MessageType::AgxMsgsAnySequence:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberAnySequence, SubscriberAnySequence)
		case EAGX_ROS2MessageType::BuiltinInterfacesTime:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTime, SubscriberTime)
		case EAGX_ROS2MessageType::BuiltinInterfacesDuration:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberDuration, SubscriberDuration)
		case EAGX_ROS2MessageType::RosgraphMsgsClock:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberClock, SubscriberClock)
		case EAGX_ROS2MessageType::StdMsgsBool:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberBool, SubscriberBool)
		case EAGX_ROS2MessageType::StdMsgsByte:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberByte, SubscriberByte)
		case EAGX_ROS2MessageType::StdMsgsByteMultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberByteMultiArray, SubscriberByteMultiArray)
		case EAGX_ROS2MessageType::StdMsgsChar:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberChar, SubscriberChar)
		case EAGX_ROS2MessageType::StdMsgsColorRGBA:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberColorRGBA, SubscriberColorRGBA)
		case EAGX_ROS2MessageType::StdMsgsEmpty:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberEmpty, SubscriberEmpty)
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberFloat32, SubscriberFloat32)
		case EAGX_ROS2MessageType::StdMsgsFloat32MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberFloat32MultiArray, SubscriberFloat32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsFloat64:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberFloat64, SubscriberFloat64)
		case EAGX_ROS2MessageType::StdMsgsFloat64MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberFloat64MultiArray, SubscriberFloat64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt16:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt16, SubscriberInt16)
		case EAGX_ROS2MessageType::StdMsgsInt16MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt16MultiArray, SubscriberInt16MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt32, SubscriberInt32)
		case EAGX_ROS2MessageType::StdMsgsInt32MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt32MultiArray, SubscriberInt32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt64:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt64, SubscriberInt64)
		case EAGX_ROS2MessageType::StdMsgsInt64MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt64MultiArray, SubscriberInt64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt8:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt8, SubscriberInt8)
		case EAGX_ROS2MessageType::StdMsgsInt8MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInt8MultiArray, SubscriberInt8MultiArray)
		case EAGX_ROS2MessageType::StdMsgsString:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberString, SubscriberString)
		case EAGX_ROS2MessageType::StdMsgsUInt16:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt16, SubscriberUInt16)
		case EAGX_ROS2MessageType::StdMsgsUInt16MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt16MultiArray, SubscriberUInt16MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt32:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt32, SubscriberUInt32)
		case EAGX_ROS2MessageType::StdMsgsUInt32MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt32MultiArray, SubscriberUInt32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt64:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt64, SubscriberUInt64)
		case EAGX_ROS2MessageType::StdMsgsUInt64MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt64MultiArray, SubscriberUInt64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt8:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt8, SubscriberUInt8)
		case EAGX_ROS2MessageType::StdMsgsUInt8MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberUInt8MultiArray, SubscriberUInt8MultiArray)
		case EAGX_ROS2MessageType::StdMsgsHeader:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberHeader, SubscriberHeader)
		case EAGX_ROS2MessageType::GeometryMsgsVector3:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberVector3, SubscriberVector3)
		case EAGX_ROS2MessageType::GeometryMsgsQuaternion:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberQuaternion, SubscriberQuaternion)
		case EAGX_ROS2MessageType::GeometryMsgsAccel:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberAccel, SubscriberAccel)
		case EAGX_ROS2MessageType::GeometryMsgsAccelStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberAccelStamped, SubscriberAccelStamped)
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovariance:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberAccelWithCovariance, SubscriberAccelWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovarianceStamped:
			AGX_ASSIGN_ROS2_NATIVE(
				FSubscriberAccelWithCovarianceStamped, SubscriberAccelWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsInertia:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInertia, SubscriberInertia)
		case EAGX_ROS2MessageType::GeometryMsgsInertiaStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberInertiaStamped, SubscriberInertiaStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPoint:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPoint, SubscriberPoint)
		case EAGX_ROS2MessageType::GeometryMsgsPoint32:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPoint32, SubscriberPoint32)
		case EAGX_ROS2MessageType::GeometryMsgsPointStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPointStamped, SubscriberPointStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPolygon:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPolygon, SubscriberPolygon)
		case EAGX_ROS2MessageType::GeometryMsgsPolygonStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPolygonStamped, SubscriberPolygonStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPose:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPose, SubscriberPose)
		case EAGX_ROS2MessageType::GeometryMsgsPose2D:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPose2D, SubscriberPose2D)
		case EAGX_ROS2MessageType::GeometryMsgsPoseArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPoseArray, SubscriberPoseArray)
		case EAGX_ROS2MessageType::GeometryMsgsPoseStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPoseStamped, SubscriberPoseStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovariance:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPoseWithCovariance, SubscriberPoseWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovarianceStamped:
			AGX_ASSIGN_ROS2_NATIVE(
				FSubscriberPoseWithCovarianceStamped, SubscriberPoseWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsQuaternionStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberQuaternionStamped, SubscriberQuaternionStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTransform:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTransform, SubscriberTransform)
		case EAGX_ROS2MessageType::GeometryMsgsTransformStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTransformStamped, SubscriberTransformStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTwist:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTwist, SubscriberTwist)
		case EAGX_ROS2MessageType::GeometryMsgsTwistStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTwistStamped, SubscriberTwistStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovariance:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTwistWithCovariance, SubscriberTwistWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovarianceStamped:
			AGX_ASSIGN_ROS2_NATIVE(
				FSubscriberTwistWithCovarianceStamped, SubscriberTwistWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsVector3Stamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberVector3Stamped, SubscriberVector3Stamped)
		case EAGX_ROS2MessageType::GeometryMsgsWrench:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberWrench, SubscriberWrench)
		case EAGX_ROS2MessageType::GeometryMsgsWrenchStamped:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberWrenchStamped, SubscriberWrenchStamped)
		case EAGX_ROS2MessageType::SensorMsgsBatteryState:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberBatteryState, SubscriberBatteryState)
		case EAGX_ROS2MessageType::SensorMsgsChannelFloat32:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberChannelFloat32, SubscriberChannelFloat32)
		case EAGX_ROS2MessageType::SensorMsgsCompressedImage:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberCompressedImage, SubscriberCompressedImage)
		case EAGX_ROS2MessageType::SensorMsgsFluidPressure:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberFluidPressure, SubscriberFluidPressure)
		case EAGX_ROS2MessageType::SensorMsgsIlluminance:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberIlluminance, SubscriberIlluminance)
		case EAGX_ROS2MessageType::SensorMsgsImage:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberImage, SubscriberImage)
		case EAGX_ROS2MessageType::SensorMsgsImu:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberImu, SubscriberImu)
		case EAGX_ROS2MessageType::SensorMsgsJointState:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberJointState, SubscriberJointState)
		case EAGX_ROS2MessageType::SensorMsgsJoy:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberJoy, SubscriberJoy)
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedback:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberJoyFeedback, SubscriberJoyFeedback)
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedbackArray:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberJoyFeedbackArray, SubscriberJoyFeedbackArray)
		case EAGX_ROS2MessageType::SensorMsgsLaserEcho:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberLaserEcho, SubscriberLaserEcho)
		case EAGX_ROS2MessageType::SensorMsgsLaserScan:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberLaserScan, SubscriberLaserScan)
		case EAGX_ROS2MessageType::SensorMsgsMagneticField:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberMagneticField, SubscriberMagneticField)
		case EAGX_ROS2MessageType::SensorMsgsMultiDOFJointState:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberMultiDOFJointState, SubscriberMultiDOFJointState)
		case EAGX_ROS2MessageType::SensorMsgsMultiEchoLaserScan:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberMultiEchoLaserScan, SubscriberMultiEchoLaserScan)
		case EAGX_ROS2MessageType::SensorMsgsNavSatStatus:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberNavSatStatus, SubscriberNavSatStatus)
		case EAGX_ROS2MessageType::SensorMsgsNavSatFix:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberNavSatFix, SubscriberNavSatFix)
		case EAGX_ROS2MessageType::SensorMsgsPointCloud:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPointCloud, SubscriberPointCloud)
		case EAGX_ROS2MessageType::SensorMsgsPointField:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPointField, SubscriberPointField)
		case EAGX_ROS2MessageType::SensorMsgsPointCloud2:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberPointCloud2, SubscriberPointCloud2)
		case EAGX_ROS2MessageType::SensorMsgsRange:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberRange, SubscriberRange)
		case EAGX_ROS2MessageType::SensorMsgsRegionOfInterest:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberRegionOfInterest, SubscriberRegionOfInterest)
		case EAGX_ROS2MessageType::SensorMsgsCameraInfo:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberCameraInfo, SubscriberCameraInfo)
		case EAGX_ROS2MessageType::SensorMsgsRelativeHumidity:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberRelativeHumidity, SubscriberRelativeHumidity)
		case EAGX_ROS2MessageType::SensorMsgsTemperature:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTemperature, SubscriberTemperature)
		case EAGX_ROS2MessageType::SensorMsgsTimeReference:
			AGX_ASSIGN_ROS2_NATIVE(FSubscriberTimeReference, SubscriberTimeReference)
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported or invalid type passed to FROS2SubscriberBarrier::AllocateNative, topic: "
			 "'%s'. Native will not be allocated."),
		*Topic);
}

#undef AGX_ASSIGN_ROS2_NATIVE

FROS2Subscriber* FROS2SubscriberBarrier::GetNative()
{
	return Native.get();
}

const FROS2Subscriber* FROS2SubscriberBarrier::GetNative() const
{
	return Native.get();
}

EAGX_ROS2MessageType FROS2SubscriberBarrier::GetMessageType() const
{
	return MessageType;
}

void FROS2SubscriberBarrier::ReleaseNative()
{
	Native = nullptr;
	MessageType = EAGX_ROS2MessageType::Invalid;
}

bool FROS2SubscriberBarrier::ReceiveMessage(FAGX_ROS2Message& OutMsg) const
{
	using namespace agxROS2::agxMsgs;
	using namespace agxROS2::builtinInterfaces;
	using namespace agxROS2::rosgraphMsgs;
	using namespace agxROS2::stdMsgs;
	using namespace agxROS2::geometryMsgs;
	using namespace agxROS2::sensorMsgs;
	check(HasNative());

	switch (MessageType)
	{
		case EAGX_ROS2MessageType::Invalid:
			break; // Log error after this switch statement.
		case EAGX_ROS2MessageType::AgxMsgsAny:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberAny, FAGX_AgxMsgsAny, Any)
		case EAGX_ROS2MessageType::AgxMsgsAnySequence:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberAnySequence, FAGX_AgxMsgsAnySequence, AnySequence)
		case EAGX_ROS2MessageType::BuiltinInterfacesTime:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberTime, FAGX_BuiltinInterfacesTime, Time)
		case EAGX_ROS2MessageType::BuiltinInterfacesDuration:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberDuration, FAGX_BuiltinInterfacesDuration, Duration)
		case EAGX_ROS2MessageType::RosgraphMsgsClock:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberClock, FAGX_RosgraphMsgsClock, Clock)
		case EAGX_ROS2MessageType::StdMsgsBool:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberBool, FAGX_StdMsgsBool, Bool)
		case EAGX_ROS2MessageType::StdMsgsByte:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberByte, FAGX_StdMsgsByte, Byte)
		case EAGX_ROS2MessageType::StdMsgsByteMultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberByteMultiArray, FAGX_StdMsgsByteMultiArray, ByteMultiArray)
		case EAGX_ROS2MessageType::StdMsgsChar:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberChar, FAGX_StdMsgsChar, Char)
		case EAGX_ROS2MessageType::StdMsgsColorRGBA:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberColorRGBA, FAGX_StdMsgsColorRGBA, ColorRGBA)
		case EAGX_ROS2MessageType::StdMsgsEmpty:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberEmpty, FAGX_StdMsgsEmpty, Empty)
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberFloat32, FAGX_StdMsgsFloat32, Float32)
		case EAGX_ROS2MessageType::StdMsgsFloat32MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberFloat32MultiArray, FAGX_StdMsgsFloat32MultiArray, Float32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsFloat64:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberFloat64, FAGX_StdMsgsFloat64, Float64)
		case EAGX_ROS2MessageType::StdMsgsFloat64MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberFloat64MultiArray, FAGX_StdMsgsFloat64MultiArray, Float64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt16:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberInt16, FAGX_StdMsgsInt16, Int16)
		case EAGX_ROS2MessageType::StdMsgsInt16MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberInt16MultiArray, FAGX_StdMsgsInt16MultiArray, Int16MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberInt32, FAGX_StdMsgsInt32, Int32)
		case EAGX_ROS2MessageType::StdMsgsInt32MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberInt32MultiArray, FAGX_StdMsgsInt32MultiArray, Int32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt64:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberInt64, FAGX_StdMsgsInt64, Int64)
		case EAGX_ROS2MessageType::StdMsgsInt64MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberInt64MultiArray, FAGX_StdMsgsInt64MultiArray, Int64MultiArray)

		case EAGX_ROS2MessageType::StdMsgsInt8:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberInt8, FAGX_StdMsgsInt8, Int8)
		case EAGX_ROS2MessageType::StdMsgsInt8MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberInt8MultiArray, FAGX_StdMsgsInt8MultiArray, Int8MultiArray)
		case EAGX_ROS2MessageType::StdMsgsString:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberString, FAGX_StdMsgsString, String)
		case EAGX_ROS2MessageType::StdMsgsUInt16:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberUInt16, FAGX_StdMsgsUInt16, UInt16)
		case EAGX_ROS2MessageType::StdMsgsUInt16MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberUInt16MultiArray, FAGX_StdMsgsUInt16MultiArray, UInt16MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt32:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberUInt32, FAGX_StdMsgsUInt32, UInt32)
		case EAGX_ROS2MessageType::StdMsgsUInt32MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberUInt32MultiArray, FAGX_StdMsgsUInt32MultiArray, UInt32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt64:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberUInt64, FAGX_StdMsgsUInt64, UInt64)
		case EAGX_ROS2MessageType::StdMsgsUInt64MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberUInt64MultiArray, FAGX_StdMsgsUInt64MultiArray, UInt64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt8:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberUInt8, FAGX_StdMsgsUInt8, UInt8)
		case EAGX_ROS2MessageType::StdMsgsUInt8MultiArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberUInt8MultiArray, FAGX_StdMsgsUInt8MultiArray, UInt8MultiArray)
		case EAGX_ROS2MessageType::StdMsgsHeader:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberHeader, FAGX_StdMsgsHeader, Header)
		case EAGX_ROS2MessageType::GeometryMsgsVector3:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberVector3, FAGX_GeometryMsgsVector3, Vector3)
		case EAGX_ROS2MessageType::GeometryMsgsQuaternion:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberQuaternion, FAGX_GeometryMsgsQuaternion, Quaternion)
		case EAGX_ROS2MessageType::GeometryMsgsAccel:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberAccel, FAGX_GeometryMsgsAccel, Accel)
		case EAGX_ROS2MessageType::GeometryMsgsAccelStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberAccelStamped, FAGX_GeometryMsgsAccelStamped, AccelStamped)
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovariance:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberAccelWithCovariance, FAGX_GeometryMsgsAccelWithCovariance,
				AccelWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovarianceStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberAccelWithCovarianceStamped, FAGX_GeometryMsgsAccelWithCovarianceStamped,
				AccelWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsInertia:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberInertia, FAGX_GeometryMsgsInertia, Inertia)
		case EAGX_ROS2MessageType::GeometryMsgsInertiaStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberInertiaStamped, FAGX_GeometryMsgsInertiaStamped, InertiaStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPoint:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPoint, FAGX_GeometryMsgsPoint, Point)
		case EAGX_ROS2MessageType::GeometryMsgsPoint32:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPoint32, FAGX_GeometryMsgsPoint32, Point32)
		case EAGX_ROS2MessageType::GeometryMsgsPointStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberPointStamped, FAGX_GeometryMsgsPointStamped, PointStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPolygon:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberPolygon, FAGX_GeometryMsgsPolygon, agxROS2::geometryMsgs::Polygon)
		case EAGX_ROS2MessageType::GeometryMsgsPolygonStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberPolygonStamped, FAGX_GeometryMsgsPolygonStamped, PolygonStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPose:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPose, FAGX_GeometryMsgsPose, Pose)
		case EAGX_ROS2MessageType::GeometryMsgsPose2D:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPose2D, FAGX_GeometryMsgsPose2D, Pose2D)
		case EAGX_ROS2MessageType::GeometryMsgsPoseArray:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPoseArray, FAGX_GeometryMsgsPoseArray, PoseArray)
		case EAGX_ROS2MessageType::GeometryMsgsPoseStamped:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPoseStamped, FAGX_GeometryMsgsPoseStamped, PoseStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovariance:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberPoseWithCovariance, FAGX_GeometryMsgsPoseWithCovariance,
				PoseWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovarianceStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberPoseWithCovarianceStamped, FAGX_GeometryMsgsPoseWithCovarianceStamped,
				PoseWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsQuaternionStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberQuaternionStamped, FAGX_GeometryMsgsQuaternionStamped, QuaternionStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTransform:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberTransform, FAGX_GeometryMsgsTransform, Transform)
		case EAGX_ROS2MessageType::GeometryMsgsTransformStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberTransformStamped, FAGX_GeometryMsgsTransformStamped, TransformStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTwist:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberTwist, FAGX_GeometryMsgsTwist, Twist)
		case EAGX_ROS2MessageType::GeometryMsgsTwistStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberTwistStamped, FAGX_GeometryMsgsTwistStamped, TwistStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovariance:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberTwistWithCovariance, FAGX_GeometryMsgsTwistWithCovariance,
				TwistWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovarianceStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberTwistWithCovarianceStamped, FAGX_GeometryMsgsTwistWithCovarianceStamped,
				TwistWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsVector3Stamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberVector3Stamped, FAGX_GeometryMsgsVector3Stamped, Vector3Stamped)
		case EAGX_ROS2MessageType::GeometryMsgsWrench:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberWrench, FAGX_GeometryMsgsWrench, Wrench)
		case EAGX_ROS2MessageType::GeometryMsgsWrenchStamped:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberWrenchStamped, FAGX_GeometryMsgsWrenchStamped, WrenchStamped)
		case EAGX_ROS2MessageType::SensorMsgsBatteryState:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberBatteryState, FAGX_SensorMsgsBatteryState, BatteryState)
		case EAGX_ROS2MessageType::SensorMsgsChannelFloat32:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberChannelFloat32, FAGX_SensorMsgsChannelFloat32, ChannelFloat32)
		case EAGX_ROS2MessageType::SensorMsgsCompressedImage:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberCompressedImage, FAGX_SensorMsgsCompressedImage, CompressedImage)
		case EAGX_ROS2MessageType::SensorMsgsFluidPressure:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberFluidPressure, FAGX_SensorMsgsFluidPressure, FluidPressure)
		case EAGX_ROS2MessageType::SensorMsgsIlluminance:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberIlluminance, FAGX_SensorMsgsIlluminance, Illuminance)
		case EAGX_ROS2MessageType::SensorMsgsImage:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberImage, FAGX_SensorMsgsImage, Image)
		case EAGX_ROS2MessageType::SensorMsgsImu:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberImu, FAGX_SensorMsgsImu, Imu)
		case EAGX_ROS2MessageType::SensorMsgsJointState:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberJointState, FAGX_SensorMsgsJointState, JointState)
		case EAGX_ROS2MessageType::SensorMsgsJoy:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberJoy, FAGX_SensorMsgsJoy, Joy)
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedback:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberJoyFeedback, FAGX_SensorMsgsJoyFeedback, JoyFeedback)
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedbackArray:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberJoyFeedbackArray, FAGX_SensorMsgsJoyFeedbackArray, JoyFeedbackArray)
		case EAGX_ROS2MessageType::SensorMsgsLaserEcho:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberLaserEcho, FAGX_SensorMsgsLaserEcho, LaserEcho)
		case EAGX_ROS2MessageType::SensorMsgsLaserScan:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberLaserScan, FAGX_SensorMsgsLaserScan, LaserScan)
		case EAGX_ROS2MessageType::SensorMsgsMagneticField:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberMagneticField, FAGX_SensorMsgsMagneticField, MagneticField)
		case EAGX_ROS2MessageType::SensorMsgsMultiDOFJointState:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberMultiDOFJointState, FAGX_SensorMsgsMultiDOFJointState,
				MultiDOFJointState)
		case EAGX_ROS2MessageType::SensorMsgsMultiEchoLaserScan:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberMultiEchoLaserScan, FAGX_SensorMsgsMultiEchoLaserScan,
				MultiEchoLaserScan)
		case EAGX_ROS2MessageType::SensorMsgsNavSatStatus:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberNavSatStatus, FAGX_SensorMsgsNavSatStatus, NavSatStatus)
		case EAGX_ROS2MessageType::SensorMsgsNavSatFix:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberNavSatFix, FAGX_SensorMsgsNavSatFix, NavSatFix)
		case EAGX_ROS2MessageType::SensorMsgsPointCloud:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPointCloud, FAGX_SensorMsgsPointCloud, PointCloud)
		case EAGX_ROS2MessageType::SensorMsgsPointField:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPointField, FAGX_SensorMsgsPointField, PointField)
		case EAGX_ROS2MessageType::SensorMsgsPointCloud2:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberPointCloud2, FAGX_SensorMsgsPointCloud2, PointCloud2)
		case EAGX_ROS2MessageType::SensorMsgsRange:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberRange, FAGX_SensorMsgsRange, Range)
		case EAGX_ROS2MessageType::SensorMsgsRegionOfInterest:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberRegionOfInterest, FAGX_SensorMsgsRegionOfInterest, RegionOfInterest)
		case EAGX_ROS2MessageType::SensorMsgsCameraInfo:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberCameraInfo, FAGX_SensorMsgsCameraInfo, CameraInfo)
		case EAGX_ROS2MessageType::SensorMsgsRelativeHumidity:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberRelativeHumidity, FAGX_SensorMsgsRelativeHumidity, RelativeHumidity)
		case EAGX_ROS2MessageType::SensorMsgsTemperature:
			AGX_RECEIVE_ROS2_MSGS(FSubscriberTemperature, FAGX_SensorMsgsTemperature, Temperature)
		case EAGX_ROS2MessageType::SensorMsgsTimeReference:
			AGX_RECEIVE_ROS2_MSGS(
				FSubscriberTimeReference, FAGX_SensorMsgsTimeReference, TimeReference)
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FROS2SubscriberBarrier::ReceivedMessage called on SubscriberBarrier with an invalid "
			 "MessageType. The message will not be received."));
	return false;
}
