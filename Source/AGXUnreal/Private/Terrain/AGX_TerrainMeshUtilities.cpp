
#include "Terrain/AGX_TerrainMeshUtilities.h"
#include "Shapes/AGX_SimpleMeshComponent.h"
#include <KismetProceduralMeshLibrary.h>

TSharedPtr<FAGX_MeshDescription> UAGX_TerrainMeshUtilities::CreateMeshDescription(
	const FVector& Center, const FVector2D& Size,  FIntVector2 Resolution, const FVector2D& UvScale,
	const FAGX_MeshVertexFunction VertexFunction, bool bAddSeamSkirts, bool bReverseWinding)
{
	//VertexRes = (Face)Resolution + 1
	//With skirts +2 extra per row/column
	FIntVector2 VertexRes = FIntVector2(
		Resolution.X + (bAddSeamSkirts ? 3 : 1), Resolution.Y + (bAddSeamSkirts ? 3 : 1));

	auto MeshDescPtr = MakeShared<FAGX_MeshDescription>(VertexRes);
	auto& MeshDesc = MeshDescPtr.Get();
	
	// Size of individual triangle
	FVector2D TriangleSize =
		FVector2D(Size.X / Resolution.X, Size.Y / Resolution.Y);

	// Hacky: With skirt, begin one row/column outside plane
	const int StartIndex = bAddSeamSkirts ? -1 : 0;

	// Populate vertices, uvs, colors
	int32 VertexIndex = 0;
	int32 TriangleIndex = 0;
	for (int32 y = 0; y < VertexRes.Y; ++y)
	{
		for (int32 x = 0; x < VertexRes.X; ++x)
		{
			FVector PlanePosition = Center + FVector(
				(x + StartIndex) * TriangleSize.X - Size.X / 2,
				(y + StartIndex) * TriangleSize.Y - Size.Y / 2, 0.0);

			// Default Vertex Position
			MeshDesc.Vertices[VertexIndex] = PlanePosition;
			MeshDesc.UV0[VertexIndex] =
				FVector2D(PlanePosition.X * UvScale.X + 0.5, PlanePosition.Y * UvScale.Y + 0.5);

			bool IsSkirtVertex = bAddSeamSkirts &&
								 (x == 0 || x == VertexRes.X - 1 || y == 0 || y == VertexRes.Y - 1);

			// Modify Vertex with VertexFunction callback
			VertexFunction(
				MeshDesc.Vertices[VertexIndex], MeshDesc.UV0[VertexIndex],
				MeshDesc.UV1[VertexIndex], MeshDesc.Colors[VertexIndex], IsSkirtVertex);


			// Skip last row and column for triangles
			if (x < VertexRes.X - 1 && y < VertexRes.Y - 1)
			{
				int32 BottomLeft = VertexIndex;
				int32 BottomRight = BottomLeft + 1;
				int32 TopLeft = BottomLeft + VertexRes.X;
				int32 TopRight = TopLeft + 1;

                if (bReverseWinding)
                {
                    // First triangle with flipped winding order
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;
                    MeshDesc.Triangles[TriangleIndex++] = TopLeft;

                    // Second triangle with flipped winding order
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = BottomRight;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;
                }
                else
                {
                    // First triangle
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;

                    // Second triangle
                    MeshDesc.Triangles[TriangleIndex++] = BottomLeft;
                    MeshDesc.Triangles[TriangleIndex++] = TopRight;
                    MeshDesc.Triangles[TriangleIndex++] = BottomRight;
                }
			}

			++VertexIndex;
		}
	}

	UKismetProceduralMeshLibrary::CalculateTangentsForMesh(
		MeshDesc.Vertices, MeshDesc.Triangles, MeshDesc.UV0, MeshDesc.Normals, MeshDesc.Tangents);

	// Move skirt vertices downwards, 
	// after lightning calculation
	// to hide seams between tiles
	if (bAddSeamSkirts)
	{
		
		float SkirtLength = 1.0f;// FMath::Min(Size.X, Size.Y) * 0.01f;
		VertexIndex = 0;
		for (int32 y = 0; y < VertexRes.Y; ++y)
		{
			for (int32 x = 0; x < VertexRes.X; ++x)
			{
				if (x == 0 || x == VertexRes.X - 1 || y == 0 || y == VertexRes.Y - 1)
				{
					FVector V = MeshDesc.Vertices[VertexIndex];

					//Clamp/move the vertex back to Size
					FVector P = FVector(
						FMath::Clamp(V.X, Center.X - Size.X / 2, Center.X + Size.X / 2),
						FMath::Clamp(V.Y, Center.Y - Size.Y / 2, Center.Y + Size.Y / 2), Center.Z);

					//Fetch the height at the moved-back-position
					FVector2D DummyUV0, DummyUV1;
					FColor DummyColor(0,0,0,0);
					VertexFunction(P, DummyUV0, DummyUV1, DummyColor, false);

					//Move the vertex slightly downwards
					P -= FVector::UpVector * SkirtLength;

					// Use original height if it was further down
					//V -= FVector::UpVector * SkirtLength;
					MeshDesc.Vertices[VertexIndex] = FVector(P.X, P.Y, FMath::Min(P.Z, V.Z));

				}
				VertexIndex++;
			}
		}
	}

	return MeshDescPtr;
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

float UAGX_TerrainMeshUtilities::GetBrownianNoise(
	const FVector& Pos, int Octaves, float Scale, float Persistance, float Lacunarity, float Exp)
{
	float Amplitude = 1.0;
	float Frequency = 1.0;
	float Normalization = 0;
	float Total = 0;
	for (int o = 0; o < Octaves; o++)
	{
		float noiseValue = FMath::PerlinNoise3D(Pos * Frequency / Scale) * 0.5f + 0.5f;
		Total += noiseValue * Amplitude;
		Normalization += Amplitude;
		Amplitude *= Persistance;
		Frequency *= Lacunarity;
	}
	Total /= Normalization;
	return FMath::Pow(Total, Exp);
}

float UAGX_TerrainMeshUtilities::GetNoiseHeight(
	const FVector& LocalPos, const FTransform Transform,
	const FAGX_BrownianNoiseParams& NoiseParams)
{

	// To make it easier to align blocks of noise together
	// we project the sampleposition to a plane
	FVector Up = Transform.GetRotation().GetUpVector();
	FVector Pos = Transform.TransformPosition(LocalPos);
	Pos = Pos - Up * FVector::DotProduct(Pos, Up);

	float Noise = GetBrownianNoise(
		Pos, NoiseParams.Octaves, NoiseParams.Scale, NoiseParams.Persistance,
		NoiseParams.Lacunarity, NoiseParams.Exp);

	return Noise * NoiseParams.NoiseHeight + NoiseParams.BaseHeight;
}

float UAGX_TerrainMeshUtilities::GetBedHeight(
	const FVector& LocalPos, const FTransform Transform, const TArray<UMeshComponent*>& BedMeshes,
	const float MaxHeight)
{
	float h = 0.0f;
	FVector Up = Transform.GetRotation().GetUpVector();
	FVector Stop = Transform.TransformPosition(FVector(LocalPos.X, LocalPos.Y, 0.0));
	FVector Start = Stop + Up * MaxHeight;
	FHitResult OutHit;

	for (auto& MeshComponent : BedMeshes)
	{
		if (UAGX_SimpleMeshComponent* ShapeComponent =
				Cast<UAGX_SimpleMeshComponent>(MeshComponent))
		{
			// UAGX_SimpleMeshComponent:
			if (ShapeComponent->LineTraceMesh(OutHit, Start, Stop))
				h = FMath::Max(MaxHeight - OutHit.Distance, h);
		}
		else
		{
			// UMeshComponents:
			FCollisionQueryParams Params;
			if (MeshComponent->LineTraceComponent(OutHit, Start, Stop, Params))
				h = FMath::Max(MaxHeight - OutHit.Distance, h);
		}
	}

	return h;
}
