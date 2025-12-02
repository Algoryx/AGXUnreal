// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_HeightFieldBoundsComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Shapes/AGX_HeightFieldShapeComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_HeightFieldUtilities.h"

// Unreal Engine includes.
#include "Landscape.h"

// Standard library includes.
#include <cmath>

UAGX_HeightFieldBoundsComponent::UAGX_HeightFieldBoundsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

namespace AGX_HeightFieldBoundsComponent_helpers
{
	FVector GetInfinateOrUserSelectedBounds(
		bool Infinite, const ALandscape& Landscape, const FVector& UserSelected)
	{
		if (!Infinite)
		{
			return UserSelected;
		}

		// Here, we take a "shortcut" of using an arbitrary large value. Calculating a bounding box
		// given the Landscape transform and this Component's owning transform along with Landscape
		// size information, taking into account the rotation of the landscape etc could be done,
		// but it is unnecessarily complicated. Really, we just want a really large bound that will
		// include everything.
		static constexpr double LargeNumber = 1.e+10f;

		// The z-value has no effect on the simulation at all, and is purely visual. We set a rather
		// low value for it because it is easier to see where the bounds actually are in that case.
		static constexpr double HalfExtentZ = 100.f;
		return FVector(LargeNumber, LargeNumber, HalfExtentZ);
	}

	int64 GetClosestVertexIndex(double Distance, double QuadSize)
	{
		return FMath::RoundToInt(FMath::RoundToDouble(Distance / QuadSize));
	}

	FVector GetCenterPosInfiniteBounds(
		double QuadSizeX, double QuadSizeY, int64 VertexCountX, int64 VertexCountY)
	{
		// Since Terrain has its center in the middle, when the Terrain covers the entire
		// Landscape the Terrain will be at the center of the Landscape, regardless of where
		// the Terrain Actor is actually placed.
		return {
			QuadSizeX * static_cast<double>(VertexCountX) / 2.0,
			QuadSizeY * static_cast<double>(VertexCountY) / 2.0, 0.0};
	}

	FVector GetCenterPosUserBounds(
		const FTransform& LandscapeTransform, const FVector& TerrainLocation)
	{
		// Not infinite bounds, so the Terrain is where the Terrain Actor is. No Scale because
		// we want the offset in world scale units, i.e. cm, not in "quad size" units.
		return LandscapeTransform.InverseTransformPositionNoScale(TerrainLocation);
	}
}

TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo>
UAGX_HeightFieldBoundsComponent::GetUserSetBounds() const
{
	using namespace AGX_HeightFieldBoundsComponent_helpers;
	TOptional<FTransformAndLandscape> TransformAndLandscape = GetLandscapeAndTransformFromOwner();
	if (!TransformAndLandscape.IsSet())
	{
		return {};
	}

	const ALandscape& Landscape = TransformAndLandscape->Landscape;
	const double QuadSizeX = Landscape.GetActorScale().X;
	const double QuadSizeY = Landscape.GetActorScale().Y;

	const FTransform& LandscapeTransform = Landscape.GetActorTransform();
	// Not necessarily a Terrain, could be a Height Field. Find a better name.
	const FTransform& TerrainTransform = TransformAndLandscape->Transform;

	const FVector SelectedHalfExtent =
		GetInfinateOrUserSelectedBounds(bInfiniteBounds, Landscape, HalfExtent);

	if (SelectedHalfExtent.X <= 0.0 || SelectedHalfExtent.Y <= 0.0 || SelectedHalfExtent.Z <= 0.0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with non-positive half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	int64 VertexCountX;
	int64 VertexCountY;
	std::tie(VertexCountX, VertexCountY) =
		AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(Landscape);

	// The position of the Terrain center in the Landscape's local coordinate system.
	const FVector CenterPosLocal = [&]()
	{
		if (bInfiniteBounds)
		{
			return GetCenterPosInfiniteBounds(QuadSizeX, QuadSizeY, VertexCountX, VertexCountY);
		}
		else
		{
			return GetCenterPosUserBounds(LandscapeTransform, TerrainTransform.GetLocation());
		}
	}();

	// Snap Terrain position to a vertex position.
	const int64 ClosestVertexX = GetClosestVertexIndex(CenterPosLocal.X, QuadSizeX);
	const int64 ClosestVertexY = GetClosestVertexIndex(CenterPosLocal.Y, QuadSizeY);
	const FVector BoundPosGlobal = LandscapeTransform.TransformPositionNoScale(FVector(
		static_cast<double>(ClosestVertexX) * QuadSizeX,
		static_cast<double>(ClosestVertexY) * QuadSizeY, 0.0));

	// Snap half extent to quad size.
	const int64 HalfExtentVertsX = GetClosestVertexIndex(SelectedHalfExtent.X, QuadSizeX);
	const int64 HalfExtentVertsY = GetClosestVertexIndex(SelectedHalfExtent.Y, QuadSizeY);
	const FVector BoundsHalfExtent {
		static_cast<double>(HalfExtentVertsX) * QuadSizeX,
		static_cast<double>(HalfExtentVertsY) * QuadSizeY, SelectedHalfExtent.Z};

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorRotation(), BoundPosGlobal);
	BoundsInfo.HalfExtent = BoundsHalfExtent;

	return BoundsInfo;
}

TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo>
UAGX_HeightFieldBoundsComponent::GetLandscapeAdjustedBounds() const
{
	using namespace AGX_HeightFieldBoundsComponent_helpers;
	TOptional<FTransformAndLandscape> TransformAndLandscape = GetLandscapeAndTransformFromOwner();
	if (!TransformAndLandscape.IsSet())
	{
		return {};
	}

	const ALandscape& Landscape = TransformAndLandscape->Landscape;
	const double QuadSizeX = Landscape.GetActorScale().X;
	const double QuadSizeY = Landscape.GetActorScale().Y;

	const FTransform& LandscapeTransform = Landscape.GetActorTransform();
	// Not necessarily a Terrain, could be a Height Field. Find a better name.
	const FTransform& TerrainTransform = TransformAndLandscape->Transform;

	auto LandscapeToWorld = [&](const FVector& Position)
	{ return LandscapeTransform.TransformPositionNoScale(Position); };

	auto WorldToLandscape = [&](const FVector& Position)
	{ return LandscapeTransform.InverseTransformPositionNoScale(Position); };

	const FVector SelectedHalfExtent =
		GetInfinateOrUserSelectedBounds(bInfiniteBounds, Landscape, HalfExtent);

	if (SelectedHalfExtent.X <= 0.0 || SelectedHalfExtent.Y <= 0.0 || SelectedHalfExtent.Z <= 0.0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' have bounds with non-positive half extent. This is not supported."),
			*GetOuter()->GetName());
		return {};
	}

	int64 VertexCountX;
	int64 VertexCountY;
	std::tie(VertexCountX, VertexCountY) =
		AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(Landscape);

	const FVector CenterPosInLandscape = [&]()
	{
		if (bInfiniteBounds)
		{
			return GetCenterPosInfiniteBounds(QuadSizeX, QuadSizeY, VertexCountX, VertexCountY);
		}
		else
		{
			return GetCenterPosUserBounds(LandscapeTransform, TerrainTransform.GetLocation());
		}
	}();

	//
	int64 ClosestVertexX;
	int64 ClosestVertexY;
	if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(Landscape))
	{
		// Closest Vertex X and Y are expressed relative to the loaded bound's min corner.
		const FBox LoadedBoundsInWorld = Landscape.GetLoadedBounds();
		const FVector LoadedMinInLandscape = WorldToLandscape(LoadedBoundsInWorld.Min);
		const FVector TerrainCenterInLoadedBounds = CenterPosInLandscape - LoadedMinInLandscape;
		ClosestVertexX = GetClosestVertexIndex(TerrainCenterInLoadedBounds.X, QuadSizeX);
		ClosestVertexY = GetClosestVertexIndex(TerrainCenterInLoadedBounds.Y, QuadSizeY);
	}
	else
	{
		// Closest Vertex X and Y are expressed relative to the Landscape's min corner, which is
		// the same as the Landscape's origin.
		//
		// TODO The above is false if Landscape segments have been added on the wrong side of the
		// Landscape's origin point. We should support that.
		ClosestVertexX = GetClosestVertexIndex(CenterPosInLandscape.X, QuadSizeX);
		ClosestVertexY = GetClosestVertexIndex(CenterPosInLandscape.Y, QuadSizeY);
	}

	if (ClosestVertexX <= 0 || ClosestVertexX > VertexCountX || ClosestVertexY <= 0 ||
		ClosestVertexY > VertexCountY)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Failed to create Landscape adjusted bounds: Closest vertex is out of bounds.\n"
				 "X: Center position local = %f. Closest vertex = %lld. Vertex count = %lld\n"
				 "Y: Center position local = %f. Closest vertex = %lld. Vertex count = %lld."),
			CenterPosInLandscape.X, ClosestVertexX, VertexCountX, CenterPosInLandscape.Y, ClosestVertexY,
			VertexCountY);
		return {};
	}

	int64 HalfExtentVertsX = GetClosestVertexIndex(SelectedHalfExtent.X, QuadSizeX);
	int64 HalfExtentVertsY = GetClosestVertexIndex(SelectedHalfExtent.Y, QuadSizeY);

	// Ensure we are not outside the Landscape edge.
	HalfExtentVertsX = std::min(HalfExtentVertsX, VertexCountX - ClosestVertexX - 1);
	HalfExtentVertsX = std::min(HalfExtentVertsX, ClosestVertexX - 0);
	HalfExtentVertsY = std::min(HalfExtentVertsY, VertexCountY - ClosestVertexY - 1);
	HalfExtentVertsY = std::min(HalfExtentVertsY, ClosestVertexY - 0);

	if (HalfExtentVertsX == 0 || HalfExtentVertsY == 0)
		return {};

	const FVector BoundPosGlobal = [&]()
	{
		if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(Landscape))
		{
			// Compute the world position of the center of the Terrain snapped to the closest
			// vertex. Must be done in the loaded bounds' space since Closest Vertex X and Y is
			// expressed relative to that bound.
			const FBox LoadedBoundsInWorld = Landscape.GetLoadedBounds();
			const FVector LoadedMinInLandscape = WorldToLandscape(LoadedBoundsInWorld.Min);
			const FVector TerrainPosInLoaded {
				LoadedMinInLandscape.X + static_cast<double>(ClosestVertexX) * QuadSizeX,
				LoadedMinInLandscape.Y + static_cast<double>(ClosestVertexY) * QuadSizeY, 0};
			return LandscapeToWorld(TerrainPosInLoaded);
		}
		else
		{
			// Compute the world position of the center of the Terrain snapped to the closest
			// vertex.
			//
			// TODO This assumes that the Landscape doesn't have any Landscape Components with
			// base X or Y coordinate less than 0. We should support that.
			const FVector TerrainInLandscape(
				static_cast<double>(ClosestVertexX) * QuadSizeX,
				static_cast<double>(ClosestVertexY) * QuadSizeY, 0);
			return LandscapeToWorld(TerrainInLandscape);
		}
	}();

	FHeightFieldBoundsInfo BoundsInfo;
	BoundsInfo.Transform = FTransform(Landscape.GetActorRotation(), BoundPosGlobal);
	BoundsInfo.HalfExtent = FVector(
		static_cast<double>(HalfExtentVertsX) * QuadSizeX,
		static_cast<double>(HalfExtentVertsY) * QuadSizeY, SelectedHalfExtent.Z);

	return BoundsInfo;
}

