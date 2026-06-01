// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

struct FRigidBodyBarrier;
struct FShapeBarrier;

class AGXUNREALBARRIER_API FShapeContactMergeSplitThresholdsBarrier
	: public FMergeSplitThresholdsBarrier
{
public:
	FShapeContactMergeSplitThresholdsBarrier();
	FShapeContactMergeSplitThresholdsBarrier(FShapeContactMergeSplitThresholdsBarrier&& Other) =
		default;
	FShapeContactMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef>&& Native);
	~FShapeContactMergeSplitThresholdsBarrier();

	void AllocateNative();

	void SetMaxImpactSpeed(double InMaxImpactSpeed);
	double GetMaxImpactSpeed() const;

	void SetMaxRelativeNormalSpeed(double InMaxRelativeNormalSpeed);
	double GetMaxRelativeNormalSpeed() const;

	void SetMaxRelativeTangentSpeed(double InMaxRelativeTangentSpeed);
	double GetMaxRelativeTangentSpeed() const;

	void SetMaxRollingSpeed(double InMaxRollingSpeed);
	double GetMaxRollingSpeed() const;

	void SetNormalAdhesion(double InNormalAdhesion);
	double GetNormalAdhesion() const;

	void SetTangentialAdhesion(double InTangentialAdhesion);
	double GetTangentialAdhesion() const;

	void SetMaySplitInGravityField(bool bMaySplitInGravityField);
	bool GetMaySplitInGravityField() const;

	void SetSplitOnLogicalImpact(bool bSplitOnLogicalImpact);
	bool GetSplitOnLogicalImpact() const;

	static FShapeContactMergeSplitThresholdsBarrier CreateFrom(const FRigidBodyBarrier& Barrier);
	static FShapeContactMergeSplitThresholdsBarrier CreateFrom(const FShapeBarrier& Barrier);

private:
	FShapeContactMergeSplitThresholdsBarrier(const FShapeContactMergeSplitThresholdsBarrier&) =
		delete;
	void operator=(const FShapeContactMergeSplitThresholdsBarrier&) = delete;
};
