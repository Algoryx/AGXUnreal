// Copyright 2024, Algoryx Simulation AB.

#pragma once

namespace BarrierUtilities
{
	template <typename BarrierT>
	uintptr_t GetNativeAddress(const BarrierT& Barrier)
	{
		if (!Barrier.HasNative())
		{
			return 0;
		}
		return reinterpret_cast<uintptr_t>(Barrier.GetNative()->Native.get());
	}

	template <typename AGXType, typename BarrierT>
	void SetNativeAddress(BarrierT& Barrier, uintptr_t NativeAddress)
	{
		if (NativeAddress == Barrier.GetNativeAdddress())
		{
			return;
		}

		if (Barrier.HasNative())
		{
			Barrier.ReleaseNative();
		}

		// GetNative returns a Ref-struct, meaning that it contains a member of type agx::ref_ptr
		// named 'Native'.
		auto& Native = Barrier.GetNative()->Native;
		if (NativeAddress == 0)
		{
			Native = nullptr;
		}
		else
		{
			Native = reinterpret_cast<AGXType*>(NativeAddress);
		}
	}
}
