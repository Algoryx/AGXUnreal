// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxNet/WebDebuggerServer.h>
#include "EndAGXIncludes.h"

// Standard libary includes.
#include <memory>

struct FWebDebuggerServerRef
{
	std::shared_ptr<agxNet::agxWebServer::WebDebuggerServer> Native;
	FWebDebuggerServerRef() = default;
};
