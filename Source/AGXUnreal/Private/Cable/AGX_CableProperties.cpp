// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CableProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

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
			Asset->CopyFrom(*Barrier, nullptr);
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

void UAGX_CableProperties::CopyFrom(
	const FCablePropertiesBarrier& Source, FAGX_ImportContext* Context)
{
	// TODO
}

void UAGX_CableProperties::CopyFrom(const UAGX_CableProperties* Source)
{
	if (Source == nullptr)
		return;

	// TODO
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
	{
		return;
	}

	// TODO
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

	// TODO
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
