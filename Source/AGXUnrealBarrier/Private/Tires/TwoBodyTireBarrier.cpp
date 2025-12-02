// Copyright 2025, Algoryx Simulation AB.

#include "Tires/TwoBodyTireBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "AGX_LogCategory.h"
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "TypeConversions.h"
#include "AGXBarrierFactories.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxModel/TwoBodyTire.h>
#include "EndAGXIncludes.h"

FTwoBodyTireBarrier::FTwoBodyTireBarrier()
	: FTireBarrier()
{
}

FTwoBodyTireBarrier::FTwoBodyTireBarrier(std::unique_ptr<FTireRef> Native)
	: FTireBarrier(std::move(Native))
{
}

FTwoBodyTireBarrier::~FTwoBodyTireBarrier()
{
}

void FTwoBodyTireBarrier::AllocateNative(
	const FRigidBodyBarrier* TireRigidBody, float OuterRadius,
	const FRigidBodyBarrier* HubRigidBody, float InnerRadius, const FVector& LocalLocation,
	const FQuat& LocalRotation)
{
	check(!HasNative());

	agx::RigidBody* TireBody = FAGX_AgxDynamicsObjectsAccess::GetFrom(TireRigidBody);
	agx::RigidBody* HubBody = FAGX_AgxDynamicsObjectsAccess::GetFrom(HubRigidBody);
	agx::AffineMatrix4x4 LocalTransform = ConvertMatrix(LocalLocation, LocalRotation);

	agx::Real OuterRadiusAgx = ConvertDistanceToAGX<agx::Real>(OuterRadius);
	agx::Real InnerRadiusAgx = ConvertDistanceToAGX<agx::Real>(InnerRadius);

	agxModel::TwoBodyTireRef Tire = new agxModel::TwoBodyTire(
		TireBody, OuterRadiusAgx, HubBody, InnerRadiusAgx, LocalTransform);

	// Use of invalid agxModel::TwoBodyTire may lead to sudden crash during runtime.
	if (!Tire->isValid())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Error during creation of agxModel::TwoBodyTire, isValid() returned false."));
		return;
	}

	NativeRef->Native = Tire;
}

namespace
{
	agxModel::TwoBodyTire* CastToTwoBodyTire(agxModel::Tire* Tire, const FString& Operation)
	{
		agxModel::TwoBodyTire* TwoBodyTire = dynamic_cast<agxModel::TwoBodyTire*>(Tire);
		if (TwoBodyTire == nullptr)
		{
			const FString Name = Tire != nullptr ? Convert(Tire->getName()) : FString("");
			UE_LOG(
				LogAGX, Error,
				TEXT("TwoBodyTireBarrier %s: operation %s failed, could not cast native Tire to "
					 "TwoBodyTire."),
				*Name, *Operation);
		}

		return TwoBodyTire;
	}
}

float FTwoBodyTireBarrier::GetOuterRadius() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetOuterRadius"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return 0.f;
	}

	agx::Real RadiusAgx = Tire->getOuterRadius();
	return ConvertDistanceToUnreal<float>(RadiusAgx);
}

float FTwoBodyTireBarrier::GetInnerRadius() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetInnerRadius"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return 0.f;
	}

	agx::Real RadiusAgx = Tire->getInnerRadius();
	return ConvertDistanceToUnreal<float>(RadiusAgx);
}

FTransform FTwoBodyTireBarrier::GetLocalTransform() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetLocalTransform"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return FTransform::Identity;
	}

	const agx::Frame* FrameAgx = Tire->getReferenceFrame();
	return ConvertLocalFrame(FrameAgx);
}

void FTwoBodyTireBarrier::SetDamping(float Damping, DeformationMode Mode)
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = CastToTwoBodyTire(NativeRef->Native.get(), FString("SetDamping"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return;
	}

	Tire->setDampingCoefficient(ConvertToAGX(Damping), Convert(Mode));
}

float FTwoBodyTireBarrier::GetDamping(DeformationMode Mode) const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire = CastToTwoBodyTire(NativeRef->Native.get(), FString("GetDamping"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return 0.f;
	}

	agx::Real DampingAgx = Tire->getDampingCoefficient(Convert(Mode));
	return ConvertToUnreal<float>(DampingAgx);
}

void FTwoBodyTireBarrier::SetStiffness(float Stiffness, DeformationMode Mode)
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("SetStiffness"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return;
	}

	Tire->setStiffness(ConvertToAGX(Stiffness), Convert(Mode));
}

float FTwoBodyTireBarrier::GetStiffness(DeformationMode Mode) const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetStiffness"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return 0.f;
	}

	agx::Real StiffnessAgx = Tire->getStiffness(Convert(Mode));
	return ConvertToUnreal<float>(StiffnessAgx);
}

void FTwoBodyTireBarrier::SetImplicitFrictionMultiplier(const FVector2D& Multiplier)
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("SetImplicitFrictionMultiplier"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return;
	}

	Tire->setImplicitFrictionMultiplier(Convert(Multiplier));
}

FVector2D FTwoBodyTireBarrier::GetImplicitFrictionMultiplier() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetImplicitFrictionMultiplier"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return FVector2D::ZeroVector;
	}

	agx::Vec2 AgxMultiplier = Tire->getImplicitFrictionMultiplier();
	return Convert(AgxMultiplier);
}

FRigidBodyBarrier FTwoBodyTireBarrier::GetTireRigidBody() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetTireRigidBody"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return FRigidBodyBarrier();
	}

	agx::RigidBody* TireBodyAGX = Tire->getTireRigidBody();
	if (TireBodyAGX == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetTireRigidBody failed: native "
				 "agxModel::TwoBodyTire::getTireRigidBody() returned nullptr."));
		return FRigidBodyBarrier();
	}

	return AGXBarrierFactories::CreateRigidBodyBarrier(TireBodyAGX);
}

FRigidBodyBarrier FTwoBodyTireBarrier::GetHubRigidBody() const
{
	check(HasNative());

	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetHubRigidBody"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return FRigidBodyBarrier();
	}

	agx::RigidBody* HubBodyAGX = Tire->getHubRigidBody();
	if (HubBodyAGX == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetHubRigidBody failed: native "
				 "agxModel::TwoBodyTire::getHubRigidBody() returned nullptr."));
		return FRigidBodyBarrier();
	}

	return AGXBarrierFactories::CreateRigidBodyBarrier(HubBodyAGX);
}

FGuid FTwoBodyTireBarrier::GetHingeGuid() const
{
	check(HasNative());
	agxModel::TwoBodyTire* Tire =
		CastToTwoBodyTire(NativeRef->Native.get(), FString("GetHingeGuid"));
	if (Tire == nullptr)
	{
		// Logging done in CastToTwoBodyTire.
		return FGuid();
	}

	agx::Hinge* Hinge = Tire->getHinge();
	if (Hinge == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("TwoBodyTireBarrier::GetHingeGuid failed: The TwoBodyTire's Hinge was nullptr."));
		return FGuid();
	}

	return Convert(Hinge->getUuid());
}
