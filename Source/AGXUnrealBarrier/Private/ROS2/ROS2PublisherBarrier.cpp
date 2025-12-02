// Copyright 2025, Algoryx Simulation AB.

#include "ROS2/ROS2PublisherBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"
#include "TypeConversions.h"
#include "Utilities/ROS2Utilities.h"

// Helper macros to minimize amount of code needed in large switch-statement.
#define AGX_SEND_ROS2_MSGS(PubType, MsgType)                                                       \
	{                                                                                              \
		if (auto Pub = dynamic_cast<const PubType*>(Native.get()))                                 \
		{                                                                                          \
			auto MsgAGX = Convert(*static_cast<const MsgType*>(&Msg));                             \
			Pub->Native->sendMessage(MsgAGX);                                                      \
			AGX_ROS2Utilities::FreeContainers(MsgAGX);                                             \
			return true;                                                                           \
		}                                                                                          \
		else                                                                                       \
		{                                                                                          \
			UE_LOG(                                                                                \
				LogAGX, Error,                                                                     \
				TEXT(                                                                              \
					"Unexpected internal error: unable to downcast to the correct Publisher type " \
					"in FROS2PublisherBarrier::SendMessage. The message will not be sent."));      \
			return false;                                                                          \
		}                                                                                          \
	}

#define AGX_ASSIGN_ROS2_NATIVE(PubTypeUnreal, PubTypeROS2)            \
	{                                                                 \
		Native = std::make_unique<PubTypeUnreal>(                     \
			new PubTypeROS2(Convert(Topic), Convert(Qos), DomainID)); \
		return;                                                       \
	}

FROS2PublisherBarrier::FROS2PublisherBarrier()
{
}

FROS2PublisherBarrier::~FROS2PublisherBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration, of FROS2Publisher.
}

FROS2PublisherBarrier::FROS2PublisherBarrier(FROS2PublisherBarrier&& Other) noexcept
{
	*this = std::move(Other);
}

FROS2PublisherBarrier& FROS2PublisherBarrier::operator=(FROS2PublisherBarrier&& Other) noexcept
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	MessageType = Other.MessageType;
	return *this;
}

bool FROS2PublisherBarrier::HasNative() const
{
	return Native != nullptr;
}

