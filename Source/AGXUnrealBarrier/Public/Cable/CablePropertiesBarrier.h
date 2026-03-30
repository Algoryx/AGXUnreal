// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "CablePropertiesBarrier.generated.h"

struct FCablePropertiesRef;


USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCablePropertiesBarrier
{
	GENERATED_BODY()

	FCablePropertiesBarrier();
	FCablePropertiesBarrier(std::shared_ptr<FCablePropertiesRef> Native);

	void SetSpookDampingBend(double SpookDamping);
	double GetSpookDampingBend() const;

	void SetSpookDampingTwist(double SpookDamping);
	double GetSpookDampingTwist() const;

	void SetSpookDampingStretch(double SpookDamping);
	double GetSpookDampingStretch() const;

	void SetPoissonsRatioBend(double PoissonsRatio);
	double GetPoissonsRatioBend() const;

	void SetPoissonsRatioTwist(double PoissonsRatio);
	double GetPoissonsRatioTwist() const;

	void SetPoissonsRatioStretch(double PoissonsRatio);
	double GetPoissonsRatioStretch() const;

	void SetYoungsModulusBend(double YoungsModulus);
	double GetYoungsModulusBend() const;

	void SetYoungsModulusTwist(double YoungsModulus);
	double GetYoungsModulusTwist() const;

	void SetYoungsModulusStretch(double YoungsModulus);
	double GetYoungsModulusStretch() const;

	
	bool HasNative() const;
	FCablePropertiesRef* GetNative();
	const FCablePropertiesRef* GetNative() const;

	void AllocateNative();
	void ReleaseNative();

	FGuid GetGuid() const;

private:
	std::shared_ptr<FCablePropertiesRef> NativeRef;
};
