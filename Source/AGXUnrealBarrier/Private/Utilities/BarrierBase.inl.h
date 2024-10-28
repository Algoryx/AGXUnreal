// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "Utilities/BarrierBase.h"

template <typename BarrierT>
bool FBarrierBase<BarrierT>::HasNative() const
{
	return GetNative()->Native != nullptr;
}

template <typename BarrierT>
uintptr_t FBarrierBase<BarrierT>::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	return reinterpret_cast<uintptr_t>(GetNative()->Native.get());
}

template <typename BarrierT>
template <typename AGXType>
void FBarrierBase<BarrierT>::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		ReleaseNative();
	}

	// GetNative returns a Ref-struct, meaning that it contains a member of type agx::ref_ptr
	// named 'Native'.
	auto& Native = GetNative()->Native;
	if (NativeAddress == 0)
	{
		Native = nullptr;
	}
	else
	{
		Native = reinterpret_cast<AGXType*>(NativeAddress);
	}
}

template <typename BarrierT>
void FBarrierBase<BarrierT>::ReleaseNative()
{
	check(Barrier().HasNative());
	Barrier().GetNative()->Native = nullptr;
}
