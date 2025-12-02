// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/AGX_ShapeUtilities.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/TrimeshHelperFunctions.h>
#include <agx/version.h>
#include "EndAGXIncludes.h"

namespace AGX_ShapeUtilities_helpers
{
	agx::Vec3Vector ToAGX(const TArray<FVector>& Vertices)
	{
		agx::Vec3Vector VerticesAGX;
		VerticesAGX.reserve(Vertices.Num());

		for (const FVector& Vertex : Vertices)
		{
			VerticesAGX.push_back(ConvertDisplacement(Vertex));
		}

		return VerticesAGX;
	}
}

bool FAGX_ShapeUtilities::ComputeOrientedBox(
	const TArray<FVector>& Vertices, FVector& OutHalfExtents, FTransform& OutTransform)
{
#if AGX_GENERATION_VERSION < 3 && AGX_MAJOR_VERSION < 31
	// AGX Dynamics version is less than 2.31.0.0, bounding volumes not supported.
	return false;
#else
	agx::Vec3Vector VerticesAGX = AGX_ShapeUtilities_helpers::ToAGX(Vertices);
	agx::Vec3 halfExtentsAGX;
	agx::AffineMatrix4x4 transformAGX;
	if (!agxUtil::computeOrientedBox(VerticesAGX, halfExtentsAGX, transformAGX))
	{
		return false;
	}
	OutHalfExtents = ConvertDistance(halfExtentsAGX);
	OutTransform = Convert(transformAGX);
	return true;
#endif
}

bool FAGX_ShapeUtilities::ComputeOrientedCylinder(
	const TArray<FVector>& Vertices, float& OutRadius, float& OutHeight, FTransform& OutTransform)
{
#if AGX_GENERATION_VERSION < 3 && AGX_MAJOR_VERSION < 31
	// AGX Dynamics version is less than 2.31.0.0, bounding volumes not supported.
	return false;
#else
	agx::Vec3Vector VerticesAGX = AGX_ShapeUtilities_helpers::ToAGX(Vertices);
	agx::Vec2 radiusHeightAGX;
	agx::AffineMatrix4x4 transformAGX;
	if (!agxUtil::computeOrientedCylinder(VerticesAGX, radiusHeightAGX, transformAGX))
	{
		return false;
	}
	OutRadius = ConvertDistanceToUnreal<float>(radiusHeightAGX.x());
	OutHeight = ConvertDistanceToUnreal<float>(radiusHeightAGX.y());
	OutTransform = Convert(transformAGX);
	return true;
#endif
}

bool FAGX_ShapeUtilities::ComputeOrientedCapsule(
	const TArray<FVector>& Vertices, float& OutRadius, float& OutHeight, FTransform& OutTransform)
{
#if AGX_GENERATION_VERSION < 3 && AGX_MAJOR_VERSION < 31
	// AGX Dynamics version is less than 2.31.0.0, bounding volumes not supported.
	return false;
#else
	agx::Vec3Vector VerticesAGX = AGX_ShapeUtilities_helpers::ToAGX(Vertices);
	agx::Vec2 radiusHeightAGX;
	agx::AffineMatrix4x4 transformAGX;
	if (!agxUtil::computeOrientedCapsule(VerticesAGX, radiusHeightAGX, transformAGX))
	{
		return false;
	}
	OutRadius = ConvertDistanceToUnreal<float>(radiusHeightAGX.x());
	OutHeight = ConvertDistanceToUnreal<float>(radiusHeightAGX.y());
	OutTransform = Convert(transformAGX);
	return true;
#endif
}
