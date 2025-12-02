// Copyright 2025, Algoryx Simulation AB.

#include "ROS2/ROS2AnyMessageParserBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXROS2Types.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "ROS2/ROS2Conversions.h"
#include "TypeConversions.h"
#include "Utilities/ROS2Utilities.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

FROS2AnyMessageParserBarrier::FROS2AnyMessageParserBarrier()
{
}

FROS2AnyMessageParserBarrier::~FROS2AnyMessageParserBarrier()
{
	// Message must be free'd using AGX_ROS2Utilities::FreeContainers.
	// This is similar to agxUtil::freeContainerMemory but on the Unreal-side.
	// Not doing this may cause a runtime crash on object destruction.
	if (Message != nullptr && Message->Native != nullptr)
	{
		AGX_ROS2Utilities::FreeContainers(*Message->Native);
	}
}

FROS2AnyMessageParserBarrier::FROS2AnyMessageParserBarrier(
	FROS2AnyMessageParserBarrier&& Other) noexcept
{
	*this = std::move(Other);
}

FROS2AnyMessageParserBarrier& FROS2AnyMessageParserBarrier::operator=(
	FROS2AnyMessageParserBarrier&& Other) noexcept
{
	Native = std::move(Other.Native);
	Other.Native = nullptr;
	return *this;
}

bool FROS2AnyMessageParserBarrier::HasNative() const
{
	return Native != nullptr && Native->Native != nullptr;
}

void FROS2AnyMessageParserBarrier::AllocateNative()
{
	check(!HasNative());
	Native = std::make_unique<FAnyMessageParser>(new agxROS2::AnyMessageParser());
}

FAnyMessageParser* FROS2AnyMessageParserBarrier::GetNative()
{
	return Native.get();
}

const FAnyMessageParser* FROS2AnyMessageParserBarrier::GetNative() const
{
	return Native.get();
}

void FROS2AnyMessageParserBarrier::ReleaseNative()
{
	Native = nullptr;
}

void FROS2AnyMessageParserBarrier::BeginParse(const FAGX_AgxMsgsAny& InMessage)
{
	check(HasNative());

	// Any previously used Message must be free'd using AGX_ROS2Utilities::FreeContainers.
	// This is similar to agxUtil::freeContainerMemory but on the Unreal-side.
	// Not doing this may cause a runtime crash on object destruction.
	if (Message != nullptr && Message->Native != nullptr)
	{
		AGX_ROS2Utilities::FreeContainers(*Message->Native);
	}

	{
		// Here we store away a copy (of AGX type) of the message, used for subsequent reads.
		agxROS2::agxMsgs::Any* MessageAGX = new agxROS2::agxMsgs::Any();
		*MessageAGX = Convert(InMessage);
		Message = std::make_unique<FAgxAny>(MessageAGX);
	}

	Native->Native->beginParse();
}

namespace ROS2AnyMessageParserBarrier_helpers
{
	void PrintMissingMessageWarning()
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Tried to read using a FROS2AnyMessageParserBarrier that does not have a reference "
				"to a Message. Ensure BeginParse has been called before calling this function."));
	}
}

int8 FROS2AnyMessageParserBarrier::ReadInt8()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readInt8(*Message->Native);
}

uint8 FROS2AnyMessageParserBarrier::ReadUInt8()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readUInt8(*Message->Native);
}

int16 FROS2AnyMessageParserBarrier::ReadInt16()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readInt16(*Message->Native);
}

uint16 FROS2AnyMessageParserBarrier::ReadUInt16()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readUInt16(*Message->Native);
}

int32 FROS2AnyMessageParserBarrier::ReadInt32()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readInt32(*Message->Native);
}

uint32 FROS2AnyMessageParserBarrier::ReadUInt32()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readUInt32(*Message->Native);
}

int64 FROS2AnyMessageParserBarrier::ReadInt64()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readInt64(*Message->Native);
}

