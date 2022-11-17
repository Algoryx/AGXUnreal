// Copyright 2022, Algoryx Simulation AB.

#include "Utilities/AGX_HeightFieldUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/EngineVersionComparison.h"

// Standard library includes.
#include <limits>

namespace AGX_HeightFieldUtilities_helpers
{

	FTransform GetAGXTransformUsingBoxFrom(
		const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent,
		bool IsTerrain)
	{
		const FVector CenterProjectedLocal = [&]()
		{
			FVector CenterLocal =
				Landscape.GetActorTransform().InverseTransformPositionNoScale(Center);
			CenterLocal.Z = 0.0;
			return CenterLocal;
		}();

		if (!IsTerrain)
		{
			return FTransform(
				Landscape.GetActorQuat(),
				Landscape.GetActorTransform().TransformPositionNoScale(CenterProjectedLocal));
		}

		// The terrain will be offset half a tile if the number of tiles are odd. This is an AGX
		// Dynamics thing.
		const auto QuadSideSizeX = Landscape.GetActorScale().X;
		const auto QuadSideSizeY = Landscape.GetActorScale().Y;
		const int32 NumQuadsX = FMath::RoundToInt(2.0 * HalfExtent.X / QuadSideSizeX);
		const int32 NumQuadsY = FMath::RoundToInt(2.0 * HalfExtent.Y / QuadSideSizeY);
		const double TerrainTileCenterOffsetX = (NumQuadsX % 2 == 0) ? 0 : QuadSideSizeX / 2;
		const double TerrainTileCenterOffsetY = (NumQuadsY % 2 == 0) ? 0 : -QuadSideSizeY / 2;
		FVector LocalTileOffset(TerrainTileCenterOffsetX, TerrainTileCenterOffsetY, 0);

		const FVector CenterProjectedGlobalAdjusted =
			Landscape.GetActorTransform().TransformPositionNoScale(
				CenterProjectedLocal + LocalTileOffset);
		return FTransform(Landscape.GetActorQuat(), CenterProjectedGlobalAdjusted);
	}

	std::tuple<int32, int32> GetLandscapeQuadCountXYNonOpenWorld(const ALandscape& Landscape)
	{
		if (Landscape.LandscapeComponents.Num() == 0)
		{
			return std::tuple<int32, int32>(0, 0);
		}

		int32 MaxX = 0;
		int32 MaxY = 0;

		for (int i = 0; i < Landscape.LandscapeComponents.Num(); i++)
		{
			if (Landscape.LandscapeComponents[i]->SectionBaseX > MaxX)
			{
				MaxX = Landscape.LandscapeComponents[i]->SectionBaseX;
			}

			if (Landscape.LandscapeComponents[i]->SectionBaseY > MaxY)
			{
				MaxY = Landscape.LandscapeComponents[i]->SectionBaseY;
			}
		}

		// MaxX and MaxY is the bottom corner of the furthest away section. We need to add the
		// number of quads per section to this result to get the final count.
		const int32 NumQuadsPerComponentSide = Landscape.ComponentSizeQuads;
		const int32 QuadCountX = MaxX + NumQuadsPerComponentSide;
		const int32 QuadCountY = MaxY + NumQuadsPerComponentSide;

		return std::tuple<int32, int32> {QuadCountX, QuadCountY};
	}

	std::tuple<double, double> GetLandscapeSizeXYNonOpenWorld(const ALandscape& Landscape)
	{
		std::tuple<int32, int32> NumQuadsXY = GetLandscapeQuadCountXYNonOpenWorld(Landscape);
		const double QuadSideSizeX = Landscape.GetActorScale().X;
		const double QuadSideSizeY = Landscape.GetActorScale().Y;
		const double SizeX = static_cast<double>(std::get<0>(NumQuadsXY)) * QuadSideSizeX;
		const double SizeY = static_cast<double>(std::get<1>(NumQuadsXY)) * QuadSideSizeY;

		return std::tuple<double, double>(SizeX, SizeY);
	}

	std::tuple<double, double> GetLandscapeSizeXYOpenWorld(const ALandscape& Landscape)
	{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		return std::tuple<double, double>(0.0, 0.0);
#else

		// @todo limitation: this will only yield the correct result if the Landscape is not rotated
		// around world Z axis. We are yet to find a reliable way to find the size in that case for
		// open world landscapes.
		// Also, this requires the landscape to be loaded in the world partitioner which is a big
		// limitation.

		AGX_CHECK(Landscape.LandscapeComponents.Num() == 0); // Open World check.
		const FBox Bounds = Landscape.GetLoadedBounds();

		const double Dx = Bounds.Max.X - Bounds.Min.X;
		const double Dy = Bounds.Max.Y - Bounds.Min.Y;

		return std::tuple<double, double>(FMath::Abs(Dx), FMath::Abs(Dy));
#endif
	}

