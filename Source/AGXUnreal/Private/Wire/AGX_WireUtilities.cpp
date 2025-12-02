// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireWinchComponent.h"

/*
 * A collection of functions dealing with winch transformations follow.
 *
 * There are a few cases to handle here. The parameters are:
 *  - Owner = {Wire, Winch}
 *  - Native = {false, true}
 *  - Body = {false, true}
 *
 * For each combination we need the transformation to use for visualization rendering.
 *  - Owner=Wire, Native=false, Body=false: Editor placement relative to Wire.
 *  - Owner=Wire, Native=false, Body=true: Editor placement relative to Body.
 *  - Owner=Wire, Native=true, Body=false: Simulation placement relative to World.
 *  - Owner=Wire, Native=true, Body=true: Simulation placement relative to Body.
 *  - Owner=Winch, Native=false, Body=false: Editor placement relative to Winch.
 *  - Owner=Winch, Native=false, Body=true: Editor placement relative to Winch.
 *  - Owner=Winch, Native=true, Body=false: Simulation placement relative to World.
 *  - Owner=Winch, Native=true, Body=true: Simulation placement relative to Body.
 *
 * For each Owner, Body combination we need to go from editor placement to simulation placement.
 *  - Owner=Wire, Body=false: Use Wire transform to convert relative to Wire to relative to World.
 *  - Owner=Wire, Body=true: Both relative to body, no change necessary.
 *  - Owner=Winch, Body=false: Use Winch transform to convert rel. to Winch to rel. to World.
 *  - Owner=Winch, Body=true: Move from relative to Winch to relative to Body.
 *
 *  In plain english:
 *
 * At edit time, a winch owned by a Wire is placed relative to the Wire unless there is a body,
 * then it is placed relative to the body.
 * At edit time, a winch owned by a Wire Winch is always placed relative to the Wire Winch,
 * regardless of if there is a body or not.
 * At simulation time, a winch with a body is placed relative to that body regardless of what type
 * the owner is.
 * At simulation time, a winch without a body is placed relative to the world regardless of what
 * type the owner is.
 */

namespace AGX_WireUtilities_helpers
{
	/*
	 * Implements the Owner=Wire part of the table above.
	 */
	FAGX_WireWinchPose GetWireWinchPose(const UAGX_WireComponent& Wire, const FAGX_WireWinch& Winch)
	{
		const UAGX_RigidBodyComponent* Body = Winch.GetBodyAttachment();
		const bool bNative = Winch.HasNative();
		const bool bBody = Body != nullptr;
		if (!bNative && !bBody)
		{
			// Owner=Wire, Native=false, Body=false: Editor placement relative to Wire.
			return {Wire.GetComponentTransform(), Winch.Location, Winch.Rotation};
		}
		else if (!bNative && bBody)
		{
			// Owner=Wire, Native=false, Body=true: Editor placement relative to Body.
			return {Body->GetComponentTransform(), Winch.Location, Winch.Rotation};
		}
		else if (bNative && !bBody)
		{
			// Owner=Wire, Native=true, Body=false: Simulation placement relative to World.
			return {FTransform::Identity, Winch.LocationSim, Winch.RotationSim};
		}
		else if (bNative && bBody)
		{
			// Owner=Wire, Native=true, Body=true: Simulation placement relative to Body.
			return {Body->GetComponentTransform(), Winch.LocationSim, Winch.RotationSim};
		}

		// The above if-else chain is supposed to be exhaustive, should never get here.
		checkNoEntry();
		return {FTransform::Identity, FVector(ForceInit), FRotator(ForceInit)};
	}

	/*
	 * Implements the Owner=Winch part of the table above.
	 */
	FAGX_WireWinchPose GetWireWinchPose(const UAGX_WireWinchComponent& WireWinch)
	{
		const FAGX_WireWinch& Winch = WireWinch.WireWinch;
		const UAGX_RigidBodyComponent* Body = WireWinch.WireWinch.GetBodyAttachment();
		const bool bNative = Winch.HasNative();
		const bool bBody = Body != nullptr;
		if (!bNative && !bBody)
		{
			// Owner=Winch, Native=false, Body=false: Editor placement relative to Winch.
			return {WireWinch.GetComponentTransform(), Winch.Location, Winch.Rotation};
		}
		else if (!bNative && bBody)
		{
			// Owner=Winch, Native=false, Body=true: Editor placement relative to Winch.
			return {WireWinch.GetComponentTransform(), Winch.Location, Winch.Rotation};
		}
		else if (bNative && !bBody)
		{
			// Owner=Winch, Native=true, Body=false: Simulation placement relative to World.
			return {FTransform::Identity, Winch.LocationSim, Winch.RotationSim};
		}
		else if (bNative && bBody)
		{
			// Owner=Winch, Native=true, Body=true: Simulation placement relative to Body.
			return {Body->GetComponentTransform(), Winch.LocationSim, Winch.RotationSim};
		}

		// The above if-else chain is supposed to be exhaustive, should never get here.
		checkNoEntry();
		return {FTransform::Identity, FVector(ForceInit), FRotator(ForceInit)};
	}
}

