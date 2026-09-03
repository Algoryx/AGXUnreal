// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraPhotodetectorBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

bool UAGX_CameraPhotodetectorBase::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier != nullptr && NativeBarrier->HasNative();
}

FCameraPhotodetectorBarrier* UAGX_CameraPhotodetectorBase::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? NativeBarrier.Get() : nullptr;
}

const FCameraPhotodetectorBarrier* UAGX_CameraPhotodetectorBase::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? NativeBarrier.Get() : nullptr;
}

void UAGX_CameraPhotodetectorBase::ReleaseNative()
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

void UAGX_CameraPhotodetectorBase::CommitToAsset()
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

UAGX_CameraPhotodetectorBase* UAGX_CameraPhotodetectorBase::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_CameraPhotodetectorBase& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_CameraPhotodetectorBase* NewInstance = NewObject<UAGX_CameraPhotodetectorBase>(
		GetTransientPackage(), Source.GetClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_CameraPhotodetectorBase* UAGX_CameraPhotodetectorBase::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_CameraPhotodetectorBase* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = CreateInstanceFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

FCameraPhotodetectorBarrier* UAGX_CameraPhotodetectorBase::GetOrCreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_CameraPhotodetectorBase '%s' "
					 "whose instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
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

bool UAGX_CameraPhotodetectorBase::IsInstance() const
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

UAGX_CameraPhotodetectorBase* UAGX_CameraPhotodetectorBase::GetInstance()
{
	return IsInstance() ? this : Instance.Get();
}

const UAGX_CameraPhotodetectorBase* UAGX_CameraPhotodetectorBase::GetInstance() const
{
	return IsInstance() ? this : Instance.Get();
}

UAGX_CameraPhotodetectorBase* UAGX_CameraPhotodetectorBase::GetAsset()
{
	return IsInstance() ? Asset.Get() : this;
}

const UAGX_CameraPhotodetectorBase* UAGX_CameraPhotodetectorBase::GetAsset() const
{
	return IsInstance() ? Asset.Get() : this;
}

void UAGX_CameraPhotodetectorBase::CopyProperties(
	const UAGX_CameraPhotodetectorBase& Source)
{
	(void) Source;
}