	void NudgePoint(
		double& X, double& Y, double MinX, double MaxX, double MinY, double MaxY,
		double NudgeDistanceX, double NudgeDistanceY)
	{
		if (X <= MinX)
			X += NudgeDistanceX;
		else if (X >= MaxX)
			X -= NudgeDistanceX;
		if (Y <= MinY)
			Y += NudgeDistanceY;
		else if (Y >= MaxY)
			Y -= NudgeDistanceY;
	}

	// Shoot single ray at landscape to measure the height. Returns false if the ray misses the
	// landscape and true otherwise. If it returns false the OutHeight is set to 0.0 but is not
	// a valid measurement.
	bool ShootSingleRay(
		const ALandscape& Landscape, double LocalX, double LocalY, double ZOffsetLocal,
		const FCollisionQueryParams& CollisionParams, FHitResult& HitResult, float& OutHeight)
	{
		OutHeight = 0.0f;

		// Ray start and end positions in global coordinates.
		const FVector RayStart = Landscape.GetTransform().TransformPositionNoScale(
			FVector(LocalX, LocalY, ZOffsetLocal));
		const FVector RayEnd = Landscape.GetTransform().TransformPositionNoScale(
			FVector(LocalX, LocalY, -ZOffsetLocal));

		if (Landscape.ActorLineTraceSingle(
				HitResult, RayStart, RayEnd, ECC_Visibility, CollisionParams))
		{
			// The hit point of the line trace in the landscape's local coordinate system.
			FVector ImpactPointLocal =
				Landscape.GetTransform().InverseTransformPositionNoScale(HitResult.ImpactPoint);

			OutHeight = ImpactPointLocal.Z;
			return true;
		}

		// Line trace missed.
		return false;
	}

