// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/AGX_AutoFitShape.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

bool AGX_AutoFitShape::AutoFit(
	TArray<FAGX_MeshWithTransform> Meshes, UWorld* World, const FString& ShapeName)
{
	if (!FAGX_Environment::IsAGXDynamicsVersionNewerOrEqualTo(2, 31, 0, 0))
	{
		const FString AGXVersion = FAGX_Environment::GetAGXDynamicsVersion();
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Could not auto-fit '%s' to meshes. Auto-fit requires AGX Dynamics version "
					 "2.31.0.0. Current version is %s"),
				*ShapeName, *AGXVersion),
			SNotificationItem::CS_Fail);
		return false;
	}

	TArray<FVector> Vertices;
	int32 numWarnings = 0;
	for (const FAGX_MeshWithTransform& Mesh : Meshes)
	{
		TArray<FVector> MeshVertices;
		TArray<FTriIndices> MeshIndices;
		const bool CollisionDataResult = AGX_MeshUtilities::GetStaticMeshCollisionData(
			Mesh, FTransform::Identity, MeshVertices, MeshIndices);
		if (CollisionDataResult)
		{
			Vertices.Append(MeshVertices);
		}
		else
		{
			numWarnings++;
		}
	}
	if (Vertices.Num() == 0)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Could not auto-fit '%s' to meshes because no collision data could be "
					 "extracted."),
				*ShapeName),
			SNotificationItem::CS_Fail);
		return false;
	}

	const bool Result = AutoFitFromVertices(Vertices);
	if (!Result)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Could not auto-fit '%s' to meshes. The Log may contain more details."),
				*ShapeName),
			SNotificationItem::CS_Fail);
		return false;
	}

	if (numWarnings > 0)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"At least one warning was detected during the auto-fit process. The Log may contain "
			"more details.",
			SNotificationItem::CS_Fail);
	}

	return true;
}

bool AGX_AutoFitShape::AutoFitToChildren(
	TArray<UStaticMeshComponent*> ChildComponents, UWorld* World, const FString& ShapeName)
{
	// Store away the world transforms of the Static Mesh Components so that we can restore them
	// after the auto-fit procedure. Also construct an Array of FAGX_MeshWithTransforms from the
	// ChildComponents.
	TMap<UStaticMeshComponent*, FTransform> OrigChildWorldTransforms;
	TArray<FAGX_MeshWithTransform> Meshes;
	for (UStaticMeshComponent* S : ChildComponents)
	{
		if (S != nullptr)
		{
			OrigChildWorldTransforms.Add(S, S->GetComponentTransform());
			Meshes.Add(FAGX_MeshWithTransform(S->GetStaticMesh(), S->GetComponentTransform()));
		}
	}

	const bool Result = AutoFit(Meshes, World, ShapeName);
	if (!Result)
	{
		return false;
	}

	// Finally, we restore the original world transform of the children Static Mesh components.
	for (UStaticMeshComponent* S : ChildComponents)
	{
		if (S != nullptr)
		{
			S->SetWorldTransform(OrigChildWorldTransforms[S]);
		}
	}

	return true;
}
