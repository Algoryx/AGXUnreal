// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraLensBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

bool UAGX_CameraLensBase::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier != nullptr && NativeBarrier->HasNative();
}

FCameraLensBarrier* UAGX_CameraLensBase::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? NativeBarrier.Get() : nullptr;
}

const FCameraLensBarrier* UAGX_CameraLensBase::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? NativeBarrier.Get() : nullptr;
}

void UAGX_CameraLensBase::ReleaseNative()
{
	if (Instance != nullptr)
	{
		Instance->ReleaseNative();
		return;
	}

	if (HasNative())
	{
		NativeBarrier->ReleaseNative();
	}
}

void UAGX_CameraLensBase::CommitToAsset()
{
	if (IsInstance())
	{
		if (Asset == nullptr)
			return;

#if WITH_EDITOR
		Asset->Modify();
#endif
		Asset->CopyProperties(*this);
#if WITH_EDITOR
		FAGX_ObjectUtilities::MarkAssetDirty(*Asset);
#endif
	}
	else if (Instance != nullptr) // IsAsset
	{
		Instance->CommitToAsset();
	}
}

UAGX_CameraLensBase* UAGX_CameraLensBase::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_CameraLensBase& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_CameraLensBase* NewInstance = NewObject<UAGX_CameraLensBase>(
		GetTransientPackage(), Source.GetClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_CameraLensBase* UAGX_CameraLensBase::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_CameraLensBase* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = CreateInstanceFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

FCameraLensBarrier* UAGX_CameraLensBase::GetOrCreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_CameraLensBase '%s' whose instance "
					 "is nullptr. Ensure e.g. GetOrCreateInstance is called prior to calling "
					 "this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative();
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

bool UAGX_CameraLensBase::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;
	AGX_CHECK(bIsInstance != IsAsset());
	return bIsInstance;
}

UAGX_CameraLensBase* UAGX_CameraLensBase::GetInstance()
{
	return IsInstance() ? this : Instance.Get();
}

const UAGX_CameraLensBase* UAGX_CameraLensBase::GetInstance() const
{
	return IsInstance() ? this : Instance.Get();
}

UAGX_CameraLensBase* UAGX_CameraLensBase::GetAsset()
{
	return IsInstance() ? Asset.Get() : this;
}

const UAGX_CameraLensBase* UAGX_CameraLensBase::GetAsset() const
{
	return IsInstance() ? Asset.Get() : this;
}

void UAGX_CameraLensBase::CopyProperties(const UAGX_CameraLensBase& Source)
{
	(void) Source;
}
