// Copyright 2026, Algoryx Simulation AB.

#pragma once

UENUM(BlueprintType)
enum class EWindAndWaterParametersCoefficient : uint8
{
	PRESSURE_DRAG,	
	VISCOUS_DRAG,	
	LIFT, 	
	BUOYANCY 
};

/**
 * Enum that specify the tessellation levels of native, non-mesh shapes, e.g, sphere, capsule and cylinder,
 * that are associated with hydro collision in the context of WindAndWaterController.
 */
UENUM(BlueprintType)
enum class EWindAndWaterShapeTessellation : uint8
{
	LOW,
	MEDIUM,
	HIGH,
	ULTRA_HIGH,
	DEFAULT_TESSELLATION = MEDIUM,
};
