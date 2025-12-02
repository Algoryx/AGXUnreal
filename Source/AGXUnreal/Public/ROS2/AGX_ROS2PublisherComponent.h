// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2PublisherBarrier.h"
#include "ROS2/AGX_ROS2Enums.h"
#include "ROS2/AGX_ROS2Qos.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_ROS2PublisherComponent.generated.h"

/**
 * Class representing a ROS2 Publisher used for sending ROS2 messages.
 * A single ROS2 Publisher Component can be used to send messages on multiple topics and message
 * types.
 */
UCLASS(
	ClassGroup = "AGX_ROS2", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Rendering, Replication))
class AGXUNREAL_API UAGX_ROS2PublisherComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_ROS2PublisherComponent();

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
	FROS2PublisherBarrier* GetOrCreateBarrier(EAGX_ROS2MessageType Type, const FString& Topic);

	// AgxMsgs

	UFUNCTION(BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send agx_msgs::Any"))
	bool SendAgxMsgsAny(const FAGX_AgxMsgsAny& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send agx_msgs::AnySequence"))
	bool SendAgxMsgsAnySequence(const FAGX_AgxMsgsAnySequence& Message, const FString& Topic);

	// BuiltinInterfaces

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send builtin_interfaces::Time"))
	bool SendBuiltinInterfacesTime(const FAGX_BuiltinInterfacesTime& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send builtin_interfaces::Duration"))
	bool SendBuiltinInterfacesDuration(
		const FAGX_BuiltinInterfacesDuration& Message, const FString& Topic);

	// RosgraphMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send rosgraph_msgs::Clock"))
	bool SendRosgraphMsgsClock(const FAGX_RosgraphMsgsClock& Message, const FString& Topic);

	// StdMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Bool"))
	bool SendStdMsgsBool(const FAGX_StdMsgsBool& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Byte"))
	bool SendStdMsgsByte(const FAGX_StdMsgsByte& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::ByteMultiArray"))
	bool SendStdMsgsByteMultiArray(const FAGX_StdMsgsByteMultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Char"))
	bool SendStdMsgsChar(const FAGX_StdMsgsChar& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::ColorRGBA"))
	bool SendStdMsgsColorRGBA(const FAGX_StdMsgsColorRGBA& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Empty"))
	bool SendStdMsgsEmpty(const FAGX_StdMsgsEmpty& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Float32"))
	bool SendStdMsgsFloat32(const FAGX_StdMsgsFloat32& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::Float32MultiArray"))
	bool SendStdMsgsFloat32MultiArray(
		const FAGX_StdMsgsFloat32MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Float64"))
	bool SendStdMsgsFloat64(const FAGX_StdMsgsFloat64& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::Float64MultiArray"))
	bool SendStdMsgsFloat64MultiArray(
		const FAGX_StdMsgsFloat64MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Int16"))
	bool SendStdMsgsInt16(const FAGX_StdMsgsInt16& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::Int16MultiArray"))
	bool SendStdMsgsInt16MultiArray(
		const FAGX_StdMsgsInt16MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Int32"))
	bool SendStdMsgsInt32(const FAGX_StdMsgsInt32& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::Int32MultiArray"))
	bool SendStdMsgsInt32MultiArray(
		const FAGX_StdMsgsInt32MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Int64"))
	bool SendStdMsgsInt64(const FAGX_StdMsgsInt64& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::Int64MultiArray"))
	bool SendStdMsgsInt64MultiArray(
		const FAGX_StdMsgsInt64MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Int8"))
	bool SendStdMsgsInt8(const FAGX_StdMsgsInt8& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::Int8MultiArray"))
	bool SendStdMsgsInt8MultiArray(const FAGX_StdMsgsInt8MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::String"))
	bool SendStdMsgsString(const FAGX_StdMsgsString& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::UInt16"))
	bool SendStdMsgsUInt16(const FAGX_StdMsgsUInt16& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::UInt16MultiArray"))
	bool SendStdMsgsUInt16MultiArray(
		const FAGX_StdMsgsUInt16MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::UInt32"))
	bool SendStdMsgsUInt32(const FAGX_StdMsgsUInt32& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::UInt32MultiArray"))
	bool SendStdMsgsUInt32MultiArray(
		const FAGX_StdMsgsUInt32MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::UInt64"))
	bool SendStdMsgsUInt64(const FAGX_StdMsgsUInt64& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::UInt64MultiArray"))
	bool SendStdMsgsUInt64MultiArray(
		const FAGX_StdMsgsUInt64MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::UInt8"))
	bool SendStdMsgsUInt8(const FAGX_StdMsgsUInt8& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send std_msgs::UInt8MultiArray"))
	bool SendStdMsgsUInt8MultiArray(
		const FAGX_StdMsgsUInt8MultiArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send std_msgs::Header"))
	bool SendStdMsgsHeader(const FAGX_StdMsgsHeader& Message, const FString& Topic);

	// GeometryMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Vector3"))
	bool SendGeometryMsgsVector3(const FAGX_GeometryMsgsVector3& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Quaternion"))
	bool SendGeometryMsgsQuaternion(
		const FAGX_GeometryMsgsQuaternion& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Accel"))
	bool SendGeometryMsgsAccel(const FAGX_GeometryMsgsAccel& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::AccelStamped"))
	bool SendGeometryMsgsAccelStamped(
		const FAGX_GeometryMsgsAccelStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::AccelWithCovariance"))
	bool SendGeometryMsgsAccelWithCovariance(
		const FAGX_GeometryMsgsAccelWithCovariance& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::AccelWithCovarianceStamped"))
	bool SendGeometryMsgsAccelWithCovarianceStamped(
		const FAGX_GeometryMsgsAccelWithCovarianceStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Inertia"))
	bool SendGeometryMsgsInertia(const FAGX_GeometryMsgsInertia& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::InertiaStamped"))
	bool SendGeometryMsgsInertiaStamped(
		const FAGX_GeometryMsgsInertiaStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Point"))
	bool SendGeometryMsgsPoint(const FAGX_GeometryMsgsPoint& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Point32"))
	bool SendGeometryMsgsPoint32(const FAGX_GeometryMsgsPoint32& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::PointStamped"))
	bool SendGeometryMsgsPointStamped(
		const FAGX_GeometryMsgsPointStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Polygon"))
	bool SendGeometryMsgsPolygon(const FAGX_GeometryMsgsPolygon& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::PolygonStamped"))
	bool SendGeometryMsgsPolygonStamped(
		const FAGX_GeometryMsgsPolygonStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send geometry_msgs::Pose"))
	bool SendGeometryMsgsPose(const FAGX_GeometryMsgsPose& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Pose2D"))
	bool SendGeometryMsgsPose2D(const FAGX_GeometryMsgsPose2D& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::PoseArray"))
	bool SendGeometryMsgsPoseArray(const FAGX_GeometryMsgsPoseArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::PoseStamped"))
	bool SendGeometryMsgsPoseStamped(
		const FAGX_GeometryMsgsPoseStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::PoseWithCovariance"))
	bool SendGeometryMsgsPoseWithCovariance(
		const FAGX_GeometryMsgsPoseWithCovariance& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::PoseWithCovarianceStamped"))
	bool SendGeometryMsgsPoseWithCovarianceStamped(
		const FAGX_GeometryMsgsPoseWithCovarianceStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::QuaternionStamped"))
	bool SendGeometryMsgsQuaternionStamped(
		const FAGX_GeometryMsgsQuaternionStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Transform"))
	bool SendGeometryMsgsTransform(const FAGX_GeometryMsgsTransform& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::TransformStamped"))
	bool SendGeometryMsgsTransformStamped(
		const FAGX_GeometryMsgsTransformStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Twist"))
	bool SendGeometryMsgsTwist(const FAGX_GeometryMsgsTwist& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::TwistStamped"))
	bool SendGeometryMsgsTwistStamped(
		const FAGX_GeometryMsgsTwistStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::TwistWithCovariance"))
	bool SendGeometryMsgsTwistWithCovariance(
		const FAGX_GeometryMsgsTwistWithCovariance& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::TwistWithCovarianceStamped"))
	bool SendGeometryMsgsTwistWithCovarianceStamped(
		const FAGX_GeometryMsgsTwistWithCovarianceStamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Vector3Stamped"))
	bool SendGeometryMsgsVector3Stamped(
		const FAGX_GeometryMsgsVector3Stamped& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::Wrench"))
	bool SendGeometryMsgsWrench(const FAGX_GeometryMsgsWrench& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send geometry_msgs::WrenchStamped"))
	bool SendGeometryMsgsWrenchStamped(
		const FAGX_GeometryMsgsWrenchStamped& Message, const FString& Topic);

	// SensorMsgs

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::BatteryState"))
	bool SendSensorMsgsBatteryState(
		const FAGX_SensorMsgsBatteryState& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::ChannelFloat32"))
	bool SendSensorMsgsChannelFloat32(
		const FAGX_SensorMsgsChannelFloat32& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::CompressedImage"))
	bool SendSensorMsgsCompressedImage(
		const FAGX_SensorMsgsCompressedImage& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::FluidPressure"))
	bool SendSensorMsgsFluidPressure(
		const FAGX_SensorMsgsFluidPressure& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::Illuminance"))
	bool SendSensorMsgsIlluminance(const FAGX_SensorMsgsIlluminance& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send sensor_msgs::Image"))
	bool SendSensorMsgsImage(const FAGX_SensorMsgsImage& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send sensor_msgs::Imu"))
	bool SendSensorMsgsImu(const FAGX_SensorMsgsImu& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::JointState"))
	bool SendSensorMsgsJointState(const FAGX_SensorMsgsJointState& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send sensor_msgs::Joy"))
	bool SendSensorMsgsJoy(const FAGX_SensorMsgsJoy& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::JoyFeedback"))
	bool SendSensorMsgsJoyFeedback(const FAGX_SensorMsgsJoyFeedback& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::JoyFeedbackArray"))
	bool SendSensorMsgsJoyFeedbackArray(
		const FAGX_SensorMsgsJoyFeedbackArray& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::LaserEcho"))
	bool SendSensorMsgsLaserEcho(const FAGX_SensorMsgsLaserEcho& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::LaserScan"))
	bool SendSensorMsgsLaserScan(const FAGX_SensorMsgsLaserScan& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::MagneticField"))
	bool SendSensorMsgsMagneticField(
		const FAGX_SensorMsgsMagneticField& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::MultiDOFJointState"))
	bool SendSensorMsgsMultiDOFJointState(
		const FAGX_SensorMsgsMultiDOFJointState& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::MultiEchoLaserScan"))
	bool SendSensorMsgsMultiEchoLaserScan(
		const FAGX_SensorMsgsMultiEchoLaserScan& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::NavSatStatus"))
	bool SendSensorMsgsNavSatStatus(
		const FAGX_SensorMsgsNavSatStatus& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::NavSatFix"))
	bool SendSensorMsgsNavSatFix(const FAGX_SensorMsgsNavSatFix& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::PointCloud"))
	bool SendSensorMsgsPointCloud(const FAGX_SensorMsgsPointCloud& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::PointField"))
	bool SendSensorMsgsPointField(const FAGX_SensorMsgsPointField& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::PointCloud2"))
	bool SendSensorMsgsPointCloud2(const FAGX_SensorMsgsPointCloud2& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2", Meta = (DisplayName = "Send sensor_msgs::Range"))
	bool SendSensorMsgsRange(const FAGX_SensorMsgsRange& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::RegionOfInterest"))
	bool SendSensorMsgsRegionOfInterest(
		const FAGX_SensorMsgsRegionOfInterest& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::CameraInfo"))
	bool SendSensorMsgsCameraInfo(const FAGX_SensorMsgsCameraInfo& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::RelativeHumidity"))
	bool SendSensorMsgsRelativeHumidity(
		const FAGX_SensorMsgsRelativeHumidity& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::Temperature"))
	bool SendSensorMsgsTemperature(const FAGX_SensorMsgsTemperature& Message, const FString& Topic);

	UFUNCTION(
		BlueprintCallable, Category = "AGX ROS2",
		Meta = (DisplayName = "Send sensor_msgs::TimeReference"))
	bool SendSensorMsgsTimeReference(
		const FAGX_SensorMsgsTimeReference& Message, const FString& Topic);

private:
#if WITH_EDITOR
	// ~Begin UActorComponent interface.
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	// ~Begin UActorComponent interface.
#endif

	// Key is the Topic.
	TMap<FString, FROS2PublisherBarrier> NativeBarriers;
};
