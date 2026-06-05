// Copyright 2026, Algoryx Simulation AB.

#include "Model/WindAndWaterControllerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "Model/DynamicWaterBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxModel/WindAndWaterController.h>
#include "EndAGXIncludes.h"

FWindAndWaterControllerBarrier::FWindAndWaterControllerBarrier()
	: NativeRef{new FWindAndWaterControllerRef}
{
}

FWindAndWaterControllerBarrier::FWindAndWaterControllerBarrier(std::unique_ptr<FWindAndWaterControllerRef> Native)
	: NativeRef{std::move(Native)}
{
	check(NativeRef->Native->is<agxModel::WindAndWaterController>());
}

FWindAndWaterControllerBarrier::FWindAndWaterControllerBarrier(FWindAndWaterControllerBarrier&& Other) noexcept
	: NativeRef{std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FWindAndWaterControllerRef);
}

FWindAndWaterControllerBarrier::~FWindAndWaterControllerBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FWindAndWaterControllerRef.
}

bool FWindAndWaterControllerBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FWindAndWaterControllerBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxModel::WindAndWaterController();
}

FWindAndWaterControllerRef* FWindAndWaterControllerBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FWindAndWaterControllerRef* FWindAndWaterControllerBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FWindAndWaterControllerBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FWindAndWaterControllerBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		this->ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		return;
	}

	NativeRef->Native = reinterpret_cast<agxModel::WindAndWaterController*>(NativeAddress);
}

void FWindAndWaterControllerBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

FWindAndWaterParametersBarrier FWindAndWaterControllerBarrier::GetOrCreateHydrodynamicsParameters(FShapeBarrier* Shape) const
{
	check(HasNative());
	return AGXBarrierFactories::CreateWindAndWaterParametersBarrier(NativeRef->Native->getOrCreateHydrodynamicsParameters(Shape->GetNative()->NativeShape));
}

FWindAndWaterParametersBarrier FWindAndWaterControllerBarrier::GetOrCreateHydrodynamicsParameters(FWireBarrier* Wire) const
{
	check(HasNative());
	return AGXBarrierFactories::CreateWindAndWaterParametersBarrier(NativeRef->Native->getOrCreateAerodynamicsParameters(Wire->GetNative()->Native));
}

FWindAndWaterParametersBarrier FWindAndWaterControllerBarrier::GetOrCreateAerodynamicsParameters(FShapeBarrier* Shape) const
{
	check(HasNative());
	return AGXBarrierFactories::CreateWindAndWaterParametersBarrier(NativeRef->Native->getOrCreateAerodynamicsParameters(Shape->GetNative()->NativeShape));
}

FWindAndWaterParametersBarrier FWindAndWaterControllerBarrier::GetOrCreateAerodynamicsParameters(FWireBarrier* Wire) const
{
	check(HasNative());
	return AGXBarrierFactories::CreateWindAndWaterParametersBarrier(NativeRef->Native->getOrCreateAerodynamicsParameters(Wire->GetNative()->Native));
}

bool FWindAndWaterControllerBarrier::SetEnableAerodynamics(FShapeBarrier* Shape, bool bEnabled)
{
	check(HasNative());
	return NativeRef->Native->setEnableAerodynamics(Shape->GetNative()->NativeGeometry, bEnabled);
}

bool FWindAndWaterControllerBarrier::SetEnableAerodynamics(FWireBarrier* Wire, bool bEnabled)
{
	check(HasNative());
	return NativeRef->Native->setEnableAerodynamics(Wire->GetNative()->Native, bEnabled);
}

bool FWindAndWaterControllerBarrier::AddWater(const FShapeBarrier* Shape)
{
	check(HasNative());
	return GetNative()->Native->addWater(Shape->GetNative()->NativeGeometry);
}

bool FWindAndWaterControllerBarrier::SetWaterWrapper(FShapeBarrier* Shape, FDynamicWaterBarrier* DynamicWater)
{
	check(HasNative());
	return GetNative()->Native->setWaterWrapper(Shape->GetNative()->NativeGeometry, DynamicWater->GetNative()->NativeWaterWrapper);
}

bool FWindAndWaterControllerBarrier::SetWaterFlowGenerator(FShapeBarrier* Shape, FDynamicWaterBarrier* DynamicWater)
{
	check(HasNative());
	return GetNative()->Native->setWaterFlowGenerator(Shape->GetNative()->NativeGeometry, DynamicWater->GetNative()->NativeWaterFlowGenerator);
}