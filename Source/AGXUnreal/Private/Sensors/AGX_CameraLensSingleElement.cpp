// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraLensSingleElement.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/CameraLensSingleElementBarrier.h"

void UAGX_CameraLensSingleElement::SetFocalLength(double InFocalLength)
{
	UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? this : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	FCameraLensSingleElementBarrier* SingleElementNativeBarrier =
		SingleElementInstance != nullptr ? SingleElementInstance->GetNativeAsSingleElement()
										 : nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		FocalLength, InFocalLength, SetFocalLength, SingleElementInstance, HasNative,
		SingleElementNativeBarrier, ->);
}

double UAGX_CameraLensSingleElement::GetFocalLength() const
{
	const UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? this : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	const FCameraLensSingleElementBarrier* SingleElementNativeBarrier =
		SingleElementInstance != nullptr ? SingleElementInstance->GetNativeAsSingleElement()
										 : nullptr;
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		FocalLength, GetFocalLength, SingleElementInstance, HasNative, SingleElementNativeBarrier,
		->);
}

void UAGX_CameraLensSingleElement::CopyProperties(const UAGX_CameraLensBase& Source)
{
	Super::CopyProperties(Source);

	const UAGX_CameraLensSingleElement* SourceSingleElement =
		Cast<UAGX_CameraLensSingleElement>(&Source);
	if (SourceSingleElement == nullptr)
		return;

	FocalLength = SourceSingleElement->FocalLength;
}

void UAGX_CameraLensSingleElement::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_CameraLensSingleElement::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_CameraLensSingleElement::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(FocalLength),
		[](ThisClass* This) { This->SetFocalLength(This->FocalLength); });
}
#endif // WITH_EDITOR

void UAGX_CameraLensSingleElement::UpdateNativeProperties()
{
	if (!IsInstance())
		return;

	FCameraLensSingleElementBarrier* Native = GetNativeAsSingleElement();
	if (Native == nullptr)
		return;

	Native->SetFocalLength(FocalLength);
}

FCameraLensSingleElementBarrier* UAGX_CameraLensSingleElement::GetNativeAsSingleElement()
{
	return const_cast<FCameraLensSingleElementBarrier*>(
		const_cast<const ThisClass*>(this)->GetNativeAsSingleElement());
}

const FCameraLensSingleElementBarrier*
UAGX_CameraLensSingleElement::GetNativeAsSingleElement() const
{
	const UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? this : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	if (SingleElementInstance == nullptr)
		return nullptr;

	const FCameraLensBarrier* Native = SingleElementInstance->UAGX_CameraLensBase::GetNative();
	if (Native == nullptr)
		return nullptr;

	AGX_CHECK(FCameraLensSingleElementBarrier::IsSingleElement(*Native));
	return static_cast<const FCameraLensSingleElementBarrier*>(Native);
}

void UAGX_CameraLensSingleElement::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_CameraLensSingleElement '%s' whose "
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

	NativeBarrier = MakeUnique<FCameraLensSingleElementBarrier>();
	NativeBarrier->AllocateNative();
	check(HasNative());
	UpdateNativeProperties();
}
