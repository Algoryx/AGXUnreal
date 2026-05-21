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