	// This function should only be used if the landscape is not rotated around world x or y axis.
	// The reason for this is that the Landscape.GetHeightAtLocation does not handle that case. It
	// will measure along the world z-axis (instead of the Landscapes local z-axis as it should)
	// such that sharp peaks will be cut off and tilted.
	TArray<float> GetHeigtsUsingApi(
		ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY)
	{
		UE_LOG(LogAGX, Log, TEXT("About to read Landscape heights using Landscape API."));

		const auto QuadSideSizeX = Landscape.GetActorScale().X;
		const auto QuadSideSizeY = Landscape.GetActorScale().Y;
		const int32 ResolutionX = FMath::RoundToInt(LengthX / QuadSideSizeX);
		const int32 ResolutionY = FMath::RoundToInt(LengthY / QuadSideSizeY);
		const int32 VerticesSideX = ResolutionX + 1;
		const int32 VerticesSideY = ResolutionY + 1;

		TArray<float> Heights;
		const int32 NumVertices = VerticesSideX * VerticesSideY;
		if (NumVertices <= 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetHeightsUsingAPI got zero sized landscape. Cannot read landscape heights "
					 "from landscape '%s'."),
				*Landscape.GetName());
			return Heights;
		}

		Heights.Reserve(NumVertices);
		const FVector StartPosLocal =
			Landscape.GetActorTransform().InverseTransformPositionNoScale(StartPos);

		// AGX terrains Y axis goes in the opposite direction from Unreal's Y axis (flipped).
		const double MaxX = StartPosLocal.X + LengthX;
		const double MaxY = StartPosLocal.Y + LengthY;
		double CurrentX = StartPosLocal.X;
		double CurrentY = StartPosLocal.Y + LengthY;
		const double NudgeDistanceX = QuadSideSizeX / 1000.0;
		const double NudgeDistanceY = QuadSideSizeY / 1000.0;
		const double ToleranceX = QuadSideSizeX / 2.0;
		const double ToleranceY = QuadSideSizeY / 2.0;
		while (CurrentY >= StartPosLocal.Y - ToleranceY)
		{
			while (CurrentX <= MaxX + ToleranceX)
			{
				FVector LocationGlobal = Landscape.GetTransform().TransformPositionNoScale(
					FVector(CurrentX, CurrentY, 0));
				TOptional<float> Height = Landscape.GetHeightAtLocation(LocationGlobal);
				if (!Height.IsSet())
				{
					// Attempt to nudge the measuring point a little and do the measurement again.
					// We do this because sometimes, measuring at the edge of a Landscape does not
					// work.
					double NudgedX = CurrentX;
					double NudgedY = CurrentY;
					NudgePoint(
						NudgedX, NudgedY, StartPosLocal.X, MaxX, StartPosLocal.Y, MaxY,
						NudgeDistanceX, NudgeDistanceY);

					LocationGlobal = Landscape.GetTransform().TransformPositionNoScale(
						FVector(NudgedX, NudgedY, 0));
					Height = Landscape.GetHeightAtLocation(LocationGlobal);
				}
				if (Height.IsSet())
				{
					// Position of height measurement in Landscapes local coordinate system.
					FVector HeightPointLocal =
						Landscape.GetTransform().InverseTransformPositionNoScale(
							FVector(LocationGlobal.X, LocationGlobal.Y, *Height));
					Heights.Add(HeightPointLocal.Z);
				}
				else
				{
					UE_LOG(
						LogAGX, Error,
						TEXT("Unexpected error: reading height from Landscape '%s' at location %f, "
							 "%f, %f failed during AGX Heightfield initialization."),
						*Landscape.GetName(), LocationGlobal.X, LocationGlobal.Y, LocationGlobal.Z);
					Heights.Add(0.f);
				}
				CurrentX += QuadSideSizeX;
			}
			CurrentX = StartPosLocal.X;
			CurrentY -= QuadSideSizeY;
		}

		check(NumVertices == Heights.Num());

		return Heights;
	}

	// This is an alternative to AGX_HeightFieldUtilities_helpers::GetHeigtsUsingApi. This function
	// is slower but can handle any Landscape orientation, which is not the case for
	// AGX_HeightFieldUtilities_helpers::GetHeigtsUsingApi (see comment above that function).
	TArray<float> GetHeightsUsingRayCasts(
		ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY)
	{
		UE_LOG(LogAGX, Log, TEXT("About to read Landscape heights with ray casting."));
		const auto QuadSideSizeX = Landscape.GetActorScale().X;
		const auto QuadSideSizeY = Landscape.GetActorScale().Y;
		const int32 ResolutionX = FMath::RoundToInt(LengthX / QuadSideSizeX);
		const int32 ResolutionY = FMath::RoundToInt(LengthY / QuadSideSizeY);
		const int32 VerticesSideX = ResolutionX + 1;
		const int32 VerticesSideY = ResolutionY + 1;
		const int32 NumVertices = VerticesSideX * VerticesSideY;
		const FVector StartPosLocal =
			Landscape.GetActorTransform().InverseTransformPositionNoScale(StartPos);

		TArray<float> Heights;
		Heights.Reserve(NumVertices);
		int32 LineTraceMisses = 0;

		// At scale = 1, the height span is +- 256 cm
		// https://docs.unrealengine.com/en-US/Engine/Landscape/TechnicalGuide/#calculatingheightmapzscale
		const double HeightSpanHalf = 256.0 * Landscape.GetActorScale3D().Z;

		// Line traces will be used to measure the heights of the landscape.
		const FCollisionQueryParams CollisionParams(FName(TEXT("LandscapeHeightFieldTracess")));
		FHitResult HitResult(ForceInit);

		// AGX terrains Y axis goes in the opposite direction from Unreal's Y axis (flipped).
		const double MaxX = StartPosLocal.X + LengthX;
		const double MaxY = StartPosLocal.Y + LengthY;
		double CurrentX = StartPosLocal.X;
		double CurrentY = StartPosLocal.Y + LengthY;
		const double NudgeDistanceX = QuadSideSizeX / 1000.0;
		const double NudgeDistanceY = QuadSideSizeY / 1000.0;
		const double ToleranceX = QuadSideSizeX / 2.0;
		const double ToleranceY = QuadSideSizeY / 2.0;
		const double NudgeDistances[4][2] = {
			{NudgeDistanceX, NudgeDistanceY},
			{-NudgeDistanceX, -NudgeDistanceY},
			{-NudgeDistanceX, NudgeDistanceY},
			{NudgeDistanceX, -NudgeDistanceY}};
		while (CurrentY >= StartPosLocal.Y - ToleranceY)
		{
			while (CurrentX <= MaxX + ToleranceX)
			{
				float Height = 0.0f;

				// Use line trace to read the landscape height for this vertex.
				bool Result = ShootSingleRay(
					Landscape, CurrentX, CurrentY, HeightSpanHalf, CollisionParams, HitResult,
					Height);

				if (!Result)
				{
					// Line trace missed. This is unusual but has been observed with large
					// landscapes at the seams between landscape components/sections, similar to
					// line traces at the very edge being missed. Re-try the line trace but force
					// the ray's intersection point to be nudged slightly.
					for (int i = 0; i < 4; i++)
					{
						const double NudgedX = CurrentX + NudgeDistances[i][0];
						const double NudgedY = CurrentY + NudgeDistances[i][1];
						Result = ShootSingleRay(
							Landscape, NudgedX, NudgedY, HeightSpanHalf, CollisionParams, HitResult,
							Height);
						if (Result)
							break;
					}
				}

				if (!Result)
					LineTraceMisses++;

				Heights.Add(Height);
				CurrentX += QuadSideSizeX;
			}
			CurrentX = StartPosLocal.X;
			CurrentY -= QuadSideSizeY;
		}

		check(Heights.Num() == NumVertices);
		if (LineTraceMisses > 0)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("%d of %d vertices could not be read from the landscape. The heights of the "
					 "coresponding vertices in the AGX Terrain may therefore be incorrect."),
				LineTraceMisses, NumVertices);
		}

		return Heights;
	}
}

