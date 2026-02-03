// Copyright 2025, Algoryx Simulation AB.

#include "AGX_Frame.h"

// AGX Dynamics for Unreal includes.
#include <AGX_LogCategory.h>

// Unreal Engine includes.
#include "Math/Transform.h"
#include "Components/SceneComponent.h"

FAGX_Frame::FAGX_Frame()
{
	Parent.ComponentType = USceneComponent::StaticClass();
}

void FAGX_Frame::SetParentComponent(USceneComponent* Component)
{
	Parent.SetComponent(Component);
}

USceneComponent* FAGX_Frame::GetParentComponent() const
{
	return Parent.GetComponent<USceneComponent>();
}

USceneComponent* FAGX_Frame::GetParentComponent(USceneComponent& Fallback) const
{
	USceneComponent* Parent = GetParentComponent();
	return Parent != nullptr ? Parent : &Fallback;
}

const USceneComponent* FAGX_Frame::GetParentComponent(const USceneComponent& Fallback) const
{
	USceneComponent* Parent = GetParentComponent();
	return Parent != nullptr ? Parent : &Fallback;
}

/*
 * Functions for setting / getting the frame's location and/or rotation relative to the world.
 */

// Both.

FTransform FAGX_Frame::GetWorldTransform() const
{
	const FTransform& Transform = GetParentTransform();
	return FTransform(LocalRotation, LocalLocation) * Transform;
}

FTransform FAGX_Frame::GetWorldTransform(const USceneComponent& FallbackParent) const
{
	const FTransform& Transform = GetParentTransform(FallbackParent);
	return FTransform(LocalRotation, LocalLocation) * Transform;
}

FTransform FAGX_Frame::GetWorldTransform(const FTransform& FallbackTransform) const
{
	const FTransform& Transform = GetParentTransform(FallbackTransform);
	return FTransform(LocalRotation, LocalLocation) * Transform;
}

void FAGX_Frame::GetWorldLocationAndRotation(FVector& OutLocation, FRotator& OutRotation) const
{
	const FTransform& Transform = GetParentTransform();
	OutLocation = Transform.TransformPosition(LocalLocation);
	OutRotation = Transform.TransformRotation(LocalRotation.Quaternion()).Rotator();
}

// Location.

FVector FAGX_Frame::GetWorldLocation() const
{
	const FTransform& Transform = GetParentTransform();
	return Transform.TransformPosition(LocalLocation);
}

FVector FAGX_Frame::GetWorldLocation(const USceneComponent& FallbackParent) const
{
	const FTransform& Transform = GetParentTransform(FallbackParent);
	return Transform.TransformPosition(LocalLocation);
}

void FAGX_Frame::SetWorldLocation(const FVector& InLocation, const USceneComponent& FallbackParent)
{
	const FTransform& ParentToWorld = GetParentTransform(FallbackParent);
	LocalLocation = ParentToWorld.InverseTransformPosition(InLocation);
}

// Rotation.

FRotator FAGX_Frame::GetWorldRotation() const
{
	const FTransform& Transform = GetParentTransform();
	return Transform.TransformRotation(LocalRotation.Quaternion()).Rotator();
}

FRotator FAGX_Frame::GetWorldRotation(const USceneComponent& FallbackParent) const
{
	const FTransform& Transform = GetParentTransform(FallbackParent);
	return Transform.TransformRotation(LocalRotation.Quaternion()).Rotator();
}

void FAGX_Frame::SetWorldRotation(const FRotator& InRotation, const USceneComponent& FallbackParent)
{
	const FTransform& ParentToWorld = GetParentTransform(FallbackParent);

	const FQuat ParentWorldQuat = ParentToWorld.GetRotation();
	const FQuat WorldQuat = InRotation.Quaternion();

	// Convert world rotation to local rotation relative to parent:
	// World = Parent * Local  =>  Local = Parent^-1 * World
	const FQuat LocalQuat = ParentWorldQuat.Inverse() * WorldQuat;

	LocalRotation = LocalQuat.Rotator();
}

/*
 * Functions for getting the frame's location and/or rotation relative to something else.
 */

namespace AGX_Frame_helpers
{
	FVector GetLocationRelativeTo(
		const FTransform& ParentTransform, const FTransform& TargetTransform,
		const FVector& LocalLocation)
	{
		// Construct the transformation that takes us from our Component's coordinate system to the
		// given Component's.
		const FTransform RelativeTransform = ParentTransform.GetRelativeTransform(TargetTransform);
		const FVector RelativeLocation = RelativeTransform.TransformPosition(LocalLocation);
		return RelativeLocation;
	}

	FRotator GetRotationRelativeTo(
		const FTransform& ParentTransform, const FTransform& TargetTransform,
		const FRotator& LocalRotation)
	{
		const FTransform RelativeTransform = ParentTransform.GetRelativeTransform(TargetTransform);
		const FQuat LocalQuat = LocalRotation.Quaternion();
		const FQuat RelativeQuat = RelativeTransform.TransformRotation(LocalQuat);
		const FRotator RelativeRotator = RelativeQuat.Rotator();
		return RelativeRotator;
	}

	void GetRelativeTo(
		const FTransform& ParentTransform, const FTransform& TargetTransform,
		const FVector& LocalLocation, const FRotator& LocalRotator, FVector& OutLocation,
		FRotator& OutRotator)
	{
		const FTransform RelativeTransform = ParentTransform.GetRelativeTransform(TargetTransform);
		OutLocation = RelativeTransform.TransformPosition(LocalLocation);
		OutRotator = RelativeTransform.TransformRotation(LocalRotator.Quaternion()).Rotator();
	}

