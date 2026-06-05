// Copyright 2026, Algoryx Simulation AB.

#include "Model/DynamicWaterBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxModel/WindAndWaterController.h>
#include "EndAGXIncludes.h"

namespace
{
	class CustomWaterWrapper : public agxModel::WaterWrapper
	{
	public:
		CustomWaterWrapper(const FDynamicWaterBarrier& Owner): Owner_(Owner)
		{
		}

		virtual agx::Real findHeightFromSurface(const agx::Vec3& worldPoint, const agx::Vec3& upVector, const agx::TimeStamp& t) const override
		{
			return ConvertToAGX(Owner_.FindHeightFromSurface(ConvertDisplacement(worldPoint), ConvertDisplacement(upVector), t));
		}

		virtual agx::Real getDensity() const override
		{
			return ConvertToAGX(Owner_.GetDensity());
		}

		virtual agx::Vec3 getVelocity(const agx::Vec3& worldPoint) const override
		{
			return ConvertVector(Owner_.GetVelocity(ConvertVector(worldPoint)));
		}

		const FDynamicWaterBarrier& Owner_;
	};

	class CustomWaterFlowGenerator : public agxModel::WindAndWaterController::WaterFlowGenerator
	{
	public:
		CustomWaterFlowGenerator(const FDynamicWaterBarrier& Owner): Owner_(Owner)
		{
		}

		virtual agx::Vec3 getVelocity(const agx::Vec3& worldPoint) const override
		{
			return ConvertVector(Owner_.GetVelocity(ConvertVector(worldPoint)));
		}

		const FDynamicWaterBarrier& Owner_;
	};
}


FDynamicWaterBarrier::FDynamicWaterBarrier()
	: NativeRef{new FDynamicWaterRef}
{
}

FDynamicWaterBarrier::FDynamicWaterBarrier(std::unique_ptr<FDynamicWaterRef> Native)
	: NativeRef{std::move(Native)}
{
	check(NativeRef->NativeWaterWrapper->is<agxModel::WaterWrapper>());
	check(NativeRef->NativeWaterFlowGenerator->is<agxModel::WindAndWaterController::WaterFlowGenerator>());
}

FDynamicWaterBarrier::FDynamicWaterBarrier(FDynamicWaterBarrier&& Other) noexcept
	: NativeRef{std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FDynamicWaterRef);
}

FDynamicWaterBarrier::~FDynamicWaterBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FExt_WaterWrapperRef.
}

bool FDynamicWaterBarrier::HasNative() const
{
	return NativeRef->NativeWaterWrapper != nullptr || NativeRef->NativeWaterFlowGenerator != nullptr;
}

void FDynamicWaterBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef.reset(new FDynamicWaterRef(new CustomWaterWrapper(*this), new CustomWaterFlowGenerator(*this)));
}

FDynamicWaterRef* FDynamicWaterBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FDynamicWaterRef* FDynamicWaterBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FDynamicWaterBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->NativeWaterWrapper.get());
}

void FDynamicWaterBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		this->ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef.reset(new FDynamicWaterRef(nullptr, nullptr));
		return;
	}
	
	NativeRef.reset(new FDynamicWaterRef(
		reinterpret_cast<agxModel::WaterWrapper*>(NativeAddress),
		nullptr));
}

void FDynamicWaterBarrier::ReleaseNative()
{
	NativeRef.reset(new FDynamicWaterRef(nullptr, nullptr));
}
