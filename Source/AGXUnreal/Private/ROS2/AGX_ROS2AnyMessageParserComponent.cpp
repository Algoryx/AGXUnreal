// Copyright 2026, Algoryx Simulation AB.

#include "ROS2/AGX_ROS2AnyMessageParserComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "Utilities/AGX_StringUtilities.h"

UAGX_ROS2AnyMessageParserComponent::UAGX_ROS2AnyMessageParserComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ROS2AnyMessageParserComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FROS2AnyMessageParserBarrier* UAGX_ROS2AnyMessageParserComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FROS2AnyMessageParserBarrier* UAGX_ROS2AnyMessageParserComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

void UAGX_ROS2AnyMessageParserComponent::BeginPlay()
{
	Super::BeginPlay();
	check(!HasNative());
	NativeBarrier.AllocateNative();
}

void UAGX_ROS2AnyMessageParserComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative())
		NativeBarrier.ReleaseNative();
}

namespace AGX_ROS2AnyMessageParserComponent_helpers
{
	void PrintMissingNativeWarning(
		const FString& FunctionName, const UAGX_ROS2AnyMessageParserComponent& Component)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' was called on Any Message Parser Component '%s' in Actor '%s' that does not "
				 "have a Native object. Only call this function during Play."),
			*FunctionName, *Component.GetName(), *GetLabelSafe(Component.GetOwner()));
	}

	template <typename FromType, typename ToType>
	TArray<ToType> ToIntArray(const TArray<FromType>& A)
	{
		TArray<ToType> Array;
		Array.Reserve(A.Num());
		for (auto Val : A)
			Array.Add(static_cast<ToType>(Val));

		return Array;
	}
}

UAGX_ROS2AnyMessageParserComponent* UAGX_ROS2AnyMessageParserComponent::BeginParse(
	const FAGX_AgxMsgsAny& Message)
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("BeginParse", *this);
		return this;
	}

	NativeBarrier.BeginParse(Message);
	return this;
}

int32 UAGX_ROS2AnyMessageParserComponent::ReadInt8()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt8", *this);
		return 0;
	}

	return static_cast<int32>(NativeBarrier.ReadInt8());
}

uint8 UAGX_ROS2AnyMessageParserComponent::ReadUInt8()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt8", *this);
		return 0;
	}

	return NativeBarrier.ReadUInt8();
}

int32 UAGX_ROS2AnyMessageParserComponent::ReadInt16()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt16", *this);
		return 0;
	}

	return static_cast<int32>(NativeBarrier.ReadInt16());
}

int32 UAGX_ROS2AnyMessageParserComponent::ReadUInt16()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt16", *this);
		return 0;
	}

	return static_cast<int32>(NativeBarrier.ReadUInt16());
}

int32 UAGX_ROS2AnyMessageParserComponent::ReadInt32()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt32", *this);
		return 0;
	}

	return NativeBarrier.ReadInt32();
}

int64 UAGX_ROS2AnyMessageParserComponent::ReadUInt32()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt32", *this);
		return 0;
	}

	return static_cast<int64>(NativeBarrier.ReadUInt32());
}

int64 UAGX_ROS2AnyMessageParserComponent::ReadInt64()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt64", *this);
		return 0;
	}

	return NativeBarrier.ReadInt64();
}

int64 UAGX_ROS2AnyMessageParserComponent::ReadUInt64()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt64", *this);
		return 0;
	}

	return static_cast<int64>(NativeBarrier.ReadUInt64());
}

float UAGX_ROS2AnyMessageParserComponent::ReadFloat32()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadFloat32", *this);
		return 0.f;
	}

	return NativeBarrier.ReadFloat32();
}

double UAGX_ROS2AnyMessageParserComponent::ReadDouble64()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadDouble64", *this);
		return 0.0;
	}

	return NativeBarrier.ReadDouble64();
}

FString UAGX_ROS2AnyMessageParserComponent::ReadString()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadString", *this);
		return FString();
	}

	return NativeBarrier.ReadString();
}

bool UAGX_ROS2AnyMessageParserComponent::ReadBool()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadBool", *this);
		return false;
	}

	return NativeBarrier.ReadBool();
}

TArray<int32> UAGX_ROS2AnyMessageParserComponent::ReadInt8Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt8Sequence", *this);
		return TArray<int32>();
	}

	return ToIntArray<int8, int32>(NativeBarrier.ReadInt8Sequence());
}

TArray<uint8> UAGX_ROS2AnyMessageParserComponent::ReadUInt8Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt8Sequence", *this);
		return TArray<uint8>();
	}

	return NativeBarrier.ReadUInt8Sequence();
}

TArray<int32> UAGX_ROS2AnyMessageParserComponent::ReadInt16Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt16Sequence", *this);
		return TArray<int32>();
	}

	return ToIntArray<int16, int32>(NativeBarrier.ReadInt16Sequence());
}

TArray<int32> UAGX_ROS2AnyMessageParserComponent::ReadUInt16Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt16Sequence", *this);
		return TArray<int32>();
	}

	return ToIntArray<uint16, int32>(NativeBarrier.ReadUInt16Sequence());
}

TArray<int32> UAGX_ROS2AnyMessageParserComponent::ReadInt32Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt32Sequence", *this);
		return TArray<int32>();
	}

	return NativeBarrier.ReadInt32Sequence();
}

TArray<int64> UAGX_ROS2AnyMessageParserComponent::ReadUInt32Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt32Sequence", *this);
		return TArray<int64>();
	}

	return ToIntArray<uint32, int64>(NativeBarrier.ReadUInt32Sequence());
}

TArray<int64> UAGX_ROS2AnyMessageParserComponent::ReadInt64Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadInt64Sequence", *this);
		return TArray<int64>();
	}

	return NativeBarrier.ReadInt64Sequence();
}

TArray<int64> UAGX_ROS2AnyMessageParserComponent::ReadUInt64Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadUInt64Sequence", *this);
		return TArray<int64>();
	}

	return ToIntArray<uint64, int64>(NativeBarrier.ReadUInt64Sequence());
}

TArray<float> UAGX_ROS2AnyMessageParserComponent::ReadFloat32Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadFloat32Sequence", *this);
		return TArray<float>();
	}

	return NativeBarrier.ReadFloat32Sequence();
}

TArray<double> UAGX_ROS2AnyMessageParserComponent::ReadDouble64Sequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadDouble64Sequence", *this);
		return TArray<double>();
	}

	return NativeBarrier.ReadDouble64Sequence();
}

TArray<FString> UAGX_ROS2AnyMessageParserComponent::ReadStringSequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadStringSequence", *this);
		return TArray<FString>();
	}

	return NativeBarrier.ReadStringSequence();
}

TArray<bool> UAGX_ROS2AnyMessageParserComponent::ReadBoolSequence()
{
	using namespace AGX_ROS2AnyMessageParserComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("ReadBoolSequence", *this);
		return TArray<bool>();
	}

	return NativeBarrier.ReadBoolSequence();
}
