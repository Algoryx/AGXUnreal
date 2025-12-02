// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include <Logging/LogVerbosity.h>

#include <memory>

struct FNotifyRef;

class FNotifyBarrier
{
public:
	FNotifyBarrier();
	FNotifyBarrier(std::unique_ptr<FNotifyRef> Native);
	FNotifyBarrier(FNotifyBarrier&& Other);
	~FNotifyBarrier();

	bool HasNative() const;
	FNotifyRef* GetNative();
	const FNotifyRef* GetNative() const;

	// Note: not allowed to call StartAgxNotify twice in a row.
	void StartAgxNotify(ELogVerbosity::Type LogVerbosity);
	void StopAgxNotify();

private:
	FNotifyBarrier(const FNotifyBarrier&) = delete;
	void operator=(const FNotifyBarrier&) = delete;

private:
	std::unique_ptr<FNotifyRef> NativeRef;
};
