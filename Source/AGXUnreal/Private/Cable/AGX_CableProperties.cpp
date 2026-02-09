// Copyright 2026, Algoryx Simulation AB.

#include "Cable/AGX_CableProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Cable/AGX_CableComponent.h"
#include "Cable/CableBarrier.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"


void UAGX_CableProperties::SetSpookDampingBend(double SpookDamping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SpookDampingBend, SpookDamping, SetSpookDampingBend);
}

double UAGX_CableProperties::GetSpookDampingBend() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SpookDampingBend, GetSpookDampingBend);
}

void UAGX_CableProperties::SetSpookDampingTwist(double SpookDamping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SpookDampingTwist, SpookDamping, SetSpookDampingTwist);
}

double UAGX_CableProperties::GetSpookDampingTwist() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SpookDampingTwist, GetSpookDampingTwist);
}

void UAGX_CableProperties::SetSpookDampingStretch(double SpookDamping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SpookDampingStretch, SpookDamping, SetSpookDampingStretch);
}

double UAGX_CableProperties::GetSpookDampingStretch() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SpookDampingStretch, GetSpookDampingStretch);
}

void UAGX_CableProperties::SetPoissonsRatioBend(double PoissonsRatio)
{
	AGX_ASSET_SETTER_IMPL_VALUE(PoissonsRatioBend, PoissonsRatio, SetPoissonsRatioBend);
}

double UAGX_CableProperties::GetPoissonsRatioBend() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(PoissonsRatioBend, GetPoissonsRatioBend);
}

void UAGX_CableProperties::SetPoissonsRatioTwist(double PoissonsRatio)
{
	AGX_ASSET_SETTER_IMPL_VALUE(PoissonsRatioTwist, PoissonsRatio, SetPoissonsRatioTwist);
}

double UAGX_CableProperties::GetPoissonsRatioTwist() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(PoissonsRatioTwist, GetPoissonsRatioTwist);
}

void UAGX_CableProperties::SetPoissonsRatioStretch(double PoissonsRatio)
{
	AGX_ASSET_SETTER_IMPL_VALUE(PoissonsRatioStretch, PoissonsRatio, SetPoissonsRatioStretch);
}

double UAGX_CableProperties::GetPoissonsRatioStretch() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(PoissonsRatioStretch, GetPoissonsRatioStretch);
}

void UAGX_CableProperties::SetYoungsModulusBend(double YoungsModulus)
{
	AGX_ASSET_SETTER_IMPL_VALUE(YoungsModulusBend, YoungsModulus, SetYoungsModulusBend);
}

double UAGX_CableProperties::GetYoungsModulusBend() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(YoungsModulusBend, GetYoungsModulusBend);
}

void UAGX_CableProperties::SetYoungsModulusTwist(double YoungsModulus)
{
	AGX_ASSET_SETTER_IMPL_VALUE(YoungsModulusTwist, YoungsModulus, SetYoungsModulusTwist);
}

double UAGX_CableProperties::GetYoungsModulusTwist() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(YoungsModulusTwist, GetYoungsModulusTwist);
}

void UAGX_CableProperties::SetYoungsModulusStretch(double YoungsModulus)
{
	AGX_ASSET_SETTER_IMPL_VALUE(YoungsModulusStretch, YoungsModulus, SetYoungsModulusStretch);
}

double UAGX_CableProperties::GetYoungsModulusStretch() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(YoungsModulusStretch, GetYoungsModulusStretch);
}


UAGX_CableProperties* UAGX_CableProperties::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}
	else
	{
		UAGX_CableProperties* InstancePtr = Instance.Get();
		if (InstancePtr == nullptr && PlayingWorld && PlayingWorld->IsGameWorld())
		{
			InstancePtr = UAGX_CableProperties::CreateInstanceFromAsset(PlayingWorld, this);
			Instance = InstancePtr;
		}

		return InstancePtr;
	}
}

bool UAGX_CableProperties::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	return Asset != nullptr;
}

UAGX_CableProperties* UAGX_CableProperties::CreateInstanceFromAsset(
	const UWorld* PlayingWorld, UAGX_CableProperties* Source)
{
	check(Source);
	check(!Source->IsInstance());
	check(PlayingWorld != nullptr);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source->GetName() + "_Instance";

	UAGX_CableProperties* NewInstance = NewObject<UAGX_CableProperties>(
		GetTransientPackage(), UAGX_CableProperties::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_CableProperties* UAGX_CableProperties::GetInstance()
{
	return Instance.Get();
}

UAGX_CableProperties* UAGX_CableProperties::GetAsset()
{
	return Asset.Get();
}

void UAGX_CableProperties::CommitToAsset()
{
	if (IsInstance())
	{
		if (FCablePropertiesBarrier* Barrier = GetNative())
		{
#if WITH_EDITOR
			Asset->Modify();
#endif
			Asset->CopyFrom(*Barrier);
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(*Asset);
#endif
		}
	}
	else if (Instance != nullptr) // IsAsset
	{
		Instance->CommitToAsset();
	}
}

namespace AGX_CableProperties_helpers
{
	FString CreatePropertiesName(const FCableBarrier& Barrier, FAGX_ImportContext& Context)
	{
		auto Cable = Context.Cables->FindRef(Barrier.GetGuid());
		const FString BaseName = Cable != nullptr ? Cable->GetName() : "Unknown";
		const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			Context.Outer, FString::Printf(TEXT("AGX_CP_%s"), *BaseName),
			UAGX_CableProperties::StaticClass());

		if (Cable == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to get Cable from FAGX_ImportContext in "
					 "AGX_CableProperties_helpers::CreatePropertiesName. The CableProperties "
					 "will get the name '%s'."),
				*Name);
		}

		return Name;
	}
}

