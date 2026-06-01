// Copyright 2026, Algoryx Simulation AB.

#include "ROS2/AGX_ROS2AnyMessageBuilderComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "Utilities/AGX_StringUtilities.h"

UAGX_ROS2AnyMessageBuilderComponent::UAGX_ROS2AnyMessageBuilderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UAGX_ROS2AnyMessageBuilderComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

FROS2AnyMessageBuilderBarrier* UAGX_ROS2AnyMessageBuilderComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FROS2AnyMessageBuilderBarrier* UAGX_ROS2AnyMessageBuilderComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

void UAGX_ROS2AnyMessageBuilderComponent::BeginPlay()
{
	Super::BeginPlay();
	check(!HasNative());
	NativeBarrier.AllocateNative();
}

void UAGX_ROS2AnyMessageBuilderComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (HasNative())
		NativeBarrier.ReleaseNative();
}

namespace AGX_ROS2AnyMessageBuilderComponent_helpers
{
	void PrintMissingNativeWarning(
		const FString& FunctionName, const UAGX_ROS2AnyMessageBuilderComponent& Component)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"'%s' was called on Any Message Builder Component '%s' in Actor '%s' that does not "
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

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::BeginMessage()
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("BeginMessage", *this);
		return this;
	}

	NativeBarrier.BeginMessage();
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt8(int32 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt8", *this);
		return this;
	}

	NativeBarrier.WriteInt8(static_cast<int8_t>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt8(uint8 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt8", *this);
		return this;
	}

	NativeBarrier.WriteUInt8(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt16(int32 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt16", *this);
		return this;
	}

	NativeBarrier.WriteInt16(static_cast<int16_t>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt16(int32 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt16", *this);
		return this;
	}

	NativeBarrier.WriteUInt16(static_cast<uint16_t>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt32(int32 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("writeInt32", *this);
		return this;
	}

	NativeBarrier.WriteInt32(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt32(int64 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt32", *this);
		return this;
	}

	NativeBarrier.WriteUInt32(static_cast<uint32_t>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt64(int64 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt64", *this);
		return this;
	}

	NativeBarrier.WriteInt64(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt64(int64 Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt64", *this);
		return this;
	}

	NativeBarrier.WriteUInt64(static_cast<uint64_t>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteFloat32(float Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteFloat32", *this);
		return this;
	}

	NativeBarrier.WriteFloat32(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteDouble64(double Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteDouble64", *this);
		return this;
	}

	NativeBarrier.WriteDouble64(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteString(
	const FString& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteString", *this);
		return this;
	}

	NativeBarrier.WriteString(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteBool(bool Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteBool", *this);
		return this;
	}

	NativeBarrier.WriteBool(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt8Sequence(
	const TArray<int32>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt8Sequence", *this);
		return this;
	}

	NativeBarrier.WriteInt8Sequence(ToIntArray<int32, int8>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt8Sequence(
	const TArray<uint8>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt8Sequence", *this);
		return this;
	}

	NativeBarrier.WriteUInt8Sequence(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt16Sequence(
	const TArray<int32>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt16Sequence", *this);
		return this;
	}

	NativeBarrier.WriteInt16Sequence(ToIntArray<int32, int16>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt16Sequence(
	const TArray<int32>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt16Sequence", *this);
		return this;
	}

	NativeBarrier.WriteUInt16Sequence(ToIntArray<int32, uint16>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt32Sequence(
	const TArray<int32>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt32Sequence", *this);
		return this;
	}

	NativeBarrier.WriteInt32Sequence(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt32Sequence(
	const TArray<int64>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt32Sequence", *this);
		return this;
	}

	NativeBarrier.WriteUInt32Sequence(ToIntArray<int64, uint32>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteInt64Sequence(
	const TArray<int64>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteInt64Sequence", *this);
		return this;
	}

	NativeBarrier.WriteInt64Sequence(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteUInt64Sequence(
	const TArray<int64>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteUInt64Sequence", *this);
		return this;
	}

	NativeBarrier.WriteUInt64Sequence(ToIntArray<int64, uint64>(Data));
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteFloat32Sequence(
	const TArray<float>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteFloat32Sequence", *this);
		return this;
	}

	NativeBarrier.WriteFloat32Sequence(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteDouble64Sequence(
	const TArray<double>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteDouble64Sequence", *this);
		return this;
	}

	NativeBarrier.WriteDouble64Sequence(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteStringSequence(
	const TArray<FString>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteStringSequence", *this);
		return this;
	}

	NativeBarrier.WriteStringSequence(Data);
	return this;
}

UAGX_ROS2AnyMessageBuilderComponent* UAGX_ROS2AnyMessageBuilderComponent::WriteBoolSequence(
	const TArray<bool>& Data)
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("WriteBoolSequence", *this);
		return this;
	}

	NativeBarrier.WriteBoolSequence(Data);
	return this;
}

FAGX_AgxMsgsAny UAGX_ROS2AnyMessageBuilderComponent::GetMessage() const
{
	using namespace AGX_ROS2AnyMessageBuilderComponent_helpers;
	if (!HasNative())
	{
		PrintMissingNativeWarning("GetMessage", *this);
		return FAGX_AgxMsgsAny();
	}

	return NativeBarrier.GetBuiltMessage();
}
