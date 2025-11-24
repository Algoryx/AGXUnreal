// Copyright 2025, Algoryx Simulation AB.

#include "ROS2/ROS2AnyMessageBuilderBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXROS2Types.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"

FROS2AnyMessageBuilderBarrier::FROS2AnyMessageBuilderBarrier()
{
}

FROS2AnyMessageBuilderBarrier::~FROS2AnyMessageBuilderBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr Native's destructor must be able to see the definition,
	// not just the forward declaration of the Native.
}

FROS2AnyMessageBuilderBarrier::FROS2AnyMessageBuilderBarrier(
	FROS2AnyMessageBuilderBarrier&& Other) noexcept
{
	*this = std::move(Other);
}

FROS2AnyMessageBuilderBarrier& FROS2AnyMessageBuilderBarrier::operator=(
	FROS2AnyMessageBuilderBarrier&& Other) noexcept
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	return *this;
}

bool FROS2AnyMessageBuilderBarrier::HasNative() const
{
	return Native != nullptr && Native->Native != nullptr;
}

void FROS2AnyMessageBuilderBarrier::AllocateNative()
{
	check(!HasNative());
	Native = std::make_unique<FAnyMessageBuilder>(new agxROS2::AnyMessageBuilder());
}

FAnyMessageBuilder* FROS2AnyMessageBuilderBarrier::GetNative()
{
	return Native.get();
}

const FAnyMessageBuilder* FROS2AnyMessageBuilderBarrier::GetNative() const
{
	return Native.get();
}

void FROS2AnyMessageBuilderBarrier::ReleaseNative()
{
	Native = nullptr;
}

void FROS2AnyMessageBuilderBarrier::BeginMessage()
{
	check(HasNative());
	Native->Native->beginMessage();
}

void FROS2AnyMessageBuilderBarrier::WriteInt8(int8 d)
{
	check(HasNative());
	Native->Native->writeInt8(d);
}

void FROS2AnyMessageBuilderBarrier::WriteUInt8(uint8 d)
{
	check(HasNative());
	Native->Native->writeUInt8(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt16(int16 d)
{
	check(HasNative());
	Native->Native->writeInt16(d);
}

void FROS2AnyMessageBuilderBarrier::WriteUInt16(uint16 d)
{
	check(HasNative());
	Native->Native->writeUInt16(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt32(int32 d)
{
	check(HasNative());
	Native->Native->writeInt32(d);
}

void FROS2AnyMessageBuilderBarrier::WriteUInt32(uint32 d)
{
	check(HasNative());
	Native->Native->writeUInt32(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt64(int64 d)
{
	check(HasNative());
	Native->Native->writeInt64(d);
}

void FROS2AnyMessageBuilderBarrier::WriteUInt64(uint64 d)
{
	check(HasNative());
	Native->Native->writeUInt64(d);
}

void FROS2AnyMessageBuilderBarrier::WriteFloat32(float d)
{
	check(HasNative());
	Native->Native->writeFloat32(d);
}

void FROS2AnyMessageBuilderBarrier::WriteDouble64(double d)
{
	check(HasNative());
	Native->Native->writeDouble64(d);
}

void FROS2AnyMessageBuilderBarrier::WriteString(const FString& d)
{
	check(HasNative());
	Native->Native->writeString(Convert(d));
}

void FROS2AnyMessageBuilderBarrier::WriteBool(bool d)
{
	check(HasNative());
	Native->Native->writeBool(d);
}

void FROS2AnyMessageBuilderBarrier::WriteInt8Sequence(const TArray<int8>& d)
{
	check(HasNative());
	Native->Native->writeInt8Sequence(ToStdArray<int8, int8_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt8Sequence(const TArray<uint8>& d)
{
	check(HasNative());
	Native->Native->writeUInt8Sequence(ToStdArray<uint8, uint8_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteInt16Sequence(const TArray<int16>& d)
{
	check(HasNative());
	Native->Native->writeInt16Sequence(ToStdArray<int16, int16_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt16Sequence(const TArray<uint16>& d)
{
	check(HasNative());
	Native->Native->writeUInt16Sequence(ToStdArray<uint16, uint16_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteInt32Sequence(const TArray<int32>& d)
{
	check(HasNative());
	Native->Native->writeInt32Sequence(ToStdArray<int32, int32_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt32Sequence(const TArray<uint32>& d)
{
	check(HasNative());
	Native->Native->writeUInt32Sequence(ToStdArray<uint32, uint32_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteInt64Sequence(const TArray<int64>& d)
{
	check(HasNative());
	Native->Native->writeInt64Sequence(ToStdArray<int64, int64_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteUInt64Sequence(const TArray<uint64>& d)
{
	check(HasNative());
	Native->Native->writeUInt64Sequence(ToStdArray<uint64, uint64_t>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteFloat32Sequence(const TArray<float>& d)
{
	check(HasNative());
	Native->Native->writeFloat32Sequence(ToStdArray<float, float>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteDouble64Sequence(const TArray<double>& d)
{
	check(HasNative());
	Native->Native->writeDouble64Sequence(ToStdArray<double, double>(d));
}

void FROS2AnyMessageBuilderBarrier::WriteStringSequence(const TArray<FString>& d)
{
	check(HasNative());
	Native->Native->writeStringSequence(ToStdStringArray(d));
}

void FROS2AnyMessageBuilderBarrier::WriteBoolSequence(const TArray<bool>& d)
{
	check(HasNative());
	Native->Native->writeBoolSequence(ToStdArray<bool, bool>(d));
}

FAGX_AgxMsgsAny FROS2AnyMessageBuilderBarrier::GetBuiltMessage() const
{
	return Convert(Native->Native->getMessage());
}
