// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ROS2Utilities.generated.h"

struct FAGX_LidarOutputPositionData;
struct FAGX_LidarOutputPositionIntensityData;
struct FAGX_LidarScanPoint;
struct FAGX_SensorMsgsImage;
struct FAGX_SensorMsgsPointCloud2;

class AGXUNREAL_API FAGX_ROS2Utilities
{
public:
	static FAGX_SensorMsgsImage Convert(
		const TArray<FColor>& Image, double TimeStamp, const FIntPoint& Resolution, bool Grayscale);

	static FAGX_SensorMsgsImage Convert(
		const TArray<FFloat16Color>& Image, double TimeStamp, const FIntPoint& Resolution,
		bool Grayscale);
};

UCLASS(ClassGroup = "AGX ROS2 Utilities")
class AGXUNREAL_API UAGX_ROS2Utilities : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()

    /**
     * Converts a distance from Unreal's unit system [cm] to ROS2's unit system [m].
     *
     * DistanceUnreal - The distance in Unreal's unit system [cm].
     * Returns the converted distance in ROS2's unit system [m].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static double ConvertDistanceToROS(double DistanceUnreal);

    /**
     * Converts a distance from ROS2's unit system [m] to Unreal's unit system [cm].
     *
     * DistanceROS - The distance in ROS2's unit system [m].
     * Returns the converted distance in Unreal's unit system [cm].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static double ConvertDistanceToUnreal(double DistanceROS);

    /**
     * Converts a position from Unreal's coordinate system [cm] to ROS2's coordinate system [m].
     * Unreal uses left-handed coordinate systems, while ROS2 uses right-handed coordinate systems.
     *
     * PosUnreal - The position in Unreal's coordinate system (left-handed) [cm].
     * Returns the converted position in ROS2's coordinate system (right-handed) [m].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FVector ConvertPositionToROS(FVector PosUnreal);

    /**
     * Converts a position from ROS2's coordinate system [m] to Unreal's coordinate system [cm].
     * ROS2 uses right-handed coordinate systems, while Unreal uses left-handed coordinate systems.
     *
     * PosROS - The position in ROS2's coordinate system (right-handed) [m].
     * Returns the converted position in Unreal's coordinate system (left-handed) [cm].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FVector ConvertPositionToUnreal(FVector PosROS);

    /**
     * Converts a velocity from Unreal's unit system [cm/s] to ROS2's unit system [m/s].
		 * ROS2 uses right-handed coordinate systems, while Unreal uses left-handed coordinate systems.
     *
     * VelUnreal - The velocity in Unreal's unit system (left-handed) [cm/s].
     * Returns the converted velocity in ROS2's unit system (right-handed) [m/s].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FVector ConvertVelocityToROS(FVector VelUnreal);

    /**
     * Converts a velocity from ROS2's unit system [m/s] to Unreal's unit system [cm/s].
		 * ROS2 uses right-handed coordinate systems, while Unreal uses left-handed coordinate systems.
     *
     * VelROS - The velocity in ROS2's unit system (right-handed) [m/s].
     * Returns the converted velocity in Unreal's unit system (left-handed) [cm/s].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FVector ConvertVelocityToUnreal(FVector VelROS);

    /**
     * Converts an angular velocity from Unreal's unit system [deg/s] to ROS2's unit system [rad/s].
		 * ROS2 uses right-handed coordinate systems, while Unreal uses left-handed coordinate systems.
     *
     * AngVelUnreal - The angular velocity in Unreal's unit system (left-handed) [deg/s].
     * Returns the converted angular velocity in ROS2's unit system (right-handed) [rad/s].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FVector ConvertAngularVelocityToROS(FVector AngVelUnreal);

    /**
     * Converts an angular velocity from ROS2's unit system [rad/s] to Unreal's unit system [deg/s].
     * ROS2 uses right-handed coordinate systems, while Unreal uses left-handed coordinate systems.
     *
     * AngVelROS - The angular velocity in ROS2's unit system (right-handed) [rad/s].
     * Returns the converted angular velocity in Unreal's unit system (left-handed) [deg/s].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FVector ConvertAngularVelocityToUnreal(FVector AngVelROS);

    /**
     * Converts a rotation from Unreal's coordinate system [deg] to ROS2's coordinate system [rad].
     * ROS2 uses right-handed coordinate systems, while Unreal uses left-handed coordinate systems.
     *
     * RotUnreal - The rotation in Unreal's coordinate system (left-handed) [deg].
     * Returns the converted rotation in ROS2's coordinate system (right-handed) [rad].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FQuat ConvertRotationToROS(FQuat RotUnreal);

    /**
     * Converts a rotation from ROS2's coordinate system [rad] to Unreal's coordinate system [deg].
     * ROS2 uses right-handed coordinate systems, while Unreal uses left-handed coordinate systems.
     *
     * RotROS - The rotation in ROS2's coordinate system (right-handed) [rad].
     * Returns the converted rotation in Unreal's coordinate system (left-handed) [deg].
     */
    UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
    static FQuat ConvertRotationToUnreal(FQuat RotROS);

	/**
	 * Takes a TimeStamp in seconds and converts it into a ROS2 builtin_interfaces::Time message.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX ROS2")
	static FAGX_BuiltinInterfacesTime ConvertTime(double TimeStamp);

	/**
	 * Takes an array of Lidar Scan Points and converts it into a ROS2 sensor_msgs::PointCloud2
	 * message.
	 * The Data member consists of position X, Y, Z and Intensity for each point written as either
	 * floats or doubles in little endian layout, i.e. 16 or 32 bytes per point.
	 *
	 * DoublePrecision - use double precision type when writing the X, Y, Z and intensity data. If
	 * set to false, single precision (float) is used.
	 *
	 * ROSCoordinates - convert points to use ROS2 coordinate system instead of Unreal's coordinate
	 * system.
	 *
	 * FrameId - corresponds to the frame_id of the std_msgs::Header message.
	 * If not set, it will be an empty string.
	 *
	 * Note that all invalid points, such as points representing scan misses, are ignored.
	 * This means that the sensor_msgs::PointCloud2 message created by this function is always
	 * dense.
	 *
	 * The timestamp written to the Header member of the sensor_msgs::PointCloud2 message is set to
	 * the timestamp of the first valid Point in the given array, even if other points have been
	 * generated at later timestamps.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX ROS2")
	static FAGX_SensorMsgsPointCloud2 ConvertXYZ(
		const TArray<FAGX_LidarScanPoint>& Points, bool DoublePrecision = false,
		bool ROSCoordinates = true, const FString& FrameId = "");

	/**
	 * Takes an array of Lidar Scan Points and converts it into a ROS2 sensor_msgs::PointCloud2
	 * message.
	 * The Data member consists of AngleX [rad] (double), AngleY [rad] (double), time of flight
	 * (TOF) [ps] (uint32) and Intensity (double) for each point in little endian layout, i.e. 28
	 * bytes per point. The speed of light used for the TOF calculation is the speed of light in
	 * vacuum.
	 *
	 * (Optional) the FrameId parameter corresponds to the frame_id of the std_msgs::Header message.
	 * If not set, it will be an empty string.
	 *
	 * Note that all invalid points, such as points representing scan misses, are ignored.
	 * This means that the sensor_msgs::PointCloud2 message created by this function is always
	 * dense.
	 *
	 * The timestamp written to the Header member of the sensor_msgs::PointCloud2 message is set to
	 * the timestamp of the first valid Point in the given array, even if other points have been
	 * generated at later timestamps.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Lidar")
	static FAGX_SensorMsgsPointCloud2 ConvertAnglesTOF(
		const TArray<FAGX_LidarScanPoint>& Points, const FString& FrameId = "");

	/**
	 * Takes an array of Lidar Output Position Data and converts it into a ROS2
	 * sensor_msgs::PointCloud2 message. The Data member of the ROS2 message consists of position x,
	 * y, z [m] for each point written as float's (32 bit) in little endian layout,
	 * i.e. 12 bytes per point.
	 *
	 * It is assumed that the given Output Data is dense, i.e. that no point misses are included.
	 * 
	 * The points written will be in ROS2 coordinates, i.e. right-handed and z up.
	 *
	 * The timestamp written to the Header member of the sensor_msgs::PointCloud2 message is set
	 * according to the given timestamp.
	 *
	 * (Optional) the FrameId parameter corresponds to the frame_id of the std_msgs::Header message.
	 * If not set, it will be an empty string.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
	static FAGX_SensorMsgsPointCloud2 ConvertPositionData(
		const TArray<FAGX_LidarOutputPositionData>& Data, double TimeStamp,
		const FString& FrameId = "");

	/**
	 * Takes an array of Lidar Output Position Intensity Data and converts it into a ROS2
	 * sensor_msgs::PointCloud2 message. The Data member of the ROS2 message consists of position x,
	 * y, z [m] and Intensity for each point written as float's (32 bit) in little endian layout,
	 * i.e. 16 bytes per point.
	 *
	 * It is assumed that the given Output Data is dense, i.e. that no point misses are included.
	 *
	 * The points written will be in ROS2 coordinates, i.e. right-handed and z up.
	 *
	 * The timestamp written to the Header member of the sensor_msgs::PointCloud2 message is set
	 * according to the given timestamp.
	 *
	 * (Optional) the FrameId parameter corresponds to the frame_id of the std_msgs::Header message.
	 * If not set, it will be an empty string.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX ROS2")
	static FAGX_SensorMsgsPointCloud2 ConvertPositionIntensityData(
		const TArray<FAGX_LidarOutputPositionIntensityData>& Data, double TimeStamp,
		const FString& FrameId = "");
};