uint64 FROS2AnyMessageParserBarrier::ReadUInt64()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0;
	}

	return Native->Native->readUInt64(*Message->Native);
}

float FROS2AnyMessageParserBarrier::ReadFloat32()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0.f;
	}

	return Native->Native->readFloat32(*Message->Native);
}

double FROS2AnyMessageParserBarrier::ReadDouble64()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return 0.0;
	}

	return Native->Native->readDouble64(*Message->Native);
}

FString FROS2AnyMessageParserBarrier::ReadString()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return FString();
	}

	FString StrUnreal;
	{
		std::string StrAGX = Native->Native->readString(*Message->Native);
		StrUnreal = Convert(StrAGX);

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(StrAGX);
	}

	return StrUnreal;
}

bool FROS2AnyMessageParserBarrier::ReadBool()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return false;
	}

	return Native->Native->readBool(*Message->Native);
}

TArray<int8> FROS2AnyMessageParserBarrier::ReadInt8Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<int8>();
	}

	return ToUnrealArray<int8_t, int8>(Native->Native->readInt8Sequence(*Message->Native));
}

TArray<uint8> FROS2AnyMessageParserBarrier::ReadUInt8Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<uint8>();
	}

	return ToUnrealArray<uint8_t, uint8>(Native->Native->readUInt8Sequence(*Message->Native));
}

TArray<int16> FROS2AnyMessageParserBarrier::ReadInt16Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<int16>();
	}

	return ToUnrealArray<int16_t, int16>(Native->Native->readInt16Sequence(*Message->Native));
}

TArray<uint16> FROS2AnyMessageParserBarrier::ReadUInt16Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<uint16>();
	}

	return ToUnrealArray<uint16_t, uint16>(Native->Native->readUInt16Sequence(*Message->Native));
}

TArray<int32> FROS2AnyMessageParserBarrier::ReadInt32Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<int32>();
	}

	return ToUnrealArray<int32_t, int32>(Native->Native->readInt32Sequence(*Message->Native));
}

TArray<uint32> FROS2AnyMessageParserBarrier::ReadUInt32Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<uint32>();
	}

	return ToUnrealArray<uint32_t, uint32>(Native->Native->readUInt32Sequence(*Message->Native));
}

TArray<int64> FROS2AnyMessageParserBarrier::ReadInt64Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<int64>();
	}

	return ToUnrealArray<int64_t, int64>(Native->Native->readInt64Sequence(*Message->Native));
}

TArray<uint64> FROS2AnyMessageParserBarrier::ReadUInt64Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<uint64>();
	}

	return ToUnrealArray<uint64_t, uint64>(Native->Native->readUInt64Sequence(*Message->Native));
}

TArray<float> FROS2AnyMessageParserBarrier::ReadFloat32Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<float>();
	}

	return ToUnrealArray<float, float>(Native->Native->readFloat32Sequence(*Message->Native));
}

TArray<double> FROS2AnyMessageParserBarrier::ReadDouble64Sequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<double>();
	}

	return ToUnrealArray<double, double>(Native->Native->readDouble64Sequence(*Message->Native));
}

TArray<FString> FROS2AnyMessageParserBarrier::ReadStringSequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<FString>();
	}

	TArray<FString> StrArrUnreal;
	{
		std::vector<std::string> StrVecAGX = Native->Native->readStringSequence(*Message->Native);
		StrArrUnreal = ToUnrealStringArray(StrVecAGX);

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(StrVecAGX);
	}
	return StrArrUnreal;
}

TArray<bool> FROS2AnyMessageParserBarrier::ReadBoolSequence()
{
	check(HasNative());
	if (Message == nullptr)
	{
		ROS2AnyMessageParserBarrier_helpers::PrintMissingMessageWarning();
		return TArray<bool>();
	}

	return ToUnrealArray<bool, bool>(Native->Native->readBoolSequence(*Message->Native));
}
