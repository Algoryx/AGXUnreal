// Copyright 2024, Algoryx Simulation AB.

#include "Shapes/TrimeshShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "AGX_LogCategory.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Trimesh.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Interfaces/Interface_CollisionDataProvider.h"
#include "Misc/AssertionMacros.h"

namespace
{
	agxCollide::Trimesh* NativeTrimesh(FTrimeshShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Trimesh>();
	}

	const agxCollide::Trimesh* NativeTrimesh(const FTrimeshShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Trimesh>();
	}

	const agxCollide::Trimesh* NativeTrimesh(
		const FTrimeshShapeBarrier* Barrier, const TCHAR* Operation)
	{
		if (!Barrier->HasNative())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Cannot %s Trimesh barrier without a native Trimesh"),
				Operation);
			return nullptr;
		}

		const agxCollide::Trimesh* Native = NativeTrimesh(Barrier);
		if (Native == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot %s Trimesh barrier whose native shape is not a Trimesh."),
				Operation);
		}

		return Native;
	}

	agxCollide::Trimesh* NativeTrimesh(FTrimeshShapeBarrier* Barrier, const TCHAR* Operation)
	{
		const FTrimeshShapeBarrier* ConstBarrier = const_cast<const FTrimeshShapeBarrier*>(Barrier);
		const agxCollide::Trimesh* ConstTrimesh = NativeTrimesh(ConstBarrier, Operation);
		agxCollide::Trimesh* Trimesh = const_cast<agxCollide::Trimesh*>(ConstTrimesh);
		return Trimesh;
	}

	static_assert(
		std::numeric_limits<std::size_t>::max() >= std::numeric_limits<int32>::max(),
		"Expecting std::size_t to hold all positive values that int32 can hold.");

	/// @return True if the given Size can be converted to an int32.
	bool CheckSize(size_t Size)
	{
		return Size <= static_cast<size_t>(std::numeric_limits<int32>::max());
	}

	/**
	 * Check that the given Size can be converted to an int32 and print a warning if too large.
	 *
	 * @param Size The size to check.
	 * @param DataName The name of the data the size is for. Included in the warning message.
	 * @return True if the given size can be converted to an int32.
	 */
	bool CheckSize(size_t Size, const TCHAR* DataName)
	{
		bool Ok = CheckSize(Size);
		if (!Ok)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Native trimesh contains more %s than Unreal can handle. Size is %zu."),
				DataName, Size);
		}
		return Ok;
	}

	/**
	 * Convert an AGX Dynamics render buffer to the corresponding Unreal Engine render buffer.
	 *
	 * @tparam AGXType The element type of the AGX Dynamics source buffer.
	 * @tparam UnrealType The element type of the Unreal Engine target buffer.
	 * @tparam FGetAGXBuffer Function fetching the AGX Dynamics buffer from a Render Data Barrier.
	 * @tparam FConvert Function converting an AGX Dynamics element to the Unreal Engine type.
	 * @param Barrier The Render Data Barrier to fetch the AGX Dynamics buffer from.
	 * @param Operation The operation being performed. Only for error reporting.
	 * @param DataName The name of the buffer being convert. Only for error reporting.
	 * @param GetAGXBuffer Callback for getting the AGX Dynamics buffer from the Render Data.
	 * @param Convert Callback for converting AGX Dynamics elements to the Unreal Engien type.
	 * @return A TArray containing the render buffer in Unreal Engine format.
	 */
	template <typename AGXType, typename UnrealType, typename FGetAGXBuffer, typename FConvert>
	TArray<UnrealType> ConvertCollisionBuffer(
		const FTrimeshShapeBarrier* Barrier, const TCHAR* Operation, const TCHAR* DataName,
		FGetAGXBuffer GetAGXBuffer, FConvert Convert)
	{
		TArray<UnrealType> DataUnreal;
		const agxCollide::Trimesh* Trimesh = NativeTrimesh(Barrier, Operation);
		if (Trimesh == nullptr)
		{
			return DataUnreal;
		}
		const agx::VectorPOD<AGXType>& DataAGX = GetAGXBuffer(Trimesh->getMeshData());
		if (!CheckSize(DataAGX.size(), DataName))
		{
			return DataUnreal;
		}
		DataUnreal.Reserve(static_cast<int32>(DataAGX.size()));
		for (AGXType DatumAGX : DataAGX)
		{
			DataUnreal.Add(Convert(DatumAGX));
		}
		return DataUnreal;
	}
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier()
	: FShapeBarrier()
{
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	/// \todo It seems Shape::is<T>() broken with Unreal Engine 2.24. Now we trip
	/// on this check when restoring AGX Dynamics archives containing
	/// trimeshes. I suspect it's at least partially related to
	///     #define dynamic_cast UE4Casts_Private::DynamicCast
	/// in Casts.h
	// check(NativeRef->NativeShape->is<agxCollide::Trimesh>());
	check(
		NativeRef->NativeShape->getType() == agxCollide::Shape::TRIMESH ||
		NativeRef->NativeShape->getType() == agxCollide::Shape::CONVEX);
}

FTrimeshShapeBarrier::FTrimeshShapeBarrier(FTrimeshShapeBarrier&& Other)
	: FShapeBarrier(std::move(Other))
{
}

