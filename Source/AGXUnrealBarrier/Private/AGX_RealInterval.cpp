// Copyright 2025, Algoryx Simulation AB.

#include "AGX_RealInterval.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/DoubleInterval.h"

// Unreal Engine includes.
#include "Math/Interval.h"
#include "Misc/EngineVersionComparison.h"

bool FAGX_RealInterval::SerializeFromMismatchedTag(
	struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot)
{
	if (Tag.Type != NAME_StructProperty)
	{
		return false;
	}

#if UE_VERSION_OLDER_THAN(5, 4, 0)
	const FName StructName = Tag.StructName;
#else
	const FName StructName = Tag.GetType().GetParameterName(0);
#endif
	UScriptStruct* FloatStruct = TBaseStructure<FFloatInterval>::Get();
	if (StructName == FloatStruct->GetFName())
	{
		// The archive has a Float Interval, conversion needed.
		FFloatInterval Restored;
		FloatStruct->SerializeItem(Slot, &Restored, nullptr);
		Min = static_cast<double>(Restored.Min);
		Max = static_cast<double>(Restored.Max);
		return true;
	}

	UScriptStruct* DoubleStruct = FAGX_DoubleInterval::StaticStruct();
	if (StructName == DoubleStruct->GetFName())
	{
		/// @todo Make sure this code works. What we can test on? Where is FAGX_DoubleInterval used
		/// as a Property?
		FAGX_DoubleInterval Restored;
		DoubleStruct->SerializeItem(Slot, &Restored, nullptr);
		Min = Restored.Min;
		Max = Restored.Max;
		return true;
	}

	return false;
}
