// Copyright 2026, Algoryx Simulation AB.

#pragma once

namespace AGX_EnumUtilities
{
	template <typename EnumT>
	FString GetEnumName(EnumT Value)
	{
		return StaticEnum<EnumT>()->GetDisplayNameTextByValue((int64) Value).ToString();
	}
}
