// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CableProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

UAGX_CableProperties* UAGX_CableProperties::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
		return this;

	if (Instance.IsValid())
		return Instance.Get();

	if (PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create runtime instance for Cable Properties asset '%s' because no "
				 "game world to create it in was given."),
			*GetPathName());
	}

	const FString InstanceName = GetName() + TEXT("_Instance");
	UAGX_CableProperties* NewInstance = NewObject<UAGX_CableProperties>(
		GetTransientPackage(), *InstanceName, RF_Transient, this);
	NewInstance->Asset = this;
	Instance = NewInstance;
	return NewInstance;
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
	if (IsInstance() && Asset.IsValid())
	{
		UEngine::CopyPropertiesForUnrelatedObjects(this, Asset.Get());
		if (UPackage* Package = Asset->GetPackage())
		{
			Package->SetDirtyFlag(true);
			Package->PackageMarkedDirtyEvent.Broadcast(Package, true);
		}
	}
	else if (Instance.IsValid())
	{
		Instance->CommitToAsset();
	}
}
