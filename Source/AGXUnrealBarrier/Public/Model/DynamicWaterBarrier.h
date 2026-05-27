// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.

// System includes.
#include <memory>

struct FDynamicWaterRef;
/**
 * Barrier between UAGX_Ext_WaterWrapperComponent and agxModel::WaterWrapper.
 * UAGX_Ext_WaterWrapperComponent holds an instance of Ext_WaterWrapperBarrier
 * and hidden behind the Ext_WaterWrapperBarrier is a agxModel::WaterWrapper.
 * This allows UAGX_Ext_WaterWrapperComponent to interact with
 * agxModel::WaterWrapper without including agx/RigidBody.h.
 * This class handles all translation between Unreal Engine types and
 * AGX Dynamics types, such as back and forth between FVector and agx::Vec3.
 */
class AGXUNREALBARRIER_API FDynamicWaterBarrier
{
public:
	FDynamicWaterBarrier();
	FDynamicWaterBarrier(std::unique_ptr<FDynamicWaterRef> Native);
	FDynamicWaterBarrier(FDynamicWaterBarrier&& Other) noexcept;
	virtual ~FDynamicWaterBarrier();

	bool HasNative() const;
	void AllocateNative();
	void ReleaseNative();
	FDynamicWaterRef* GetNative();
	const FDynamicWaterRef* GetNative() const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);
	virtual double FindHeightFromSurface(const FVector& WorldPoint, const FVector& UpVector, const double& Time) const = 0;
	virtual double GetDensity() const = 0;
	virtual FVector GetVelocity(const FVector& WorldPoint) const = 0;

protected:
	std::unique_ptr<FDynamicWaterRef> NativeRef;
};
