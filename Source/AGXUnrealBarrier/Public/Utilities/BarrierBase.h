// Copyright 2024, Algoryx Simulation AB.

#pragma once

template <typename BarrierT>
class AGXUNREALBARRIER_API FBarrierBase
{
public:
	bool HasNative() const;
	uintptr_t GetNativeAddress() const;
	template <typename AGXType>
	void SetNativeAddress(uintptr_t NativeAddress);
	void ReleaseNative();

private:
	auto GetNative()
	{
		return Barrier().GetNative();
	}

	auto GetNative() const
	{
		return Barrier().GetNative();
	}

	BarrierT& Barrier()
	{
		return static_cast<BarrierT&>(*this);
	}

	const BarrierT& Barrier() const
	{
		return static_cast<const BarrierT&>(*this);
	}
};
