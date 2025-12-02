// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM()
enum class EAGX_ROS2MessageType
{
	Invalid,
	AgxMsgsAny,
	AgxMsgsAnySequence,
	BuiltinInterfacesTime,
	BuiltinInterfacesDuration,
	RosgraphMsgsClock,
	StdMsgsBool,
	StdMsgsByte,
	StdMsgsByteMultiArray,
	StdMsgsChar,
	StdMsgsColorRGBA,
	StdMsgsEmpty,
	StdMsgsFloat32,
	StdMsgsFloat32MultiArray,
	StdMsgsFloat64,
	StdMsgsFloat64MultiArray,
	StdMsgsInt16,
	StdMsgsInt16MultiArray,
	StdMsgsInt32,
	StdMsgsInt32MultiArray,
	StdMsgsInt64,
	StdMsgsInt64MultiArray,
	StdMsgsInt8,
	StdMsgsInt8MultiArray,
	StdMsgsString,
	StdMsgsUInt16,
	StdMsgsUInt16MultiArray,
	StdMsgsUInt32,
	StdMsgsUInt32MultiArray,
	StdMsgsUInt64,
	StdMsgsUInt64MultiArray,
	StdMsgsUInt8,
	StdMsgsUInt8MultiArray,
	StdMsgsHeader,
	GeometryMsgsVector3,
	GeometryMsgsQuaternion,
	GeometryMsgsAccel,
	GeometryMsgsAccelStamped,
	GeometryMsgsAccelWithCovariance,
	GeometryMsgsAccelWithCovarianceStamped,
	GeometryMsgsInertia,
	GeometryMsgsInertiaStamped,
	GeometryMsgsPoint,
	GeometryMsgsPoint32,
	GeometryMsgsPointStamped,
	GeometryMsgsPolygon,
	GeometryMsgsPolygonStamped,
	GeometryMsgsPose,
	GeometryMsgsPose2D,
	GeometryMsgsPoseArray,
	GeometryMsgsPoseStamped,
	GeometryMsgsPoseWithCovariance,
	GeometryMsgsPoseWithCovarianceStamped,
	GeometryMsgsQuaternionStamped,
	GeometryMsgsTransform,
	GeometryMsgsTransformStamped,
	GeometryMsgsTwist,
	GeometryMsgsTwistStamped,
	GeometryMsgsTwistWithCovariance,
	GeometryMsgsTwistWithCovarianceStamped,
	GeometryMsgsVector3Stamped,
	GeometryMsgsWrench,
	GeometryMsgsWrenchStamped,
	SensorMsgsBatteryState,
	SensorMsgsChannelFloat32,
	SensorMsgsCompressedImage,
	SensorMsgsFluidPressure,
	SensorMsgsIlluminance,
	SensorMsgsImage,
	SensorMsgsImu,
	SensorMsgsJointState,
	SensorMsgsJoy,
	SensorMsgsJoyFeedback,
	SensorMsgsJoyFeedbackArray,
	SensorMsgsLaserEcho,
	SensorMsgsLaserScan,
	SensorMsgsMagneticField,
	SensorMsgsMultiDOFJointState,
	SensorMsgsMultiEchoLaserScan,
	SensorMsgsNavSatStatus,
	SensorMsgsNavSatFix,
	SensorMsgsPointCloud,
	SensorMsgsPointField,
	SensorMsgsPointCloud2,
	SensorMsgsRange,
	SensorMsgsRegionOfInterest,
	SensorMsgsCameraInfo,
	SensorMsgsRelativeHumidity,
	SensorMsgsTemperature,
	SensorMsgsTimeReference
};

UENUM()
enum class EAGX_ROS2QosReliability
{
	ReliabilityDefault,
	BestEffort,
	Reliable
};

UENUM()
enum class EAGX_ROS2QosDurability
{
	DurabilityDefault,
	Volatile,
	TransientLocal,
	Transient,
	Persistent
};

UENUM()
enum class EAGX_ROS2QosHistory
{
	HistoryDefault,
	KeepLastHistoryQos,
	KeepAllHistoryQos
};