void UAGX_CableProperties::CopyFrom(
	const FCableBarrier& Source, FAGX_ImportContext* Context)
{
	if (!Source.HasNative())
		return;

	FCablePropertiesBarrier PropBarrier = Source.GetCableProperties();
	if (!PropBarrier.HasNative())
		return;

	CopyFrom(PropBarrier);

	if (Context == nullptr || Context->Cables == nullptr || Context->CableProperties == nullptr)
		return; // We are done.

	ImportGuid = Source.GetGuid();
	Rename(*AGX_CableProperties_helpers::CreatePropertiesName(Source, *Context));
	AGX_CHECK(!Context->CableProperties->Contains(ImportGuid));
	Context->CableProperties->Add(ImportGuid, this);
}

void UAGX_CableProperties::CopyFrom(const FCablePropertiesBarrier& Source)
{
	if (!Source.HasNative())
		return;

	SpookDampingBend = Source.GetSpookDampingBend();
	SpookDampingTwist = Source.GetSpookDampingTwist();
	SpookDampingStretch = Source.GetSpookDampingStretch();

	PoissonsRatioBend = Source.GetPoissonsRatioBend();
	PoissonsRatioTwist = Source.GetPoissonsRatioTwist();
	PoissonsRatioStretch = Source.GetPoissonsRatioStretch();

	YoungsModulusBend = Source.GetYoungsModulusBend();
	YoungsModulusTwist = Source.GetYoungsModulusTwist();
	YoungsModulusStretch = Source.GetYoungsModulusStretch();
}

void UAGX_CableProperties::CopyFrom(const UAGX_CableProperties* Source)
{
	if (Source == nullptr)
		return;

	SpookDampingBend = Source->SpookDampingBend;
	SpookDampingTwist = Source->SpookDampingTwist;
	SpookDampingStretch = Source->SpookDampingStretch;

	PoissonsRatioBend = Source->PoissonsRatioBend;
	PoissonsRatioTwist = Source->PoissonsRatioTwist;
	PoissonsRatioStretch = Source->PoissonsRatioStretch;

	YoungsModulusBend = Source->YoungsModulusBend;
	YoungsModulusTwist = Source->YoungsModulusTwist;
	YoungsModulusStretch = Source->YoungsModulusStretch;
}

bool UAGX_CableProperties::HasNative() const
{
	if (IsInstance())
	{
		return NativeBarrier.HasNative();
	}
	else
	{
		return Instance != nullptr && Instance->HasNative();
	}
}

FCablePropertiesBarrier* UAGX_CableProperties::GetNative()
{
	if (IsInstance())
		return NativeBarrier.HasNative() ? &NativeBarrier : nullptr;
	else
		return Instance != nullptr ? Instance->GetNative() : nullptr;
}

const FCablePropertiesBarrier* UAGX_CableProperties::GetNative() const
{
	if (IsInstance())
		return NativeBarrier.HasNative() ? &NativeBarrier : nullptr;
	else
		return Instance != nullptr ? Instance->GetNative() : nullptr;
}

FCablePropertiesBarrier* UAGX_CableProperties::GetOrCreateNative()
{
	if (IsInstance())
	{
		if (!HasNative())
		{
			CreateNative();
		}
		return GetNative();
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_CableProperties '%s' who's instance is "
					 "nullptr. Ensure e.g. GetOrCreateInstance is called prior to calling this "
					 "function"),
				*GetName());
			return nullptr;
		}
		return Instance->GetOrCreateNative();
	}
}

void UAGX_CableProperties::UpdateNativeProperties()
{
	if (!IsInstance() || !HasNative())
		return;

	NativeBarrier.SetSpookDampingBend(SpookDampingBend);
	NativeBarrier.SetSpookDampingTwist(SpookDampingTwist);
	NativeBarrier.SetSpookDampingStretch(SpookDampingStretch);

	NativeBarrier.SetPoissonsRatioBend(PoissonsRatioBend);
	NativeBarrier.SetPoissonsRatioTwist(PoissonsRatioTwist);
	NativeBarrier.SetPoissonsRatioStretch(PoissonsRatioStretch);

	NativeBarrier.SetYoungsModulusBend(YoungsModulusBend);
	NativeBarrier.SetYoungsModulusTwist(YoungsModulusTwist);
	NativeBarrier.SetYoungsModulusStretch(YoungsModulusStretch);
}

void UAGX_CableProperties::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_CableProperties::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_CableProperties::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER(SpookDampingBend);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SpookDampingTwist);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SpookDampingStretch);

	AGX_COMPONENT_DEFAULT_DISPATCHER(PoissonsRatioBend);
	AGX_COMPONENT_DEFAULT_DISPATCHER(PoissonsRatioTwist);
	AGX_COMPONENT_DEFAULT_DISPATCHER(PoissonsRatioStretch);

	AGX_COMPONENT_DEFAULT_DISPATCHER(YoungsModulusBend);
	AGX_COMPONENT_DEFAULT_DISPATCHER(YoungsModulusTwist);
	AGX_COMPONENT_DEFAULT_DISPATCHER(YoungsModulusStretch);
}
#endif // WITH_EDITOR

void UAGX_CableProperties::CreateNative()
{
	if (IsInstance())
	{
		check(!HasNative());
		NativeBarrier.AllocateNative();
		if (!HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("UAGX_CableProperties '%s' failed to create native AGX Dynamics instance. See "
					 "the AGXDynamics log channel for additional information."),
				*GetName());
			return;
		}
		UpdateNativeProperties();
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"CreateNative was colled on an UAGX_CableProperties who's instance is nullptr. "
					"Ensure e.g. GetOrCreateInstance is called prior to calling this function"));
			return;
		}
		return Instance->CreateNative();
	}
}
