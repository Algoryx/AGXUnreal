// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_ContactMaterialEnums.h"

// Standard library includes.
#include <memory>

struct FContactMaterialRef;
class FShapeMaterialBarrier;
struct FRigidBodyBarrier;

/**
 * Acts as an interface to a native AGX Contact Material, and encapsulates it so that it is
 * completely hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FContactMaterialBarrier
{
public:
	FContactMaterialBarrier();
	FContactMaterialBarrier(FContactMaterialBarrier&& Other);
	FContactMaterialBarrier(std::unique_ptr<FContactMaterialRef> Native);
	virtual ~FContactMaterialBarrier();

	bool HasNative() const;
	FContactMaterialRef* GetNative();
	const FContactMaterialRef* GetNative() const;

	void AllocateNative(
		const FShapeMaterialBarrier* Material1, const FShapeMaterialBarrier* Material2);
	void ReleaseNative();

	void SetFrictionSolveType(EAGX_ContactSolver SolveType);
	EAGX_ContactSolver GetFrictionSolveType() const;

	void SetFrictionModel(EAGX_FrictionModel FrictionModel);
	EAGX_FrictionModel GetFrictionModel() const;

	bool SetNormalForceMagnitude(double NormalForceMagnitude);
	bool GetNormalForceMagnitude(double& NormalForceMagnitude) const;

	bool SetEnableScaleNormalForceWithDepth(bool bEnabled);
	bool GetEnableScaleNormalForceWithDepth(bool& bEnabled) const;

	void SetRestitution(double Restitution);
	double GetRestitution() const;

	void SetSurfaceFrictionEnabled(bool bEnabled);
	bool GetSurfaceFrictionEnabled() const;

	// Set both primary and secondary friction coefficient.
	void SetFrictionCoefficient(double Coefficient);
	void SetPrimaryFrictionCoefficient(double Coefficient);
	void SetSecondaryFrictionCoefficient(double Coefficient);
	void SetFrictionCoefficient(
		double Coefficient, bool bPrimaryDirection, bool bSecondaryDirection);

	double GetFrictionCoefficient() const;
	double GetPrimaryFrictionCoefficient() const;
	double GetSecondaryFrictionCoefficient() const;

	// Set both primary and secondary surface Viscosity.
	void SetSurfaceViscosity(double Viscosity);
	void SetPrimarySurfaceViscosity(double Viscosity);
	void SetSecondarySurfaceViscosity(double Viscosity);
	void SetSurfaceViscosity(double Viscosity, bool bPrimaryDirection, bool bSecondaryDirection);

	double GetSurfaceViscosity() const;
	double GetPrimarySurfaceViscosity() const;
	double GetSecondarySurfaceViscosity() const;

	bool SetPrimaryDirection(const FVector& Direction);
	bool GetPrimaryDirection(FVector& Direction) const;

	// Functions that deal with oriented friction models. These do nothing / return empty string or
	// guid if the friction model isn't one of the oriented ones.
	bool SetOrientedFrictionModelReferenceFrame(FRigidBodyBarrier* RigidBody);
	FString GetOrientedFrictionModelReferenceFrameBodyName() const;
	FGuid GetOrientedFrictionModelReferenceFrameBodyGuid() const;

	void SetAdhesiveForce(double AdhesiveForce) const;
	void SetAdhesiveOverlap(double AdhesiveOverlap) const;
	void SetAdhesion(double AdhesiveForce, double AdhesiveOverlap);
	double GetAdhesiveForce() const;
	double GetAdhesiveOverlap() const;

	void SetYoungsModulus(double YoungsModulus);
	double GetYoungsModulus() const;

	void SetSpookDamping(double SpookDamping);
	double GetSpookDamping() const;

	void SetMinElasticRestLength(double MinElasticRestLength);
	void SetMaxElasticRestLength(double MaxElasticRestLength);
	void SetMinMaxElasticRestLength(double MinElasticRestLength, double MaxElasticRestLength);
	double GetMinElasticRestLength() const;
	double GetMaxElasticRestLength() const;

	void SetContactReductionMode(EAGX_ContactReductionMode ReductionMode);
	EAGX_ContactReductionMode GetContactReductionMode() const;

	void SetContactReductionLevel(EAGX_ContactReductionLevel ContactReductionLevel);
	EAGX_ContactReductionLevel GetContactReductionLevel() const;

	void SetUseContactAreaApproach(bool bUse);
	bool GetUseContactAreaApproach() const;

	FShapeMaterialBarrier GetMaterial1() const;
	FShapeMaterialBarrier GetMaterial2() const;

	FGuid GetGuid() const;

private:
	FContactMaterialBarrier(const FContactMaterialBarrier&) = delete;
	void operator=(const FContactMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FContactMaterialRef> NativeRef;
};
