// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/AGX_ROS2Qos.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxROS2/MessageTypes.h>
#include <agxROS2/Qos.h>
#include "EndAGXIncludes.h"

//
// Qos
//

inline agxROS2::QOS_RELIABILITY Convert(EAGX_ROS2QosReliability Reliability)
{
	switch (Reliability)
	{
		case EAGX_ROS2QosReliability::ReliabilityDefault:
			return agxROS2::QOS_RELIABILITY::DEFAULT;
		case EAGX_ROS2QosReliability::BestEffort:
			return agxROS2::QOS_RELIABILITY::BEST_EFFORT;
		case EAGX_ROS2QosReliability::Reliable:
			return agxROS2::QOS_RELIABILITY::RELIABLE;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported Reliability QOS passed to Convert() function. Default Reliability QOS "
			 "will be used."));
	return agxROS2::QOS_RELIABILITY::DEFAULT;
}

inline agxROS2::QOS_DURABILITY Convert(EAGX_ROS2QosDurability Durability)
{
	switch (Durability)
	{
		case EAGX_ROS2QosDurability::DurabilityDefault:
			return agxROS2::QOS_DURABILITY::DEFAULT;
		case EAGX_ROS2QosDurability::Volatile:
			return agxROS2::QOS_DURABILITY::VOLATILE;
		case EAGX_ROS2QosDurability::TransientLocal:
			return agxROS2::QOS_DURABILITY::TRANSIENT_LOCAL;
		case EAGX_ROS2QosDurability::Transient:
			return agxROS2::QOS_DURABILITY::TRANSIENT;
		case EAGX_ROS2QosDurability::Persistent:
			return agxROS2::QOS_DURABILITY::PERSISTENT;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported Durability QOS passed to Convert() function. Default Durability QOS "
			 "will be used."));
	return agxROS2::QOS_DURABILITY::DEFAULT;
}

inline agxROS2::QOS_HISTORY Convert(EAGX_ROS2QosHistory History)
{
	switch (History)
	{
		case EAGX_ROS2QosHistory::HistoryDefault:
			return agxROS2::QOS_HISTORY::DEFAULT;
		case EAGX_ROS2QosHistory::KeepLastHistoryQos:
			return agxROS2::QOS_HISTORY::KEEP_LAST_HISTORY_QOS;
		case EAGX_ROS2QosHistory::KeepAllHistoryQos:
			return agxROS2::QOS_HISTORY::KEEP_ALL_HISTORY_QOS;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unsupported History QOS passed to Convert() function. Default History QOS "
			 "will be used."));
	return agxROS2::QOS_HISTORY::DEFAULT;
}

inline agxROS2::QOS Convert(const FAGX_ROS2Qos& Qos)
{
	agxROS2::QOS QosROS2;
	QosROS2.reliability = Convert(Qos.Reliability);
	QosROS2.durability = Convert(Qos.Durability);
	QosROS2.history = Convert(Qos.History);
	QosROS2.historyDepth = Qos.HistoryDepth;

	return QosROS2;
}

//
// AgxMsgs
//

inline FAGX_AgxMsgsAny Convert(const agxROS2::agxMsgs::Any& InMsg)
{
	FAGX_AgxMsgsAny Msg;

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

inline FAGX_AgxMsgsAnySequence Convert(const agxROS2::agxMsgs::AnySequence& InMsg)
{
	FAGX_AgxMsgsAnySequence Msg;

	Msg.Data.SetNum(InMsg.data.size());
	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = Convert(InMsg.data[i]);
	}

	return Msg;
}

inline agxROS2::agxMsgs::Any Convert(const FAGX_AgxMsgsAny& InMsg)
{
	agxROS2::agxMsgs::Any Msg;

	Msg.data.reserve(InMsg.Data.Num());
	for (const auto& Element : InMsg.Data)
	{
		Msg.data.push_back(Element);
	}

	return Msg;
}

inline agxROS2::agxMsgs::AnySequence Convert(const FAGX_AgxMsgsAnySequence& InMsg)
{
	agxROS2::agxMsgs::AnySequence Msg;

	Msg.data.reserve(InMsg.Data.Num());
	for (const auto& AnyElement : InMsg.Data)
	{
		Msg.data.emplace_back(Convert(AnyElement));
	}

	return Msg;
}

//
// BuiltinInterfaces
//

inline FAGX_BuiltinInterfacesTime Convert(const agxROS2::builtinInterfaces::Time& InMsg)
{
	FAGX_BuiltinInterfacesTime Msg;
	Msg.Sec = InMsg.sec;
	Msg.Nanosec = static_cast<int64>(InMsg.nanosec);
	return Msg;
}

inline FAGX_BuiltinInterfacesDuration Convert(const agxROS2::builtinInterfaces::Duration& InMsg)
{
	FAGX_BuiltinInterfacesDuration Msg;
	Msg.Sec = InMsg.sec;
	Msg.Nanosec = static_cast<int64>(InMsg.nanosec);
	return Msg;
}

inline agxROS2::builtinInterfaces::Time Convert(const FAGX_BuiltinInterfacesTime& InMsg)
{
	agxROS2::builtinInterfaces::Time Msg;
	Msg.sec = InMsg.Sec;
	Msg.nanosec = static_cast<uint32_t>(InMsg.Nanosec);
	return Msg;
}

inline agxROS2::builtinInterfaces::Duration Convert(const FAGX_BuiltinInterfacesDuration& InMsg)
{
	agxROS2::builtinInterfaces::Duration Msg;
	Msg.sec = InMsg.Sec;
	Msg.nanosec = static_cast<uint32_t>(InMsg.Nanosec);
	return Msg;
}

//
// RosgraphMsgs
//

inline FAGX_RosgraphMsgsClock Convert(const agxROS2::rosgraphMsgs::Clock& InMsg)
{
	FAGX_RosgraphMsgsClock Msg;
	Msg.Clock = Convert(InMsg.clock);
	return Msg;
}

inline agxROS2::rosgraphMsgs::Clock Convert(const FAGX_RosgraphMsgsClock& InMsg)
{
	agxROS2::rosgraphMsgs::Clock Msg;
	Msg.clock = Convert(InMsg.Clock);
	return Msg;
}

//
// StdMsgs
//

inline FAGX_StdMsgsMultiArrayDimension Convert(const agxROS2::stdMsgs::MultiArrayDimension& InMsg)
{
	FAGX_StdMsgsMultiArrayDimension Msg;
	Msg.Label = FString(InMsg.label.c_str());
	Msg.Size = static_cast<int64>(InMsg.size);
	Msg.Stride = static_cast<int64>(InMsg.stride);
	return Msg;
}

inline FAGX_StdMsgsMultiArrayLayout Convert(const agxROS2::stdMsgs::MultiArrayLayout& InMsg)
{
	FAGX_StdMsgsMultiArrayLayout Msg;
	Msg.DataOffset = static_cast<int64>(InMsg.data_offset);

	for (const auto& dimension : InMsg.dim)
	{
		Msg.Dim.Add(Convert(dimension));
	}

	return Msg;
}

