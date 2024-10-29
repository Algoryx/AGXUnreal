// Copyright 2024, Algoryx Simulation AB.

#include "DriveTrain/CombustionEngineBarrier.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"
#include "BarrierOnly/PowerLine/PowerLineRefs.h"
#include "Utilities/BarrierBase.inl.h"

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agx/version.h>
#include <agxDriveTrain/CombustionEngine.h>
#include <agxDriveTrain/CombustionEngineParameters.h>
#include <EndAGXIncludes.h>

namespace CombustionEngineBarrier_helpers
{
	agxDriveTrain::CombustionEngineParameters Convert(
		const FAGX_CombustionEngineParameters& Parameters)
	{
		agxDriveTrain::CombustionEngineParameters Result;
		Result.displacementVolume = ConvertVolumeToAGX(Parameters.DisplacementVolume);
		UE_LOG(
			LogAGX, Warning, TEXT("Volume: %f cm^3 = %f m^3"), Parameters.DisplacementVolume.Value,
			Result.displacementVolume);
		return Result;
	}

	agxDriveTrain::CombustionEngine* GetAGX(FCombustionEngineBarrier& Barrier)
	{
		return Barrier.GetNative()->Native->as<agxDriveTrain::CombustionEngine>();
	}

	const agxDriveTrain::CombustionEngine* GetAGX(const FCombustionEngineBarrier& Barrier)
	{
		return Barrier.GetNative()->Native->as<const agxDriveTrain::CombustionEngine>();
	}
}

FCombustionEngineBarrier::FCombustionEngineBarrier()
	: FPowerLineUnitBarrier(std::make_unique<FUnitRef>())
{
}

FCombustionEngineBarrier::FCombustionEngineBarrier(std::unique_ptr<FUnitRef> Native)
	: FPowerLineUnitBarrier(std::move(Native))
{
}

FCombustionEngineBarrier::FCombustionEngineBarrier(const FCombustionEngineBarrier& Other)
	: FPowerLineUnitBarrier(std::make_unique<FUnitRef>(Other.NativeRef->Native))
{
}

FCombustionEngineBarrier::FCombustionEngineBarrier(FCombustionEngineBarrier&& Other)
	: FPowerLineUnitBarrier {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FUnitRef());
}

FCombustionEngineBarrier& FCombustionEngineBarrier::operator=(const FCombustionEngineBarrier& Other)
{
	NativeRef->Native = Other.NativeRef->Native;
	return *this;
}

void FCombustionEngineBarrier::AllocateNative(FAGX_CombustionEngineParameters InParameters)
{
	using namespace CombustionEngineBarrier_helpers;
	check(!HasNative());
	agxDriveTrain::CombustionEngineParameters Parameters = Convert(InParameters);
	NativeRef->Native = new agxDriveTrain::CombustionEngine(Parameters);
}


#if AGX_VERSION_GREATER_OR_EQUAL(2, 39, 0, 0)
void FCombustionEngineBarrier::SetCombustionEngineParameters(
	const FAGX_CombustionEngineParameters& InParameters)
{
	check(HasNative());
	using namespace CombustionEngineBarrier_helpers;
	agxDriveTrain::CombustionEngineParameters Parameters = Convert(InParameters);
	NativeRef->Native->setCombustionEngineParameters(Parameters);
}
#else
void FCombustionEngineBarrier::SetCombustionEngineParameters(const FAGX_CombustionEngineParameters&)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("Current version of AGX Dynamics does not support setting combustion engine "
			 "parameters on an already initialized combustion engine. Doing nothing."));
}
#endif

void FCombustionEngineBarrier::SetThrottle(double Throttle)
{
	using namespace CombustionEngineBarrier_helpers;
	check(HasNative());
	agx::Real VolumeAGX = ConvertVolumeToAGX(Throttle);
	GetAGX(*this)->setThrottle(Throttle);
}
