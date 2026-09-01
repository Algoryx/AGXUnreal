// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraOutputBase.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraSensorComponent.h"
#include "Sensors/CameraOutputBarrier.h"

namespace AGX_CameraOutputBase_helpers
{
	FIntPoint ClampResolution(FIntPoint InResolution)
	{
		return {FMath::Max(1, InResolution.X), FMath::Max(1, InResolution.Y)};
	}

	double ClampFrameRate(double InFrameRate)
	{
		return FMath::Max(0.0, InFrameRate);
	}
}

FAGX_CameraOutputBase::FAGX_CameraOutputBase(const FAGX_CameraOutputBase& Other)
	: Resolution(Other.Resolution)
	, FrameRate(Other.FrameRate)
	, bConstantCapture(Other.bConstantCapture)
{
	// This is needed to be able to declare e.g. TArray's containing this struct.
	// It is assumed not to be called during play, therefore no native is copied here.
}

FAGX_CameraOutputBase::~FAGX_CameraOutputBase() = default;

void FAGX_CameraOutputBase::SetResolution(FIntPoint InResolution)
{
	Resolution = AGX_CameraOutputBase_helpers::ClampResolution(InResolution);
	if (HasNative())
		GetNative()->SetResolution(Resolution);
}

FIntPoint FAGX_CameraOutputBase::GetResolution() const
{
	if (HasNative())
		return GetNative()->GetResolution();

	return Resolution;
}

void FAGX_CameraOutputBase::SetFrameRate(double InFrameRate)
{
	FrameRate = AGX_CameraOutputBase_helpers::ClampFrameRate(InFrameRate);
	if (HasNative() && bConstantCapture)
		GetNative()->SetConstantCapture(FrameRate);
}

double FAGX_CameraOutputBase::GetFrameRate() const
{
	if (HasNative())
		return GetNative()->GetFrameRate();

	return FrameRate;
}

void FAGX_CameraOutputBase::SetConstantCapture(bool bInConstantCapture)
{
	bConstantCapture = bInConstantCapture;
	if (!HasNative())
		return;

	if (bConstantCapture)
		GetNative()->SetConstantCapture(FrameRate);
	else
		GetNative()->SetManualCapture();
}

bool FAGX_CameraOutputBase::GetConstantCapture() const
{
	if (HasNative())
		return GetNative()->GetConstantCapture();

	return bConstantCapture;
}

bool FAGX_CameraOutputBase::HasNative() const
{
	return NativeBarrier != nullptr && NativeBarrier->HasNative();
}

FCameraOutputBarrier* FAGX_CameraOutputBase::GetOrCreateNative()
{
	if (NativeBarrier == nullptr)
		NativeBarrier = CreateNativeBarrier();

	if (NativeBarrier == nullptr)
		return nullptr;

	if (!NativeBarrier->HasNative())
		NativeBarrier->AllocateNative();

	return GetNative();
}

const FCameraOutputBarrier* FAGX_CameraOutputBase::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return NativeBarrier.Get();
}

FCameraOutputBarrier* FAGX_CameraOutputBase::GetNative()
{
	if (!HasNative())
		return nullptr;

	return NativeBarrier.Get();
}

bool FAGX_CameraOutputBase::AddTo(UAGX_CameraSensorComponent* Camera)
{
	if (Camera == nullptr)
		return false;

	return Camera->AddOutput(*this);
}

FAGX_CameraOutputBase& FAGX_CameraOutputBase::operator=(const FAGX_CameraOutputBase& Other)
{
	// This operator is needed to be able to declare e.g. TArray's containing this struct.
	// It is assumed not to be called during play, therefore no native is copied here.
	return *this;
}

bool FAGX_CameraOutputBase::operator==(const FAGX_CameraOutputBase& Other) const
{
	return HasNative() && Other.HasNative() && GetNative() == Other.GetNative();
}

void FAGX_CameraOutputBase::ApplyBasePropertiesToNative(FCameraOutputBarrier& Native) const
{
	check(Native.HasNative());
	Native.SetResolution(AGX_CameraOutputBase_helpers::ClampResolution(Resolution));

	if (bConstantCapture)
		Native.SetConstantCapture(AGX_CameraOutputBase_helpers::ClampFrameRate(FrameRate));
	else
		Native.SetManualCapture();
}