FAGX_WireWinchPose FAGX_WireUtilities::GetWireWinchPose(
	const UAGX_WireComponent& Wire, EWireSide Side)
{
	switch (Wire.GetWinchOwnerType(Side))
	{
		case EWireWinchOwnerType::Wire:
		{
			const FAGX_WireWinch* Winch = Wire.GetOwnedWinch(Side);
			checkf(
				Winch != nullptr, TEXT("Trying to get owned Wire Winch pose for a Wire Component "
									   "with no Wire Winch."));
			return AGX_WireUtilities_helpers::GetWireWinchPose(Wire, *Winch);
		}
		case EWireWinchOwnerType::WireWinch:
		{
			const UAGX_WireWinchComponent* WireWinch = Wire.GetWinchComponent(Side);
			checkf(
				WireWinch != nullptr, TEXT("Trying to get Wire Winch Component pose for a Wire "
										   "Component with no Wire Winch."));
			return AGX_WireUtilities_helpers::GetWireWinchPose(*WireWinch);
		}
		case EWireWinchOwnerType::Other:
		{
			// We know nothing of these Wire Winches, so their location and rotation must be
			// in the world coordinate system at all times.
			const FAGX_WireWinch* Winch = Wire.GetBorrowedWinch(Side);
			check(Winch != nullptr);
			if (Winch->HasNative())
			{
				return {FTransform::Identity, Winch->LocationSim, Winch->RotationSim};
			}
			else
			{
				return {FTransform::Identity, Winch->Location, Winch->Rotation};
			}
		}
		case EWireWinchOwnerType::None:
			// Should never ask for the pose of a winch that doesn't exist.
			checkNoEntry();
			break;
	}

	// The above switch statement is supposed to be exhaustive, should never get here.
	checkNoEntry();
	return {FTransform::Identity, FVector(ForceInit), FRotator(ForceInit)};
}

FAGX_WireWinchPose FAGX_WireUtilities::GetWireWinchPose(const UAGX_WireWinchComponent& WireWinch)
{
	return AGX_WireUtilities_helpers::GetWireWinchPose(WireWinch);
}

const FTransform& FAGX_WireUtilities::GetWinchLocalToWorld(
	const UAGX_WireComponent& Wire, EWireSide Side)
{
	return GetWireWinchPose(Wire, Side).LocalToWorld;
}

const FTransform& FAGX_WireUtilities::GetWinchLocalToWorld(const UAGX_WireWinchComponent& WireWinch)
{
	return GetWireWinchPose(WireWinch).LocalToWorld;
}

void FAGX_WireUtilities::ComputeSimulationPlacement(
	const UAGX_WireComponent& Owner, FAGX_WireWinch& Winch)
{
	if (Winch.GetBodyAttachment() == nullptr)
	{
		// Owner=Wire, Body=false: Use Wire transform to convert rel. to Wire to rel. to World.
		Winch.LocationSim = Owner.GetComponentTransform().TransformPosition(Winch.Location);
		Winch.RotationSim = Owner.GetComponentRotation() + Winch.Rotation;
	}
	else
	{
		// Owner=Wire, Body=true: Both relative to body, no change necessary.
		Winch.LocationSim = Winch.Location;
		Winch.RotationSim = Winch.Rotation;
	}
}

void FAGX_WireUtilities::ComputeSimulationPlacement(
	const UAGX_WireWinchComponent& Owner, FAGX_WireWinch& Winch)
{
	const FTransform& WinchTransform = Owner.GetComponentTransform();
	const FRotator& WinchRotation = Owner.GetComponentRotation();
	if (const UAGX_RigidBodyComponent* Body = Winch.GetBodyAttachment())
	{
		// Owner=Winch, Body=true: Move from relative to Winch to relative to Body.
		const FTransform& BodyTransform = Body->GetComponentTransform();
		const FVector WorldLocation = WinchTransform.TransformPosition(Winch.Location);
		Winch.LocationSim = BodyTransform.InverseTransformPosition(WorldLocation);
		const FQuat& WorldRotation = WinchTransform.TransformRotation(Winch.Rotation.Quaternion());
		Winch.RotationSim = BodyTransform.InverseTransformRotation(WorldRotation).Rotator();
	}
	else
	{
		// Owner=Winch, Body=false: Use Winch transform to convert rel. to Winch to rel. to World.
		Winch.LocationSim = WinchTransform.TransformPosition(Winch.Location);
		Winch.RotationSim = WinchRotation + Winch.Rotation;
	}
}

void FAGX_WireUtilities::ComputeSimulationPlacement(FAGX_WireWinch& Winch)
{
	Winch.LocationSim = Winch.Location;
	Winch.RotationSim = Winch.Rotation;
}
