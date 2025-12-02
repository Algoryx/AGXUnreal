// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <cstdint>
#include <memory>

struct FAgxAny;
struct FAnyMessageParser;
struct FAGX_AgxMsgsAny;

class AGXUNREALBARRIER_API FROS2AnyMessageParserBarrier
{
public:
	FROS2AnyMessageParserBarrier();
	~FROS2AnyMessageParserBarrier();
	FROS2AnyMessageParserBarrier(FROS2AnyMessageParserBarrier&& Other) noexcept;
	FROS2AnyMessageParserBarrier& operator=(FROS2AnyMessageParserBarrier&& Other) noexcept;

	bool HasNative() const;

	void AllocateNative();

	FAnyMessageParser* GetNative();
	const FAnyMessageParser* GetNative() const;

	void ReleaseNative();

	void BeginParse(const FAGX_AgxMsgsAny& Message);
	int8 ReadInt8();
	uint8 ReadUInt8();
	int16 ReadInt16();
	uint16 ReadUInt16();
	int32 ReadInt32();
	uint32 ReadUInt32();
	int64 ReadInt64();
	uint64 ReadUInt64();
	float ReadFloat32();
	double ReadDouble64();
	FString ReadString();
	bool ReadBool();

	TArray<int8> ReadInt8Sequence();
	TArray<uint8> ReadUInt8Sequence();
	TArray<int16> ReadInt16Sequence();
	TArray<uint16> ReadUInt16Sequence();
	TArray<int32> ReadInt32Sequence();
	TArray<uint32> ReadUInt32Sequence();
	TArray<int64> ReadInt64Sequence();
	TArray<uint64> ReadUInt64Sequence();
	TArray<float> ReadFloat32Sequence();
	TArray<double> ReadDouble64Sequence();
	TArray<FString> ReadStringSequence();
	TArray<bool> ReadBoolSequence();

private:
	FROS2AnyMessageParserBarrier(const FROS2AnyMessageParserBarrier&) = delete;
	void operator=(const FROS2AnyMessageParserBarrier&) = delete;

private:
	std::unique_ptr<FAnyMessageParser> Native;
	std::unique_ptr<FAgxAny> Message;
};
