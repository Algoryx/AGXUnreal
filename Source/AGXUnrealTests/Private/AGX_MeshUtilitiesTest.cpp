// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Tests/AutomationCommon.h"
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAGX_MeshUtilitiesTest, "AGXUnreal.Editor.AGX_MeshUtilities",
	EAutomationTestFlags::ProductFilter | EAutomationTestFlags::ApplicationContextMask)

namespace MeshGenerationTest_Helper
{
	bool ValidMeshDataSanityCheck(
		const TArray<FVector>& Positions, const TArray<FVector>& Normals,
		const TArray<uint32>& Indices, const TArray<FVector2D>& TexCoords)
	{
		if (Positions.Num() <= 0 || Normals.Num() <= 0 || Indices.Num() <= 0 ||
			TexCoords.Num() <= 0)
		{
			return false;
		}

		// We expect Positions, Normals and Texture coordinates arrays to have equal length.
		if (Normals.Num() != Positions.Num() || TexCoords.Num() != Positions.Num())
		{
			return false;
		}

		return true;
	}

	bool IsIndicesInBounds(const TArray<FVector>& Positions, const TArray<uint32>& Indices)
	{
		const uint32 MaxIndex = Positions.Num() - 1;
		const uint32 MinIndex = 0;

		for (const uint32 Ind : Indices)
		{
			if (Ind > MaxIndex || Ind < MinIndex)
			{
				return false;
			}
		}

		return true;
	}

	bool IsMeshDataValid(
		const TArray<FVector>& Positions, const TArray<FVector>& Normals,
		const TArray<uint32>& Indices, const TArray<FVector2D>& TexCoords)
	{
		if (!ValidMeshDataSanityCheck(Positions, Normals, Indices, TexCoords))
		{
			return false;
		}

		if (!IsIndicesInBounds(Positions, Indices))
		{
			return false;
		}

		return true;
	}
}

namespace
{
	bool BoxMeshGenerationTest(const FVector& HalfSize)
	{
		TArray<FVector> Positions;
		TArray<FVector> Normals;
		TArray<uint32> Indices;
		TArray<FVector2D> TexCoords;

		AGX_MeshUtilities::MakeCube(Positions, Normals, Indices, TexCoords, HalfSize);

		return MeshGenerationTest_Helper::IsMeshDataValid(Positions, Normals, Indices, TexCoords);
	}

	bool SphereMeshGenerationTest(float Radius, uint32 NumSegments)
	{
		TArray<FVector> Positions;
		TArray<FVector> Normals;
		TArray<uint32> Indices;
		TArray<FVector2D> TexCoords;

		AGX_MeshUtilities::MakeSphere(Positions, Normals, Indices, TexCoords, Radius, NumSegments);

		return MeshGenerationTest_Helper::IsMeshDataValid(Positions, Normals, Indices, TexCoords);
	}

	bool CylindereMeshGenerationTest(
		float Radius, float Height, uint32 NumCircleSegments, uint32 NumHeightSegments)
	{
		TArray<FVector> Positions;
		TArray<FVector> Normals;
		TArray<uint32> Indices;
		TArray<FVector2D> TexCoords;

		AGX_MeshUtilities::MakeCylinder(
			Positions, Normals, Indices, TexCoords,
			AGX_MeshUtilities::CylinderConstructionData(
				Radius, Height, NumCircleSegments, NumHeightSegments));

		return MeshGenerationTest_Helper::IsMeshDataValid(Positions, Normals, Indices, TexCoords);
	}

	bool CapsuleMeshGenerationTest(
		float Radius, float Height, uint32 NumCircleSegments, uint32 NumHeightSegments)
	{
		TArray<FVector> Positions;
		TArray<FVector> Normals;
		TArray<uint32> Indices;
		TArray<FVector2D> TexCoords;

		AGX_MeshUtilities::MakeCapsule(
			Positions, Normals, Indices, TexCoords,
			AGX_MeshUtilities::CapsuleConstructionData(
				Radius, Height, NumCircleSegments, NumHeightSegments));

		return MeshGenerationTest_Helper::IsMeshDataValid(Positions, Normals, Indices, TexCoords);
	}
}

bool FAGX_MeshUtilitiesTest::RunTest(const FString&)
{
	bool Result;

	// Box mesh generation tests.
	Result = ::BoxMeshGenerationTest({0.5f, 0.5f, 0.5f});
	TestEqual("Box Mesh Generation Test 0.5f, 0.5f, 0.5f", Result, true);
	Result = ::BoxMeshGenerationTest({1.5f, 2.1f, 0.1f});
	TestEqual("Box Mesh Generation Test 1.5f, 2.1f, 0.1f", Result, true);

	// Sphere mesh generation tests.
	Result = ::SphereMeshGenerationTest(0.5f, 32);
	TestEqual("Sphere Mesh Generation Test 0.5f, 32", Result, true);
	Result = ::SphereMeshGenerationTest(1.5f, 11);
	TestEqual("Sphere Mesh Generation Test 1.5f, 11", Result, true);

	// Cylinder mesh generation tests.
	Result = ::CylindereMeshGenerationTest(0.5f, 0.5f, 32, 16);
	TestEqual("Cylinder Mesh Generation Test 0.5, 0.5, 32, 16", Result, true);
	Result = ::CylindereMeshGenerationTest(1.5f, 2.5f, 13, 11);
	TestEqual("Cylinder Mesh Generation Test 1.5f, 2.5f, 13, 11", Result, true);

	// Capsule mesh generation tests.
	Result = ::CapsuleMeshGenerationTest(0.5f, 0.5f, 32, 16);
	TestEqual("Capsule Mesh Generation Test 0.5, 0.5, 32, 16", Result, true);
	Result = ::CapsuleMeshGenerationTest(1.5f, 2.5f, 13, 11);
	TestEqual("Capsule Mesh Generation Test 1.5f, 2.5f, 13, 11", Result, true);

	return true;
}