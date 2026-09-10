// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LensDistortionBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

bool UAGX_LensDistortionBase::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier != nullptr && NativeBarrier->HasNative();
}

FLensDistortionBarrier* UAGX_LensDistortionBase::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? NativeBarrier.Get() : nullptr;
}

const FLensDistortionBarrier* UAGX_LensDistortionBase::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? NativeBarrier.Get() : nullptr;
}

void UAGX_LensDistortionBase::ReleaseNative()
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

void UAGX_LensDistortionBase::CommitToAsset()
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
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
}

UAGX_LensDistortionBase* UAGX_LensDistortionBase::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_LensDistortionBase& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_LensDistortionBase* NewInstance = NewObject<UAGX_LensDistortionBase>(
		GetTransientPackage(), Source.GetClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_LensDistortionBase* UAGX_LensDistortionBase::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_LensDistortionBase* InstancePtr = Instance.Get();
	if (InstancePtr == nullptr && PlayingWorld != nullptr && PlayingWorld->IsGameWorld())
	{
		InstancePtr = CreateInstanceFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

FLensDistortionBarrier* UAGX_LensDistortionBase::GetOrCreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_LensDistortionBase '%s' whose "
					 "instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior to "
					 "calling this function."),
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

bool UAGX_LensDistortionBase::IsInstance() const
{
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	const bool bIsInstance = Asset != nullptr;
	return bIsInstance;
}

UAGX_LensDistortionBase* UAGX_LensDistortionBase::GetInstance()
{
	return IsInstance() ? this : Instance.Get();
}

const UAGX_LensDistortionBase* UAGX_LensDistortionBase::GetInstance() const
{
	return IsInstance() ? this : Instance.Get();
}

UAGX_LensDistortionBase* UAGX_LensDistortionBase::GetAsset()
{
	return IsInstance() ? Asset.Get() : this;
}

const UAGX_LensDistortionBase* UAGX_LensDistortionBase::GetAsset() const
{
	return IsInstance() ? Asset.Get() : this;
}

void UAGX_LensDistortionBase::CopyProperties(const UAGX_LensDistortionBase& Source)
{
	(void) Source;
}
