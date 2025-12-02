// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Misc/EngineVersionComparison.h"

#if UE_VERSION_OLDER_THAN(5, 0, 0)
/**
 * Unreal Engine 5 replaced the class FVector with the class template TVector<T> and the
 * two template specializations (or is it just type aliases?) FVector3f and FVector3d along with
 * a type alias FVector for FVector3d. We have a fair amount of code that used to work with only
 * FVector but that now mixes FVector3f and FVector3d, in particular code that bridge between AGX
 * Dynamics and Unreal Engine rendering code. This using directive makes it so that we can write
 * Unreal Engine 5 code that is still compatible with Unreal Engine 4.
 */
using FVector3f = FVector;
using FVector3d = FVector;
using FVector2f = FVector2D;
using FVector2d = FVector2D;
using FQuat4f = FQuat;
using FQuat4d = FQuat;
using FRotator3f = FRotator;
using FRotator3d = FRotator;
using FTransform3f = FTransform;
using FTransform3d = FTransform;
#endif

inline FVector3f ToMeshVector(const FVector& V)
{
	// On Unreal Engine 4 this is basically a no-op.
	using EType = decltype(FVector3f::X);
	return FVector3f(static_cast<EType>(V.X), static_cast<EType>(V.Y), static_cast<EType>(V.Z));
}

inline FVector FromMeshVector(const FVector3f& V)
{
	// On Unreal Engine 4 this is basically a no-op.
	using EType = decltype(FVector::X);
	return FVector(static_cast<EType>(V.X), static_cast<EType>(V.Y), static_cast<EType>(V.Z));
}

inline FQuat4f ToMeshQuat(const FQuat& Q)
{
	using EType = decltype(FQuat4f::X);
	return FQuat4f(
		static_cast<EType>(Q.X), static_cast<EType>(Q.Y), static_cast<EType>(Q.Z),
		static_cast<EType>(Q.W));
}

inline FQuat FromMeshQuat(const FQuat4f& Q)
{
	using EType = decltype(FQuat::X);
	return FQuat(
		static_cast<EType>(Q.X), static_cast<EType>(Q.Y), static_cast<EType>(Q.Z),
		static_cast<EType>(Q.W));
}
