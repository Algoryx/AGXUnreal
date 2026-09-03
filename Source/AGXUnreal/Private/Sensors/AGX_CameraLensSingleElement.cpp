// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraLensSingleElement.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/CameraLensSingleElementBarrier.h"

namespace AGX_CameraLensSingleElement_helpers
{
	UAGX_CameraLensSingleElement* GetSingleElementInstance(UAGX_CameraLensSingleElement& Lens)
	{
		return Lens.IsInstance()
			       ? &Lens
			       : Cast<UAGX_CameraLensSingleElement>(Lens.GetInstance());
	}

	template <typename SetPropertyFunc, typename UpdateNativeFunc>
	void SetSingleElementProperty(
		UAGX_CameraLensSingleElement& Lens, SetPropertyFunc SetProperty,
		UpdateNativeFunc UpdateNative)
	{
		UAGX_CameraLensSingleElement* SingleElementInstance = GetSingleElementInstance(Lens);
		UAGX_CameraLensSingleElement* Target =
			SingleElementInstance != nullptr ? SingleElementInstance : &Lens;
		const bool bModifyAsset = !Target->IsInstance();
		if (bModifyAsset)
			AGX_WithEditorWrappers::Modify(*Target);

		SetProperty(*Target);
		FCameraLensSingleElementBarrier* Native = Target->GetNativeAsSingleElement();
		if (Native != nullptr)
			UpdateNative(*Target, *Native);

		if (bModifyAsset)
			AGX_WithEditorWrappers::MarkAssetDirty(*Target);
	}
}

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
		IsInstance() ? nullptr : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	const UAGX_CameraLensSingleElement* NativeOwner =
		SingleElementInstance != nullptr ? SingleElementInstance : this;
	const FCameraLensSingleElementBarrier* SingleElementNativeBarrier =
		NativeOwner->GetNativeAsSingleElement();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		FocalLength, GetFocalLength, SingleElementInstance, HasNative, SingleElementNativeBarrier,
		->);
}

void UAGX_CameraLensSingleElement::SetFStop(double InFStop)
{
	UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? this : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	FCameraLensSingleElementBarrier* SingleElementNativeBarrier =
		SingleElementInstance != nullptr ? SingleElementInstance->GetNativeAsSingleElement()
										 : nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		FStop, InFStop, SetFStop, SingleElementInstance, HasNative, SingleElementNativeBarrier,
		->);
}

double UAGX_CameraLensSingleElement::GetFStop() const
{
	const UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	const UAGX_CameraLensSingleElement* NativeOwner =
		SingleElementInstance != nullptr ? SingleElementInstance : this;
	const FCameraLensSingleElementBarrier* SingleElementNativeBarrier =
		NativeOwner->GetNativeAsSingleElement();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		FStop, GetFStop, SingleElementInstance, HasNative, SingleElementNativeBarrier, ->);
}

void UAGX_CameraLensSingleElement::SetUseAutofocus(bool bInUseAutofocus)
{
	using namespace AGX_CameraLensSingleElement_helpers;

	SetSingleElementProperty(
		*this,
		[bInUseAutofocus](ThisClass& Lens) { Lens.bUseAutofocus = bInUseAutofocus; },
		[](ThisClass& Lens, FCameraLensSingleElementBarrier& Native)
		{
			if (Lens.bUseAutofocus)
				Native.SetAutofocus(Lens.MinimumFocusDistance);
			else
				Native.SetFocusDistance(Lens.FocusDistance);
		});
}

bool UAGX_CameraLensSingleElement::GetUseAutofocus() const
{
	const UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	const UAGX_CameraLensSingleElement* NativeOwner =
		SingleElementInstance != nullptr ? SingleElementInstance : this;
	const FCameraLensSingleElementBarrier* SingleElementNativeBarrier =
		NativeOwner->GetNativeAsSingleElement();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		bUseAutofocus, GetUseAutofocus, SingleElementInstance, HasNative,
		SingleElementNativeBarrier, ->);
}

void UAGX_CameraLensSingleElement::SetMinimumFocusDistance(double InMinimumFocusDistance)
{
	using namespace AGX_CameraLensSingleElement_helpers;

	SetSingleElementProperty(
		*this,
		[InMinimumFocusDistance](ThisClass& Lens)
		{ Lens.MinimumFocusDistance = InMinimumFocusDistance; },
		[](ThisClass& Lens, FCameraLensSingleElementBarrier& Native)
		{
			if (Lens.bUseAutofocus)
				Native.SetAutofocus(Lens.MinimumFocusDistance);
		});
}

double UAGX_CameraLensSingleElement::GetMinimumFocusDistance() const
{
	const UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	if (SingleElementInstance != nullptr)
		return SingleElementInstance->GetMinimumFocusDistance();

	const FCameraLensSingleElementBarrier* Native = GetNativeAsSingleElement();
	if (Native != nullptr && GetUseAutofocus())
		return Native->GetMinimumFocusDistance();

	return MinimumFocusDistance;
}

void UAGX_CameraLensSingleElement::SetFocusDistance(double InFocusDistance)
{
	using namespace AGX_CameraLensSingleElement_helpers;

	SetSingleElementProperty(
		*this, [InFocusDistance](ThisClass& Lens) { Lens.FocusDistance = InFocusDistance; },
		[](ThisClass& Lens, FCameraLensSingleElementBarrier& Native)
		{
			if (!Lens.bUseAutofocus)
				Native.SetFocusDistance(Lens.FocusDistance);
		});
}

double UAGX_CameraLensSingleElement::GetFocusDistance() const
{
	const UAGX_CameraLensSingleElement* SingleElementInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraLensSingleElement>(Instance.Get());
	if (SingleElementInstance != nullptr)
		return SingleElementInstance->GetFocusDistance();

	const FCameraLensSingleElementBarrier* Native = GetNativeAsSingleElement();
	if (Native != nullptr && !GetUseAutofocus())
		return Native->GetFocusDistance();

	return FocusDistance;
}

void UAGX_CameraLensSingleElement::CopyProperties(const UAGX_CameraLensBase& Source)
{
	Super::CopyProperties(Source);

	const UAGX_CameraLensSingleElement* SourceSingleElement =
		Cast<UAGX_CameraLensSingleElement>(&Source);
	if (SourceSingleElement == nullptr)
		return;

	FocalLength = SourceSingleElement->FocalLength;
	FStop = SourceSingleElement->FStop;
	bUseAutofocus = SourceSingleElement->bUseAutofocus;
	MinimumFocusDistance = SourceSingleElement->MinimumFocusDistance;
	FocusDistance = SourceSingleElement->FocusDistance;
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
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(FStop), [](ThisClass* This) { This->SetFStop(This->FStop); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bUseAutofocus),
		[](ThisClass* This) { This->SetUseAutofocus(This->bUseAutofocus); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(MinimumFocusDistance),
		[](ThisClass* This) { This->SetMinimumFocusDistance(This->MinimumFocusDistance); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(FocusDistance),
		[](ThisClass* This) { This->SetFocusDistance(This->FocusDistance); });
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
	Native->SetFStop(FStop);
	if (bUseAutofocus)
		Native->SetAutofocus(MinimumFocusDistance);
	else
		Native->SetFocusDistance(FocusDistance);
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
