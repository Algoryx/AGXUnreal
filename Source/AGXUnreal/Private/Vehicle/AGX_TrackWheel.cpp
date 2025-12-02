// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackWheel.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"

bool FAGX_TrackWheel::GetTransformRelativeToBody(FVector& RelPosition, FQuat& RelRotation) const
{
	UAGX_RigidBodyComponent* Body = RigidBody.GetRigidBody();
	if (!IsValid(Body))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_TrackWheel::GetTransformRelativeToBody() failed because the Rigid Body "
				 "is null or invalid."));
		RelPosition = FVector::ZeroVector;
		RelRotation = FQuat::Identity;
		return false;
	}

	RelPosition = RelativeLocation;
	RelRotation = RelativeRotation.Quaternion();

	if (bUseFrameDefiningComponent)
	{
		USceneComponent* FrameComponent = FrameDefiningComponent.GetSceneComponent();
		if (!FrameComponent)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("TrackWheel failed to find the Frame Defining Component '%s' in '%s'. "
					 "The properties RelativeLocation and RelativeRotation will be interpreted as "
					 "relative to the Rigid Body Component instead."),
				*FrameDefiningComponent.Name.ToString(),
				*GetNameSafe(FrameDefiningComponent.GetScope()));
		}
		else
		{
			// Make sure world transforms are up-to-date.
			Body->ConditionalUpdateComponentToWorld();
			FrameComponent->ConditionalUpdateComponentToWorld();

			// Given the Frame Component and a Position and Rotation in its local coordinate system,
			// compute the Frame Position and Rotation in the coordinate system of the body.
			const FVector WorldPosition =
				FrameComponent->GetComponentTransform().TransformPositionNoScale(RelPosition);
			const FQuat WorldRotation =
				FrameComponent->GetComponentTransform().TransformRotation(RelRotation);
			RelPosition =
				Body->GetComponentTransform().InverseTransformPositionNoScale(WorldPosition);
			RelRotation = Body->GetComponentTransform().InverseTransformRotation(WorldRotation);
		}
	}

	return true;
}
