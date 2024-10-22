#pragma once

#include "CoreMinimal.h"
#include <functional>
#include "ProceduralMeshComponent.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AGX_TerrainMeshUtilities.generated.h"

struct HfMeshDescription
{
	HfMeshDescription(FIntVector2 VertexRes)
	{
		int NrOfVerts = VertexRes.X * VertexRes.Y;
		Vertices.SetNum(NrOfVerts);
		Triangles.SetNum((VertexRes.X - 1) * (VertexRes.Y - 1) * 6);
		UV0.SetNum(NrOfVerts);
		UV1.SetNum(NrOfVerts);
		Tangents.SetNum(NrOfVerts);
		Normals.SetNum(NrOfVerts);
		Colors.SetNum(NrOfVerts);
	}

	TArray<FVector> Vertices;
	TArray<int> Triangles;
	TArray<FVector2D> UV0;
	TArray<FVector2D> UV1;
	TArray<FVector> Normals;
	TArray<struct FProcMeshTangent> Tangents;
	TArray<FColor> Colors;
};
/**
 *
 */
UCLASS()
class AGXUNREAL_API UAGX_TerrainMeshUtilities : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static TSharedPtr<HfMeshDescription> CreateMeshDescription(
		const FVector& Center, const FVector2D& Size, const FIntVector2& Resolution, double UvScale,
		std::function<float(const FVector&)> HeightFunction, bool IsUseSkirt = false);

	static float GetBrownianNoise(
		const FVector& Pos, int Octaves, float Scale, float Persistance, float Lacunarity,
		float Exp);

	static float GetRaycastedHeight(
		const FVector& Pos, const TArray<UMeshComponent*>& MeshComponents,
		const FVector& Up = FVector::UpVector, const float RayLength = 1000.0f);

	static float SampleHeightArray(
		FVector2D UV, const TArray<float>& HeightArray, int Width, int Height);

	static FBox CreateEncapsulatingBoundingBox(
		const TArray<UMeshComponent*>& Meshes, const FTransform& worldTransform);

	template <typename T>
	static TArray<FName> GetComponentNamesOfType(UObject* Outer);

private:
	static void GenerateTriangles(
		HfMeshDescription& MeshDesc, const FVector& Center, const FVector2D& Size,
		const FIntVector2& Resolution, double UvScale,
		std::function<float(const FVector&)> HeightFunction, bool IsUseSkirt);
};

template <typename T>
inline TArray<FName> UAGX_TerrainMeshUtilities::GetComponentNamesOfType(UObject* Outer)
{
	TArray<FName> Names;

	// Check if the outer object is a BlueprintGeneratedClass
	UBlueprintGeneratedClass* OwningGenClass = Cast<UBlueprintGeneratedClass>(Outer);
	if (OwningGenClass == nullptr)
		return Names;

	// Get the construction script associated with the BlueprintGeneratedClass
	const TObjectPtr<USimpleConstructionScript> ConstructionScript =
		OwningGenClass->SimpleConstructionScript;
	if (ConstructionScript == nullptr)
		return Names;

	// Iterate over all nodes in the construction script
	for (const USCS_Node* Component : ConstructionScript->GetAllNodes())
	{
		// Try casting each component to the type T
		T* CastComponent = Cast<T>(Component->ComponentTemplate);
		if (CastComponent != nullptr)
		{
			// Get the component's variable name and add it to the list
			Names.Add(Component->GetVariableName());
		}
	}

	return Names;
}
