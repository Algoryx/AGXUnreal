// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtLambertianOpaqueMaterial;

class AGXUNREALBARRIER_API FRtLambertianOpaqueMaterialBarrier
{
public:
	FRtLambertianOpaqueMaterialBarrier();
	virtual ~FRtLambertianOpaqueMaterialBarrier();

	void SetReflectivity(float Reflectivity);
	float GetReflectivity() const;

	virtual void AllocateNative();

	bool HasNative() const;
	FRtLambertianOpaqueMaterial* GetNative();
	const FRtLambertianOpaqueMaterial* GetNative() const;
	void ReleaseNative();


protected:
	std::shared_ptr<FRtLambertianOpaqueMaterial> Native;
};