inline FAGX_StdMsgsBool Convert(const agxROS2::stdMsgs::Bool& InMsg)
{
	FAGX_StdMsgsBool Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsByte Convert(const agxROS2::stdMsgs::Byte& InMsg)
{
	FAGX_StdMsgsByte Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsByteMultiArray Convert(const agxROS2::stdMsgs::ByteMultiArray& InMsg)
{
	FAGX_StdMsgsByteMultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

inline FAGX_StdMsgsChar Convert(const agxROS2::stdMsgs::Char& InMsg)
{
	FAGX_StdMsgsChar Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsColorRGBA Convert(const agxROS2::stdMsgs::ColorRGBA& InMsg)
{
	FAGX_StdMsgsColorRGBA Msg;
	Msg.R = InMsg.r;
	Msg.G = InMsg.g;
	Msg.B = InMsg.b;
	Msg.A = InMsg.a;
	return Msg;
}

inline FAGX_StdMsgsEmpty Convert(const agxROS2::stdMsgs::Empty& InMsg)
{
	FAGX_StdMsgsEmpty Msg;
	return Msg;
}

inline FAGX_StdMsgsFloat32 Convert(const agxROS2::stdMsgs::Float32& InMsg)
{
	FAGX_StdMsgsFloat32 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsFloat32MultiArray Convert(const agxROS2::stdMsgs::Float32MultiArray& InMsg)
{
	FAGX_StdMsgsFloat32MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

inline FAGX_StdMsgsFloat64 Convert(const agxROS2::stdMsgs::Float64& InMsg)
{
	FAGX_StdMsgsFloat64 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsFloat64MultiArray Convert(const agxROS2::stdMsgs::Float64MultiArray& InMsg)
{
	FAGX_StdMsgsFloat64MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

inline FAGX_StdMsgsInt16 Convert(const agxROS2::stdMsgs::Int16& InMsg)
{
	FAGX_StdMsgsInt16 Msg;
	Msg.Data = static_cast<int32>(InMsg.data);
	return Msg;
}

inline FAGX_StdMsgsInt16MultiArray Convert(const agxROS2::stdMsgs::Int16MultiArray& InMsg)
{
	FAGX_StdMsgsInt16MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int32>(InMsg.data[i]);
	}

	return Msg;
}

inline FAGX_StdMsgsInt32 Convert(const agxROS2::stdMsgs::Int32& InMsg)
{
	FAGX_StdMsgsInt32 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsInt32MultiArray Convert(const agxROS2::stdMsgs::Int32MultiArray& InMsg)
{
	FAGX_StdMsgsInt32MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

inline FAGX_StdMsgsInt64 Convert(const agxROS2::stdMsgs::Int64& InMsg)
{
	FAGX_StdMsgsInt64 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsInt64MultiArray Convert(const agxROS2::stdMsgs::Int64MultiArray& InMsg)
{
	FAGX_StdMsgsInt64MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

inline FAGX_StdMsgsInt8 Convert(const agxROS2::stdMsgs::Int8& InMsg)
{
	FAGX_StdMsgsInt8 Msg;
	Msg.Data = static_cast<int32>(InMsg.data);
	return Msg;
}

inline FAGX_StdMsgsInt8MultiArray Convert(const agxROS2::stdMsgs::Int8MultiArray& InMsg)
{
	FAGX_StdMsgsInt8MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int32>(InMsg.data[i]);
	}

	return Msg;
}

inline FAGX_StdMsgsString Convert(const agxROS2::stdMsgs::String& InMsg)
{
	FAGX_StdMsgsString Msg;
	Msg.Data = FString(InMsg.data.c_str());
	return Msg;
}

inline FAGX_StdMsgsUInt16 Convert(const agxROS2::stdMsgs::UInt16& InMsg)
{
	FAGX_StdMsgsUInt16 Msg;
	Msg.Data = static_cast<int32>(InMsg.data);
	return Msg;
}

inline FAGX_StdMsgsUInt16MultiArray Convert(const agxROS2::stdMsgs::UInt16MultiArray& InMsg)
{
	FAGX_StdMsgsUInt16MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int32>(InMsg.data[i]);
	}

	return Msg;
}

inline FAGX_StdMsgsUInt32 Convert(const agxROS2::stdMsgs::UInt32& InMsg)
{
	FAGX_StdMsgsUInt32 Msg;
	Msg.Data = static_cast<int64>(InMsg.data);
	return Msg;
}

inline FAGX_StdMsgsUInt32MultiArray Convert(const agxROS2::stdMsgs::UInt32MultiArray& InMsg)
{
	FAGX_StdMsgsUInt32MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.SetNum(InMsg.data.size());
	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int64>(InMsg.data[i]);
	}

	return Msg;
}

inline FAGX_StdMsgsUInt64 Convert(const agxROS2::stdMsgs::UInt64& InMsg)
{
	FAGX_StdMsgsUInt64 Msg;
	Msg.Data = static_cast<int64>(InMsg.data);
	return Msg;
}

inline FAGX_StdMsgsUInt64MultiArray Convert(const agxROS2::stdMsgs::UInt64MultiArray& InMsg)
{
	FAGX_StdMsgsUInt64MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);
	Msg.Data.SetNum(InMsg.data.size());

	for (int32 i = 0; i < InMsg.data.size(); ++i)
	{
		Msg.Data[i] = static_cast<int64>(InMsg.data[i]);
	}

	return Msg;
}

inline FAGX_StdMsgsUInt8 Convert(const agxROS2::stdMsgs::UInt8& InMsg)
{
	FAGX_StdMsgsUInt8 Msg;
	Msg.Data = InMsg.data;
	return Msg;
}

inline FAGX_StdMsgsUInt8MultiArray Convert(const agxROS2::stdMsgs::UInt8MultiArray& InMsg)
{
	FAGX_StdMsgsUInt8MultiArray Msg;
	Msg.Layout = Convert(InMsg.layout);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Value : InMsg.data)
	{
		Msg.Data.Add(Value);
	}

	return Msg;
}

inline FAGX_StdMsgsHeader Convert(const agxROS2::stdMsgs::Header& InMsg)
{
	FAGX_StdMsgsHeader Msg;
	Msg.Stamp = Convert(InMsg.stamp);
	Msg.FrameId = FString(InMsg.frame_id.c_str());
	return Msg;
}

inline agxROS2::stdMsgs::MultiArrayDimension Convert(const FAGX_StdMsgsMultiArrayDimension& InMsg)
{
	agxROS2::stdMsgs::MultiArrayDimension Msg;
	Msg.label = TCHAR_TO_UTF8(*InMsg.Label);
	Msg.size = static_cast<uint32_t>(InMsg.Size);
	Msg.stride = static_cast<uint32_t>(InMsg.Stride);
	return Msg;
}

inline agxROS2::stdMsgs::MultiArrayLayout Convert(const FAGX_StdMsgsMultiArrayLayout& InMsg)
{
	agxROS2::stdMsgs::MultiArrayLayout Msg;
	Msg.dim.reserve(InMsg.Dim.Num());

	for (const auto& Dimension : InMsg.Dim)
	{
		Msg.dim.emplace_back(Convert(Dimension));
	}

	Msg.data_offset = static_cast<uint32_t>(InMsg.DataOffset);
	return Msg;
}

inline agxROS2::stdMsgs::Bool Convert(const FAGX_StdMsgsBool& InMsg)
{
	agxROS2::stdMsgs::Bool Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::Byte Convert(const FAGX_StdMsgsByte& InMsg)
{
	agxROS2::stdMsgs::Byte Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::ByteMultiArray Convert(const FAGX_StdMsgsByteMultiArray& InMsg)
{
	agxROS2::stdMsgs::ByteMultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& ByteElement : InMsg.Data)
	{
		Msg.data.push_back(ByteElement);
	}

	return Msg;
}

inline agxROS2::stdMsgs::Char Convert(const FAGX_StdMsgsChar& InMsg)
{
	agxROS2::stdMsgs::Char Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::ColorRGBA Convert(const FAGX_StdMsgsColorRGBA& InMsg)
{
	agxROS2::stdMsgs::ColorRGBA Msg;
	Msg.r = InMsg.R;
	Msg.g = InMsg.G;
	Msg.b = InMsg.B;
	Msg.a = InMsg.A;
	return Msg;
}

inline agxROS2::stdMsgs::Empty Convert(const FAGX_StdMsgsEmpty& InMsg)
{
	agxROS2::stdMsgs::Empty Msg;
	return Msg;
}

inline agxROS2::stdMsgs::Float32 Convert(const FAGX_StdMsgsFloat32& InMsg)
{
	agxROS2::stdMsgs::Float32 Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::Float32MultiArray Convert(const FAGX_StdMsgsFloat32MultiArray& InMsg)
{
	agxROS2::stdMsgs::Float32MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& FloatElement : InMsg.Data)
	{
		Msg.data.push_back(FloatElement);
	}

	return Msg;
}

inline agxROS2::stdMsgs::Float64 Convert(const FAGX_StdMsgsFloat64& InMsg)
{
	agxROS2::stdMsgs::Float64 Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::Float64MultiArray Convert(const FAGX_StdMsgsFloat64MultiArray& InMsg)
{
	agxROS2::stdMsgs::Float64MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& DoubleElement : InMsg.Data)
	{
		Msg.data.push_back(DoubleElement);
	}

	return Msg;
}

inline agxROS2::stdMsgs::Int16 Convert(const FAGX_StdMsgsInt16& InMsg)
{
	agxROS2::stdMsgs::Int16 Msg;
	Msg.data = static_cast<int16_t>(InMsg.Data);
	return Msg;
}

inline agxROS2::stdMsgs::Int16MultiArray Convert(const FAGX_StdMsgsInt16MultiArray& InMsg)
{
	agxROS2::stdMsgs::Int16MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& Int16Element : InMsg.Data)
	{
		Msg.data.push_back(static_cast<int16_t>(Int16Element));
	}

	return Msg;
}

inline agxROS2::stdMsgs::Int32 Convert(const FAGX_StdMsgsInt32& InMsg)
{
	agxROS2::stdMsgs::Int32 Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::Int32MultiArray Convert(const FAGX_StdMsgsInt32MultiArray& InMsg)
{
	agxROS2::stdMsgs::Int32MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& Int32Element : InMsg.Data)
	{
		Msg.data.push_back(Int32Element);
	}

	return Msg;
}

inline agxROS2::stdMsgs::Int64 Convert(const FAGX_StdMsgsInt64& InMsg)
{
	agxROS2::stdMsgs::Int64 Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::Int64MultiArray Convert(const FAGX_StdMsgsInt64MultiArray& InMsg)
{
	agxROS2::stdMsgs::Int64MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& Int64Element : InMsg.Data)
	{
		Msg.data.push_back(Int64Element);
	}

	return Msg;
}

inline agxROS2::stdMsgs::Int8 Convert(const FAGX_StdMsgsInt8& InMsg)
{
	agxROS2::stdMsgs::Int8 Msg;
	Msg.data = static_cast<int8_t>(InMsg.Data);
	return Msg;
}

inline agxROS2::stdMsgs::Int8MultiArray Convert(const FAGX_StdMsgsInt8MultiArray& InMsg)
{
	agxROS2::stdMsgs::Int8MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& Int8Element : InMsg.Data)
	{
		Msg.data.push_back(static_cast<int8_t>(Int8Element));
	}

	return Msg;
}

inline agxROS2::stdMsgs::String Convert(const FAGX_StdMsgsString& InMsg)
{
	agxROS2::stdMsgs::String Msg;
	Msg.data = TCHAR_TO_UTF8(*InMsg.Data);
	return Msg;
}

inline agxROS2::stdMsgs::UInt16 Convert(const FAGX_StdMsgsUInt16& InMsg)
{
	agxROS2::stdMsgs::UInt16 Msg;
	Msg.data = static_cast<uint16_t>(InMsg.Data);
	return Msg;
}

inline agxROS2::stdMsgs::UInt16MultiArray Convert(const FAGX_StdMsgsUInt16MultiArray& InMsg)
{
	agxROS2::stdMsgs::UInt16MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& UInt16Element : InMsg.Data)
	{
		Msg.data.push_back(static_cast<uint16_t>(UInt16Element));
	}

	return Msg;
}

inline agxROS2::stdMsgs::UInt32 Convert(const FAGX_StdMsgsUInt32& InMsg)
{
	agxROS2::stdMsgs::UInt32 Msg;
	Msg.data = static_cast<uint32_t>(InMsg.Data);
	return Msg;
}

inline agxROS2::stdMsgs::UInt32MultiArray Convert(const FAGX_StdMsgsUInt32MultiArray& InMsg)
{
	agxROS2::stdMsgs::UInt32MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& UInt32Element : InMsg.Data)
	{
		Msg.data.push_back(static_cast<uint32_t>(UInt32Element));
	}

	return Msg;
}

inline agxROS2::stdMsgs::UInt64 Convert(const FAGX_StdMsgsUInt64& InMsg)
{
	agxROS2::stdMsgs::UInt64 Msg;
	Msg.data = static_cast<uint64_t>(InMsg.Data);
	return Msg;
}

inline agxROS2::stdMsgs::UInt64MultiArray Convert(const FAGX_StdMsgsUInt64MultiArray& InMsg)
{
	agxROS2::stdMsgs::UInt64MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& UInt64Element : InMsg.Data)
	{
		Msg.data.push_back(static_cast<uint64_t>(UInt64Element));
	}

	return Msg;
}

inline agxROS2::stdMsgs::UInt8 Convert(const FAGX_StdMsgsUInt8& InMsg)
{
	agxROS2::stdMsgs::UInt8 Msg;
	Msg.data = InMsg.Data;
	return Msg;
}

inline agxROS2::stdMsgs::UInt8MultiArray Convert(const FAGX_StdMsgsUInt8MultiArray& InMsg)
{
	agxROS2::stdMsgs::UInt8MultiArray Msg;
	Msg.layout = Convert(InMsg.Layout);
	Msg.data.reserve(InMsg.Data.Num());

	for (const auto& UInt8Element : InMsg.Data)
	{
		Msg.data.push_back(UInt8Element);
	}

	return Msg;
}

inline agxROS2::stdMsgs::Header Convert(const FAGX_StdMsgsHeader& InMsg)
{
	agxROS2::stdMsgs::Header Msg;
	Msg.stamp = Convert(InMsg.Stamp);
	Msg.frame_id = TCHAR_TO_UTF8(*InMsg.FrameId);
	return Msg;
}

//
// GeometryMsgs
//

inline FAGX_GeometryMsgsVector3 Convert(const agxROS2::geometryMsgs::Vector3& InMsg)
{
	FAGX_GeometryMsgsVector3 Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	return Msg;
}

inline FAGX_GeometryMsgsQuaternion Convert(const agxROS2::geometryMsgs::Quaternion& InMsg)
{
	FAGX_GeometryMsgsQuaternion Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	Msg.W = InMsg.w;
	return Msg;
}

inline FAGX_GeometryMsgsAccel Convert(const agxROS2::geometryMsgs::Accel& InMsg)
{
	FAGX_GeometryMsgsAccel Msg;
	Msg.Linear = Convert(InMsg.linear);
	Msg.Angular = Convert(InMsg.angular);
	return Msg;
}

inline FAGX_GeometryMsgsAccelStamped Convert(const agxROS2::geometryMsgs::AccelStamped& InMsg)
{
	FAGX_GeometryMsgsAccelStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Accel = Convert(InMsg.accel);
	return Msg;
}

inline FAGX_GeometryMsgsAccelWithCovariance Convert(
	const agxROS2::geometryMsgs::AccelWithCovariance& InMsg)
{
	FAGX_GeometryMsgsAccelWithCovariance Msg;
	Msg.Accel = Convert(InMsg.accel);

	// Note: Covariance is a dynamic array on the Unreal side.
	Msg.Covariance.SetNum(36);
	for (int32 i = 0; i < 36; ++i)
	{
		Msg.Covariance[i] = static_cast<double>(InMsg.covariance[i]);
	}
	return Msg;
}

inline FAGX_GeometryMsgsAccelWithCovarianceStamped Convert(
	const agxROS2::geometryMsgs::AccelWithCovarianceStamped& InMsg)
{
	FAGX_GeometryMsgsAccelWithCovarianceStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Accel = Convert(InMsg.accel);
	return Msg;
}

inline FAGX_GeometryMsgsInertia Convert(const agxROS2::geometryMsgs::Inertia& InMsg)
{
	FAGX_GeometryMsgsInertia Msg;
	Msg.M = InMsg.m;
	Msg.COM = Convert(InMsg.com);
	Msg.Ixx = InMsg.ixx;
	Msg.Ixy = InMsg.ixy;
	Msg.Ixz = InMsg.ixz;
	Msg.Iyy = InMsg.iyy;
	Msg.Iyz = InMsg.iyz;
	Msg.Izz = InMsg.izz;
	return Msg;
}

inline FAGX_GeometryMsgsInertiaStamped Convert(const agxROS2::geometryMsgs::InertiaStamped& InMsg)
{
	FAGX_GeometryMsgsInertiaStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Inertia = Convert(InMsg.inertia);
	return Msg;
}

inline FAGX_GeometryMsgsPoint Convert(const agxROS2::geometryMsgs::Point& InMsg)
{
	FAGX_GeometryMsgsPoint Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	return Msg;
}

inline FAGX_GeometryMsgsPoint32 Convert(const agxROS2::geometryMsgs::Point32& InMsg)
{
	FAGX_GeometryMsgsPoint32 Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Z = InMsg.z;
	return Msg;
}

inline FAGX_GeometryMsgsPointStamped Convert(const agxROS2::geometryMsgs::PointStamped& InMsg)
{
	FAGX_GeometryMsgsPointStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Point = Convert(InMsg.point);
	return Msg;
}

inline FAGX_GeometryMsgsPolygon Convert(const agxROS2::geometryMsgs::Polygon& InMsg)
{
	FAGX_GeometryMsgsPolygon Msg;
	for (const auto& point : InMsg.points)
	{
		Msg.Points.Add(Convert(point));
	}
	return Msg;
}

inline FAGX_GeometryMsgsPolygonStamped Convert(const agxROS2::geometryMsgs::PolygonStamped& InMsg)
{
	FAGX_GeometryMsgsPolygonStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Polygon = Convert(InMsg.polygon);
	return Msg;
}

inline FAGX_GeometryMsgsPose Convert(const agxROS2::geometryMsgs::Pose& InMsg)
{
	FAGX_GeometryMsgsPose Msg;
	Msg.Position = Convert(InMsg.position);
	Msg.Orientation = Convert(InMsg.orientation);
	return Msg;
}

inline FAGX_GeometryMsgsPose2D Convert(const agxROS2::geometryMsgs::Pose2D& InMsg)
{
	FAGX_GeometryMsgsPose2D Msg;
	Msg.X = InMsg.x;
	Msg.Y = InMsg.y;
	Msg.Theta = InMsg.theta;
	return Msg;
}

inline FAGX_GeometryMsgsPoseArray Convert(const agxROS2::geometryMsgs::PoseArray& InMsg)
{
	FAGX_GeometryMsgsPoseArray Msg;
	Msg.Header = Convert(InMsg.header);
	for (const auto& pose : InMsg.poses)
	{
		Msg.Poses.Add(Convert(pose));
	}
	return Msg;
}

inline FAGX_GeometryMsgsPoseStamped Convert(const agxROS2::geometryMsgs::PoseStamped& InMsg)
{
	FAGX_GeometryMsgsPoseStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Pose = Convert(InMsg.pose);
	return Msg;
}

inline FAGX_GeometryMsgsPoseWithCovariance Convert(
	const agxROS2::geometryMsgs::PoseWithCovariance& InMsg)
{
	FAGX_GeometryMsgsPoseWithCovariance Msg;
	Msg.Pose = Convert(InMsg.pose);

	// Note: Covariance is a dynamic array on the Unreal side.
	Msg.Covariance.SetNum(36);
	for (int32 i = 0; i < 36; ++i)
	{
		Msg.Covariance[i] = static_cast<double>(InMsg.covariance[i]);
	}
	return Msg;
}

inline FAGX_GeometryMsgsPoseWithCovarianceStamped Convert(
	const agxROS2::geometryMsgs::PoseWithCovarianceStamped& InMsg)
{
	FAGX_GeometryMsgsPoseWithCovarianceStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Pose = Convert(InMsg.pose);
	return Msg;
}

inline FAGX_GeometryMsgsQuaternionStamped Convert(
	const agxROS2::geometryMsgs::QuaternionStamped& InMsg)
{
	FAGX_GeometryMsgsQuaternionStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Quaternion = Convert(InMsg.quaternion);
	return Msg;
}

inline FAGX_GeometryMsgsTransform Convert(const agxROS2::geometryMsgs::Transform& InMsg)
{
	FAGX_GeometryMsgsTransform Msg;
	Msg.Translation = Convert(InMsg.translation);
	Msg.Rotation = Convert(InMsg.rotation);
	return Msg;
}

inline FAGX_GeometryMsgsTransformStamped Convert(
	const agxROS2::geometryMsgs::TransformStamped& InMsg)
{
	FAGX_GeometryMsgsTransformStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.ChildFrameId = FString(InMsg.child_frame_id.c_str());
	Msg.Transform = Convert(InMsg.transform);
	return Msg;
}

inline FAGX_GeometryMsgsTwist Convert(const agxROS2::geometryMsgs::Twist& InMsg)
{
	FAGX_GeometryMsgsTwist Msg;
	Msg.Linear = Convert(InMsg.linear);
	Msg.Angular = Convert(InMsg.angular);
	return Msg;
}

inline FAGX_GeometryMsgsTwistStamped Convert(const agxROS2::geometryMsgs::TwistStamped& InMsg)
{
	FAGX_GeometryMsgsTwistStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Twist = Convert(InMsg.twist);
	return Msg;
}

inline FAGX_GeometryMsgsTwistWithCovariance Convert(
	const agxROS2::geometryMsgs::TwistWithCovariance& InMsg)
{
	FAGX_GeometryMsgsTwistWithCovariance Msg;
	Msg.Twist = Convert(InMsg.twist);

	// Note: Covariance is a dynamic array on the Unreal side.
	Msg.Covariance.SetNum(36);
	for (int32 i = 0; i < 36; ++i)
	{
		Msg.Covariance[i] = static_cast<double>(InMsg.covariance[i]);
	}
	return Msg;
}

inline FAGX_GeometryMsgsTwistWithCovarianceStamped Convert(
	const agxROS2::geometryMsgs::TwistWithCovarianceStamped& InMsg)
{
	FAGX_GeometryMsgsTwistWithCovarianceStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Twist = Convert(InMsg.twist);
	return Msg;
}

inline FAGX_GeometryMsgsVector3Stamped Convert(const agxROS2::geometryMsgs::Vector3Stamped& InMsg)
{
	FAGX_GeometryMsgsVector3Stamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Vector = Convert(InMsg.vector);
	return Msg;
}

inline FAGX_GeometryMsgsWrench Convert(const agxROS2::geometryMsgs::Wrench& InMsg)
{
	FAGX_GeometryMsgsWrench Msg;
	Msg.Force = Convert(InMsg.force);
	Msg.Torque = Convert(InMsg.torque);
	return Msg;
}

inline FAGX_GeometryMsgsWrenchStamped Convert(const agxROS2::geometryMsgs::WrenchStamped& InMsg)
{
	FAGX_GeometryMsgsWrenchStamped Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Wrench = Convert(InMsg.wrench);
	return Msg;
}

inline agxROS2::geometryMsgs::Vector3 Convert(const FAGX_GeometryMsgsVector3& InMsg)
{
	agxROS2::geometryMsgs::Vector3 Msg;
	Msg.x = InMsg.X;
	Msg.y = InMsg.Y;
	Msg.z = InMsg.Z;
	return Msg;
}

inline agxROS2::geometryMsgs::Quaternion Convert(const FAGX_GeometryMsgsQuaternion& InMsg)
{
	agxROS2::geometryMsgs::Quaternion Msg;
	Msg.x = InMsg.X;
	Msg.y = InMsg.Y;
	Msg.z = InMsg.Z;
	Msg.w = InMsg.W;
	return Msg;
}

inline agxROS2::geometryMsgs::Accel Convert(const FAGX_GeometryMsgsAccel& InMsg)
{
	agxROS2::geometryMsgs::Accel Msg;
	Msg.linear = Convert(InMsg.Linear);
	Msg.angular = Convert(InMsg.Angular);
	return Msg;
}

inline agxROS2::geometryMsgs::AccelStamped Convert(const FAGX_GeometryMsgsAccelStamped& InMsg)
{
	agxROS2::geometryMsgs::AccelStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.accel = Convert(InMsg.Accel);
	return Msg;
}

inline agxROS2::geometryMsgs::AccelWithCovariance Convert(
	const FAGX_GeometryMsgsAccelWithCovariance& InMsg)
{
	agxROS2::geometryMsgs::AccelWithCovariance Msg;
	Msg.accel = Convert(InMsg.Accel);

	const int32 Max = std::min(InMsg.Covariance.Num(), 36);
	for (int32 i = 0; i < Max; i++)
		Msg.covariance[i] = InMsg.Covariance[i];

	return Msg;
}

inline agxROS2::geometryMsgs::AccelWithCovarianceStamped Convert(
	const FAGX_GeometryMsgsAccelWithCovarianceStamped& InMsg)
{
	agxROS2::geometryMsgs::AccelWithCovarianceStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.accel = Convert(InMsg.Accel);
	return Msg;
}

inline agxROS2::geometryMsgs::Inertia Convert(const FAGX_GeometryMsgsInertia& InMsg)
{
	agxROS2::geometryMsgs::Inertia Msg;
	Msg.m = InMsg.M;
	Msg.com = Convert(InMsg.COM);
	Msg.ixx = InMsg.Ixx;
	Msg.ixy = InMsg.Ixy;
	Msg.ixz = InMsg.Ixz;
	Msg.iyy = InMsg.Iyy;
	Msg.iyz = InMsg.Iyz;
	Msg.izz = InMsg.Izz;
	return Msg;
}

inline agxROS2::geometryMsgs::InertiaStamped Convert(const FAGX_GeometryMsgsInertiaStamped& InMsg)
{
	agxROS2::geometryMsgs::InertiaStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.inertia = Convert(InMsg.Inertia);
	return Msg;
}

inline agxROS2::geometryMsgs::Point Convert(const FAGX_GeometryMsgsPoint& InMsg)
{
	agxROS2::geometryMsgs::Point Msg;
	Msg.x = InMsg.X;
	Msg.y = InMsg.Y;
	Msg.z = InMsg.Z;
	return Msg;
}

inline agxROS2::geometryMsgs::Point32 Convert(const FAGX_GeometryMsgsPoint32& InMsg)
{
	agxROS2::geometryMsgs::Point32 Msg;
	Msg.x = InMsg.X;
	Msg.y = InMsg.Y;
	Msg.z = InMsg.Z;
	return Msg;
}

inline agxROS2::geometryMsgs::PointStamped Convert(const FAGX_GeometryMsgsPointStamped& InMsg)
{
	agxROS2::geometryMsgs::PointStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.point = Convert(InMsg.Point);
	return Msg;
}

inline agxROS2::geometryMsgs::Polygon Convert(const FAGX_GeometryMsgsPolygon& InMsg)
{
	agxROS2::geometryMsgs::Polygon Msg;
	for (const auto& Point32Element : InMsg.Points)
	{
		Msg.points.push_back(Convert(Point32Element));
	}
	return Msg;
}

inline agxROS2::geometryMsgs::PolygonStamped Convert(const FAGX_GeometryMsgsPolygonStamped& InMsg)
{
	agxROS2::geometryMsgs::PolygonStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.polygon = Convert(InMsg.Polygon);
	return Msg;
}

inline agxROS2::geometryMsgs::Pose Convert(const FAGX_GeometryMsgsPose& InMsg)
{
	agxROS2::geometryMsgs::Pose Msg;
	Msg.position = Convert(InMsg.Position);
	Msg.orientation = Convert(InMsg.Orientation);
	return Msg;
}

inline agxROS2::geometryMsgs::Pose2D Convert(const FAGX_GeometryMsgsPose2D& InMsg)
{
	agxROS2::geometryMsgs::Pose2D Msg;
	Msg.x = InMsg.X;
	Msg.y = InMsg.Y;
	Msg.theta = InMsg.Theta;
	return Msg;
}

inline agxROS2::geometryMsgs::PoseArray Convert(const FAGX_GeometryMsgsPoseArray& InMsg)
{
	agxROS2::geometryMsgs::PoseArray Msg;
	Msg.header = Convert(InMsg.Header);
	for (const auto& PoseElement : InMsg.Poses)
	{
		Msg.poses.push_back(Convert(PoseElement));
	}
	return Msg;
}

inline agxROS2::geometryMsgs::PoseStamped Convert(const FAGX_GeometryMsgsPoseStamped& InMsg)
{
	agxROS2::geometryMsgs::PoseStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.pose = Convert(InMsg.Pose);
	return Msg;
}

inline agxROS2::geometryMsgs::PoseWithCovariance Convert(
	const FAGX_GeometryMsgsPoseWithCovariance& InMsg)
{
	agxROS2::geometryMsgs::PoseWithCovariance Msg;
	Msg.pose = Convert(InMsg.Pose);

	const int32 Max = std::min(InMsg.Covariance.Num(), 36);
	for (int32 i = 0; i < Max; i++)
		Msg.covariance[i] = InMsg.Covariance[i];

	return Msg;
}

inline agxROS2::geometryMsgs::PoseWithCovarianceStamped Convert(
	const FAGX_GeometryMsgsPoseWithCovarianceStamped& InMsg)
{
	agxROS2::geometryMsgs::PoseWithCovarianceStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.pose = Convert(InMsg.Pose);
	return Msg;
}

inline agxROS2::geometryMsgs::QuaternionStamped Convert(
	const FAGX_GeometryMsgsQuaternionStamped& InMsg)
{
	agxROS2::geometryMsgs::QuaternionStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.quaternion = Convert(InMsg.Quaternion);
	return Msg;
}

inline agxROS2::geometryMsgs::Transform Convert(const FAGX_GeometryMsgsTransform& InMsg)
{
	agxROS2::geometryMsgs::Transform Msg;
	Msg.translation = Convert(InMsg.Translation);
	Msg.rotation = Convert(InMsg.Rotation);
	return Msg;
}

inline agxROS2::geometryMsgs::TransformStamped Convert(
	const FAGX_GeometryMsgsTransformStamped& InMsg)
{
	agxROS2::geometryMsgs::TransformStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.child_frame_id = TCHAR_TO_UTF8(*InMsg.ChildFrameId);
	Msg.transform = Convert(InMsg.Transform);
	return Msg;
}

inline agxROS2::geometryMsgs::Twist Convert(const FAGX_GeometryMsgsTwist& InMsg)
{
	agxROS2::geometryMsgs::Twist Msg;
	Msg.linear = Convert(InMsg.Linear);
	Msg.angular = Convert(InMsg.Angular);
	return Msg;
}

inline agxROS2::geometryMsgs::TwistStamped Convert(const FAGX_GeometryMsgsTwistStamped& InMsg)
{
	agxROS2::geometryMsgs::TwistStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.twist = Convert(InMsg.Twist);
	return Msg;
}

inline agxROS2::geometryMsgs::TwistWithCovariance Convert(
	const FAGX_GeometryMsgsTwistWithCovariance& InMsg)
{
	agxROS2::geometryMsgs::TwistWithCovariance Msg;
	Msg.twist = Convert(InMsg.Twist);

	const int32 Max = std::min(InMsg.Covariance.Num(), 36);
	for (int32 i = 0; i < Max; i++)
		Msg.covariance[i] = InMsg.Covariance[i];

	return Msg;
}

inline agxROS2::geometryMsgs::TwistWithCovarianceStamped Convert(
	const FAGX_GeometryMsgsTwistWithCovarianceStamped& InMsg)
{
	agxROS2::geometryMsgs::TwistWithCovarianceStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.twist = Convert(InMsg.Twist);
	return Msg;
}

inline agxROS2::geometryMsgs::Vector3Stamped Convert(const FAGX_GeometryMsgsVector3Stamped& InMsg)
{
	agxROS2::geometryMsgs::Vector3Stamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.vector = Convert(InMsg.Vector);
	return Msg;
}

inline agxROS2::geometryMsgs::Wrench Convert(const FAGX_GeometryMsgsWrench& InMsg)
{
	agxROS2::geometryMsgs::Wrench Msg;
	Msg.force = Convert(InMsg.Force);
	Msg.torque = Convert(InMsg.Torque);
	return Msg;
}

inline agxROS2::geometryMsgs::WrenchStamped Convert(const FAGX_GeometryMsgsWrenchStamped& InMsg)
{
	agxROS2::geometryMsgs::WrenchStamped Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.wrench = Convert(InMsg.Wrench);
	return Msg;
}

//
// SensorMsgs
//

inline FAGX_SensorMsgsBatteryState Convert(const agxROS2::sensorMsgs::BatteryState& InMsg)
{
	FAGX_SensorMsgsBatteryState Msg;

	Msg.Header = Convert(InMsg.header);

	Msg.Voltage = InMsg.voltage;
	Msg.Temperature = InMsg.temperature;
	Msg.Current = InMsg.current;
	Msg.Charge = InMsg.charge;
	Msg.Capacity = InMsg.capacity;
	Msg.DesignCapacity = InMsg.design_capacity;
	Msg.Percentage = InMsg.percentage;
	Msg.PowerSupplyStatus = InMsg.power_supply_status;
	Msg.PowerSupplyHealth = InMsg.power_supply_health;
	Msg.PowerSupplyTechnology = InMsg.power_supply_technology;
	Msg.Present = InMsg.present;

	Msg.CellVoltage.Reserve(InMsg.cell_voltage.size());
	for (const auto& Value : InMsg.cell_voltage)
	{
		Msg.CellVoltage.Add(Value);
	}

	Msg.CellTemperature.Reserve(InMsg.cell_temperature.size());
	for (const auto& Value : InMsg.cell_temperature)
	{
		Msg.CellTemperature.Add(Value);
	}

	Msg.Location = FString(InMsg.location.c_str());
	Msg.SerialNumber = FString(InMsg.serial_number.c_str());

	return Msg;
}

inline FAGX_SensorMsgsChannelFloat32 Convert(const agxROS2::sensorMsgs::ChannelFloat32& InMsg)
{
	FAGX_SensorMsgsChannelFloat32 Msg;
	Msg.Name = FString(InMsg.name.c_str());

	Msg.Values.SetNum(InMsg.values.size());
	for (int32 i = 0; i < InMsg.values.size(); ++i)
		Msg.Values[i] = InMsg.values[i];

	return Msg;
}

inline FAGX_SensorMsgsCompressedImage Convert(const agxROS2::sensorMsgs::CompressedImage& InMsg)
{
	FAGX_SensorMsgsCompressedImage Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Format = FString(InMsg.format.c_str());

	Msg.Data.SetNum(InMsg.data.size());
	for (int32 i = 0; i < InMsg.data.size(); ++i)
		Msg.Data[i] = InMsg.data[i];

	return Msg;
}

inline FAGX_SensorMsgsFluidPressure Convert(const agxROS2::sensorMsgs::FluidPressure& InMsg)
{
	FAGX_SensorMsgsFluidPressure Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.FluidPressure = InMsg.fluid_pressure;
	Msg.Variance = InMsg.variance;
	return Msg;
}

inline FAGX_SensorMsgsIlluminance Convert(const agxROS2::sensorMsgs::Illuminance& InMsg)
{
	FAGX_SensorMsgsIlluminance Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Illuminance = InMsg.illuminance;
	Msg.Variance = InMsg.variance;
	return Msg;
}

inline FAGX_SensorMsgsImage Convert(const agxROS2::sensorMsgs::Image& InMsg)
{
	FAGX_SensorMsgsImage Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Height = static_cast<int64>(InMsg.height);
	Msg.Width = static_cast<int64>(InMsg.width);
	Msg.Encoding = FString(InMsg.encoding.c_str());
	Msg.IsBigendian = static_cast<uint8>(InMsg.is_bigendian);
	Msg.Step = static_cast<int64>(InMsg.step);

	Msg.Data.SetNum(InMsg.data.size());
	for (int32 i = 0; i < InMsg.data.size(); ++i)
		Msg.Data[i] = InMsg.data[i];

	return Msg;
}

inline FAGX_SensorMsgsImu Convert(const agxROS2::sensorMsgs::Imu& InMsg)
{
	FAGX_SensorMsgsImu Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.Orientation = Convert(InMsg.orientation);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.OrientationCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.OrientationCovariance[i] = InMsg.orientation_covariance[i];

	Msg.AngularVelocity = Convert(InMsg.angular_velocity);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.AngularVelocityCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.AngularVelocityCovariance[i] = InMsg.angular_velocity_covariance[i];

	Msg.LinearAcceleration = Convert(InMsg.linear_acceleration);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.LinearAccelerationCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.LinearAccelerationCovariance[i] = InMsg.linear_acceleration_covariance[i];

	return Msg;
}

inline FAGX_SensorMsgsJointState Convert(const agxROS2::sensorMsgs::JointState& InMsg)
{
	FAGX_SensorMsgsJointState Msg;
	Msg.Header = Convert(InMsg.header);

	Msg.Name.SetNum(InMsg.name.size());
	for (int32 i = 0; i < InMsg.name.size(); ++i)
		Msg.Name[i] = FString(InMsg.name[i].c_str());

	Msg.Position.SetNum(InMsg.position.size());
	for (int32 i = 0; i < InMsg.position.size(); ++i)
		Msg.Position[i] = InMsg.position[i];

	Msg.Velocity.SetNum(InMsg.velocity.size());
	for (int32 i = 0; i < InMsg.velocity.size(); ++i)
		Msg.Velocity[i] = InMsg.velocity[i];

	Msg.Effort.SetNum(InMsg.effort.size());
	for (int32 i = 0; i < InMsg.effort.size(); ++i)
		Msg.Effort[i] = InMsg.effort[i];

	return Msg;
}

inline FAGX_SensorMsgsJoy Convert(const agxROS2::sensorMsgs::Joy& InMsg)
{
	FAGX_SensorMsgsJoy Msg;
	Msg.Header = Convert(InMsg.header);

	Msg.Axes.SetNum(InMsg.axes.size());
	for (int32 i = 0; i < InMsg.axes.size(); ++i)
		Msg.Axes[i] = InMsg.axes[i];

	Msg.Buttons.SetNum(InMsg.buttons.size());
	for (int32 i = 0; i < InMsg.buttons.size(); ++i)
		Msg.Buttons[i] = InMsg.buttons[i];

	return Msg;
}

inline FAGX_SensorMsgsJoyFeedback Convert(const agxROS2::sensorMsgs::JoyFeedback& InMsg)
{
	FAGX_SensorMsgsJoyFeedback Msg;
	Msg.Type = InMsg.type;
	Msg.Id = InMsg.id;
	Msg.Intensity = InMsg.intensity;
	return Msg;
}

inline FAGX_SensorMsgsJoyFeedbackArray Convert(const agxROS2::sensorMsgs::JoyFeedbackArray& InMsg)
{
	FAGX_SensorMsgsJoyFeedbackArray Msg;

	Msg.Array.SetNum(InMsg.array.size());
	for (int32 i = 0; i < InMsg.array.size(); ++i)
		Msg.Array[i] = Convert(InMsg.array[i]);

	return Msg;
}

inline FAGX_SensorMsgsLaserEcho Convert(const agxROS2::sensorMsgs::LaserEcho& InMsg)
{
	FAGX_SensorMsgsLaserEcho Msg;

	Msg.Echoes.SetNum(InMsg.echoes.size());
	for (int32 i = 0; i < InMsg.echoes.size(); ++i)
		Msg.Echoes[i] = InMsg.echoes[i];

	return Msg;
}

inline FAGX_SensorMsgsLaserScan Convert(const agxROS2::sensorMsgs::LaserScan& InMsg)
{
	FAGX_SensorMsgsLaserScan Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.AngleMin = InMsg.angle_min;
	Msg.AngleMax = InMsg.angle_max;
	Msg.AngleIncrement = InMsg.angle_increment;
	Msg.TimeIncrement = InMsg.time_increment;
	Msg.ScanTime = InMsg.scan_time;
	Msg.RangeMin = InMsg.range_min;
	Msg.RangeMax = InMsg.range_max;

	Msg.Ranges.SetNum(InMsg.ranges.size());
	for (int32 i = 0; i < InMsg.ranges.size(); ++i)
		Msg.Ranges[i] = InMsg.ranges[i];

	Msg.Intensities.SetNum(InMsg.intensities.size());
	for (int32 i = 0; i < InMsg.intensities.size(); ++i)
		Msg.Intensities[i] = InMsg.intensities[i];

	return Msg;
}

inline FAGX_SensorMsgsMagneticField Convert(const agxROS2::sensorMsgs::MagneticField& InMsg)
{
	FAGX_SensorMsgsMagneticField Msg;
	Msg.Header = Convert(InMsg.header);
	Msg.MagneticField = Convert(InMsg.magnetic_field);

	// Note: This is a dynamic array on the Unreal side, static array on agxROS2 side.
	Msg.MagneticFieldCovariance.SetNum(9);
	for (int32 i = 0; i < 9; ++i)
		Msg.MagneticFieldCovariance[i] = InMsg.magnetic_field_covariance[i];

	return Msg;
}

inline FAGX_SensorMsgsMultiDOFJointState Convert(
	const agxROS2::sensorMsgs::MultiDOFJointState& InMsg)
{
	FAGX_SensorMsgsMultiDOFJointState Msg;

	Msg.Header = Convert(InMsg.header);

	Msg.JointNames.Reserve(InMsg.joint_names.size());
	for (const auto& JointName : InMsg.joint_names)
	{
		Msg.JointNames.Add(JointName.c_str());
	}

	Msg.Transforms.Reserve(InMsg.transforms.size());
	for (const auto& Transform : InMsg.transforms)
	{
		Msg.Transforms.Add(Convert(Transform));
	}

	Msg.Twist.Reserve(InMsg.twist.size());
	for (const auto& Twist : InMsg.twist)
	{
		Msg.Twist.Add(Convert(Twist));
	}

	Msg.Wrench.Reserve(InMsg.wrench.size());
	for (const auto& Wrench : InMsg.wrench)
	{
		Msg.Wrench.Add(Convert(Wrench));
	}

	return Msg;
}

inline FAGX_SensorMsgsMultiEchoLaserScan Convert(
	const agxROS2::sensorMsgs::MultiEchoLaserScan& InMsg)
{
	FAGX_SensorMsgsMultiEchoLaserScan Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.AngleMin = InMsg.angle_min;
	Msg.AngleMax = InMsg.angle_max;
	Msg.AngleIncrement = InMsg.angle_increment;
	Msg.TimeIncrement = InMsg.time_increment;
	Msg.ScanTime = InMsg.scan_time;
	Msg.RangeMin = InMsg.range_min;
	Msg.RangeMax = InMsg.range_max;

	Msg.Ranges.Reserve(InMsg.ranges.size());
	for (const auto& Range : InMsg.ranges)
	{
		Msg.Ranges.Add(Convert(Range));
	}

	Msg.Intensities.Reserve(InMsg.intensities.size());
	for (const auto& Intensity : InMsg.intensities)
	{
		Msg.Intensities.Add(Convert(Intensity));
	}

	return Msg;
}

inline FAGX_SensorMsgsNavSatStatus Convert(const agxROS2::sensorMsgs::NavSatStatus& InMsg)
{
	FAGX_SensorMsgsNavSatStatus Msg;

	Msg.Status = static_cast<int32>(InMsg.status);
	Msg.Service = static_cast<int32>(InMsg.service);

	return Msg;
}

inline FAGX_SensorMsgsNavSatFix Convert(const agxROS2::sensorMsgs::NavSatFix& InMsg)
{
	FAGX_SensorMsgsNavSatFix Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.Status = Convert(InMsg.status);
	Msg.Latitude = InMsg.latitude;
	Msg.Longitude = InMsg.longitude;
	Msg.Altitude = InMsg.altitude;

	Msg.PositionCovariance.Reserve(9);
	for (int i = 0; i < 9; ++i)
	{
		Msg.PositionCovariance.Add(InMsg.position_covariance[i]);
	}

	Msg.PositionCovarianceType = InMsg.position_covariance_type;

	return Msg;
}

inline FAGX_SensorMsgsPointCloud Convert(const agxROS2::sensorMsgs::PointCloud& InMsg)
{
	FAGX_SensorMsgsPointCloud Msg;

	Msg.Header = Convert(InMsg.header);

	Msg.Points.Reserve(InMsg.points.size());
	for (const auto& Point : InMsg.points)
	{
		Msg.Points.Add(Convert(Point));
	}

	Msg.Channels.Reserve(InMsg.channels.size());
	for (const auto& Channel : InMsg.channels)
	{
		Msg.Channels.Add(Convert(Channel));
	}

	return Msg;
}

inline FAGX_SensorMsgsPointField Convert(const agxROS2::sensorMsgs::PointField& InMsg)
{
	FAGX_SensorMsgsPointField Msg;

	Msg.Name = InMsg.name.c_str();
	Msg.Offset = static_cast<int64>(InMsg.offset);
	Msg.Datatype = InMsg.datatype;
	Msg.Count = static_cast<int64>(InMsg.count);

	return Msg;
}

inline FAGX_SensorMsgsPointCloud2 Convert(const agxROS2::sensorMsgs::PointCloud2& InMsg)
{
	FAGX_SensorMsgsPointCloud2 Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.Height = static_cast<int64>(InMsg.height);
	Msg.Width = static_cast<int64>(InMsg.width);

	Msg.Fields.Reserve(InMsg.fields.size());
	for (const auto& Field : InMsg.fields)
	{
		Msg.Fields.Add(Convert(Field));
	}

	Msg.IsBigendian = InMsg.is_bigendian;
	Msg.PointStep = static_cast<int64>(InMsg.point_step);
	Msg.RowStep = static_cast<int64>(InMsg.row_step);

	Msg.Data.Reserve(InMsg.data.size());
	for (const auto& Byte : InMsg.data)
	{
		Msg.Data.Add(Byte);
	}

	Msg.IsDense = InMsg.is_dense;

	return Msg;
}

inline FAGX_SensorMsgsRange Convert(const agxROS2::sensorMsgs::Range& InMsg)
{
	FAGX_SensorMsgsRange Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.RadiationType = InMsg.radiation_type;
	Msg.FieldOfView = InMsg.field_of_view;
	Msg.MinRange = InMsg.min_range;
	Msg.MaxRange = InMsg.max_range;
	Msg.Range = InMsg.range;

	return Msg;
}

inline FAGX_SensorMsgsRegionOfInterest Convert(const agxROS2::sensorMsgs::RegionOfInterest& InMsg)
{
	FAGX_SensorMsgsRegionOfInterest Msg;

	Msg.XOffset = static_cast<int64>(InMsg.x_offset);
	Msg.YOffset = static_cast<int64>(InMsg.y_offset);
	Msg.Height = static_cast<int64>(InMsg.height);
	Msg.Width = static_cast<int64>(InMsg.width);
	Msg.DoRectify = InMsg.do_rectify;

	return Msg;
}

inline FAGX_SensorMsgsCameraInfo Convert(const agxROS2::sensorMsgs::CameraInfo& InMsg)
{
	FAGX_SensorMsgsCameraInfo Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.Height = static_cast<int64>(InMsg.height);
	Msg.Width = static_cast<int64>(InMsg.width);
	Msg.DistortionModel = InMsg.distortion_model.c_str();

	Msg.D.Reserve(InMsg.d.size());
	for (const auto& D : InMsg.d)
	{
		Msg.D.Add(D);
	}

	for (int i = 0; i < 9; ++i)
	{
		Msg.K.Add(InMsg.k[i]);
		Msg.R.Add(InMsg.r[i]);
	}

	for (int i = 0; i < 12; ++i)
	{
		Msg.P.Add(InMsg.p[i]);
	}

	Msg.BinningX = static_cast<int64>(InMsg.binning_x);
	Msg.BinningY = static_cast<int64>(InMsg.binning_y);
	Msg.ROI = Convert(InMsg.roi);

	return Msg;
}

inline FAGX_SensorMsgsRelativeHumidity Convert(const agxROS2::sensorMsgs::RelativeHumidity& InMsg)
{
	FAGX_SensorMsgsRelativeHumidity Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.RelativeHumidity = InMsg.relative_humidity;
	Msg.Variance = InMsg.variance;

	return Msg;
}

inline FAGX_SensorMsgsTemperature Convert(const agxROS2::sensorMsgs::Temperature& InMsg)
{
	FAGX_SensorMsgsTemperature Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.Temperature = InMsg.temperature;
	Msg.Variance = InMsg.variance;

	return Msg;
}

inline FAGX_SensorMsgsTimeReference Convert(const agxROS2::sensorMsgs::TimeReference& InMsg)
{
	FAGX_SensorMsgsTimeReference Msg;

	Msg.Header = Convert(InMsg.header);
	Msg.TimeRef = Convert(InMsg.time_ref);
	Msg.Source = InMsg.source.c_str();

	return Msg;
}

inline agxROS2::sensorMsgs::BatteryState Convert(const FAGX_SensorMsgsBatteryState& InMsg)
{
	agxROS2::sensorMsgs::BatteryState Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.voltage = InMsg.Voltage;
	Msg.temperature = InMsg.Temperature;
	Msg.current = InMsg.Current;
	Msg.charge = InMsg.Charge;
	Msg.capacity = InMsg.Capacity;
	Msg.design_capacity = InMsg.DesignCapacity;
	Msg.percentage = InMsg.Percentage;
	Msg.power_supply_status = InMsg.PowerSupplyStatus;
	Msg.power_supply_health = InMsg.PowerSupplyHealth;
	Msg.power_supply_technology = InMsg.PowerSupplyTechnology;
	Msg.present = InMsg.Present;

	Msg.cell_voltage.reserve(InMsg.CellVoltage.Num());
	for (const auto& CV : InMsg.CellVoltage)
		Msg.cell_voltage.push_back(CV);

	Msg.cell_temperature.reserve(InMsg.CellTemperature.Num());
	for (const auto& CT : InMsg.CellTemperature)
		Msg.cell_temperature.push_back(CT);

	Msg.location = TCHAR_TO_UTF8(*InMsg.Location);
	Msg.serial_number = TCHAR_TO_UTF8(*InMsg.SerialNumber);

	return Msg;
}

inline agxROS2::sensorMsgs::ChannelFloat32 Convert(const FAGX_SensorMsgsChannelFloat32& InMsg)
{
	agxROS2::sensorMsgs::ChannelFloat32 Msg;
	Msg.name = TCHAR_TO_UTF8(*InMsg.Name);

	Msg.values.reserve(InMsg.Values.Num());
	for (float Value : InMsg.Values)
	{
		Msg.values.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::CompressedImage Convert(const FAGX_SensorMsgsCompressedImage& InMsg)
{
	agxROS2::sensorMsgs::CompressedImage Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.format = TCHAR_TO_UTF8(*InMsg.Format);

	Msg.data.reserve(InMsg.Data.Num());
	for (uint8 Value : InMsg.Data)
	{
		Msg.data.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::FluidPressure Convert(const FAGX_SensorMsgsFluidPressure& InMsg)
{
	agxROS2::sensorMsgs::FluidPressure Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.fluid_pressure = InMsg.FluidPressure;
	Msg.variance = InMsg.Variance;

	return Msg;
}

inline agxROS2::sensorMsgs::Illuminance Convert(const FAGX_SensorMsgsIlluminance& InMsg)
{
	agxROS2::sensorMsgs::Illuminance Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.illuminance = InMsg.Illuminance;
	Msg.variance = InMsg.Variance;

	return Msg;
}

inline agxROS2::sensorMsgs::Image Convert(const FAGX_SensorMsgsImage& InMsg)
{
	agxROS2::sensorMsgs::Image Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.height = static_cast<uint32_t>(InMsg.Height);
	Msg.width = static_cast<uint32_t>(InMsg.Width);
	Msg.encoding = TCHAR_TO_UTF8(*InMsg.Encoding);
	Msg.is_bigendian = InMsg.IsBigendian;
	Msg.step = static_cast<uint32_t>(InMsg.Step);

	Msg.data.reserve(InMsg.Data.Num());
	for (uint8 Value : InMsg.Data)
	{
		Msg.data.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::Imu Convert(const FAGX_SensorMsgsImu& InMsg)
{
	agxROS2::sensorMsgs::Imu Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.orientation = Convert(InMsg.Orientation);

	{
		const int32 MaxOc = std::min(9, InMsg.OrientationCovariance.Num());
		for (int i = 0; i < MaxOc; i++)
			Msg.orientation_covariance[i] = InMsg.OrientationCovariance[i];
	}

	Msg.angular_velocity = Convert(InMsg.AngularVelocity);

	{
		const int32 MaxAvc = std::min(9, InMsg.AngularVelocityCovariance.Num());
		for (int i = 0; i < MaxAvc; i++)
			Msg.angular_velocity_covariance[i] = InMsg.AngularVelocityCovariance[i];
	}

	Msg.linear_acceleration = Convert(InMsg.LinearAcceleration);

	{
		const int32 MaxLac = std::min(9, InMsg.LinearAccelerationCovariance.Num());
		for (int i = 0; i < MaxLac; i++)
			Msg.linear_acceleration_covariance[i] = InMsg.LinearAccelerationCovariance[i];
	}

	return Msg;
}

inline agxROS2::sensorMsgs::JointState Convert(const FAGX_SensorMsgsJointState& InMsg)
{
	agxROS2::sensorMsgs::JointState Msg;
	Msg.header = Convert(InMsg.Header);

	Msg.name.reserve(InMsg.Name.Num());
	for (const FString& Value : InMsg.Name)
	{
		Msg.name.push_back(TCHAR_TO_UTF8(*Value));
	}

	Msg.position.reserve(InMsg.Position.Num());
	for (double Value : InMsg.Position)
	{
		Msg.position.push_back(Value);
	}

	Msg.velocity.reserve(InMsg.Velocity.Num());
	for (double Value : InMsg.Velocity)
	{
		Msg.velocity.push_back(Value);
	}

	Msg.effort.reserve(InMsg.Effort.Num());
	for (double Value : InMsg.Effort)
	{
		Msg.effort.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::Joy Convert(const FAGX_SensorMsgsJoy& InMsg)
{
	agxROS2::sensorMsgs::Joy Msg;
	Msg.header = Convert(InMsg.Header);

	Msg.axes.reserve(InMsg.Axes.Num());
	for (float Value : InMsg.Axes)
	{
		Msg.axes.push_back(Value);
	}

	Msg.buttons.reserve(InMsg.Buttons.Num());
	for (int32 Value : InMsg.Buttons)
	{
		Msg.buttons.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::JoyFeedback Convert(const FAGX_SensorMsgsJoyFeedback& InMsg)
{
	agxROS2::sensorMsgs::JoyFeedback Msg;
	Msg.type = InMsg.Type;
	Msg.id = InMsg.Id;
	Msg.intensity = InMsg.Intensity;

	return Msg;
}

inline agxROS2::sensorMsgs::JoyFeedbackArray Convert(const FAGX_SensorMsgsJoyFeedbackArray& InMsg)
{
	agxROS2::sensorMsgs::JoyFeedbackArray Msg;

	Msg.array.reserve(InMsg.Array.Num());
	for (const FAGX_SensorMsgsJoyFeedback& Value : InMsg.Array)
	{
		Msg.array.push_back(Convert(Value));
	}

	return Msg;
}

inline agxROS2::sensorMsgs::LaserEcho Convert(const FAGX_SensorMsgsLaserEcho& InMsg)
{
	agxROS2::sensorMsgs::LaserEcho Msg;

	Msg.echoes.reserve(InMsg.Echoes.Num());
	for (float Value : InMsg.Echoes)
	{
		Msg.echoes.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::LaserScan Convert(const FAGX_SensorMsgsLaserScan& InMsg)
{
	agxROS2::sensorMsgs::LaserScan Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.angle_min = InMsg.AngleMin;
	Msg.angle_max = InMsg.AngleMax;
	Msg.angle_increment = InMsg.AngleIncrement;
	Msg.time_increment = InMsg.TimeIncrement;
	Msg.scan_time = InMsg.ScanTime;
	Msg.range_min = InMsg.RangeMin;
	Msg.range_max = InMsg.RangeMax;

	Msg.ranges.reserve(InMsg.Ranges.Num());
	for (float Value : InMsg.Ranges)
	{
		Msg.ranges.push_back(Value);
	}

	Msg.intensities.reserve(InMsg.Intensities.Num());
	for (float Value : InMsg.Intensities)
	{
		Msg.intensities.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::MagneticField Convert(const FAGX_SensorMsgsMagneticField& InMsg)
{
	agxROS2::sensorMsgs::MagneticField Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.magnetic_field = Convert(InMsg.MagneticField);

	{
		const int32 Max = std::min(9, InMsg.MagneticFieldCovariance.Num());
		for (int i = 0; i < Max; i++)
			Msg.magnetic_field_covariance[i] = InMsg.MagneticFieldCovariance[i];
	}

	return Msg;
}

inline agxROS2::sensorMsgs::MultiDOFJointState Convert(
	const FAGX_SensorMsgsMultiDOFJointState& InMsg)
{
	agxROS2::sensorMsgs::MultiDOFJointState Msg;
	Msg.header = Convert(InMsg.Header);

	Msg.joint_names.reserve(InMsg.JointNames.Num());
	for (const FString& Value : InMsg.JointNames)
	{
		Msg.joint_names.push_back(TCHAR_TO_UTF8(*Value));
	}

	Msg.transforms.reserve(InMsg.Transforms.Num());
	for (const FAGX_GeometryMsgsTransform& Value : InMsg.Transforms)
	{
		Msg.transforms.push_back(Convert(Value));
	}

	Msg.twist.reserve(InMsg.Twist.Num());
	for (const FAGX_GeometryMsgsTwist& Value : InMsg.Twist)
	{
		Msg.twist.push_back(Convert(Value));
	}

	Msg.wrench.reserve(InMsg.Wrench.Num());
	for (const FAGX_GeometryMsgsWrench& Value : InMsg.Wrench)
	{
		Msg.wrench.push_back(Convert(Value));
	}

	return Msg;
}

inline agxROS2::sensorMsgs::MultiEchoLaserScan Convert(
	const FAGX_SensorMsgsMultiEchoLaserScan& InMsg)
{
	agxROS2::sensorMsgs::MultiEchoLaserScan Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.angle_min = InMsg.AngleMin;
	Msg.angle_max = InMsg.AngleMax;
	Msg.angle_increment = InMsg.AngleIncrement;
	Msg.time_increment = InMsg.TimeIncrement;
	Msg.scan_time = InMsg.ScanTime;
	Msg.range_min = InMsg.RangeMin;
	Msg.range_max = InMsg.RangeMax;

	Msg.ranges.reserve(InMsg.Ranges.Num());
	for (const FAGX_SensorMsgsLaserEcho& Value : InMsg.Ranges)
	{
		Msg.ranges.push_back(Convert(Value));
	}

	Msg.intensities.reserve(InMsg.Intensities.Num());
	for (const FAGX_SensorMsgsLaserEcho& Value : InMsg.Intensities)
	{
		Msg.intensities.push_back(Convert(Value));
	}

	return Msg;
}

inline agxROS2::sensorMsgs::NavSatStatus Convert(const FAGX_SensorMsgsNavSatStatus& InMsg)
{
	agxROS2::sensorMsgs::NavSatStatus Msg;
	Msg.status = InMsg.Status;
	Msg.service = static_cast<uint16_t>(InMsg.Service);

	return Msg;
}

inline agxROS2::sensorMsgs::NavSatFix Convert(const FAGX_SensorMsgsNavSatFix& InMsg)
{
	agxROS2::sensorMsgs::NavSatFix Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.status = Convert(InMsg.Status);
	Msg.latitude = InMsg.Latitude;
	Msg.longitude = InMsg.Longitude;
	Msg.altitude = InMsg.Altitude;

	{
		const int32 Max = std::min(9, InMsg.PositionCovariance.Num());
		for (int i = 0; i < Max; i++)
			Msg.position_covariance[i] = InMsg.PositionCovariance[i];
	}

	Msg.position_covariance_type = InMsg.PositionCovarianceType;

	return Msg;
}

inline agxROS2::sensorMsgs::PointCloud Convert(const FAGX_SensorMsgsPointCloud& InMsg)
{
	agxROS2::sensorMsgs::PointCloud Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.points.reserve(InMsg.Points.Num());
	Msg.channels.reserve(InMsg.Channels.Num());

	for (const FAGX_GeometryMsgsPoint32& Value : InMsg.Points)
	{
		Msg.points.push_back(Convert(Value));
	}

	for (const FAGX_SensorMsgsChannelFloat32& Value : InMsg.Channels)
	{
		Msg.channels.push_back(Convert(Value));
	}

	return Msg;
}

inline agxROS2::sensorMsgs::PointField Convert(const FAGX_SensorMsgsPointField& InMsg)
{
	agxROS2::sensorMsgs::PointField Msg;
	Msg.name = TCHAR_TO_UTF8(*InMsg.Name);
	Msg.offset = static_cast<uint32_t>(InMsg.Offset);
	Msg.datatype = InMsg.Datatype;
	Msg.count = static_cast<uint32_t>(InMsg.Count);

	return Msg;
}

inline agxROS2::sensorMsgs::PointCloud2 Convert(const FAGX_SensorMsgsPointCloud2& InMsg)
{
	agxROS2::sensorMsgs::PointCloud2 Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.height = InMsg.Height;
	Msg.width = InMsg.Width;
	Msg.fields.reserve(InMsg.Fields.Num());

	for (const FAGX_SensorMsgsPointField& Value : InMsg.Fields)
	{
		Msg.fields.push_back(Convert(Value));
	}

	Msg.is_bigendian = InMsg.IsBigendian;
	Msg.point_step = InMsg.PointStep;
	Msg.row_step = InMsg.RowStep;
	Msg.is_dense = InMsg.IsDense;

	Msg.data.reserve(InMsg.Data.Num());
	for (uint8 Value : InMsg.Data)
	{
		Msg.data.push_back(Value);
	}

	return Msg;
}

inline agxROS2::sensorMsgs::Range Convert(const FAGX_SensorMsgsRange& InMsg)
{
	agxROS2::sensorMsgs::Range Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.radiation_type = InMsg.RadiationType;
	Msg.field_of_view = InMsg.FieldOfView;
	Msg.min_range = InMsg.MinRange;
	Msg.max_range = InMsg.MaxRange;
	Msg.range = InMsg.Range;

	return Msg;
}

inline agxROS2::sensorMsgs::RegionOfInterest Convert(const FAGX_SensorMsgsRegionOfInterest& InMsg)
{
	agxROS2::sensorMsgs::RegionOfInterest Msg;
	Msg.x_offset = InMsg.XOffset;
	Msg.y_offset = InMsg.YOffset;
	Msg.height = InMsg.Height;
	Msg.width = InMsg.Width;
	Msg.do_rectify = InMsg.DoRectify;

	return Msg;
}

inline agxROS2::sensorMsgs::CameraInfo Convert(const FAGX_SensorMsgsCameraInfo& InMsg)
{
	agxROS2::sensorMsgs::CameraInfo Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.height = static_cast<uint32_t>(InMsg.Height);
	Msg.width = static_cast<uint32_t>(InMsg.Width);
	Msg.distortion_model = TCHAR_TO_UTF8(*InMsg.DistortionModel);

	Msg.d.reserve(InMsg.D.Num());
	for (double Value : InMsg.D)
	{
		Msg.d.push_back(Value);
	}

	{
		const int32 Maxk = std::min(InMsg.K.Num(), 9);
		for (int32 i = 0; i < Maxk; i++)
			Msg.k[i] = InMsg.K[i];
	}

	{
		const int32 Maxr = std::min(InMsg.R.Num(), 9);
		for (int32 i = 0; i < Maxr; i++)
			Msg.r[i] = InMsg.R[i];
	}

	{
		const int32 Maxp = std::min(InMsg.P.Num(), 12);
		for (int32 i = 0; i < Maxp; i++)
			Msg.p[i] = InMsg.P[i];
	}

	Msg.binning_x = static_cast<uint32_t>(InMsg.BinningX);
	Msg.binning_y = static_cast<uint32_t>(InMsg.BinningY);
	Msg.roi = Convert(InMsg.ROI);

	return Msg;
}

inline agxROS2::sensorMsgs::RelativeHumidity Convert(const FAGX_SensorMsgsRelativeHumidity& InMsg)
{
	agxROS2::sensorMsgs::RelativeHumidity Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.relative_humidity = InMsg.RelativeHumidity;
	Msg.variance = InMsg.Variance;

	return Msg;
}

inline agxROS2::sensorMsgs::Temperature Convert(const FAGX_SensorMsgsTemperature& InMsg)
{
	agxROS2::sensorMsgs::Temperature Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.temperature = InMsg.Temperature;
	Msg.variance = InMsg.Variance;

	return Msg;
}

inline agxROS2::sensorMsgs::TimeReference Convert(const FAGX_SensorMsgsTimeReference& InMsg)
{
	agxROS2::sensorMsgs::TimeReference Msg;
	Msg.header = Convert(InMsg.Header);
	Msg.time_ref = Convert(InMsg.TimeRef);
	Msg.source = TCHAR_TO_UTF8(*InMsg.Source);

	return Msg;
}
