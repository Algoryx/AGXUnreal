// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_SteeringParametersData.generated.h"

/**
 * Struct holding data for Steering parameters, used for e.g. Ackermann, BellCrank and others.
 */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_SteeringParametersData
{
	GENERATED_BODY()

	FAGX_SteeringParametersData() = default;

	FAGX_SteeringParametersData(
		double InPhi0, double InL, double InAlpha0, double InLc, double InLr, double InGear,
		uint32 InSide)
		: Phi0(InPhi0)
		, L(InL)
		, Alpha0(InAlpha0)
		, Lc(InLc)
		, Lr(InLr)
		, Gear(InGear)
		, Side(InSide)
	{
	}

	/**
	 * Initial angle of the kingpin/knuckle of the right wheel [deg].
	 * Measured from the direction along the axle, pointing to the right.
	 * With Phi0 = 0, the knuckle points directly along the axle (bad).
	 * With Phi0 = pi/2, the kingpin is parallel to the wheel (also not ideal).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	double Phi0 {0.0};

	/**
	 * Length of the knuckle as a fraction of the wheel base.
	 * Wheel base is the distance between centers of the two wheels on the same axle.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	double L {0.0};

	/**
	 * Initial angle of the right tie rod which connects the knuckle to the steering column or rack
	 * [deg]. Measured from the axle of the right wheel.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	double Alpha0 {0.0};

	/**
	 * Distance (fraction) of the steering column from the tie rods along the line connecting the
	 * wheels, as a fraction of the knuckle length. Only relevant for mechanisms like Bell crank.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	double Lc {0.0};

	/**
	 * Rack length as a fraction of the wheel base for rack and pinion steering.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	double Lr {0.0};

	/**
	 * Gear ratio between steering wheel and the control angle of the linkage mechanism.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	double Gear {0.0};

	/**
	 * Side of the steering column position: 0 = left wheel, 1 = right wheel.
	 * Specially used for Ackermann steering mechanisms.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering")
	int32 Side {0};
};
