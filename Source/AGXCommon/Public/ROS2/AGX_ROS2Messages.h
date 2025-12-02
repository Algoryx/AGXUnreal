// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_ROS2Messages.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_ROS2Message
{
	GENERATED_BODY()
};

//
// AgxMsgs
//

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_AgxMsgsAny : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_AgxMsgsAnySequence : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_AgxMsgsAny> Data;
};

//
// BuiltinInterfaces
//

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_BuiltinInterfacesTime : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Sec {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Nanosec {0}; // uint32 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_BuiltinInterfacesDuration : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Sec {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Nanosec {0}; // uint32 not supported by Blueprints.
};

//
// RosgraphMsgs
//

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_RosgraphMsgsClock : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_BuiltinInterfacesTime Clock;
};

//
// StdMsgs
//

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsMultiArrayDimension : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Size {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Stride {0}; // uint32 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsMultiArrayLayout : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_StdMsgsMultiArrayDimension> Dim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 DataOffset {0}; // uint32 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsBool : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	bool Data {false};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsByte : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 Data {0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsByteMultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsChar : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 Data {0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsColorRGBA : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float R {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float G {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float B {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float A {0.f};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsEmpty : public FAGX_ROS2Message
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsFloat32 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Data {0.f};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsFloat32MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsFloat64 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Data {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsFloat64MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt8 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Data {0}; // int8 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt8MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int32> Data; // int8 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt16 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Data {0}; // int16 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt16MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int32> Data; // int16 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt32 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Data {0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt32MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int32> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt64 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Data {0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsInt64MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int64> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt8 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 Data {0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt8MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt16 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Data {0}; // uint16 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt16MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int32> Data; // uint16 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt32 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Data {0}; // uint32 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt32MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int64> Data; // uint32 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt64 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Data {0}; // uint64 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsUInt64MultiArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsMultiArrayLayout Layout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int64> Data; // uint64 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsString : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_StdMsgsHeader : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_BuiltinInterfacesTime Stamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString FrameId;
};

//
// GeometryMsgs
//

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsVector3 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double X {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Y {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Z {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsQuaternion : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double X {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Y {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Z {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double W {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsAccel : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Linear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Angular;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsAccelStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsAccel Accel;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsAccelWithCovariance : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsAccel Accel;

	/** Static arrays not supported by Blueprints. 36 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> Covariance;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsAccelWithCovarianceStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsAccelWithCovariance Accel;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsInertia : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double M {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 COM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Ixx {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Ixy {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Ixz {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Iyy {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Iyz {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Izz {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsInertiaStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsInertia Inertia;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPoint : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double X {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Y {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Z {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPoint32 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float X {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Y {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Z {0.f};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPointStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsPoint Point;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPolygon : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_GeometryMsgsPoint32> Points;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPolygonStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsPolygon Polygon;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPose : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsPoint Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsQuaternion Orientation;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPose2D : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double X {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Y {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Theta {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPoseArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_GeometryMsgsPose> Poses;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPoseStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsPose Pose;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPoseWithCovariance : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsPose Pose;

	/** Static arrays not supported by Blueprints. 36 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> Covariance;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsPoseWithCovarianceStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsPoseWithCovariance Pose;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsQuaternionStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsQuaternion Quaternion;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsTransform : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Translation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsQuaternion Rotation;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsTransformStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString ChildFrameId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsTransform Transform;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsTwist : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Linear;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Angular;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsTwistStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsTwist Twist;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsTwistWithCovariance : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsTwist Twist;

	/** Static arrays not supported by Blueprints. 36 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> Covariance;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsTwistWithCovarianceStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsTwistWithCovariance Twist;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsVector3Stamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Vector;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsWrench : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Force;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 Torque;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_GeometryMsgsWrenchStamped : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsWrench Wrench;
};

//
// SensorMsgs
//

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsBatteryState : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Voltage {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Temperature {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Current {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Charge {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Capacity {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float DesignCapacity {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Percentage {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 PowerSupplyStatus {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 PowerSupplyHealth {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 PowerSupplyTechnology {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	bool Present {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> CellVoltage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> CellTemperature;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString SerialNumber;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsChannelFloat32 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> Values;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsCompressedImage : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Format;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsFluidPressure : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double FluidPressure {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Variance {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsIlluminance : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Illuminance {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Variance {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsImage : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Height {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Width {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Encoding;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 IsBigendian {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Step {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<uint8> Data;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsImu : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsQuaternion Orientation;

	/** Static arrays not supported by Blueprints. 9 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> OrientationCovariance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 AngularVelocity;

	/** Static arrays not supported by Blueprints. 9 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> AngularVelocityCovariance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 LinearAcceleration;

	/** Static arrays not supported by Blueprints. 9 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> LinearAccelerationCovariance;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsJointState : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FString> Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> Position;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> Velocity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> Effort;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsJoy : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> Axes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<int32> Buttons;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsJoyFeedback : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 Type {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 Id {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Intensity {0.f};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsJoyFeedbackArray : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_SensorMsgsJoyFeedback> Array;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsLaserEcho : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> Echoes;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsLaserScan : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float AngleMin {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float AngleMax {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float AngleIncrement {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float TimeIncrement {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float ScanTime {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float RangeMin {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float RangeMax {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> Ranges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<float> Intensities;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsMagneticField : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_GeometryMsgsVector3 MagneticField;

	/** Static arrays not supported by Blueprints. 9 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> MagneticFieldCovariance;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsMultiDOFJointState : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FString> JointNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_GeometryMsgsTransform> Transforms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_GeometryMsgsTwist> Twist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_GeometryMsgsWrench> Wrench;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsMultiEchoLaserScan : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float AngleMin {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float AngleMax {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float AngleIncrement {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float TimeIncrement {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float ScanTime {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float RangeMin {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float RangeMax {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_SensorMsgsLaserEcho> Ranges;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_SensorMsgsLaserEcho> Intensities;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsNavSatStatus : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Status {0}; // int8 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int32 Service {0}; // uint16 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsNavSatFix : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_SensorMsgsNavSatStatus Status;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Latitude {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Longitude {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Altitude {0.0};

	/** Static arrays not supported by Blueprints. 9 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> PositionCovariance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 PositionCovarianceType {0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsPointCloud : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_GeometryMsgsPoint32> Points;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_SensorMsgsChannelFloat32> Channels;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsPointField : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Offset {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 Datatype {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Count {0}; // uint32 not supported by Blueprints.
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsPointCloud2 : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Height {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Width {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<FAGX_SensorMsgsPointField> Fields;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	bool IsBigendian {false};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 PointStep {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 RowStep {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<uint8> Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	bool IsDense {false};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsRange : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	uint8 RadiationType {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float FieldOfView {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float MinRange {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float MaxRange {0.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	float Range {0.f};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsRegionOfInterest : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 XOffset {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 YOffset {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Height {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Width {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	bool DoRectify {false};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsCameraInfo : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Height {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 Width {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString DistortionModel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> D;

	/** Static arrays not supported by Blueprints. 9 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> K;

	/** Static arrays not supported by Blueprints. 9 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> R;

	/** Static arrays not supported by Blueprints. 12 elements is expected.*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	TArray<double> P;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 BinningX {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	int64 BinningY {0}; // uint32 not supported by Blueprints.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_SensorMsgsRegionOfInterest ROI;
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsRelativeHumidity : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double RelativeHumidity {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Variance {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsTemperature : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Temperature {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	double Variance {0.0};
};

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SensorMsgsTimeReference : public FAGX_ROS2Message
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_StdMsgsHeader Header;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FAGX_BuiltinInterfacesTime TimeRef;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX ROS2")
	FString Source;
};
