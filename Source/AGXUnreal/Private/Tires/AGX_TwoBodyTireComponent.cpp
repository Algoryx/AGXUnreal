// Copyright 2024, Algoryx Simulation AB.

#include "Tires/AGX_TwoBodyTireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Import/AGX_ImportContext.h"
#include "Tires/AGX_TwoBodyTireActor.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

UAGX_TwoBodyTireComponent::UAGX_TwoBodyTireComponent()
{
	UpdateReferencesLocalScope();
}

UAGX_RigidBodyComponent* UAGX_TwoBodyTireComponent::GetHubRigidBody() const
{
	return HubRigidBody.GetRigidBody();
}

UAGX_RigidBodyComponent* UAGX_TwoBodyTireComponent::GetTireRigidBody() const
{
	return TireRigidBody.GetRigidBody();
}

FTransform UAGX_TwoBodyTireComponent::GetGlobalTireTransform() const
{
	UAGX_RigidBodyComponent* TireBody = GetTireRigidBody();
	if (TireBody == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s GetGlobalTireTransform failed, Tire Rigid Body was nullptr."),
			*GetFName().ToString());
		return FTransform::Identity;
	}

	// This reflects the behaviour of the AGX Dynamics agxModel::TwoBodyTire where a local transform
	// relative to the tire Rigid Body is used to define the final transform of the Tire model. The
	// axis of rotation is along the y-axis of this final transform in AGX Dynamics, which
	// corresponds to the negative y-axis in Unreal.
	FVector Pos = TireBody->GetComponentTransform().TransformPositionNoScale(LocalLocation);
	FQuat Rot = TireBody->GetComponentTransform().TransformRotation(LocalRotation.Quaternion());

	return FTransform(Rot, Pos);
}

namespace AGX_TwoBodyTireComponent_helpers
{
	bool SetupRigidBodies(
		const FTwoBodyTireBarrier& Barrier, UAGX_TwoBodyTireComponent& OutTire,
		FAGX_ImportContext& Context)
	{
		const auto TireBarrier = Barrier.GetTireRigidBody();
		const auto HubBarrier = Barrier.GetHubRigidBody();

		if (!TireBarrier.HasNative() || !HubBarrier.HasNative())
			return false;

		auto TireBody = Context.RigidBodies->FindRef(TireBarrier.GetGuid());
		auto HubBody = Context.RigidBodies->FindRef(HubBarrier.GetGuid());
		if (TireBody == nullptr || HubBody == nullptr)
			return false;

		OutTire.HubRigidBody.Name = *HubBody->GetName();
		OutTire.TireRigidBody.Name = *TireBody->GetName();
		return true;
	}
}

void UAGX_TwoBodyTireComponent::CopyFrom(
	const FTwoBodyTireBarrier& Barrier, FAGX_ImportContext* Context)
{
	using namespace AGX_TwoBodyTireComponent_helpers;
	if (!Barrier.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s could not copy properties from Barrier because the Barrier does not have "
				 "a native object allocated."),
			*GetFName().ToString());
		return;
	}

	ImportGuid = Barrier.GetGuid();
	OuterRadius = Barrier.GetOuterRadius();
	InnerRadius = Barrier.GetInnerRadius();

	const FTransform LocalTransform = Barrier.GetLocalTransform();
	LocalLocation = LocalTransform.GetLocation();
	LocalRotation = FRotator(LocalTransform.GetRotation());

	RadialStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::RADIAL);
	LateralStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::LATERAL);
	BendingStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::BENDING);
	TorsionalStiffness = Barrier.GetStiffness(FTwoBodyTireBarrier::TORSIONAL);

	RadialDamping = Barrier.GetDamping(FTwoBodyTireBarrier::RADIAL);
	LateralDamping = Barrier.GetDamping(FTwoBodyTireBarrier::LATERAL);
	BendingDamping = Barrier.GetDamping(FTwoBodyTireBarrier::BENDING);
	TorsionalDamping = Barrier.GetDamping(FTwoBodyTireBarrier::TORSIONAL);

	ImplicitFrictionMultiplier = Barrier.GetImplicitFrictionMultiplier();

	if (Context == nullptr || Context->Tires == nullptr || Context->RigidBodies == nullptr)
		return; // We are done.

	if (SetupRigidBodies(Barrier, *this, *Context))
	{
		AGX_CHECK(!Context->Tires->Contains(ImportGuid));
		Context->Tires->Add(ImportGuid, this);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to setup Rigid Bodies for Two Body Tire: '%s'. The Tire will not be "
				 "added."),
			*GetName());
	}
}

