// Copyright 2026, Algoryx Simulation AB.

#include "Shapes/RenderDataBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/Shapes/RenderDataRef.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Shapes/RenderMaterial.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

FRenderDataBarrier::FRenderDataBarrier()
	: NativeRef(new FRenderDataRef())
{
}

FRenderDataBarrier::FRenderDataBarrier(FRenderDataBarrier&& Other)
	: NativeRef(std::move(Other.NativeRef))
{
}

FRenderDataBarrier::FRenderDataBarrier(std::unique_ptr<FRenderDataRef>&& InNativeRef)
	: NativeRef(std::move(InNativeRef))
{
}

FRenderDataBarrier::~FRenderDataBarrier()
{
}

namespace RenderDataBarrier_helpers
{
	static_assert(
		std::numeric_limits<std::size_t>::max() >= std::numeric_limits<int32>::max(),
		"Expecting std::size_t to hold all positive values that int32 can hold.");

	/// @return True if the given Size fit in an int32.
	bool CheckSize(size_t Size)
	{
		const size_t MaxAllowed = static_cast<size_t>(std::numeric_limits<int32>::max());
		return Size <= MaxAllowed;
	}

	/**
	 * Like CheckSize(size_t) but also prints a warning when the size is too large.
	 *
	 * @param Size The size to check.
	 * @param DataName The name of the data the size belong to. Only used for logging.
	 * @param Guid The GUID of the Render Data that the data is part of. Only used for logging.
	 * @return True if the given Size fit in an int32.
	 */
	bool CheckSize(size_t Size, const TCHAR* DataName, const FGuid& Guid)
	{
		const bool Ok = CheckSize(Size);
		if (!Ok)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Native Render Data %s contains more %s than Unreal can handle. Size is %zu."),
				*Guid.ToString(), DataName, Size);
		}
		return Ok;
	}

	/**
	 * Convert the given Size to an int32, clamping it to the largest possible int32 if too large.
	 *
	 * @param Size The Size to convert/clamp.
	 * @param DataName The name of the data the size belong to. Only used for logging.
	 * @param Guid The GUID of the Render Data that the data is part of. Only used for logging.
	 * @return The Size as an int32, possibly clamped.
	 */
	int32 CastWithSaturate(size_t Size, const TCHAR* DataName, const FGuid& Guid)
	{
		if (CheckSize(Size, DataName, Guid))
		{
			return static_cast<int32>(Size);
		}
		else
		{
			return std::numeric_limits<int32>::max();
		}
	}

	/**
	 * Convert an AGX Dynamics render buffer to the corresponding Unreal Engine render buffer.
	 *
	 * @tparam AGXType The element type of the AGX Dynamics source buffer.
	 * @tparam UnrealType The element type of the Unreal Engine target buffer.
	 * @tparam FGetAGXBuffer Function fetching the AGX Dynamics buffer from a Render Data Barrier.
	 * @tparam FConvert Function converting an AGX Dynamics element to the Unreal Engine type.
	 * @param Barrier The Render Data Barrier to fetch the AGX Dynamics buffer from.
	 * @param DataName The name of the buffer being converted. Only for error reporting.
	 * @param GetAGXBuffer Callback for getting the AGX Dynamics buffer from the Render Data.
	 * @param Convert Callback for converting AGX Dynamics elements to the Unreal Engine type.
	 * @return A TArray containing the render buffer in Unreal Engine format.
	 */
	template <typename AGXType, typename UnrealType, typename FGetAGXBuffer, typename FConvert>
	TArray<UnrealType> ConvertRenderBuffer(
		const FRenderDataBarrier& Barrier, const TCHAR* DataName, FGetAGXBuffer GetAGXBuffer,
		FConvert Convert)
	{
		TArray<UnrealType> DataUnreal;
		const agxCollide::RenderData* RenderData = Barrier.GetNative()->Native;
		if (RenderData == nullptr)
		{
			return DataUnreal;
		}
		const agx::VectorPOD<AGXType>& DataAGX = GetAGXBuffer(RenderData);
		const size_t SizeAGX = DataAGX.size();
		const int32 Size = CastWithSaturate(SizeAGX, DataName, Barrier.GetGuid());
		DataUnreal.Reserve(Size);
		for (const AGXType& DatumAGX : DataAGX)
		{
			DataUnreal.Add(Convert(DatumAGX));
		}
		return DataUnreal;
	}
}

bool FRenderDataBarrier::GetShouldRender() const
{
	check(HasNative());
	return NativeRef->Native->getShouldRender();
}

bool FRenderDataBarrier::HasMesh() const
{
	check(HasNative());
	return GetNumTriangles() != 0;
}

