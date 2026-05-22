// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

// OpenPLX includes.
#include "openplx/Visuals/Properties/Color.h"
#include "openplx/Visuals/Textures/Texture.h"
#include "openplx/Visuals/Textures/TextureData.h"

// Unreal Engine includes.
#include "Misc/Base64.h"

// Standard library includes.
#include <exception>

FOpenPLXMaterialBarrier::FOpenPLXMaterialBarrier()
	: NativeRef {new FOpenPLXMaterialRef}
{
}

FOpenPLXMaterialBarrier::FOpenPLXMaterialBarrier(std::shared_ptr<FOpenPLXMaterialRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FOpenPLXMaterialBarrier::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Native != nullptr;
}

FOpenPLXMaterialRef* FOpenPLXMaterialBarrier::GetNative()
{
	check(NativeRef);
	return NativeRef.get();
}

const FOpenPLXMaterialRef* FOpenPLXMaterialBarrier::GetNative() const
{
	check(NativeRef);
	return NativeRef.get();
}

FString FOpenPLXMaterialBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

FGuid FOpenPLXMaterialBarrier::GetGuid() const
{
	check(HasNative());
	return FGuid(Convert(NativeRef->Native->getUuid()));
}

bool FOpenPLXMaterialBarrier::HasTrait(const FString& Trait) const
{
	check(HasNative());
	const agx::String TraitString = Convert(Trait);

	// Because hasTrait moves from the passed parameter, we need to ensure the string we pass in is
	// allocated on the AGX side to not crash when it is destroyed.
	agx::String TraitAgxAllocated = agxUtil::copyContainerMemory(TraitString);
	return NativeRef->Native->hasTrait(std::move(TraitAgxAllocated));
}

TOptional<FLinearColor> FOpenPLXMaterialBarrier::GetBaseColor() const
{
	check(HasNative());
	if (!HasTrait(TEXT("Visuals.Materials.SurfaceFeatures.BaseColor")))
		return {};

	auto Color = std::dynamic_pointer_cast<openplx::Visuals::Properties::Color>(
		NativeRef->Native->getDynamic("base_color_tint").asObject());
	if (Color == nullptr)
		return {};

	return FLinearColor(
		static_cast<float>(Color->r()), static_cast<float>(Color->g()),
		static_cast<float>(Color->b()), static_cast<float>(Color->a()));
}

TOptional<FOpenPLXTextureData> FOpenPLXMaterialBarrier::GetBaseColorTextureData() const
{
	check(HasNative());
	if (!HasTrait(TEXT("Visuals.Materials.SurfaceFeatures.BaseColor")))
		return {};

	auto BaseColorMap = std::dynamic_pointer_cast<openplx::Visuals::Textures::Texture>(
		NativeRef->Native->getDynamic("base_color_map").asObject());
	if (BaseColorMap == nullptr)
		return {};

	auto TextureData = BaseColorMap->data();
	if (TextureData == nullptr)
		return {};

	std::string Base64DataAGX = TextureData->data();
	const FString Base64Data = UTF8_TO_TCHAR(Base64DataAGX.c_str());
	agxUtil::freeContainerMemory(Base64DataAGX);

	FOpenPLXTextureData Result;
	Result.Name = Convert(BaseColorMap->getName());
	Result.Guid = FGuid(Convert(BaseColorMap->getUuid()));
	Result.Width = static_cast<int32>(TextureData->width());
	Result.Height = static_cast<int32>(TextureData->height());
	Result.NumChannels = static_cast<int32>(TextureData->format()) + 1;

	if (Result.Width <= 0 || Result.Height <= 0 || Result.NumChannels <= 0 ||
		Result.NumChannels > 4)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX material '%s' has invalid base color texture metadata: texture='%s', "
				 "size=%dx%d, channels=%d."),
			*GetName(), *Result.Name, Result.Width, Result.Height, Result.NumChannels);
		return {};
	}

	if (!FBase64::Decode(Base64Data, Result.Pixels))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX material '%s' failed to decode base color texture '%s'."), *GetName(),
			*Result.Name);
		return {};
	}

	const int64 ExpectedNumBytes =
		static_cast<int64>(Result.Width) * Result.Height * Result.NumChannels;
	if (Result.Pixels.Num() != ExpectedNumBytes)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX material '%s' decoded base color texture '%s' has unexpected byte "
				 "count: got=%d, expected=%lld."),
			*GetName(), *Result.Name, Result.Pixels.Num(), ExpectedNumBytes);
		return {};
	}

	return Result;
}
