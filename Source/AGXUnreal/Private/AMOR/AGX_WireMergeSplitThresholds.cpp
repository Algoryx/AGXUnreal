// Copyright 2024, Algoryx Simulation AB.

#include "AMOR/AGX_WireMergeSplitThresholds.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

void UAGX_WireMergeSplitThresholds::SetForcePropagationDecayScale_BP(
	float InForcePropagationDecayScale)
{
	SetForcePropagationDecayScale(static_cast<double>(InForcePropagationDecayScale));
}

void UAGX_WireMergeSplitThresholds::SetForcePropagationDecayScale(
	double InForcePropagationDecayScale)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		ForcePropagationDecayScale, InForcePropagationDecayScale, SetForcePropagationDecayScale);
}

float UAGX_WireMergeSplitThresholds::GetForcePropagationDecayScale_BP() const
{
	return static_cast<float>(GetForcePropagationDecayScale());
}

double UAGX_WireMergeSplitThresholds::GetForcePropagationDecayScale() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ForcePropagationDecayScale, GetForcePropagationDecayScale);
}

void UAGX_WireMergeSplitThresholds::SetMergeTensionScale_BP(float InMergeTensionScale)
{
	SetMergeTensionScale(static_cast<double>(InMergeTensionScale));
}

void UAGX_WireMergeSplitThresholds::SetMergeTensionScale(double InMergeTensionScale)
{
	AGX_ASSET_SETTER_IMPL_VALUE(MergeTensionScale, InMergeTensionScale, SetMergeTensionScale);
}

float UAGX_WireMergeSplitThresholds::GetMergeTensionScale_BP() const
{
	return static_cast<float>(GetMergeTensionScale());
}

double UAGX_WireMergeSplitThresholds::GetMergeTensionScale() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MergeTensionScale, GetMergeTensionScale);
}

#if WITH_EDITOR
void UAGX_WireMergeSplitThresholds::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_WireMergeSplitThresholds::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_WireMergeSplitThresholds::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireMergeSplitThresholds, ForcePropagationDecayScale),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				ForcePropagationDecayScale, SetForcePropagationDecayScale)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireMergeSplitThresholds, MergeTensionScale),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MergeTensionScale, SetMergeTensionScale) });
}
#endif

UAGX_WireMergeSplitThresholds* UAGX_WireMergeSplitThresholds::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_WireMergeSplitThresholds* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_WireMergeSplitThresholds::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_WireMergeSplitThresholds* UAGX_WireMergeSplitThresholds::GetInstance()
{
	if (IsInstance())
	{
		return this;
	}

	return Instance.Get();
}

void UAGX_WireMergeSplitThresholds::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_WireMergeSplitThresholds "
					 "'%s' who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called "
					 "prior "
					 "to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateNative();
		return;
	}

	AGX_CHECK(IsInstance());
	NativeBarrier.AllocateNative();
	AGX_CHECK(HasNative());

	SetNativeProperties();
}

FWireMergeSplitThresholdsBarrier* UAGX_WireMergeSplitThresholds::GetOrCreateNative(
	UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_WireMergeSplitThresholds '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative();
	}

	return &NativeBarrier;
}

FWireMergeSplitThresholdsBarrier* UAGX_WireMergeSplitThresholds::GetNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			return nullptr;
		}

		return Instance->GetNative();
	}

	AGX_CHECK(IsInstance());
	return HasNative() ? &NativeBarrier : nullptr;
}

const FWireMergeSplitThresholdsBarrier* UAGX_WireMergeSplitThresholds::GetNative() const
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			return nullptr;
		}

		return Instance->GetNative();
	}

	AGX_CHECK(IsInstance());
	return HasNative() ? &NativeBarrier : nullptr;
}

bool UAGX_WireMergeSplitThresholds::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

UAGX_WireMergeSplitThresholds* UAGX_WireMergeSplitThresholds::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_WireMergeSplitThresholds& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());
	AGX_CHECK(!Source.IsInstance());

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_WireMergeSplitThresholds>(
		GetTransientPackage(), UAGX_WireMergeSplitThresholds::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

bool UAGX_WireMergeSplitThresholds::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;

	// Internal testing the hypothesis that UObject::IsAsset is a valid inverse of this function.
	// @todo Consider removing this function and instead use UObject::IsAsset, if the below check
	// has never failed.
	AGX_CHECK(bIsInstance != IsAsset());

	return bIsInstance;
}

void UAGX_WireMergeSplitThresholds::CopyFrom(const FMergeSplitThresholdsBarrier& Barrier)
{
	const FWireMergeSplitThresholdsBarrier* WmstBarrier =
		static_cast<const FWireMergeSplitThresholdsBarrier*>(&Barrier);
	ForcePropagationDecayScale = WmstBarrier->GetForcePropagationDecayScale();
	MergeTensionScale = WmstBarrier->GetMergeTensionScale();
	ImportGuid = WmstBarrier->GetGuid();
}

void UAGX_WireMergeSplitThresholds::CopyFrom(const UAGX_WireMergeSplitThresholds& Source)
{
	ForcePropagationDecayScale = Source.ForcePropagationDecayScale;
	MergeTensionScale = Source.MergeTensionScale;
}

void UAGX_WireMergeSplitThresholds::CopyTo(FWireMergeSplitThresholdsBarrier& Barrier)
{
	Barrier.SetForcePropagationDecayScale(ForcePropagationDecayScale);
	Barrier.SetMergeTensionScale(MergeTensionScale);
}

void UAGX_WireMergeSplitThresholds::SetNativeProperties()
{
	if (HasNative())
	{
		CopyTo(NativeBarrier);
	}
}
