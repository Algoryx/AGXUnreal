// Copyright 2026, Algoryx Simulation AB.

#include "Vehicle/AGX_AckermannSteeringParameters.h"

UAGX_AckermannSteeringParameters::UAGX_AckermannSteeringParameters()
{
	SteeringData.Phi0 = -115.0;
	SteeringData.L = 0.16;
	SteeringData.Alpha0 = 0.0;
	SteeringData.Lc = 0.0;
	SteeringData.Lr = 0.0;
	SteeringData.Gear = 1.0;
	SteeringData.Side = 0;
}
