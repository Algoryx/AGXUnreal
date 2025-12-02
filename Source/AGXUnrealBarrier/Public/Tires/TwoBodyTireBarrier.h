// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Tires/TireBarrier.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FTwoBodyTireBarrier : public FTireBarrier
{
public:
	enum DeformationMode
	{
		RADIAL, // Translation orthogonal to rotation axis.
		LATERAL, // Translation in rotation axis.
		BENDING, // Rotation orthogonal to rotation axis.
		TORSIONAL // Rotation in rotation axis.
	};

	FTwoBodyTireBarrier();
	FTwoBodyTireBarrier(FTwoBodyTireBarrier&& Other) = default;
	FTwoBodyTireBarrier(std::unique_ptr<FTireRef> Native);
	virtual ~FTwoBodyTireBarrier();

	void AllocateNative(
		const FRigidBodyBarrier* TireRigidBody, float OuterRadius,
		const FRigidBodyBarrier* HubRigidBody, float InnerRadius, const FVector& LocalLocation,
		const FQuat& LocalRotation);

	float GetOuterRadius() const;
	float GetInnerRadius() const;

	FTransform GetLocalTransform() const;

	void SetDamping(float Damping, DeformationMode Mode);
	float GetDamping(DeformationMode Mode) const;

	void SetStiffness(float Stiffness, DeformationMode Mode);
	float GetStiffness(DeformationMode Mode) const;

	void SetImplicitFrictionMultiplier(const FVector2D& Multiplier);
	FVector2D GetImplicitFrictionMultiplier() const;

	FRigidBodyBarrier GetTireRigidBody() const;
	FRigidBodyBarrier GetHubRigidBody() const;

	FGuid GetHingeGuid() const;

private:
	FTwoBodyTireBarrier(const FTwoBodyTireBarrier&) = delete;
	void operator=(const FTwoBodyTireBarrier&) = delete;
};
