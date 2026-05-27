// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.

// System includes.
#include <memory>

#include "WindAndWaterParametersBarrier.h"


class FExt_WaterFlowGeneratorBarrier;
class FDynamicWaterBarrier;
class FWireBarrier;
struct FShapeBarrier;
struct FWindAndWaterControllerRef;
struct FGeometryRef;

/**
 * Barrier between UAGX_WindAndWaterController and agxModel::WindAndWaterController.
 * UAGX_WindAndWaterController holds an instance of WindAndWaterControllerBarrier
 * and hidden behind the WindAndWaterControllerBarrier is a agxModel::WindAndWaterController.
 * This allows UAGX_WindAndWaterController to interact with
 * agxModel::WindAndWaterController without including agxModel/WindAndWaterController.h.
 * This class handles all translation between Unreal Engine types and
 * AGX Dynamics types, such as back and forth between FVector and agx::Vec3.
 */
class AGXUNREALBARRIER_API FWindAndWaterControllerBarrier
{
public:
	FWindAndWaterControllerBarrier();
	FWindAndWaterControllerBarrier(std::unique_ptr<FWindAndWaterControllerRef> Native);
	FWindAndWaterControllerBarrier(FWindAndWaterControllerBarrier&& Other) noexcept;
	~FWindAndWaterControllerBarrier();
	
	bool HasNative() const;
	void AllocateNative();
	FWindAndWaterControllerRef* GetNative();
	const FWindAndWaterControllerRef* GetNative() const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);
	void ReleaseNative();
	
	FWindAndWaterParametersBarrier GetOrCreateHydrodynamicsParameters(FShapeBarrier* Shape) const;
	FWindAndWaterParametersBarrier GetOrCreateHydrodynamicsParameters(FWireBarrier* Wire) const;
	
	FWindAndWaterParametersBarrier GetOrCreateAerodynamicsParameters(FShapeBarrier* Shape) const;
	FWindAndWaterParametersBarrier GetOrCreateAerodynamicsParameters(FWireBarrier* Wire) const;

	bool SetEnableAerodynamics(FShapeBarrier* Shape, bool bEnabled);
	bool SetEnableAerodynamics(FWireBarrier* Wire, bool bEnabled);
	bool SetEnableHydrodynamics(FShapeBarrier* Shape, bool bEnabled);
	bool SetEnableHydrodynamics(FWireBarrier* Wire, bool bEnabled);
	bool AddWater(const FShapeBarrier* Shape);
	bool SetWaterWrapper(FShapeBarrier* Shape, FDynamicWaterBarrier* DynamicWater);
	bool SetWaterFlowGenerator(FShapeBarrier* Shape, FDynamicWaterBarrier* WaterWrapper);

private:
	std::unique_ptr<FWindAndWaterControllerRef> NativeRef;
};