void FROS2PublisherBarrier::AllocateNative(
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
			AGX_ASSIGN_ROS2_NATIVE(FPublisherAny, PublisherAny);
		case EAGX_ROS2MessageType::AgxMsgsAnySequence:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherAnySequence, PublisherAnySequence);
		case EAGX_ROS2MessageType::BuiltinInterfacesTime:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTime, PublisherTime);
		case EAGX_ROS2MessageType::BuiltinInterfacesDuration:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherDuration, PublisherDuration);
		case EAGX_ROS2MessageType::RosgraphMsgsClock:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherClock, PublisherClock);
		case EAGX_ROS2MessageType::StdMsgsBool:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherBool, PublisherBool);
		case EAGX_ROS2MessageType::StdMsgsByte:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherByte, PublisherByte);
		case EAGX_ROS2MessageType::StdMsgsByteMultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherByteMultiArray, PublisherByteMultiArray);
		case EAGX_ROS2MessageType::StdMsgsChar:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherChar, PublisherChar);
		case EAGX_ROS2MessageType::StdMsgsColorRGBA:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherColorRGBA, PublisherColorRGBA);
		case EAGX_ROS2MessageType::StdMsgsEmpty:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherEmpty, PublisherEmpty);
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherFloat32, PublisherFloat32);
		case EAGX_ROS2MessageType::StdMsgsFloat32MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherFloat32MultiArray, PublisherFloat32MultiArray);
		case EAGX_ROS2MessageType::StdMsgsFloat64:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherFloat64, PublisherFloat64);
		case EAGX_ROS2MessageType::StdMsgsFloat64MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherFloat64MultiArray, PublisherFloat64MultiArray);
		case EAGX_ROS2MessageType::StdMsgsInt16:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt16, PublisherInt16);
		case EAGX_ROS2MessageType::StdMsgsInt16MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt16MultiArray, PublisherInt16MultiArray);
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt32, PublisherInt32);
		case EAGX_ROS2MessageType::StdMsgsInt32MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt32MultiArray, PublisherInt32MultiArray);
		case EAGX_ROS2MessageType::StdMsgsInt64:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt64, PublisherInt64);
		case EAGX_ROS2MessageType::StdMsgsInt64MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt64MultiArray, PublisherInt64MultiArray);
		case EAGX_ROS2MessageType::StdMsgsInt8:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt8, PublisherInt8);
		case EAGX_ROS2MessageType::StdMsgsInt8MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInt8MultiArray, PublisherInt8MultiArray);
		case EAGX_ROS2MessageType::StdMsgsString:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherString, PublisherString);
		case EAGX_ROS2MessageType::StdMsgsUInt16:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt16, PublisherUInt16);
		case EAGX_ROS2MessageType::StdMsgsUInt16MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt16MultiArray, PublisherUInt16MultiArray);
		case EAGX_ROS2MessageType::StdMsgsUInt32:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt32, PublisherUInt32);
		case EAGX_ROS2MessageType::StdMsgsUInt32MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt32MultiArray, PublisherUInt32MultiArray);
		case EAGX_ROS2MessageType::StdMsgsUInt64:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt64, PublisherUInt64);
		case EAGX_ROS2MessageType::StdMsgsUInt64MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt64MultiArray, PublisherUInt64MultiArray);
		case EAGX_ROS2MessageType::StdMsgsUInt8:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt8, PublisherUInt8);
		case EAGX_ROS2MessageType::StdMsgsUInt8MultiArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherUInt8MultiArray, PublisherUInt8MultiArray);
		case EAGX_ROS2MessageType::StdMsgsHeader:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherHeader, PublisherHeader);
		case EAGX_ROS2MessageType::GeometryMsgsVector3:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherVector3, PublisherVector3);
		case EAGX_ROS2MessageType::GeometryMsgsQuaternion:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherQuaternion, PublisherQuaternion);
		case EAGX_ROS2MessageType::GeometryMsgsAccel:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherAccel, PublisherAccel);
		case EAGX_ROS2MessageType::GeometryMsgsAccelStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherAccelStamped, PublisherAccelStamped);
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovariance:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherAccelWithCovariance, PublisherAccelWithCovariance);
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovarianceStamped:
			AGX_ASSIGN_ROS2_NATIVE(
				FPublisherAccelWithCovarianceStamped, PublisherAccelWithCovarianceStamped);
		case EAGX_ROS2MessageType::GeometryMsgsInertia:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInertia, PublisherInertia);
		case EAGX_ROS2MessageType::GeometryMsgsInertiaStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherInertiaStamped, PublisherInertiaStamped);
		case EAGX_ROS2MessageType::GeometryMsgsPoint:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPoint, PublisherPoint);
		case EAGX_ROS2MessageType::GeometryMsgsPoint32:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPoint32, PublisherPoint32);
		case EAGX_ROS2MessageType::GeometryMsgsPointStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPointStamped, PublisherPointStamped);
		case EAGX_ROS2MessageType::GeometryMsgsPolygon:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPolygon, PublisherPolygon);
		case EAGX_ROS2MessageType::GeometryMsgsPolygonStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPolygonStamped, PublisherPolygonStamped);
		case EAGX_ROS2MessageType::GeometryMsgsPose:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPose, PublisherPose);
		case EAGX_ROS2MessageType::GeometryMsgsPose2D:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPose2D, PublisherPose2D);
		case EAGX_ROS2MessageType::GeometryMsgsPoseArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPoseArray, PublisherPoseArray);
		case EAGX_ROS2MessageType::GeometryMsgsPoseStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPoseStamped, PublisherPoseStamped);
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovariance:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPoseWithCovariance, PublisherPoseWithCovariance);
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovarianceStamped:
			AGX_ASSIGN_ROS2_NATIVE(
				FPublisherPoseWithCovarianceStamped, PublisherPoseWithCovarianceStamped);
		case EAGX_ROS2MessageType::GeometryMsgsQuaternionStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherQuaternionStamped, PublisherQuaternionStamped);
		case EAGX_ROS2MessageType::GeometryMsgsTransform:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTransform, PublisherTransform);
		case EAGX_ROS2MessageType::GeometryMsgsTransformStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTransformStamped, PublisherTransformStamped);
		case EAGX_ROS2MessageType::GeometryMsgsTwist:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTwist, PublisherTwist);
		case EAGX_ROS2MessageType::GeometryMsgsTwistStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTwistStamped, PublisherTwistStamped);
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovariance:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTwistWithCovariance, PublisherTwistWithCovariance);
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovarianceStamped:
			AGX_ASSIGN_ROS2_NATIVE(
				FPublisherTwistWithCovarianceStamped, PublisherTwistWithCovarianceStamped);
		case EAGX_ROS2MessageType::GeometryMsgsVector3Stamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherVector3Stamped, PublisherVector3Stamped);
		case EAGX_ROS2MessageType::GeometryMsgsWrench:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherWrench, PublisherWrench);
		case EAGX_ROS2MessageType::GeometryMsgsWrenchStamped:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherWrenchStamped, PublisherWrenchStamped);
		case EAGX_ROS2MessageType::SensorMsgsBatteryState:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherBatteryState, PublisherBatteryState);
		case EAGX_ROS2MessageType::SensorMsgsChannelFloat32:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherChannelFloat32, PublisherChannelFloat32);
		case EAGX_ROS2MessageType::SensorMsgsCompressedImage:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherCompressedImage, PublisherCompressedImage);
		case EAGX_ROS2MessageType::SensorMsgsFluidPressure:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherFluidPressure, PublisherFluidPressure);
		case EAGX_ROS2MessageType::SensorMsgsIlluminance:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherIlluminance, PublisherIlluminance);
		case EAGX_ROS2MessageType::SensorMsgsImage:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherImage, PublisherImage);
		case EAGX_ROS2MessageType::SensorMsgsImu:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherImu, PublisherImu);
		case EAGX_ROS2MessageType::SensorMsgsJointState:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherJointState, PublisherJointState);
		case EAGX_ROS2MessageType::SensorMsgsJoy:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherJoy, PublisherJoy);
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedback:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherJoyFeedback, PublisherJoyFeedback);
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedbackArray:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherJoyFeedbackArray, PublisherJoyFeedbackArray);
		case EAGX_ROS2MessageType::SensorMsgsLaserEcho:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherLaserEcho, PublisherLaserEcho);
		case EAGX_ROS2MessageType::SensorMsgsLaserScan:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherLaserScan, PublisherLaserScan);
		case EAGX_ROS2MessageType::SensorMsgsMagneticField:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherMagneticField, PublisherMagneticField);
		case EAGX_ROS2MessageType::SensorMsgsMultiDOFJointState:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherMultiDOFJointState, PublisherMultiDOFJointState);
		case EAGX_ROS2MessageType::SensorMsgsMultiEchoLaserScan:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherMultiEchoLaserScan, PublisherMultiEchoLaserScan);
		case EAGX_ROS2MessageType::SensorMsgsNavSatStatus:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherNavSatStatus, PublisherNavSatStatus);
		case EAGX_ROS2MessageType::SensorMsgsNavSatFix:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherNavSatFix, PublisherNavSatFix);
		case EAGX_ROS2MessageType::SensorMsgsPointCloud:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPointCloud, PublisherPointCloud);
		case EAGX_ROS2MessageType::SensorMsgsPointField:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPointField, PublisherPointField);
		case EAGX_ROS2MessageType::SensorMsgsPointCloud2:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherPointCloud2, PublisherPointCloud2);
		case EAGX_ROS2MessageType::SensorMsgsRange:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherRange, PublisherRange);
		case EAGX_ROS2MessageType::SensorMsgsRegionOfInterest:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherRegionOfInterest, PublisherRegionOfInterest);
		case EAGX_ROS2MessageType::SensorMsgsCameraInfo:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherCameraInfo, PublisherCameraInfo);
		case EAGX_ROS2MessageType::SensorMsgsRelativeHumidity:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherRelativeHumidity, PublisherRelativeHumidity);
		case EAGX_ROS2MessageType::SensorMsgsTemperature:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTemperature, PublisherTemperature);
		case EAGX_ROS2MessageType::SensorMsgsTimeReference:
			AGX_ASSIGN_ROS2_NATIVE(FPublisherTimeReference, PublisherTimeReference);
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported or invalid type passed to FROS2PublisherBarrier::AllocateNative, topic: "
			 "'%s'. Native will not be allocated."),
		*Topic);
}