TArray<float> GetHeights(
	ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY)
{
	using namespace AGX_HeightFieldUtilities_helpers;
	const FRotator LandsapeRotation = Landscape.GetActorRotation();
	if (FMath::IsNearlyZero(LandsapeRotation.Roll, KINDA_SMALL_NUMBER) &&
		FMath::IsNearlyZero(LandsapeRotation.Pitch, KINDA_SMALL_NUMBER))
	{
		// If the Landscape is not rotated around x or y, we can use the Landscape API to read the
		// heights which is much faster than ray-casting.
		return GetHeigtsUsingApi(Landscape, StartPos, LengthX, LengthY);
	}
	else
	{
		return GetHeightsUsingRayCasts(Landscape, StartPos, LengthX, LengthY);
	}
}

FHeightFieldShapeBarrier AGX_HeightFieldUtilities::CreateHeightField(
	ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY)
{
	TArray<float> Heights = GetHeights(Landscape, StartPos, LengthX, LengthY);
	const auto QuadSideSizeX = Landscape.GetActorScale().X;
	const auto QuadSideSizeY = Landscape.GetActorScale().Y;
	const int32 ResolutionX = FMath::RoundToInt(LengthX / QuadSideSizeX) + 1;
	const int32 ResolutionY = FMath::RoundToInt(LengthY / QuadSideSizeY) + 1;
	FHeightFieldShapeBarrier HeightField;
	HeightField.AllocateNative(
		ResolutionX, ResolutionY, LengthX, LengthY,
		Heights);

	return HeightField;
}

FTransform AGX_HeightFieldUtilities::GetTerrainTransformUsingBoxFrom(
	const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent)
{
	return AGX_HeightFieldUtilities_helpers::GetAGXTransformUsingBoxFrom(
		Landscape, Center, HalfExtent, true);
}

FTransform AGX_HeightFieldUtilities::GetHeightFieldTransformUsingBoxFrom(
	const ALandscape& Landscape, const FVector& Center, const FVector& HalfExtent)
{
	return AGX_HeightFieldUtilities_helpers::GetAGXTransformUsingBoxFrom(
		Landscape, Center, HalfExtent, false);
}

std::tuple<int32, int32> AGX_HeightFieldUtilities::GetLandscapeNumberOfVertsXY(
	const ALandscape& Landscape)
{
	std::tuple<double, double> Size = GetLandscapeSizeXY(Landscape);
	const double QuadSideSizeX = Landscape.GetActorScale().X;
	const double QuadSideSizeY = Landscape.GetActorScale().Y;

	const int32 QuadCountX = FMath::RoundToInt(std::get<0>(Size) / QuadSideSizeX);
	const int32 QuadCountY = FMath::RoundToInt(std::get<1>(Size) / QuadSideSizeY);
	return std::tuple<int32, int32>(QuadCountX + 1, QuadCountY + 1);
}

std::tuple<double, double> AGX_HeightFieldUtilities::GetLandscapeSizeXY(const ALandscape& Landscape)
{
	using namespace AGX_HeightFieldUtilities_helpers;
	if (IsOpenWorldLandscape(Landscape))
	{
		return GetLandscapeSizeXYOpenWorld(Landscape);
	}
	else
	{
		return GetLandscapeSizeXYNonOpenWorld(Landscape);
	}
}

bool AGX_HeightFieldUtilities::IsOpenWorldLandscape(const ALandscape& Landscape)
{
	// This is just an observation that holds true for OpenWorldLandscapes, would be better
	// with a more "correct" way of determining this.
	return Landscape.LandscapeComponents.Num() == 0;
}
