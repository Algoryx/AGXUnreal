// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraCMOSSensor.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/CameraCMOSSensorBarrier.h"

namespace AGX_CameraCMOSSensor_helpers
{
	UAGX_CameraCMOSSensor* GetCMOSSensorInstance(UAGX_CameraCMOSSensor& Sensor)
	{
		return Sensor.IsInstance()
			       ? &Sensor
			       : Cast<UAGX_CameraCMOSSensor>(Sensor.GetInstance());
	}

	template <typename SetPropertyFunc, typename UpdateNativeFunc>
	void SetCMOSSensorProperty(
		UAGX_CameraCMOSSensor& Sensor, SetPropertyFunc SetProperty,
		UpdateNativeFunc UpdateNative)
	{
		UAGX_CameraCMOSSensor* CMOSSensorInstance = GetCMOSSensorInstance(Sensor);
		UAGX_CameraCMOSSensor* Target =
			CMOSSensorInstance != nullptr ? CMOSSensorInstance : &Sensor;
		const bool bModifyAsset = !Target->IsInstance();
		if (bModifyAsset)
			AGX_WithEditorWrappers::Modify(*Target);

		SetProperty(*Target);
		FCameraCMOSSensorBarrier* Native = Target->GetNativeAsCMOSSensor();
		if (Native != nullptr)
			UpdateNative(*Target, *Native);

		if (bModifyAsset)
			AGX_WithEditorWrappers::MarkAssetDirty(*Target);
	}
}

void UAGX_CameraCMOSSensor::SetSize(FVector2D InSize)
{
	UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? this : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	FCameraCMOSSensorBarrier* CMOSSensorNativeBarrier =
		CMOSSensorInstance != nullptr ? CMOSSensorInstance->GetNativeAsCMOSSensor() : nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		Size, InSize, SetSize, CMOSSensorInstance, HasNative, CMOSSensorNativeBarrier, ->);
}

FVector2D UAGX_CameraCMOSSensor::GetSize() const
{
	const UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	const UAGX_CameraCMOSSensor* NativeOwner =
		CMOSSensorInstance != nullptr ? CMOSSensorInstance : this;
	const FCameraCMOSSensorBarrier* CMOSSensorNativeBarrier =
		NativeOwner->GetNativeAsCMOSSensor();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		Size, GetSize, CMOSSensorInstance, HasNative, CMOSSensorNativeBarrier, ->);
}

void UAGX_CameraCMOSSensor::SetISO(double InISO)
{
	UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? this : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	FCameraCMOSSensorBarrier* CMOSSensorNativeBarrier =
		CMOSSensorInstance != nullptr ? CMOSSensorInstance->GetNativeAsCMOSSensor() : nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		ISO, InISO, SetISO, CMOSSensorInstance, HasNative, CMOSSensorNativeBarrier, ->);
}

double UAGX_CameraCMOSSensor::GetISO() const
{
	const UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	const UAGX_CameraCMOSSensor* NativeOwner =
		CMOSSensorInstance != nullptr ? CMOSSensorInstance : this;
	const FCameraCMOSSensorBarrier* CMOSSensorNativeBarrier =
		NativeOwner->GetNativeAsCMOSSensor();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		ISO, GetISO, CMOSSensorInstance, HasNative, CMOSSensorNativeBarrier, ->);
}

void UAGX_CameraCMOSSensor::SetShutterSpeed(double InShutterSpeed)
{
	UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? this : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	FCameraCMOSSensorBarrier* CMOSSensorNativeBarrier =
		CMOSSensorInstance != nullptr ? CMOSSensorInstance->GetNativeAsCMOSSensor() : nullptr;
	AGX_ASSET_SETTER_IMPL_INTERNAL(
		ShutterSpeed, InShutterSpeed, SetShutterSpeed, CMOSSensorInstance, HasNative,
		CMOSSensorNativeBarrier, ->);
}

double UAGX_CameraCMOSSensor::GetShutterSpeed() const
{
	const UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	const UAGX_CameraCMOSSensor* NativeOwner =
		CMOSSensorInstance != nullptr ? CMOSSensorInstance : this;
	const FCameraCMOSSensorBarrier* CMOSSensorNativeBarrier =
		NativeOwner->GetNativeAsCMOSSensor();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		ShutterSpeed, GetShutterSpeed, CMOSSensorInstance, HasNative,
		CMOSSensorNativeBarrier, ->);
}

void UAGX_CameraCMOSSensor::SetUseAutoExposure(bool bInUseAutoExposure)
{
	using namespace AGX_CameraCMOSSensor_helpers;

	SetCMOSSensorProperty(
		*this,
		[bInUseAutoExposure](ThisClass& Sensor)
		{ Sensor.bUseAutoExposure = bInUseAutoExposure; },
		[](ThisClass& Sensor, FCameraCMOSSensorBarrier& Native)
		{
			if (Sensor.bUseAutoExposure)
				Native.SetAutoExposure(Sensor.DynamicRange);
			else
				Native.SetManualExposureCompensation(Sensor.ExposureCompensation);
		});
}

bool UAGX_CameraCMOSSensor::GetUseAutoExposure() const
{
	const UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	const UAGX_CameraCMOSSensor* NativeOwner =
		CMOSSensorInstance != nullptr ? CMOSSensorInstance : this;
	const FCameraCMOSSensorBarrier* CMOSSensorNativeBarrier =
		NativeOwner->GetNativeAsCMOSSensor();
	AGX_ASSET_GETTER_IMPL_INTERNAL(
		bUseAutoExposure, GetUseAutoExposure, CMOSSensorInstance, HasNative,
		CMOSSensorNativeBarrier, ->);
}

