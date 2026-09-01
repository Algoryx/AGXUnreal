// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraOutputColor.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraOutputColorBarrier.h"

namespace AGX_CameraOutputColor_helpers
{
	uint8 ClampChannelCount(uint8 InChannelCount)
	{
		return FMath::Clamp<uint8>(InChannelCount, 1, 4);
	}

	FCameraOutputColorBarrier* GetNativeAsCameraOutputColor(FAGX_CameraOutputColor& Output)
	{
		return static_cast<FCameraOutputColorBarrier*>(Output.GetNative());
	}

	const FCameraOutputColorBarrier* GetNativeAsCameraOutputColor(
		const FAGX_CameraOutputColor& Output)
	{
		return static_cast<const FCameraOutputColorBarrier*>(Output.GetNative());
	}
}

TUniquePtr<FCameraOutputBarrier> FAGX_CameraOutputColor::CreateNativeBarrier() const
{
	using namespace AGX_CameraOutputColor_helpers;

	TUniquePtr<FCameraOutputBarrier> Native = MakeUnique<FCameraOutputColorBarrier>();
	Native->AllocateNative();
	ApplyBasePropertiesToNative(*Native);

	FCameraOutputColorBarrier* ColorNative = static_cast<FCameraOutputColorBarrier*>(Native.Get());
	ColorNative->SetChannelType(ChannelType);
	ColorNative->SetGamma(FMath::Max(0.0, Gamma));
	ColorNative->SetColorMappingMatrix(ColorMappingMatrix);
	ColorNative->SetChannelCount(ClampChannelCount(ChannelCount));
	return Native;
}

void FAGX_CameraOutputColor::SetChannelType(EAGX_CameraOutputChannelType InChannelType)
{
	using namespace AGX_CameraOutputColor_helpers;

	ChannelType = InChannelType;
	if (HasNative())
		GetNativeAsCameraOutputColor(*this)->SetChannelType(ChannelType);
}

EAGX_CameraOutputChannelType FAGX_CameraOutputColor::GetChannelType() const
{
	using namespace AGX_CameraOutputColor_helpers;

	if (HasNative())
		return GetNativeAsCameraOutputColor(*this)->GetChannelType();

	return ChannelType;
}

void FAGX_CameraOutputColor::SetGamma(double InGamma)
{
	using namespace AGX_CameraOutputColor_helpers;

	Gamma = FMath::Max(0.0, InGamma);
	if (HasNative())
		GetNativeAsCameraOutputColor(*this)->SetGamma(Gamma);
}

double FAGX_CameraOutputColor::GetGamma() const
{
	using namespace AGX_CameraOutputColor_helpers;

	if (HasNative())
		return GetNativeAsCameraOutputColor(*this)->GetGamma();

	return Gamma;
}

void FAGX_CameraOutputColor::SetColorMappingMatrix(FAGX_ColorMappingMatrix InColorMappingMatrix)
{
	using namespace AGX_CameraOutputColor_helpers;

	ColorMappingMatrix = InColorMappingMatrix;
	if (HasNative())
		GetNativeAsCameraOutputColor(*this)->SetColorMappingMatrix(ColorMappingMatrix);
}

FAGX_ColorMappingMatrix FAGX_CameraOutputColor::GetColorMappingMatrix() const
{
	using namespace AGX_CameraOutputColor_helpers;

	if (HasNative())
		return GetNativeAsCameraOutputColor(*this)->GetColorMappingMatrix();

	return ColorMappingMatrix;
}

void FAGX_CameraOutputColor::SetChannelCount(uint8 InChannelCount)
{
	using namespace AGX_CameraOutputColor_helpers;

	ChannelCount = ClampChannelCount(InChannelCount);
	if (HasNative())
		GetNativeAsCameraOutputColor(*this)->SetChannelCount(ChannelCount);
}

uint8 FAGX_CameraOutputColor::GetChannelCount() const
{
	using namespace AGX_CameraOutputColor_helpers;

	if (HasNative())
		return GetNativeAsCameraOutputColor(*this)->GetChannelCount();

	return ChannelCount;
}

FAGX_CameraOutputColor& FAGX_CameraOutputColor::operator=(const FAGX_CameraOutputColor& Other)
{
	FAGX_CameraOutputBase::operator=(Other);
	ChannelType = Other.ChannelType;
	Gamma = Other.Gamma;
	ColorMappingMatrix = Other.ColorMappingMatrix;
	ChannelCount = Other.ChannelCount;
	return *this;
}

bool FAGX_CameraOutputColor::operator==(const FAGX_CameraOutputColor& Other) const
{
	return FAGX_CameraOutputBase::operator==(Other);
}

void FAGX_CameraOutputColor::GetData(TArray<FColor>& OutData)
{
	using namespace AGX_CameraOutputColor_helpers;

	if (HasNative())
		GetNativeAsCameraOutputColor(*this)->GetData(OutData);
}