#undef AGX_ASSIGN_ROS2_NATIVE

FROS2Publisher* FROS2PublisherBarrier::GetNative()
{
	return Native.get();
}

const FROS2Publisher* FROS2PublisherBarrier::GetNative() const
{
	return Native.get();
}

EAGX_ROS2MessageType FROS2PublisherBarrier::GetMessageType() const
{
	return MessageType;
}

bool FROS2PublisherBarrier::SendMsg(const FAGX_ROS2Message& Msg) const
{
	check(HasNative());

	switch (MessageType)
	{
		case EAGX_ROS2MessageType::Invalid:
			break; // Log error after this switch statement.
		case EAGX_ROS2MessageType::AgxMsgsAny:
			AGX_SEND_ROS2_MSGS(FPublisherAny, FAGX_AgxMsgsAny)
		case EAGX_ROS2MessageType::AgxMsgsAnySequence:
			AGX_SEND_ROS2_MSGS(FPublisherAnySequence, FAGX_AgxMsgsAnySequence)
		case EAGX_ROS2MessageType::BuiltinInterfacesTime:
			AGX_SEND_ROS2_MSGS(FPublisherTime, FAGX_BuiltinInterfacesTime)
		case EAGX_ROS2MessageType::BuiltinInterfacesDuration:
			AGX_SEND_ROS2_MSGS(FPublisherDuration, FAGX_BuiltinInterfacesDuration)
		case EAGX_ROS2MessageType::RosgraphMsgsClock:
			AGX_SEND_ROS2_MSGS(FPublisherClock, FAGX_RosgraphMsgsClock)
		case EAGX_ROS2MessageType::StdMsgsBool:
			AGX_SEND_ROS2_MSGS(FPublisherBool, FAGX_StdMsgsBool)
		case EAGX_ROS2MessageType::StdMsgsByte:
			AGX_SEND_ROS2_MSGS(FPublisherByte, FAGX_StdMsgsByte)
		case EAGX_ROS2MessageType::StdMsgsByteMultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherByteMultiArray, FAGX_StdMsgsByteMultiArray)
		case EAGX_ROS2MessageType::StdMsgsChar:
			AGX_SEND_ROS2_MSGS(FPublisherChar, FAGX_StdMsgsChar)
		case EAGX_ROS2MessageType::StdMsgsColorRGBA:
			AGX_SEND_ROS2_MSGS(FPublisherColorRGBA, FAGX_StdMsgsColorRGBA)
		case EAGX_ROS2MessageType::StdMsgsEmpty:
			AGX_SEND_ROS2_MSGS(FPublisherEmpty, FAGX_StdMsgsEmpty)
		case EAGX_ROS2MessageType::StdMsgsFloat32:
			AGX_SEND_ROS2_MSGS(FPublisherFloat32, FAGX_StdMsgsFloat32)
		case EAGX_ROS2MessageType::StdMsgsFloat32MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherFloat32MultiArray, FAGX_StdMsgsFloat32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsFloat64:
			AGX_SEND_ROS2_MSGS(FPublisherFloat64, FAGX_StdMsgsFloat64)
		case EAGX_ROS2MessageType::StdMsgsFloat64MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherFloat64MultiArray, FAGX_StdMsgsFloat64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt16:
			AGX_SEND_ROS2_MSGS(FPublisherInt16, FAGX_StdMsgsInt16)
		case EAGX_ROS2MessageType::StdMsgsInt16MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherInt16MultiArray, FAGX_StdMsgsInt16MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt32:
			AGX_SEND_ROS2_MSGS(FPublisherInt32, FAGX_StdMsgsInt32)
		case EAGX_ROS2MessageType::StdMsgsInt32MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherInt32MultiArray, FAGX_StdMsgsInt32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt64:
			AGX_SEND_ROS2_MSGS(FPublisherInt64, FAGX_StdMsgsInt64)
		case EAGX_ROS2MessageType::StdMsgsInt64MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherInt64MultiArray, FAGX_StdMsgsInt64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsInt8:
			AGX_SEND_ROS2_MSGS(FPublisherInt8, FAGX_StdMsgsInt8)
		case EAGX_ROS2MessageType::StdMsgsInt8MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherInt8MultiArray, FAGX_StdMsgsInt8MultiArray)
		case EAGX_ROS2MessageType::StdMsgsString:
			AGX_SEND_ROS2_MSGS(FPublisherString, FAGX_StdMsgsString)
		case EAGX_ROS2MessageType::StdMsgsUInt16:
			AGX_SEND_ROS2_MSGS(FPublisherUInt16, FAGX_StdMsgsUInt16)
		case EAGX_ROS2MessageType::StdMsgsUInt16MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherUInt16MultiArray, FAGX_StdMsgsUInt16MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt32:
			AGX_SEND_ROS2_MSGS(FPublisherUInt32, FAGX_StdMsgsUInt32)
		case EAGX_ROS2MessageType::StdMsgsUInt32MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherUInt32MultiArray, FAGX_StdMsgsUInt32MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt64:
			AGX_SEND_ROS2_MSGS(FPublisherUInt64, FAGX_StdMsgsUInt64)
		case EAGX_ROS2MessageType::StdMsgsUInt64MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherUInt64MultiArray, FAGX_StdMsgsUInt64MultiArray)
		case EAGX_ROS2MessageType::StdMsgsUInt8:
			AGX_SEND_ROS2_MSGS(FPublisherUInt8, FAGX_StdMsgsUInt8)
		case EAGX_ROS2MessageType::StdMsgsUInt8MultiArray:
			AGX_SEND_ROS2_MSGS(FPublisherUInt8MultiArray, FAGX_StdMsgsUInt8MultiArray)
		case EAGX_ROS2MessageType::StdMsgsHeader:
			AGX_SEND_ROS2_MSGS(FPublisherHeader, FAGX_StdMsgsHeader)
		case EAGX_ROS2MessageType::GeometryMsgsVector3:
			AGX_SEND_ROS2_MSGS(FPublisherVector3, FAGX_GeometryMsgsVector3)
		case EAGX_ROS2MessageType::GeometryMsgsQuaternion:
			AGX_SEND_ROS2_MSGS(FPublisherQuaternion, FAGX_GeometryMsgsQuaternion)
		case EAGX_ROS2MessageType::GeometryMsgsAccel:
			AGX_SEND_ROS2_MSGS(FPublisherAccel, FAGX_GeometryMsgsAccel)
		case EAGX_ROS2MessageType::GeometryMsgsAccelStamped:
			AGX_SEND_ROS2_MSGS(FPublisherAccelStamped, FAGX_GeometryMsgsAccelStamped)
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovariance:
			AGX_SEND_ROS2_MSGS(FPublisherAccelWithCovariance, FAGX_GeometryMsgsAccelWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsAccelWithCovarianceStamped:
			AGX_SEND_ROS2_MSGS(
				FPublisherAccelWithCovarianceStamped, FAGX_GeometryMsgsAccelWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsInertia:
			AGX_SEND_ROS2_MSGS(FPublisherInertia, FAGX_GeometryMsgsInertia)
		case EAGX_ROS2MessageType::GeometryMsgsInertiaStamped:
			AGX_SEND_ROS2_MSGS(FPublisherInertiaStamped, FAGX_GeometryMsgsInertiaStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPoint:
			AGX_SEND_ROS2_MSGS(FPublisherPoint, FAGX_GeometryMsgsPoint)
		case EAGX_ROS2MessageType::GeometryMsgsPoint32:
			AGX_SEND_ROS2_MSGS(FPublisherPoint32, FAGX_GeometryMsgsPoint32)
		case EAGX_ROS2MessageType::GeometryMsgsPointStamped:
			AGX_SEND_ROS2_MSGS(FPublisherPointStamped, FAGX_GeometryMsgsPointStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPolygon:
			AGX_SEND_ROS2_MSGS(FPublisherPolygon, FAGX_GeometryMsgsPolygon)
		case EAGX_ROS2MessageType::GeometryMsgsPolygonStamped:
			AGX_SEND_ROS2_MSGS(FPublisherPolygonStamped, FAGX_GeometryMsgsPolygonStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPose:
			AGX_SEND_ROS2_MSGS(FPublisherPose, FAGX_GeometryMsgsPose)
		case EAGX_ROS2MessageType::GeometryMsgsPose2D:
			AGX_SEND_ROS2_MSGS(FPublisherPose2D, FAGX_GeometryMsgsPose2D)
		case EAGX_ROS2MessageType::GeometryMsgsPoseArray:
			AGX_SEND_ROS2_MSGS(FPublisherPoseArray, FAGX_GeometryMsgsPoseArray)
		case EAGX_ROS2MessageType::GeometryMsgsPoseStamped:
			AGX_SEND_ROS2_MSGS(FPublisherPoseStamped, FAGX_GeometryMsgsPoseStamped)
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovariance:
			AGX_SEND_ROS2_MSGS(FPublisherPoseWithCovariance, FAGX_GeometryMsgsPoseWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsPoseWithCovarianceStamped:
			AGX_SEND_ROS2_MSGS(
				FPublisherPoseWithCovarianceStamped, FAGX_GeometryMsgsPoseWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsQuaternionStamped:
			AGX_SEND_ROS2_MSGS(FPublisherQuaternionStamped, FAGX_GeometryMsgsQuaternionStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTransform:
			AGX_SEND_ROS2_MSGS(FPublisherTransform, FAGX_GeometryMsgsTransform)
		case EAGX_ROS2MessageType::GeometryMsgsTransformStamped:
			AGX_SEND_ROS2_MSGS(FPublisherTransformStamped, FAGX_GeometryMsgsTransformStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTwist:
			AGX_SEND_ROS2_MSGS(FPublisherTwist, FAGX_GeometryMsgsTwist)
		case EAGX_ROS2MessageType::GeometryMsgsTwistStamped:
			AGX_SEND_ROS2_MSGS(FPublisherTwistStamped, FAGX_GeometryMsgsTwistStamped)
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovariance:
			AGX_SEND_ROS2_MSGS(FPublisherTwistWithCovariance, FAGX_GeometryMsgsTwistWithCovariance)
		case EAGX_ROS2MessageType::GeometryMsgsTwistWithCovarianceStamped:
			AGX_SEND_ROS2_MSGS(
				FPublisherTwistWithCovarianceStamped, FAGX_GeometryMsgsTwistWithCovarianceStamped)
		case EAGX_ROS2MessageType::GeometryMsgsVector3Stamped:
			AGX_SEND_ROS2_MSGS(FPublisherVector3Stamped, FAGX_GeometryMsgsVector3Stamped)
		case EAGX_ROS2MessageType::GeometryMsgsWrench:
			AGX_SEND_ROS2_MSGS(FPublisherWrench, FAGX_GeometryMsgsWrench)
		case EAGX_ROS2MessageType::GeometryMsgsWrenchStamped:
			AGX_SEND_ROS2_MSGS(FPublisherWrenchStamped, FAGX_GeometryMsgsWrenchStamped)
		case EAGX_ROS2MessageType::SensorMsgsBatteryState:
			AGX_SEND_ROS2_MSGS(FPublisherBatteryState, FAGX_SensorMsgsBatteryState)
		case EAGX_ROS2MessageType::SensorMsgsChannelFloat32:
			AGX_SEND_ROS2_MSGS(FPublisherChannelFloat32, FAGX_SensorMsgsChannelFloat32)
		case EAGX_ROS2MessageType::SensorMsgsCompressedImage:
			AGX_SEND_ROS2_MSGS(FPublisherCompressedImage, FAGX_SensorMsgsCompressedImage)
		case EAGX_ROS2MessageType::SensorMsgsFluidPressure:
			AGX_SEND_ROS2_MSGS(FPublisherFluidPressure, FAGX_SensorMsgsFluidPressure)
		case EAGX_ROS2MessageType::SensorMsgsIlluminance:
			AGX_SEND_ROS2_MSGS(FPublisherIlluminance, FAGX_SensorMsgsIlluminance)
		case EAGX_ROS2MessageType::SensorMsgsImage:
			AGX_SEND_ROS2_MSGS(FPublisherImage, FAGX_SensorMsgsImage)
		case EAGX_ROS2MessageType::SensorMsgsImu:
			AGX_SEND_ROS2_MSGS(FPublisherImu, FAGX_SensorMsgsImu)
		case EAGX_ROS2MessageType::SensorMsgsJointState:
			AGX_SEND_ROS2_MSGS(FPublisherJointState, FAGX_SensorMsgsJointState)
		case EAGX_ROS2MessageType::SensorMsgsJoy:
			AGX_SEND_ROS2_MSGS(FPublisherJoy, FAGX_SensorMsgsJoy)
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedback:
			AGX_SEND_ROS2_MSGS(FPublisherJoyFeedback, FAGX_SensorMsgsJoyFeedback)
		case EAGX_ROS2MessageType::SensorMsgsJoyFeedbackArray:
			AGX_SEND_ROS2_MSGS(FPublisherJoyFeedbackArray, FAGX_SensorMsgsJoyFeedbackArray)
		case EAGX_ROS2MessageType::SensorMsgsLaserEcho:
			AGX_SEND_ROS2_MSGS(FPublisherLaserEcho, FAGX_SensorMsgsLaserEcho)
		case EAGX_ROS2MessageType::SensorMsgsLaserScan:
			AGX_SEND_ROS2_MSGS(FPublisherLaserScan, FAGX_SensorMsgsLaserScan)
		case EAGX_ROS2MessageType::SensorMsgsMagneticField:
			AGX_SEND_ROS2_MSGS(FPublisherMagneticField, FAGX_SensorMsgsMagneticField)
		case EAGX_ROS2MessageType::SensorMsgsMultiDOFJointState:
			AGX_SEND_ROS2_MSGS(FPublisherMultiDOFJointState, FAGX_SensorMsgsMultiDOFJointState)
		case EAGX_ROS2MessageType::SensorMsgsMultiEchoLaserScan:
			AGX_SEND_ROS2_MSGS(FPublisherMultiEchoLaserScan, FAGX_SensorMsgsMultiEchoLaserScan)
		case EAGX_ROS2MessageType::SensorMsgsNavSatStatus:
			AGX_SEND_ROS2_MSGS(FPublisherNavSatStatus, FAGX_SensorMsgsNavSatStatus)
		case EAGX_ROS2MessageType::SensorMsgsNavSatFix:
			AGX_SEND_ROS2_MSGS(FPublisherNavSatFix, FAGX_SensorMsgsNavSatFix)
		case EAGX_ROS2MessageType::SensorMsgsPointCloud:
			AGX_SEND_ROS2_MSGS(FPublisherPointCloud, FAGX_SensorMsgsPointCloud)
		case EAGX_ROS2MessageType::SensorMsgsPointField:
			AGX_SEND_ROS2_MSGS(FPublisherPointField, FAGX_SensorMsgsPointField)
		case EAGX_ROS2MessageType::SensorMsgsPointCloud2:
			AGX_SEND_ROS2_MSGS(FPublisherPointCloud2, FAGX_SensorMsgsPointCloud2)
		case EAGX_ROS2MessageType::SensorMsgsRange:
			AGX_SEND_ROS2_MSGS(FPublisherRange, FAGX_SensorMsgsRange)
		case EAGX_ROS2MessageType::SensorMsgsRegionOfInterest:
			AGX_SEND_ROS2_MSGS(FPublisherRegionOfInterest, FAGX_SensorMsgsRegionOfInterest)
		case EAGX_ROS2MessageType::SensorMsgsCameraInfo:
			AGX_SEND_ROS2_MSGS(FPublisherCameraInfo, FAGX_SensorMsgsCameraInfo)
		case EAGX_ROS2MessageType::SensorMsgsRelativeHumidity:
			AGX_SEND_ROS2_MSGS(FPublisherRelativeHumidity, FAGX_SensorMsgsRelativeHumidity)
		case EAGX_ROS2MessageType::SensorMsgsTemperature:
			AGX_SEND_ROS2_MSGS(FPublisherTemperature, FAGX_SensorMsgsTemperature)
		case EAGX_ROS2MessageType::SensorMsgsTimeReference:
			AGX_SEND_ROS2_MSGS(FPublisherTimeReference, FAGX_SensorMsgsTimeReference)
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("FROS2PublisherBarrier::SendMessage called on PublisherBarrier with an invalid "
			 "MessageType. The message will not be sent."));
	return false;
}

void FROS2PublisherBarrier::ReleaseNative()
{
	Native = nullptr;
	MessageType = EAGX_ROS2MessageType::Invalid;
}
