// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2SubscriberBarrier.h"
#include "ROS2/AGX_ROS2Enums.h"
#include "ROS2/AGX_ROS2Qos.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_ROS2SubscriberComponent.generated.h"

/**
 * Class representing a ROS2 Subscriber used for receiving ROS2 messages.
 * A single ROS2 Subscriber Component can be used to receive messages on multiple topics and message
 * types.
 */
UCLASS(
	ClassGroup = "AGX_ROS2", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2SubscriberComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2SubscriberComponent();

	/**
	 * Struct containing Quality of Service (QOS) settings.
	 * Reliability, durability, history and historyDepth are supported.
	 * By default the QOS settings are the same as in ROS2.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	FAGX_ROS2Qos Qos;

	/**
	 * Specifies which DDS domain ID to use (advanced).
	 * Only Publishers/Subscribers using the same Domain ID can communicate, meaning it can be used
	 * to isolate groups of Publishers/Subscribers from other groups.
	 * Default value for ROS2 is 0.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX ROS2")
	uint8 DomainID {0};

	/**
	 * Returns a Barrier object from the Barrier pool, given the specific Topic.
	 * If no previous Barrier exists for the passed Topic, a new one is created and stored.
	 */
	FROS2SubscriberBarrier* GetOrCreateBarrier(EAGX_ROS2MessageType Type, const FString& Topic);

	// AgxMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive agx_msgs::Any"))
	bool ReceiveAgxMsgsAny(FAGX_AgxMsgsAny& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive agx_msgs::AnySequence"))
	bool ReceiveAgxMsgsAnySequence(FAGX_AgxMsgsAnySequence& OutMessage, const FString& Topic);

	// BuiltinInterfaces

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive builtin_interfaces::Time"))
	bool ReceiveBuiltinInterfacesTime(FAGX_BuiltinInterfacesTime& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive builtin_interfaces::Duration"))
	bool ReceiveBuiltinInterfacesDuration(
		FAGX_BuiltinInterfacesDuration& OutMessage, const FString& Topic);

	// RosgraphMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive rosgraph_msgs::Clock"))
	bool ReceiveRosgraphMsgsClock(FAGX_RosgraphMsgsClock& OutMessage, const FString& Topic);

	// StdMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Bool"))
	bool ReceiveStdMsgsBool(FAGX_StdMsgsBool& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Byte"))
	bool ReceiveStdMsgsByte(FAGX_StdMsgsByte& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::ByteMultiArray"))
	bool ReceiveStdMsgsByteMultiArray(FAGX_StdMsgsByteMultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Char"))
	bool ReceiveStdMsgsChar(FAGX_StdMsgsChar& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::ColorRGBA"))
	bool ReceiveStdMsgsColorRGBA(FAGX_StdMsgsColorRGBA& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Empty"))
	bool ReceiveStdMsgsEmpty(FAGX_StdMsgsEmpty& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Float32"))
	bool ReceiveStdMsgsFloat32(FAGX_StdMsgsFloat32& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Float32MultiArray"))
	bool ReceiveStdMsgsFloat32MultiArray(
		FAGX_StdMsgsFloat32MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Float64"))
	bool ReceiveStdMsgsFloat64(FAGX_StdMsgsFloat64& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Float64MultiArray"))
	bool ReceiveStdMsgsFloat64MultiArray(
		FAGX_StdMsgsFloat64MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Int16"))
	bool ReceiveStdMsgsInt16(FAGX_StdMsgsInt16& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Int16MultiArray"))
	bool ReceiveStdMsgsInt16MultiArray(
		FAGX_StdMsgsInt16MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Int32"))
	bool ReceiveStdMsgsInt32(FAGX_StdMsgsInt32& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Int32MultiArray"))
	bool ReceiveStdMsgsInt32MultiArray(
		FAGX_StdMsgsInt32MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Int64"))
	bool ReceiveStdMsgsInt64(FAGX_StdMsgsInt64& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Int64MultiArray"))
	bool ReceiveStdMsgsInt64MultiArray(
		FAGX_StdMsgsInt64MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Int8"))
	bool ReceiveStdMsgsInt8(FAGX_StdMsgsInt8& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::Int8MultiArray"))
	bool ReceiveStdMsgsInt8MultiArray(FAGX_StdMsgsInt8MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::String"))
	bool ReceiveStdMsgsString(FAGX_StdMsgsString& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::UInt16"))
	bool ReceiveStdMsgsUInt16(FAGX_StdMsgsUInt16& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::UInt16MultiArray"))
	bool ReceiveStdMsgsUInt16MultiArray(
		FAGX_StdMsgsUInt16MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::UInt32"))
	bool ReceiveStdMsgsUInt32(FAGX_StdMsgsUInt32& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::UInt32MultiArray"))
	bool ReceiveStdMsgsUInt32MultiArray(
		FAGX_StdMsgsUInt32MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::UInt64"))
	bool ReceiveStdMsgsUInt64(FAGX_StdMsgsUInt64& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::UInt64MultiArray"))
	bool ReceiveStdMsgsUInt64MultiArray(
		FAGX_StdMsgsUInt64MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::UInt8"))
	bool ReceiveStdMsgsUInt8(FAGX_StdMsgsUInt8& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive std_msgs::UInt8MultiArray"))
	bool ReceiveStdMsgsUInt8MultiArray(
		FAGX_StdMsgsUInt8MultiArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive std_msgs::Header"))
	bool ReceiveStdMsgsHeader(FAGX_StdMsgsHeader& OutMessage, const FString& Topic);

	// GeometryMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Vector3"))
	bool ReceiveGeometryMsgsVector3(FAGX_GeometryMsgsVector3& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Quaternion"))
	bool ReceiveGeometryMsgsQuaternion(
		FAGX_GeometryMsgsQuaternion& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Accel"))
	bool ReceiveGeometryMsgsAccel(FAGX_GeometryMsgsAccel& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::AccelStamped"))
	bool ReceiveGeometryMsgsAccelStamped(
		FAGX_GeometryMsgsAccelStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::AccelWithCovariance"))
	bool ReceiveGeometryMsgsAccelWithCovariance(
		FAGX_GeometryMsgsAccelWithCovariance& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::AccelWithCovarianceStamped"))
	bool ReceiveGeometryMsgsAccelWithCovarianceStamped(
		FAGX_GeometryMsgsAccelWithCovarianceStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Inertia"))
	bool ReceiveGeometryMsgsInertia(FAGX_GeometryMsgsInertia& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::InertiaStamped"))
	bool ReceiveGeometryMsgsInertiaStamped(
		FAGX_GeometryMsgsInertiaStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Point"))
	bool ReceiveGeometryMsgsPoint(FAGX_GeometryMsgsPoint& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Point32"))
	bool ReceiveGeometryMsgsPoint32(FAGX_GeometryMsgsPoint32& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::PointStamped"))
	bool ReceiveGeometryMsgsPointStamped(
		FAGX_GeometryMsgsPointStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Polygon"))
	bool ReceiveGeometryMsgsPolygon(FAGX_GeometryMsgsPolygon& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::PolygonStamped"))
	bool ReceiveGeometryMsgsPolygonStamped(
		FAGX_GeometryMsgsPolygonStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Pose"))
	bool ReceiveGeometryMsgsPose(FAGX_GeometryMsgsPose& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Pose2D"))
	bool ReceiveGeometryMsgsPose2D(FAGX_GeometryMsgsPose2D& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::PoseArray"))
	bool ReceiveGeometryMsgsPoseArray(FAGX_GeometryMsgsPoseArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::PoseStamped"))
	bool ReceiveGeometryMsgsPoseStamped(
		FAGX_GeometryMsgsPoseStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::PoseWithCovariance"))
	bool ReceiveGeometryMsgsPoseWithCovariance(
		FAGX_GeometryMsgsPoseWithCovariance& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::PoseWithCovarianceStamped"))
	bool ReceiveGeometryMsgsPoseWithCovarianceStamped(
		FAGX_GeometryMsgsPoseWithCovarianceStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::QuaternionStamped"))
	bool ReceiveGeometryMsgsQuaternionStamped(
		FAGX_GeometryMsgsQuaternionStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Transform"))
	bool ReceiveGeometryMsgsTransform(FAGX_GeometryMsgsTransform& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::TransformStamped"))
	bool ReceiveGeometryMsgsTransformStamped(
		FAGX_GeometryMsgsTransformStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Twist"))
	bool ReceiveGeometryMsgsTwist(FAGX_GeometryMsgsTwist& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::TwistStamped"))
	bool ReceiveGeometryMsgsTwistStamped(
		FAGX_GeometryMsgsTwistStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::TwistWithCovariance"))
	bool ReceiveGeometryMsgsTwistWithCovariance(
		FAGX_GeometryMsgsTwistWithCovariance& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::TwistWithCovarianceStamped"))
	bool ReceiveGeometryMsgsTwistWithCovarianceStamped(
		FAGX_GeometryMsgsTwistWithCovarianceStamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Vector3Stamped"))
	bool ReceiveGeometryMsgsVector3Stamped(
		FAGX_GeometryMsgsVector3Stamped& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::Wrench"))
	bool ReceiveGeometryMsgsWrench(FAGX_GeometryMsgsWrench& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive geometry_msgs::WrenchStamped"))
	bool ReceiveGeometryMsgsWrenchStamped(
		FAGX_GeometryMsgsWrenchStamped& OutMessage, const FString& Topic);

	// SensorMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::BatteryState"))
	bool ReceiveSensorMsgsBatteryState(
		FAGX_SensorMsgsBatteryState& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::ChannelFloat32"))
	bool ReceiveSensorMsgsChannelFloat32(
		FAGX_SensorMsgsChannelFloat32& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::CompressedImage"))
	bool ReceiveSensorMsgsCompressedImage(
		FAGX_SensorMsgsCompressedImage& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::FluidPressure"))
	bool ReceiveSensorMsgsFluidPressure(
		FAGX_SensorMsgsFluidPressure& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::Illuminance"))
	bool ReceiveSensorMsgsIlluminance(FAGX_SensorMsgsIlluminance& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::Image"))
	bool ReceiveSensorMsgsImage(FAGX_SensorMsgsImage& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive sensor_msgs::Imu"))
	bool ReceiveSensorMsgsImu(FAGX_SensorMsgsImu& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::JointState"))
	bool ReceiveSensorMsgsJointState(FAGX_SensorMsgsJointState& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Receive sensor_msgs::Joy"))
	bool ReceiveSensorMsgsJoy(FAGX_SensorMsgsJoy& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::JoyFeedback"))
	bool ReceiveSensorMsgsJoyFeedback(FAGX_SensorMsgsJoyFeedback& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::JoyFeedbackArray"))
	bool ReceiveSensorMsgsJoyFeedbackArray(
		FAGX_SensorMsgsJoyFeedbackArray& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::LaserEcho"))
	bool ReceiveSensorMsgsLaserEcho(FAGX_SensorMsgsLaserEcho& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::LaserScan"))
	bool ReceiveSensorMsgsLaserScan(FAGX_SensorMsgsLaserScan& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::MagneticField"))
	bool ReceiveSensorMsgsMagneticField(
		FAGX_SensorMsgsMagneticField& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::MultiDOFJointState"))
	bool ReceiveSensorMsgsMultiDOFJointState(
		FAGX_SensorMsgsMultiDOFJointState& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::MultiEchoLaserScan"))
	bool ReceiveSensorMsgsMultiEchoLaserScan(
		FAGX_SensorMsgsMultiEchoLaserScan& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::NavSatStatus"))
	bool ReceiveSensorMsgsNavSatStatus(
		FAGX_SensorMsgsNavSatStatus& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::NavSatFix"))
	bool ReceiveSensorMsgsNavSatFix(FAGX_SensorMsgsNavSatFix& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::PointCloud"))
	bool ReceiveSensorMsgsPointCloud(FAGX_SensorMsgsPointCloud& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::PointField"))
	bool ReceiveSensorMsgsPointField(FAGX_SensorMsgsPointField& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::PointCloud2"))
	bool ReceiveSensorMsgsPointCloud2(FAGX_SensorMsgsPointCloud2& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::Range"))
	bool ReceiveSensorMsgsRange(FAGX_SensorMsgsRange& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::RegionOfInterest"))
	bool ReceiveSensorMsgsRegionOfInterest(
		FAGX_SensorMsgsRegionOfInterest& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::CameraInfo"))
	bool ReceiveSensorMsgsCameraInfo(FAGX_SensorMsgsCameraInfo& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::RelativeHumidity"))
	bool ReceiveSensorMsgsRelativeHumidity(
		FAGX_SensorMsgsRelativeHumidity& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::Temperature"))
	bool ReceiveSensorMsgsTemperature(FAGX_SensorMsgsTemperature& OutMessage, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Receive sensor_msgs::TimeReference"))
	bool ReceiveSensorMsgsTimeReference(
		FAGX_SensorMsgsTimeReference& OutMessage, const FString& Topic);

private:
#if WITH_EDITOR
	// ~Begin UActorComponent interface.
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	// ~Begin UActorComponent interface.
#endif

	// Key is the Topic.
	TMap<FString, FROS2SubscriberBarrier> NativeBarriers;
};
