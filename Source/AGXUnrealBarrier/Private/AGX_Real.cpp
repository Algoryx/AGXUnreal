// Copyright 2025, Algoryx Simulation AB.

#include <AGX_Real.h>

bool FAGX_Real::SerializeFromMismatchedTag(
	struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot)
{
	if (Tag.Type == NAME_DoubleProperty)
	{
		Slot << Value;
		return true;
	}
	if (Tag.Type == NAME_FloatProperty)
	{
		float Restored;
		Slot << Restored;
		Value = static_cast<double>(Restored);
		return true;
	}
	else
	{
		return false;
	}
}
