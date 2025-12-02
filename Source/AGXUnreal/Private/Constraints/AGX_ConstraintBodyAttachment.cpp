// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintBodyAttachment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_SceneComponentReference.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintFrameComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "UObject/UObjectGlobals.h"

FAGX_ConstraintBodyAttachment::FAGX_ConstraintBodyAttachment(USceneComponent* InOuter)
	: Outer {InOuter}
{
	AActor* Owner = FAGX_ObjectUtilities::GetRootParentActor(Outer->GetOwner());
	RigidBody.LocalScope = Owner;
	FrameDefiningComponent.LocalScope = Owner;
}

FAGX_ConstraintBodyAttachment& FAGX_ConstraintBodyAttachment::operator=(
	const FAGX_ConstraintBodyAttachment& Other)
{
	// Copy all the user-visible configuration data.
	RigidBody = Other.RigidBody;
	FrameDefiningSource = Other.FrameDefiningSource;
	FrameDefiningComponent = Other.FrameDefiningComponent;
	LocalFrameLocation = Other.LocalFrameLocation;
	LocalFrameRotation = Other.LocalFrameRotation;

	// Deliberately not copying Outer because the Outer is intrinsically linked to the hierarchy,
	// or nesting of objects, and copying a FAGX_ConstraintBodyAttachment from one USceneComponent
	// to another should not drag the "Outership" along with it, the FAGX_ConstraintBodyAttachment
	// that is being copied over should retain its initial Outer since it hasn't moved.

	// Deliberately not copying RecentFrameDefiningActor because that Actor knows about the instance
	// that is being copied from, but knows nothing about the instance being copied into. We may
	// need to make it know in the future.

	// For more information and rationale see the comment on the Outer declaration in
	// AGX_ConstraintBodyAttachment.h.

	return *this;
}

UAGX_RigidBodyComponent* FAGX_ConstraintBodyAttachment::GetRigidBody() const
{
	return RigidBody.GetRigidBody();
}

FVector FAGX_ConstraintBodyAttachment::GetLocalFrameLocationFromBody() const
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetLocalFrameLocationFromBody() called on AGX_ConstraintBodyAttachment whose "
				 "RigidBody is not set. Returning zero vector."));
		return FVector::ZeroVector;
	}

	// If the FrameDefiningSource is RigidBody, the LocalFrameLocation is already given in
	// RigidBody's frame by definition and we can simply return the value directly.
	if (FrameDefiningSource == EAGX_FrameDefiningSource::RigidBody)
	{
		return LocalFrameLocation;
	}

	return Body->GetComponentTransform().InverseTransformPositionNoScale(GetGlobalFrameLocation());
}

FQuat FAGX_ConstraintBodyAttachment::GetLocalFrameRotationFromBody() const
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetLocalFrameRotationFromBody() called on AGX_ConstraintBodyAttachment whose "
				 "RigidBody is not set. Returning identity quaternion."));
		return FQuat::Identity;
	}

	// If the FrameDefiningSource is RigidBody, the LocalFrameRotation is already given in
	// RigidBody's frame by definition and we can simply return the value directly.
	if (FrameDefiningSource == EAGX_FrameDefiningSource::RigidBody)
	{
		return LocalFrameRotation.Quaternion();
	}

	return Body->GetComponentTransform().InverseTransformRotation(GetGlobalFrameRotation());
}

FVector FAGX_ConstraintBodyAttachment::GetGlobalFrameLocation() const
{
	if (USceneComponent* FrameDefiningComp = GetFinalFrameDefiningComponent())
	{
		return FrameDefiningComp->GetComponentTransform().TransformPositionNoScale(
			LocalFrameLocation);
	}

	return LocalFrameLocation;
}

FQuat FAGX_ConstraintBodyAttachment::GetGlobalFrameRotation() const
{
	if (USceneComponent* FrameDefiningComp = GetFinalFrameDefiningComponent())
	{
		return FrameDefiningComp->GetComponentTransform().TransformRotation(
			LocalFrameRotation.Quaternion());
	}

	return LocalFrameRotation.Quaternion();
}

FMatrix FAGX_ConstraintBodyAttachment::GetGlobalFrameMatrix() const
{
	FQuat Rotation = GetGlobalFrameRotation();
	FVector Location = GetGlobalFrameLocation();
	return FMatrix(Rotation.GetAxisX(), Rotation.GetAxisY(), Rotation.GetAxisZ(), Location);
}

FRigidBodyBarrier* FAGX_ConstraintBodyAttachment::GetRigidBodyBarrier()
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		return nullptr;
	}

	return Body->GetNative();
}

FRigidBodyBarrier* FAGX_ConstraintBodyAttachment::GetOrCreateRigidBodyBarrier()
{
	UAGX_RigidBodyComponent* Body = GetRigidBody();
	if (Body == nullptr)
	{
		return nullptr;
	}

	return Body->GetOrCreateNative();
}

#if WITH_EDITOR

void FAGX_ConstraintBodyAttachment::OnFrameDefiningComponentChanged(
	UAGX_ConstraintComponent* Parent)
{
	UAGX_ConstraintFrameComponent* Previous =
		Cast<UAGX_ConstraintFrameComponent>(PreviousFrameDefiningComponent);
	UAGX_ConstraintFrameComponent* Next =
		Cast<UAGX_ConstraintFrameComponent>(FrameDefiningComponent.GetSceneComponent());

	PreviousFrameDefiningComponent = FrameDefiningComponent.GetSceneComponent();

	if (Previous)
	{
		Previous->RemoveConstraintUsage(Parent);
	}
	if (Next)
	{
		Next->AddConstraintUsage(Parent);
	}
}

void FAGX_ConstraintBodyAttachment::UnregisterFromConstraintFrameComponent(
	UAGX_ConstraintComponent* Parent)
{
	UAGX_ConstraintFrameComponent* ConstraintFrame =
		Cast<UAGX_ConstraintFrameComponent>(FrameDefiningComponent.GetSceneComponent());

	if (ConstraintFrame)
	{
		ConstraintFrame->RemoveConstraintUsage(Parent);
	}
}
#endif

USceneComponent* FAGX_ConstraintBodyAttachment::GetFinalFrameDefiningComponent() const
{
	switch (FrameDefiningSource)
	{
		case EAGX_FrameDefiningSource::Constraint:
			return Outer;
		case EAGX_FrameDefiningSource::RigidBody:
			return GetRigidBody();
		case EAGX_FrameDefiningSource::Other:
			return FrameDefiningComponent.GetSceneComponent();
	}

	return nullptr;
}