	FTransform GetRelativeTo(
		const FTransform& ParentTransform, const FTransform& TargetTransform,
		const FVector& LocalLocation, const FRotator& LocalRotator)
	{
		const FTransform RelativeTransform = ParentTransform.GetRelativeTransform(TargetTransform);
		return FTransform(LocalRotator, LocalLocation) * RelativeTransform;
	}
}

// Location.

FVector FAGX_Frame::GetLocationRelativeTo(const USceneComponent& Component) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		return Component.GetComponentTransform().InverseTransformPosition(LocalLocation);
	}

	return AGX_Frame_helpers::GetLocationRelativeTo(
		ActualParent->GetComponentTransform(), Component.GetComponentTransform(), LocalLocation);
}

FVector FAGX_Frame::GetLocationRelativeTo(
	const USceneComponent& Component, const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent(FallbackParent);
	return AGX_Frame_helpers::GetLocationRelativeTo(
		ActualParent->GetComponentTransform(), Component.GetComponentTransform(), LocalLocation);
}

FVector FAGX_Frame::GetLocationRelativeTo(
	const USceneComponent& Component, const FTransform& FallbackTransform) const
{
	return AGX_Frame_helpers::GetLocationRelativeTo(
		GetParentTransform(FallbackTransform), Component.GetComponentTransform(), LocalLocation);
}

// Rotation.

FRotator FAGX_Frame::GetRotationRelativeTo(const USceneComponent& Component) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		const FQuat LocalQuat = LocalRotation.Quaternion();
		const FQuat RelativeQuat =
			Component.GetComponentTransform().InverseTransformRotation(LocalQuat);
		return RelativeQuat.Rotator();
	}

	return AGX_Frame_helpers::GetRotationRelativeTo(
		ActualParent->GetComponentTransform(), Component.GetComponentTransform(), LocalRotation);
}

FRotator FAGX_Frame::GetRotationRelativeTo(
	const USceneComponent& Component, const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent(FallbackParent);
	return AGX_Frame_helpers::GetRotationRelativeTo(
		ActualParent->GetComponentTransform(), Component.GetComponentTransform(), LocalRotation);
}

FRotator FAGX_Frame::GetRotationRelativeTo(
	const USceneComponent& Component, const FTransform& FallbackTransform) const
{
	return AGX_Frame_helpers::GetRotationRelativeTo(
		GetParentTransform(FallbackTransform), Component.GetComponentTransform(), LocalRotation);
}

// Both.

void FAGX_Frame::GetRelativeTo(
	const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		// If there is no parent then the frame origin is the world origin.
		const FTransform& Transform = Component.GetComponentTransform();
		OutLocation = Transform.InverseTransformPosition(LocalLocation);
		const FQuat LocalQuat = LocalRotation.Quaternion();
		const FQuat RelativeQuat = Transform.InverseTransformRotation(LocalQuat);
		OutRotation = RelativeQuat.Rotator();
		return;
	}

	AGX_Frame_helpers::GetRelativeTo(
		ActualParent->GetComponentTransform(), Component.GetComponentTransform(), LocalLocation,
		LocalRotation, OutLocation, OutRotation);
}

void FAGX_Frame::GetRelativeTo(
	const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation,
	const USceneComponent& FallbackParent) const
{
	const USceneComponent* ActualParent = GetParentComponent(FallbackParent);
	AGX_Frame_helpers::GetRelativeTo(
		ActualParent->GetComponentTransform(), Component.GetComponentTransform(), LocalLocation,
		LocalRotation, OutLocation, OutRotation);
}

void FAGX_Frame::GetRelativeTo(
	const USceneComponent& Component, FVector& OutLocation, FRotator& OutRotation,
	const FTransform& FallbackTransform) const
{
	AGX_Frame_helpers::GetRelativeTo(
		GetParentTransform(FallbackTransform), Component.GetComponentTransform(), LocalLocation,
		LocalRotation, OutLocation, OutRotation);
}

FTransform FAGX_Frame::GetRelativeTo(
	const USceneComponent& Component, const USceneComponent& FallbackParent) const
{
	return AGX_Frame_helpers::GetRelativeTo(
		GetParentTransform(FallbackParent), Component.GetComponentTransform(), LocalLocation,
		LocalRotation);
}

FTransform FAGX_Frame::GetRelativeTo(
	const USceneComponent& Component, const FTransform& FallbackTransform) const
{
	return AGX_Frame_helpers::GetRelativeTo(
		GetParentTransform(FallbackTransform), Component.GetComponentTransform(), LocalLocation,
		LocalRotation);
}

/*
 * End of functions for getting the frame's location and/or rotation relative to something else.
 */

const FTransform& FAGX_Frame::GetParentTransform() const
{
	return GetParentTransform(FTransform::Identity);
}

const FTransform& FAGX_Frame::GetParentTransform(const USceneComponent& FallbackParent) const
{
	return GetParentTransform(FallbackParent.GetComponentTransform());
}

const FTransform& FAGX_Frame::GetParentTransform(const FTransform& FallbackTransform) const
{
	const USceneComponent* ActualParent = GetParentComponent();
	if (ActualParent == nullptr)
	{
		return FallbackTransform;
	}
	return ActualParent->GetComponentTransform();
}