FTrimeshShapeBarrier::~FTrimeshShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FTrimeshShapeRef.
}

int32 FTrimeshShapeBarrier::GetNumPositions() const
{
	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch num positions from"));
	if (Trimesh == nullptr)
	{
		return -1;
	}
	if (!CheckSize(Trimesh->getNumVertices(), TEXT("positions")))
	{
		return -1;
	}
	return static_cast<int>(Trimesh->getNumVertices());
}

int32 FTrimeshShapeBarrier::GetNumIndices() const
{
	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch num indices"));
	if (Trimesh == nullptr)
	{
		return -1;
	}
	const size_t NumIndices = Trimesh->getMeshData()->getIndices().size();
	if (NumIndices % 3 != 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Trimesh '%s' has invalid collision data. The number of vertex indices isn't a "
				 "multiple of 3. The last triangle will be skipped."),
			*GetSourceName())
	}
	if (!CheckSize(NumIndices, TEXT("indices")))
	{
		return -1;
	}
	return static_cast<int>(NumIndices);
}

int32 FTrimeshShapeBarrier::GetNumTriangles() const
{
	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch num triangles"));
	if (Trimesh == nullptr)
	{
		return -1;
	}
	const size_t NumTriangles = Trimesh->getNumTriangles();
	if (!CheckSize(NumTriangles, TEXT("triangles")))
	{
		return -1;
	}
	return static_cast<int>(NumTriangles);
}

TArray<FVector> FTrimeshShapeBarrier::GetVertexPositions() const
{
	return ConvertCollisionBuffer<agx::Vec3, FVector>(
		this, TEXT("fetch positions from"), TEXT("positions"),
		[](const agxCollide::MeshData* Mesh) -> auto& { return Mesh->getVertices(); },
		[](const agx::Vec3& Position) { return ConvertDisplacement(Position); });
}

TArray<uint32> FTrimeshShapeBarrier::GetVertexIndices() const
{
	return ConvertCollisionBuffer<agx::UInt32, uint32>(
		this, TEXT("fetch indices from"), TEXT("vertex indices"),
		[](const agxCollide::MeshData* Mesh) -> auto& { return Mesh->getIndices(); },
		[](const agx::UInt32 Index) { return static_cast<uint32>(Index); });
}

TArray<FVector> FTrimeshShapeBarrier::GetTriangleNormals() const
{
	return ConvertCollisionBuffer<agx::Vec3, FVector>(
		this, TEXT("fetch triangle normals from"), TEXT("normals"),
		[](const agxCollide::CollisionMeshData* Mesh) -> auto& { return Mesh->getNormals(); },
		[](const agx::Vec3& Normal) { return ConvertVector(Normal); });
}

FString FTrimeshShapeBarrier::GetSourceName() const
{
	FString SourceName;
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Cannot fetch triangle source name from Trimesh barrier without a native Trimesh"));
		return SourceName;
	}

	const agxCollide::Trimesh* Trimesh = NativeTrimesh(this);
	SourceName = Convert(Trimesh->getSourceName());
	return SourceName;
}

FGuid FTrimeshShapeBarrier::GetMeshDataGuid() const
{
	check(HasNative());
	if (const agxCollide::Trimesh* Trimesh = NativeTrimesh(this, TEXT("fetch mesh data guid")))
	{
		return Convert(Trimesh->getMeshData()->getUuid());
	}
	return FGuid();
}

void FTrimeshShapeBarrier::AllocateNative(
	const TArray<FVector>& Vertices, const TArray<FTriIndices>& TriIndices, bool bClockwise,
	const FString& SourceName)
{
	{
		// Create temporary allocation parameters structure for AllocateNativeShape() to use.

		std::shared_ptr<AllocationParameters> Params =
			std::make_shared<AllocationParameters>(SourceName);
		Params->Vertices = &Vertices;
		Params->TriIndices = &TriIndices;
		Params->bClockwise = bClockwise;

		TemporaryAllocationParameters = Params;

		FShapeBarrier::AllocateNative(); // Will implicitly invoke AllocateNativeShape(). See below.
	}
	// Temporary allocation parameters structure destroyed by smart pointer.
}

void FTrimeshShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());

	// Retrieve the temporary allocation parameters.

	std::shared_ptr<AllocationParameters> Params = TemporaryAllocationParameters.lock();
	check(Params != nullptr);

	// Transfer to native buffers.
	const agx::Vec3Vector NativeVertices = ConvertVertices(*Params->Vertices);
	const agx::UInt32Vector NativeIndices = ConvertIndices(*Params->TriIndices);

	agxCollide::Trimesh::TrimeshOptionsFlags OptionsMask =
		Params->bClockwise ? agxCollide::Trimesh::TrimeshOptionsFlags::CLOCKWISE_ORIENTATION
						   : static_cast<agxCollide::Trimesh::TrimeshOptionsFlags>(0);

	// Create the native object.

	NativeRef->NativeShape = new agxCollide::Trimesh(
		&NativeVertices, &NativeIndices, Convert(Params->SourceName).c_str(), OptionsMask);
}

void FTrimeshShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
