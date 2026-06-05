// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.

// System includes.
#include <memory>

enum class EWindAndWaterParametersCoefficient : uint8;
enum class EWindAndWaterShapeTessellation : uint8;
struct FRigidBodyBarrier;
class FWindAndWaterControllerBarrier;
struct FWindAndWaterParametersRef;

/**
 * Barrier between UAGX_WindAndWaterParameters and agxModel::WindAndWaterParameters. UAGX_RigidBody holds an
 * instance of WindAndWaterParametersBarrier and hidden behind the WindAndWaterParametersBarrier is a
 * agxModel::WindAndWaterParameters. This allows UAGX_WindAndWaterParameters to interact with
 * agxModel::WindAndWaterParameters without including agxModel/WindAndWaterParameters.h
 *
 * This class handles all translation between Unreal Engine types and
 * AGX Dynamics types, such as back and forth between FVector and agx::Vec3.
 */
class AGXUNREALBARRIER_API FWindAndWaterParametersBarrier
{
public:
	FWindAndWaterParametersBarrier();
	FWindAndWaterParametersBarrier(std::unique_ptr<FWindAndWaterParametersRef> Native);
	FWindAndWaterParametersBarrier(FWindAndWaterParametersBarrier&& Other) noexcept;
	~FWindAndWaterParametersBarrier();
	
	bool HasNative() const;
	FWindAndWaterParametersRef* GetNative();
	const FWindAndWaterParametersRef* GetNative() const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);
	void ReleaseNative();

	void SetCoefficient(EWindAndWaterParametersCoefficient Coefficient, double Value) const;
	void SetShapeTessellation(EWindAndWaterShapeTessellation ShapeTessellation) const;
	static void SetHydrodynamicCoefficient(FWindAndWaterControllerBarrier* Controller, FRigidBodyBarrier* RigidBody, EWindAndWaterParametersCoefficient Coefficient, double Value);
	static void SetAerodynamicCoefficient(FWindAndWaterControllerBarrier* Controller, FRigidBodyBarrier* RigidBody, EWindAndWaterParametersCoefficient Coefficient, double Value);

protected:
	std::unique_ptr<FWindAndWaterParametersRef> NativeRef;
};
