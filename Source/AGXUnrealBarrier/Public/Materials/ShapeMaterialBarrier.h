// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include <memory>

#include "Containers/UnrealString.h"

struct FMaterialRef;

/**
 * Acts as an interface to a native AGX Material, and encapsulates it so that it is completely
 * hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FShapeMaterialBarrier
{
public:
	FShapeMaterialBarrier();
	FShapeMaterialBarrier(FShapeMaterialBarrier&& Other);
	FShapeMaterialBarrier(std::unique_ptr<FMaterialRef> Native);
	virtual ~FShapeMaterialBarrier();

	bool HasNative() const;
	FMaterialRef* GetNative();
	const FMaterialRef* GetNative() const;

	void AllocateNative(const FString& Name);
	void ReleaseNative();

	void SetName(const FString& Name);
	FString GetName() const;

	// Bulk properties.

	void SetDensity(double Density);
	double GetDensity() const;

	void SetYoungsModulus(double YoungsModulus);
	double GetYoungsModulus() const;

	void SetBulkViscosity(double Viscosity);
	double GetBulkViscosity() const;

	void SetSpookDamping(double SpookDamping);
	double GetSpookDamping() const;

	void SetMinMaxElasticRestLength(double MinElasticRestLength, double MaxElasticRestLength);
	double GetMinElasticRestLength() const;
	double GetMaxElasticRestLength() const;

	// Surface properties.

	void SetFrictionEnabled(bool bEnabled);
	bool GetFrictionEnabled() const;

	void SetRoughness(double Roughness);
	double GetRoughness() const;

	void SetSurfaceViscosity(double Viscosity);
	double GetSurfaceViscosity() const;

	void SetAdhesion(double AdhesiveForce, double AdhesiveOverlap);
	double GetAdhesiveForce() const;
	double GetAdhesiveOverlap() const;

	// Wire properties.

	double GetYoungsModulusStretch() const;
	void SetYoungsModulusStretch(double YoungsModulus) const;

	double GetYoungsModulusBend() const;
	void SetYoungsModulusBend(double YoungsModulus) const;

	double GetSpookDampingStretch() const;
	void SetSpookDampingStretch(double Damping) const;

	double GetSpookDampingBend() const;
	void SetSpookDampingBend(double Damping) const;

	FGuid GetGuid() const;

private:
	FShapeMaterialBarrier(const FShapeMaterialBarrier&) = delete;
	void operator=(const FShapeMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FMaterialRef> NativeRef;
};
