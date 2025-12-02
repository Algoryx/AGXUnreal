// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MeshWithTransform.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class UStaticMeshComponent;
class UWorld;

class AGXUNREAL_API AGX_AutoFitShape
{
public:
	/*
	 * Auto-fits to the collection of FAGX_MeshWithTransforms.
	 */
	bool AutoFit(TArray<FAGX_MeshWithTransform> Meshes, UWorld* World, const FString& ShapeName);

	/*
	 * Auto-fits to the collection of UStaticMeshComponen Children. Ensures the original world
	 * transform of the Children is preserved.
	 */
	bool AutoFitToChildren(
		TArray<UStaticMeshComponent*> Children, UWorld* World, const FString& ShapeName);

	virtual bool AutoFitFromVertices(const TArray<FVector>& Vertices) = 0;
};
