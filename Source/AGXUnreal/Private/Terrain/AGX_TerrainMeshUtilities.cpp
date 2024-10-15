
#include "Terrain/AGX_TerrainMeshUtilities.h"

#include <KismetProceduralMeshLibrary.h>

TSharedPtr<HfMeshDescription> UAGX_TerrainMeshUtilities::CreateMeshDescription(
	const FVector& center, const FVector2D& size, const FIntVector2& resolution, double uvScale,
	std::function<float(const FVector&)> heightFunction, bool isUseSkirt)
{
	// Vertex count with or without skirt
	FIntVector2 nrOfVerts;
	if (isUseSkirt)
		nrOfVerts = FIntVector2(resolution.X + 3, resolution.Y + 3);
	else
		nrOfVerts = FIntVector2(resolution.X + 1, resolution.Y + 1);

	// Create pointer
	auto meshDescPtr = MakeShared<HfMeshDescription>(nrOfVerts);
	auto& meshDesc = *meshDescPtr;

	// Populate vertices, uvs, colors
	GenerateTriangles(meshDesc, center, size, resolution, uvScale, heightFunction, isUseSkirt);

	// Compute tangents and normals, while skirt is still being flat
	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
		meshDesc.Vertices, meshDesc.Triangles, meshDesc.UV0, meshDesc.Normals, meshDesc.Tangents);

	// Move skirt vertices downwards
	if (isUseSkirt)
	{
		FVector skirtOffset = FVector::UpVector * size.Length() * 0.05f;
		int vertexIndex = 0;
		for (int32 y = 0; y < nrOfVerts.Y; ++y)
		{
			for (int32 x = 0; x < nrOfVerts.X; ++x)
			{
				if (x == 0 || x == nrOfVerts.X - 1 || y == 0 || y == nrOfVerts.Y - 1)
				{
					meshDesc.Vertices[vertexIndex] -= skirtOffset;
				}
				vertexIndex++;
			}
		}
	}

	return meshDescPtr;
}

void UAGX_TerrainMeshUtilities::GenerateTriangles(
	HfMeshDescription& meshDesc, const FVector& center, const FVector2D& size,
	const FIntVector2& resolution, double uvScale,
	std::function<float(const FVector&)> heightFunction, bool isUseSkirt)
{
	// Vertex count with or without skirt
	FIntVector2 nrOfVerts;
	if (isUseSkirt)
		nrOfVerts = FIntVector2(resolution.X + 3, resolution.Y + 3);
	else
		nrOfVerts = FIntVector2(resolution.X + 1, resolution.Y + 1);

	FVector2D triangleSize = FVector2D(size.X / resolution.X, size.Y / resolution.Y);

	// Fill the vertices and triangles
	int startOffset = isUseSkirt ? -1 : 0;
	int32 vertexIndex = 0;
	int32 triangleIndex = 0;
	for (int32 y = 0; y < nrOfVerts.Y; ++y)
	{
		for (int32 x = 0; x < nrOfVerts.X; ++x)
		{
			FVector planePosition = FVector(
				(x + startOffset) * triangleSize.X - size.X / 2,
				(y + startOffset) * triangleSize.Y - size.Y / 2, 0.0f);
			FVector localPosition = planePosition + center;

			// Call height function
			float height = heightFunction(localPosition);

			meshDesc.Vertices[vertexIndex] = planePosition + FVector::UpVector * height;
			meshDesc.UV0[vertexIndex] =
				FVector2D(uvScale * localPosition.X, uvScale * localPosition.Y);

			// Skip last row and column for triangles
			if (x < nrOfVerts.X - 1 && y < nrOfVerts.Y - 1)
			{
				int32 bottomLeft = vertexIndex;
				int32 bottomRIght = bottomLeft + 1;
				int32 topLeft = bottomLeft + nrOfVerts.X;
				int32 topRight = topLeft + 1;

				// First triangle
				meshDesc.Triangles[triangleIndex++] = bottomLeft;
				meshDesc.Triangles[triangleIndex++] = topLeft;
				meshDesc.Triangles[triangleIndex++] = topRight;

				// Second triangle
				meshDesc.Triangles[triangleIndex++] = bottomLeft;
				meshDesc.Triangles[triangleIndex++] = topRight;
				meshDesc.Triangles[triangleIndex++] = bottomRIght;
			}

			++vertexIndex;
		}
	}
}

float UAGX_TerrainMeshUtilities::GetBrownianNoise(
	const FVector& pos, int octaves, float scale, float persistance, float lacunarity, float exp)
{
	float amplitude = 1.0;
	float frequency = 1.0;
	float normalization = 0;
	float total = 0;
	for (int o = 0; o < octaves; o++)
	{
		float noiseValue = FMath::PerlinNoise3D(pos * frequency / scale) * 0.5f + 0.5f;
		total += noiseValue * amplitude;
		normalization += amplitude;
		amplitude *= persistance;
		frequency *= lacunarity;
	}
	total /= normalization;
	return FMath::Pow(total, exp);
}

float UAGX_TerrainMeshUtilities::GetRaycastedHeight(
	const FVector& pos, const TArray<UMeshComponent*>& meshComponents, const FVector& up,
	const float rayLength)
{
	float bedHeight = 0.0f;
	for (auto uMesh : meshComponents)
	{
		FHitResult hit;
		FCollisionQueryParams params;
		FVector start = pos + FVector::UpVector * rayLength;
		FVector stop = pos;

		if (uMesh->LineTraceComponent(hit, start, stop, params))
		{
			bedHeight = FMath::Max(bedHeight, rayLength - hit.Distance);
		}
	}

	return bedHeight;
}

float UAGX_TerrainMeshUtilities::SampleHeightArray(
	FVector2D UV, const TArray<float>& HeightArray, int Width, int Height)
{
	// Normalize UV coordinates to the range [0, 1]
	UV.X = FMath::Clamp(UV.X, 0.0f, 1.0f);
	UV.Y = FMath::Clamp(UV.Y, 0.0f, 1.0f);

	// Calculate the positions of the four surrounding texels
	double X = UV.X * (Width - 1);
	double Y = UV.Y * (Height - 1);

	int32 X1 = FMath::FloorToInt(X);
	int32 Y1 = FMath::FloorToInt(Y);
	int32 X2 = FMath::Clamp(X1 + 1, 0, Width - 1);
	int32 Y2 = FMath::Clamp(Y1 + 1, 0, Height - 1);

	double FracX = X - X1;
	double FracY = Y - Y1;

	auto GetValue = [&](int32 X, int32 Y) -> float { return HeightArray[Y * Width + X]; };

	float C00 = GetValue(X1, Y1);
	float C10 = GetValue(X2, Y1);
	float C01 = GetValue(X1, Y2);
	float C11 = GetValue(X2, Y2);

	// Bilinear interpolation
	return FMath::BiLerp(C00, C10, C01, C11, FracX, FracY);
}

