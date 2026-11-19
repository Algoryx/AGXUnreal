// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_SignalRerouter.h"

// Define the special members in AGXCommon so they are exported for derived classes in other
// modules.
FOpenPLX_SignalRerouter::FOpenPLX_SignalRerouter() = default;

FOpenPLX_SignalRerouter::FOpenPLX_SignalRerouter(const FOpenPLX_SignalRerouter& Other) = default;

FOpenPLX_SignalRerouter& FOpenPLX_SignalRerouter::operator=(const FOpenPLX_SignalRerouter& Other) =
	default;

FOpenPLX_SignalRerouter::~FOpenPLX_SignalRerouter() = default;
