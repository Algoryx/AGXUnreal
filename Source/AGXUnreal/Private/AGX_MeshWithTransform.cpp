// Copyright 2025, Algoryx Simulation AB.

#include "AGX_MeshWithTransform.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"

FAGX_MeshWithTransform::FAGX_MeshWithTransform(
	const UStaticMesh* InMesh, const FTransform& InTransform)
	: Mesh(InMesh)
	, Transform(InTransform)
{
}

bool FAGX_MeshWithTransform::IsValid() const
{
	return Mesh.Get() != nullptr && Transform.IsValid();
}
