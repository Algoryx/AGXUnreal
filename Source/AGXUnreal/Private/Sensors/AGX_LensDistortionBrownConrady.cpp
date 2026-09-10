// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LensDistortionBrownConrady.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/LensDistortionBrownConradyBarrier.h"

void UAGX_LensDistortionBrownConrady::SetK1(double InK1)
{
	UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? this : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		BrownConradyInstance != nullptr ? BrownConradyInstance->GetNativeAsBrownConrady()
										: nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		K1, InK1, SetK1, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

double UAGX_LensDistortionBrownConrady::GetK1() const
{
	const UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? nullptr : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	const UAGX_LensDistortionBrownConrady* NativeOwner =
		BrownConradyInstance != nullptr ? BrownConradyInstance : this;
	const FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		NativeOwner->GetNativeAsBrownConrady();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		K1, GetK1, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

void UAGX_LensDistortionBrownConrady::SetK2(double InK2)
{
	UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? this : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		BrownConradyInstance != nullptr ? BrownConradyInstance->GetNativeAsBrownConrady()
										: nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		K2, InK2, SetK2, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

double UAGX_LensDistortionBrownConrady::GetK2() const
{
	const UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? nullptr : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	const UAGX_LensDistortionBrownConrady* NativeOwner =
		BrownConradyInstance != nullptr ? BrownConradyInstance : this;
	const FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		NativeOwner->GetNativeAsBrownConrady();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		K2, GetK2, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

void UAGX_LensDistortionBrownConrady::SetK3(double InK3)
{
	UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? this : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		BrownConradyInstance != nullptr ? BrownConradyInstance->GetNativeAsBrownConrady()
										: nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		K3, InK3, SetK3, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

double UAGX_LensDistortionBrownConrady::GetK3() const
{
	const UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? nullptr : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	const UAGX_LensDistortionBrownConrady* NativeOwner =
		BrownConradyInstance != nullptr ? BrownConradyInstance : this;
	const FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		NativeOwner->GetNativeAsBrownConrady();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		K3, GetK3, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

void UAGX_LensDistortionBrownConrady::SetP1(double InP1)
{
	UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? this : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		BrownConradyInstance != nullptr ? BrownConradyInstance->GetNativeAsBrownConrady()
										: nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		P1, InP1, SetP1, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

double UAGX_LensDistortionBrownConrady::GetP1() const
{
	const UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? nullptr : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	const UAGX_LensDistortionBrownConrady* NativeOwner =
		BrownConradyInstance != nullptr ? BrownConradyInstance : this;
	const FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		NativeOwner->GetNativeAsBrownConrady();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		P1, GetP1, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

void UAGX_LensDistortionBrownConrady::SetP2(double InP2)
{
	UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? this : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		BrownConradyInstance != nullptr ? BrownConradyInstance->GetNativeAsBrownConrady()
										: nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		P2, InP2, SetP2, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

double UAGX_LensDistortionBrownConrady::GetP2() const
{
	const UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? nullptr : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	const UAGX_LensDistortionBrownConrady* NativeOwner =
		BrownConradyInstance != nullptr ? BrownConradyInstance : this;
	const FLensDistortionBrownConradyBarrier* BrownConradyNativeBarrier =
		NativeOwner->GetNativeAsBrownConrady();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		P2, GetP2, BrownConradyInstance, HasNative, BrownConradyNativeBarrier, ->);
}

void UAGX_LensDistortionBrownConrady::CopyProperties(
	const UAGX_LensDistortionBase& Source)
{
	Super::CopyProperties(Source);

	const UAGX_LensDistortionBrownConrady* SourceBrownConrady =
		Cast<UAGX_LensDistortionBrownConrady>(&Source);
	if (SourceBrownConrady == nullptr)
		return;

	K1 = SourceBrownConrady->K1;
	K2 = SourceBrownConrady->K2;
	K3 = SourceBrownConrady->K3;
	P1 = SourceBrownConrady->P1;
	P2 = SourceBrownConrady->P2;
}

void UAGX_LensDistortionBrownConrady::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_LensDistortionBrownConrady::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_LensDistortionBrownConrady::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(K1), [](ThisClass* This) { This->SetK1(This->K1); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(K2), [](ThisClass* This) { This->SetK2(This->K2); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(K3), [](ThisClass* This) { This->SetK3(This->K3); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(P1), [](ThisClass* This) { This->SetP1(This->P1); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(P2), [](ThisClass* This) { This->SetP2(This->P2); });
}
#endif // WITH_EDITOR

void UAGX_LensDistortionBrownConrady::UpdateNativeProperties()
{
	if (!IsInstance())
		return;

	FLensDistortionBrownConradyBarrier* Native = GetNativeAsBrownConrady();
	if (Native == nullptr)
		return;

	Native->SetK1(K1);
	Native->SetK2(K2);
	Native->SetK3(K3);
	Native->SetP1(P1);
	Native->SetP2(P2);
}

FLensDistortionBrownConradyBarrier*
UAGX_LensDistortionBrownConrady::GetNativeAsBrownConrady()
{
	return const_cast<FLensDistortionBrownConradyBarrier*>(
		const_cast<const ThisClass*>(this)->GetNativeAsBrownConrady());
}

const FLensDistortionBrownConradyBarrier*
UAGX_LensDistortionBrownConrady::GetNativeAsBrownConrady() const
{
	const UAGX_LensDistortionBrownConrady* BrownConradyInstance =
		IsInstance() ? this : Cast<UAGX_LensDistortionBrownConrady>(Instance.Get());
	if (BrownConradyInstance == nullptr)
		return nullptr;

	const FLensDistortionBarrier* Native =
		BrownConradyInstance->UAGX_LensDistortionBase::GetNative();
	if (Native == nullptr)
		return nullptr;

	AGX_CHECK(FLensDistortionBrownConradyBarrier::IsBrownConrady(*Native));
	return static_cast<const FLensDistortionBrownConradyBarrier*>(Native);
}

void UAGX_LensDistortionBrownConrady::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_LensDistortionBrownConrady '%s' whose "
					 "instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior to "
					 "calling this function."),
				*GetName());
			return;
		}
		Instance->GetOrCreateNative();
		return;
	}

	AGX_CHECK(IsInstance());
	if (NativeBarrier != nullptr && NativeBarrier->HasNative())
	{
		NativeBarrier->ReleaseNative();
	}

	NativeBarrier = MakeUnique<FLensDistortionBrownConradyBarrier>();
	NativeBarrier->AllocateNative();
	check(HasNative());
	UpdateNativeProperties();
}