int32 FRenderDataBarrier::GetNumTriangles() const
{
	using namespace RenderDataBarrier_helpers;
	check(HasNative());
	const size_t NumIndicesAGX = NativeRef->Native->getIndexArray().size();
	if (NumIndicesAGX % 3 != 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Render Data with GUID %s has number of indices not divisible by three. It may "
				 "render incorrectly."),
			*GetGuid().ToString());
	}
	const int32 NumIndices = CastWithSaturate(NumIndicesAGX, TEXT("indices"), GetGuid());
	return NumIndices / 3;
}

int32 FRenderDataBarrier::GetNumIndices() const
{
	using namespace RenderDataBarrier_helpers;
	check(HasNative());
	const size_t NumIndicesAGX = NativeRef->Native->getIndexArray().size();
	return CastWithSaturate(NumIndicesAGX, TEXT("indices"), GetGuid());
}

int32 FRenderDataBarrier::GetNumPositions() const
{
	const size_t Size = NativeRef->Native->getVertexArray().size();
	return Convert(Size, TEXT("Render Data positions array."));
}

int32 FRenderDataBarrier::GetNumNormals() const
{
	const size_t Size = NativeRef->Native->getNormalArray().size();
	return Convert(Size, TEXT("Render Data normals array."));
}

int32 FRenderDataBarrier::GetNumTextureCoordinates() const
{
	const size_t Size = NativeRef->Native->getTexCoordArray().size();
	return Convert(Size, TEXT("Render Data texture coordinates array."));
}

TArray<FVector> FRenderDataBarrier::GetPositions() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::Vec3, FVector>(
		*this, TEXT("positions"),
		[](const agxCollide::RenderData* Data) -> auto& { return Data->getVertexArray(); },
		[](const agx::Vec3& Vec3) { return ConvertDisplacement(Vec3); });
}

TArray<uint32> FRenderDataBarrier::GetIndices() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::UInt32, uint32>(
		*this, TEXT("indices"),
		[](const agxCollide::RenderData* Data) -> auto& { return Data->getIndexArray(); },
		[](const agx::UInt32 Index) { return static_cast<uint32>(Index); });
}

TArray<FVector> FRenderDataBarrier::GetNormals() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::Vec3, FVector>(
		*this, TEXT("normals"),
		[](const agxCollide::RenderData* Data) -> auto& { return Data->getNormalArray(); },
		[](const agx::Vec3& Vec3) { return ConvertVector(Vec3); });
}

TArray<FVector2D> FRenderDataBarrier::GetTextureCoordinates() const
{
	return RenderDataBarrier_helpers::ConvertRenderBuffer<agx::Vec2, FVector2D>(
		*this, TEXT("texture coordinates"), [](const agxCollide::RenderData* Data) -> auto&
		{ return Data->getTexCoordArray(); }, [](const agx::Vec2& Vec2) { return Convert(Vec2); });
}

bool FRenderDataBarrier::HasMaterial() const
{
	check(HasNative());
	return NativeRef->Native->getRenderMaterial() != nullptr;
}

FAGX_RenderMaterial FRenderDataBarrier::GetMaterial() const
{
	check(HasNative());
	check(HasMaterial());

	FAGX_RenderMaterial RenderMaterial;

	const agxCollide::RenderData* RenderDataAGX = NativeRef->Native.get();
	const agxCollide::RenderMaterial* RenderMaterialAGX = RenderDataAGX->getRenderMaterial();

	RenderMaterial.Guid = Convert(RenderMaterialAGX->getUuid());
	{
		agx::String NameAGX = RenderMaterialAGX->getName();
		RenderMaterial.Name = NameAGX.empty() ? NAME_None : FName(*Convert(NameAGX));

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(NameAGX);
	}

	if ((RenderMaterial.bHasDiffuse = RenderMaterialAGX->hasDiffuseColor()) == true)
	{
		const agx::Vec4f DiffuseAGX = RenderMaterialAGX->getDiffuseColor();
		RenderMaterial.Diffuse = Convert(DiffuseAGX);
	}
	if ((RenderMaterial.bHasAmbient = RenderMaterialAGX->hasAmbientColor()) == true)
	{
		const agx::Vec4f AmbientAGX = RenderMaterialAGX->getAmbientColor();
		RenderMaterial.Ambient = Convert(AmbientAGX);
	}
	if ((RenderMaterial.bHasEmissive = RenderMaterialAGX->hasEmissiveColor()) == true)
	{
		const agx::Vec4f EmissiveAGX = RenderMaterialAGX->getEmissiveColor();
		RenderMaterial.Emissive = Convert(EmissiveAGX);
	}
	if ((RenderMaterial.bHasShininess = RenderMaterialAGX->hasShininess()) == true)
	{
		RenderMaterial.Shininess = RenderMaterialAGX->getShininess();
	}

	return RenderMaterial;
}

FGuid FRenderDataBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

bool FRenderDataBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}
FRenderDataRef* FRenderDataBarrier::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

const FRenderDataRef* FRenderDataBarrier::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

void FRenderDataBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef = nullptr;
}