void UAGX_CameraCMOSSensor::SetDynamicRange(double InDynamicRange)
{
	using namespace AGX_CameraCMOSSensor_helpers;

	SetCMOSSensorProperty(
		*this,
		[InDynamicRange](ThisClass& Sensor) { Sensor.DynamicRange = InDynamicRange; },
		[](ThisClass& Sensor, FCameraCMOSSensorBarrier& Native)
		{
			if (Sensor.bUseAutoExposure)
				Native.SetAutoExposure(Sensor.DynamicRange);
		});
}

double UAGX_CameraCMOSSensor::GetDynamicRange() const
{
	const UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	if (CMOSSensorInstance != nullptr)
		return CMOSSensorInstance->GetDynamicRange();

	const FCameraCMOSSensorBarrier* Native = GetNativeAsCMOSSensor();
	if (Native != nullptr && GetUseAutoExposure())
		return Native->GetDynamicRange();

	return DynamicRange;
}

void UAGX_CameraCMOSSensor::SetExposureCompensation(double InExposureCompensation)
{
	using namespace AGX_CameraCMOSSensor_helpers;

	SetCMOSSensorProperty(
		*this,
		[InExposureCompensation](ThisClass& Sensor)
		{ Sensor.ExposureCompensation = InExposureCompensation; },
		[](ThisClass& Sensor, FCameraCMOSSensorBarrier& Native)
		{
			if (!Sensor.bUseAutoExposure)
				Native.SetManualExposureCompensation(Sensor.ExposureCompensation);
		});
}

double UAGX_CameraCMOSSensor::GetExposureCompensation() const
{
	const UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? nullptr : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	if (CMOSSensorInstance != nullptr)
		return CMOSSensorInstance->GetExposureCompensation();

	const FCameraCMOSSensorBarrier* Native = GetNativeAsCMOSSensor();
	if (Native != nullptr && !GetUseAutoExposure())
		return Native->GetExposureCompensation();

	return ExposureCompensation;
}

void UAGX_CameraCMOSSensor::CopyProperties(const UAGX_CameraPhotodetectorBase& Source)
{
	Super::CopyProperties(Source);

	const UAGX_CameraCMOSSensor* SourceCMOSSensor = Cast<UAGX_CameraCMOSSensor>(&Source);
	if (SourceCMOSSensor == nullptr)
		return;

	Size = SourceCMOSSensor->Size;
	ISO = SourceCMOSSensor->ISO;
	ShutterSpeed = SourceCMOSSensor->ShutterSpeed;
	bUseAutoExposure = SourceCMOSSensor->bUseAutoExposure;
	DynamicRange = SourceCMOSSensor->DynamicRange;
	ExposureCompensation = SourceCMOSSensor->ExposureCompensation;
}

void UAGX_CameraCMOSSensor::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_CameraCMOSSensor::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_CameraCMOSSensor::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(Size), [](ThisClass* This) { This->SetSize(This->Size); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ISO), [](ThisClass* This) { This->SetISO(This->ISO); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ShutterSpeed),
		[](ThisClass* This) { This->SetShutterSpeed(This->ShutterSpeed); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bUseAutoExposure),
		[](ThisClass* This) { This->SetUseAutoExposure(This->bUseAutoExposure); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(DynamicRange),
		[](ThisClass* This) { This->SetDynamicRange(This->DynamicRange); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ExposureCompensation),
		[](ThisClass* This) { This->SetExposureCompensation(This->ExposureCompensation); });
}
#endif // WITH_EDITOR

void UAGX_CameraCMOSSensor::UpdateNativeProperties()
{
	if (!IsInstance())
		return;

	FCameraCMOSSensorBarrier* Native = GetNativeAsCMOSSensor();
	if (Native == nullptr)
		return;

	Native->SetSize(Size);
	Native->SetISO(ISO);
	Native->SetShutterSpeed(ShutterSpeed);
	if (bUseAutoExposure)
		Native->SetAutoExposure(DynamicRange);
	else
		Native->SetManualExposureCompensation(ExposureCompensation);
}

FCameraCMOSSensorBarrier* UAGX_CameraCMOSSensor::GetNativeAsCMOSSensor()
{
	return const_cast<FCameraCMOSSensorBarrier*>(
		const_cast<const ThisClass*>(this)->GetNativeAsCMOSSensor());
}

const FCameraCMOSSensorBarrier* UAGX_CameraCMOSSensor::GetNativeAsCMOSSensor() const
{
	const UAGX_CameraCMOSSensor* CMOSSensorInstance =
		IsInstance() ? this : Cast<UAGX_CameraCMOSSensor>(Instance.Get());
	if (CMOSSensorInstance == nullptr)
		return nullptr;

	const FCameraPhotodetectorBarrier* Native =
		CMOSSensorInstance->UAGX_CameraPhotodetectorBase::GetNative();
	if (Native == nullptr)
		return nullptr;

	AGX_CHECK(FCameraCMOSSensorBarrier::IsCMOSSensor(*Native));
	return static_cast<const FCameraCMOSSensorBarrier*>(Native);
}

void UAGX_CameraCMOSSensor::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_CameraCMOSSensor '%s' whose instance is "
					 "nullptr. Ensure e.g. GetOrCreateInstance is called prior to calling this "
					 "function."),
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

	NativeBarrier = MakeUnique<FCameraCMOSSensorBarrier>();
	NativeBarrier->AllocateNative();
	check(HasNative());
	UpdateNativeProperties();
}