bool UAGX_TwoBodyTireComponent::IsDefaultSubObjectOfTwoBodyTireActor() const
{
	AActor* Owner = GetOwner();
	return IsDefaultSubobject() && Owner != nullptr &&
		   Cast<AAGX_TwoBodyTireActor>(Owner) != nullptr;
}

void UAGX_TwoBodyTireComponent::PostLoad()
{
	Super::PostLoad();
	UpdateReferencesLocalScope();
}

void UAGX_TwoBodyTireComponent::OnRegister()
{
	Super::OnRegister();
	UpdateReferencesLocalScope();
}

void UAGX_TwoBodyTireComponent::AllocateNative()
{
	NativeBarrier.Reset(CreateTwoBodyTireBarrier());
}

void UAGX_TwoBodyTireComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s UpdateNativeProperties failed, HasNative() returned false."),
			*GetFName().ToString());
		return;
	}

	FTwoBodyTireBarrier* Barrier = static_cast<FTwoBodyTireBarrier*>(GetNative());
	if (Barrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s UpdateNativeProperties, casting of FTireBarrier to FTwoBodyTireBarrier "
				 "failed."),
			*GetFName().ToString());
		return;
	}

	Barrier->SetStiffness(RadialStiffness, FTwoBodyTireBarrier::RADIAL);
	Barrier->SetStiffness(LateralStiffness, FTwoBodyTireBarrier::LATERAL);
	Barrier->SetStiffness(BendingStiffness, FTwoBodyTireBarrier::BENDING);
	Barrier->SetStiffness(TorsionalStiffness, FTwoBodyTireBarrier::TORSIONAL);

	Barrier->SetDamping(RadialDamping, FTwoBodyTireBarrier::RADIAL);
	Barrier->SetDamping(LateralDamping, FTwoBodyTireBarrier::LATERAL);
	Barrier->SetDamping(BendingDamping, FTwoBodyTireBarrier::BENDING);
	Barrier->SetDamping(TorsionalDamping, FTwoBodyTireBarrier::TORSIONAL);

	Barrier->SetImplicitFrictionMultiplier(ImplicitFrictionMultiplier);
}

void UAGX_TwoBodyTireComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAGX_TwoBodyTireComponent::PostInitProperties()
{
	Super::PostInitProperties();
	UpdateReferencesLocalScope();
}

void UAGX_TwoBodyTireComponent::UpdateReferencesLocalScope()
{
	AActor* Owner = FAGX_ObjectUtilities::GetRootParentActor(this);
	HubRigidBody.LocalScope = Owner;
	TireRigidBody.LocalScope = Owner;
}

FTwoBodyTireBarrier* UAGX_TwoBodyTireComponent::CreateTwoBodyTireBarrier()
{
	FTwoBodyTireBarrier* Barrier = new FTwoBodyTireBarrier;
	UAGX_RigidBodyComponent* TireBody = GetTireRigidBody();
	UAGX_RigidBodyComponent* HubBody = GetHubRigidBody();
	if (TireBody == nullptr || HubBody == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tire %s creation failed: at least one of the Rigid Bodies used when trying to "
				 "create the Tire was nullptr."),
			*GetFName().ToString());
		return Barrier;
	}

	FRigidBodyBarrier* TireBarrier = TireBody->GetOrCreateNative();
	FRigidBodyBarrier* HubBarrier = HubBody->GetOrCreateNative();
	if (TireBarrier == nullptr || HubBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Tire %s creation failed: at least one of the Rigid Bodies' Barriers was nullptr."),
			*GetFName().ToString());
		return Barrier;
	}

	Barrier->AllocateNative(
		TireBarrier, OuterRadius, HubBarrier, InnerRadius, LocalLocation,
		LocalRotation.Quaternion());

	return Barrier;
}
