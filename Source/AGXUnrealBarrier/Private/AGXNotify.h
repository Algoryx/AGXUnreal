// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Notify.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Logging/LogVerbosity.h"

class FAGXNotify : public agx::NotifyCallback
{
public:
	void StartAgxNotify(ELogVerbosity::Type LogVerbosity);
	void StopAgxNotify();

private:
	virtual void message(const agx::String& msg, int notifyLevel) override;
};

struct FNotifyRef
{
	agx::ref_ptr<FAGXNotify> Native;

	FNotifyRef() = default;
	FNotifyRef(FAGXNotify* InNative)
		: Native(InNative)
	{
	}
};
