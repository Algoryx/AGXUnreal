#include "DepthTextureReader.h"

#include "DrawDebugHelpers.h"

UDepthTextureReader::UDepthTextureReader()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDepthTextureReader::Execute(const FVector& RefPos, const FQuat& RefRot)
{
	FTextureRenderTargetResource* RtResource = DepthTexture->GameThread_GetRenderTargetResource();
	TArray<FFloat16Color> PixelData;
	RtResource->ReadFloat16Pixels(PixelData);

	static constexpr double RT_PIXEL_WIDTH = 1200.0;
	static constexpr double RT_PIXEL_HEIGHT = 250.0;

	// From 120 deg POV and unit distance 1 to plane.
	static const double NearPlaneWidth = 2 * FMath::Sqrt(3); 

	static const double NearPlaneHeight = RT_PIXEL_HEIGHT / RT_PIXEL_WIDTH * NearPlaneWidth;
	static const double Stride = NearPlaneWidth / (RT_PIXEL_WIDTH - 1.0);
	static const FVector NearPlaneTopLeftCorner(
		1.0, -NearPlaneWidth / 2.0, NearPlaneHeight / 2.0); // Local Space.

	auto GetWorldPoint = [&](int32 X, int32 Y)
	{
		// X and Y is texture coordinates, so +y and -z directions on local space.
		const FVector NearPlanePoint =
			NearPlaneTopLeftCorner + FVector(0.0, (double) X * Stride, -(double) Y * Stride);
		FVector PointDir = NearPlanePoint.GetSafeNormal();
		const float FlatDistance = PixelData[X + Y * FMath::RoundToInt(RT_PIXEL_WIDTH)].R.GetFloat();
		const float ActualDistance = FlatDistance / (PointDir | FVector(1.0, 0.0, 0.0));
		const FVector PointLocal = PointDir * ActualDistance;
		return FTransform(RefRot, RefPos).TransformPosition(PointLocal);
	};


	for (int x = 500; x < 700; x++)
	{
		for (int y = 80; y < 170; y++)
		{
			const FVector WorldPoint = GetWorldPoint(x, y);
			if (WorldPoint.X < 10000.0)
				DrawDebugPoint(GetWorld(), GetWorldPoint(x, y), 5.f, FColor::Red, false, -1.f, 99);
		}
	}
}