TOptional<UAGX_HeightFieldBoundsComponent::FTransformAndLandscape>
UAGX_HeightFieldBoundsComponent::GetLandscapeAndTransformFromOwner() const
{
	if (AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(GetOwner()))
	{
		if (Terrain->SourceLandscape != nullptr)
		{
			return FTransformAndLandscape(*Terrain->SourceLandscape, Terrain->GetActorTransform());
		}
	}

	if (UAGX_HeightFieldShapeComponent* HeightField =
			Cast<UAGX_HeightFieldShapeComponent>(GetOuter()))
	{
		if (HeightField->SourceLandscape != nullptr)
		{
			return FTransformAndLandscape(
				*HeightField->SourceLandscape, HeightField->GetComponentTransform());
		}
	}

	return {};
}

#if WITH_EDITOR
bool UAGX_HeightFieldBoundsComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	UWorld* World = GetWorld();
	if (World == nullptr || !World->IsPlayInEditor())
	{
		return SuperCanEditChange;
	}

	const FName Prop = InProperty->GetFName();
	if (Prop == GET_MEMBER_NAME_CHECKED(UAGX_HeightFieldBoundsComponent, HalfExtent))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(UAGX_HeightFieldBoundsComponent, bInfiniteBounds))
		return false;
	else
		return SuperCanEditChange;
}
#endif
