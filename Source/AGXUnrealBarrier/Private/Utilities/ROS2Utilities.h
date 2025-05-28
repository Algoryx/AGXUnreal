// Copyright 2024, Algoryx Simulation AB.

#pragma once

namespace AGX_ROS2Utilities
{
	/**
	 * These FreeContainsers functions are similar to the agxUtils::FreeContainerMemory dance we do
	 * for agx container types. That is, we need to free cross-dll-broundery container memory in a
	 * way that does not crash due to different allocators/deallocators being used in agx and
	 * Unreal.
	 * The rule here is: if a message has a container type that does heap allocation, we
	 * need to do this dance.
	 */

	template <typename ContainerType>
	inline void FreeSingleContainer(ContainerType& C)
	{
		C.resize(0);
		C.shrink_to_fit();
	}

	template <typename MessageType>
	inline void FreeContainers(MessageType&)
	{
		// Base implementation, do nothing.
	}

	//
	// AgxMsgs
	//

	template <>
	inline void FreeContainers(agxROS2::agxMsgs::Any& Msg)
	{
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::agxMsgs::AnySequence& Msg)
	{
		for (auto& D : Msg.data)
			FreeContainers(D);

		FreeSingleContainer(Msg.data);
	}

	//
	// StdMsgs
	//

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::MultiArrayDimension& Msg)
	{
		FreeSingleContainer(Msg.label);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::MultiArrayLayout& Msg)
	{
		for (auto& D : Msg.dim)
			FreeContainers(D);

		FreeSingleContainer(Msg.dim);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::ByteMultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::Float32MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::Float64MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::Int16MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::Int32MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::Int64MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::Int8MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::String& Msg)
	{
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::UInt16MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::UInt32MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::UInt64MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::UInt8MultiArray& Msg)
	{
		FreeContainers(Msg.layout);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::stdMsgs::Header& Msg)
	{
		FreeSingleContainer(Msg.frame_id);
	}

	//
	// GeometryMsgs
	//

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::AccelStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::AccelWithCovarianceStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::InertiaStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::PointStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::Polygon& Msg)
	{
		FreeSingleContainer(Msg.points);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::PolygonStamped& Msg)
	{
		FreeContainers(Msg.header);
		FreeContainers(Msg.polygon);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::PoseArray& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.poses);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::PoseStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::PoseWithCovarianceStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::QuaternionStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::TransformStamped& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.child_frame_id);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::TwistStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::TwistWithCovariance& Msg)
	{
		FreeContainers(Msg.twist);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::TwistWithCovarianceStamped& Msg)
	{
		FreeContainers(Msg.header);
		FreeContainers(Msg.twist);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::Vector3Stamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::geometryMsgs::WrenchStamped& Msg)
	{
		FreeContainers(Msg.header);
	}

	//
	// SensorMsgs
	//

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::BatteryState& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.cell_voltage);
		FreeSingleContainer(Msg.cell_temperature);
		FreeSingleContainer(Msg.location);
		FreeSingleContainer(Msg.serial_number);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::ChannelFloat32& Msg)
	{
		FreeSingleContainer(Msg.name);
		FreeSingleContainer(Msg.values);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::CompressedImage& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.format);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::FluidPressure& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::Illuminance& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::Image& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.encoding);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::Imu& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::JointState& Msg)
	{
		FreeContainers(Msg.header);
		for (auto& N : Msg.name)
			FreeSingleContainer(N);

		FreeSingleContainer(Msg.name);
		FreeSingleContainer(Msg.position);
		FreeSingleContainer(Msg.velocity);
		FreeSingleContainer(Msg.effort);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::Joy& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.axes);
		FreeSingleContainer(Msg.buttons);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::JoyFeedbackArray& Msg)
	{
		FreeSingleContainer(Msg.array);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::LaserEcho& Msg)
	{
		FreeSingleContainer(Msg.echoes);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::LaserScan& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.ranges);
		FreeSingleContainer(Msg.intensities);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::MagneticField& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::MultiDOFJointState& Msg)
	{
		FreeContainers(Msg.header);
		for (auto& S : Msg.joint_names)
			FreeSingleContainer(S);

		FreeSingleContainer(Msg.joint_names);
		FreeSingleContainer(Msg.transforms);
		FreeSingleContainer(Msg.twist);
		FreeSingleContainer(Msg.wrench);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::MultiEchoLaserScan& Msg)
	{
		FreeContainers(Msg.header);
		for (auto& R : Msg.ranges)
			FreeContainers(R);

		FreeSingleContainer(Msg.ranges);
		for (auto& R : Msg.intensities)
			FreeContainers(R);

		FreeSingleContainer(Msg.intensities);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::NavSatFix& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::PointCloud& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.points);
		for (auto& C : Msg.channels)
			FreeContainers(C);

		FreeSingleContainer(Msg.channels);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::PointField& Msg)
	{
		FreeSingleContainer(Msg.name);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::PointCloud2& Msg)
	{
		FreeContainers(Msg.header);
		for (auto& F : Msg.fields)
			FreeContainers(F);

		FreeSingleContainer(Msg.fields);
		FreeSingleContainer(Msg.data);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::Range& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::CameraInfo& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.distortion_model);
		FreeSingleContainer(Msg.d);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::RelativeHumidity& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::Temperature& Msg)
	{
		FreeContainers(Msg.header);
	}

	template <>
	inline void FreeContainers(agxROS2::sensorMsgs::TimeReference& Msg)
	{
		FreeContainers(Msg.header);
		FreeSingleContainer(Msg.source);
	}
}
