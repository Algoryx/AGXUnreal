// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_HingeConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/HingeBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

class FRigidBodyBarrier;

UAGX_HingeConstraintComponent::UAGX_HingeConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagTranslational3, EDofFlag::DofFlagRotational1,
		   EDofFlag::DofFlagRotational2},
		  /*bbIsSecondaryConstraintRotational*/ true)
{
	NativeBarrier.Reset(new FHingeBarrier());
}

UAGX_HingeConstraintComponent::~UAGX_HingeConstraintComponent()
{
}

void UAGX_HingeConstraintComponent::SetAttachmentProjection(EAGX_AttachmentProjection Projection)
{
	if (HasNative())
	{
		GetNativeHinge()->SetAttachmentProjection(Projection);
	}
	AttachmentProjection = Projection;
}

bool UAGX_HingeConstraintComponent::IsAttachmentProjectionEnabled() const
{
	if (HasNative())
	{
		return GetNativeHinge()->IsAttachmentProjectionEnabled();
	}
	else
	{
		return AttachmentProjection != EAGX_AttachmentProjection::Disabled;
	}

}

FHingeBarrier* UAGX_HingeConstraintComponent::GetNativeHinge()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FHingeBarrier* UAGX_HingeConstraintComponent::GetNativeHinge() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_HingeConstraintComponent::AllocateNative()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));
}
