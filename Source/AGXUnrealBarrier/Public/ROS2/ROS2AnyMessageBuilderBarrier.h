// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct FAGX_AgxMsgsAny;
struct FAnyMessageBuilder;

class AGXUNREALBARRIER_API FROS2AnyMessageBuilderBarrier
{
public:
	FROS2AnyMessageBuilderBarrier();
	~FROS2AnyMessageBuilderBarrier();
	FROS2AnyMessageBuilderBarrier(FROS2AnyMessageBuilderBarrier&& Other) noexcept;
	FROS2AnyMessageBuilderBarrier& operator=(FROS2AnyMessageBuilderBarrier&& Other) noexcept;

	bool HasNative() const;

	void AllocateNative();

	FAnyMessageBuilder* GetNative();
	const FAnyMessageBuilder* GetNative() const;

	void ReleaseNative();

	void BeginMessage();

	void WriteInt8(int8 d);
	void WriteUInt8(uint8 d);
	void WriteInt16(int16 d);
	void WriteUInt16(uint16 d);
	void WriteInt32(int32 d);
	void WriteUInt32(uint32 d);
	void WriteInt64(int64 d);
	void WriteUInt64(uint64 d);
	void WriteFloat32(float d);
	void WriteDouble64(double d);
	void WriteString(const FString& d);
	void WriteBool(bool d);

	void WriteInt8Sequence(const TArray<int8>& d);
	void WriteUInt8Sequence(const TArray<uint8>& d);
	void WriteInt16Sequence(const TArray<int16>& d);
	void WriteUInt16Sequence(const TArray<uint16>& d);
	void WriteInt32Sequence(const TArray<int32>& d);
	void WriteUInt32Sequence(const TArray<uint32>& d);
	void WriteInt64Sequence(const TArray<int64>& d);
	void WriteUInt64Sequence(const TArray<uint64>& d);
	void WriteFloat32Sequence(const TArray<float>& d);
	void WriteDouble64Sequence(const TArray<double>& d);
	void WriteStringSequence(const TArray<FString>& d);
	void WriteBoolSequence(const TArray<bool>& d);

	FAGX_AgxMsgsAny GetBuiltMessage() const;

private:
	FROS2AnyMessageBuilderBarrier(const FROS2AnyMessageBuilderBarrier&) = delete;
	void operator=(const FROS2AnyMessageBuilderBarrier&) = delete;

private:
	std::unique_ptr<FAnyMessageBuilder> Native;
};
