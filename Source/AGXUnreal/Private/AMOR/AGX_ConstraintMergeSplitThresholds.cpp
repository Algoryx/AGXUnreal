// Copyright 2024, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholds.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredForceRangeDiff_BP(
	float InMaxDesiredForceRangeDiff)
{
	SetMaxDesiredForceRangeDiff(static_cast<double>(InMaxDesiredForceRangeDiff));
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredForceRangeDiff(
	double InMaxDesiredForceRangeDiff)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaxDesiredForceRangeDiff, InMaxDesiredForceRangeDiff, SetMaxDesiredForceRangeDiff);
}

float UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredForceRangeDiff_BP() const
{
	return static_cast<float>(GetMaxDesiredForceRangeDiff());
}

double UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredForceRangeDiff() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxDesiredForceRangeDiff, GetMaxDesiredForceRangeDiff);
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredLockAngleDiff_BP(
	float InMaxDesiredLockAngleDiff)
{
	SetMaxDesiredLockAngleDiff(static_cast<double>(InMaxDesiredLockAngleDiff));
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredLockAngleDiff(
	double InMaxDesiredLockAngleDiff)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaxDesiredLockAngleDiff, InMaxDesiredLockAngleDiff, SetMaxDesiredLockAngleDiff);
}

float UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredLockAngleDiff_BP() const
{
	return static_cast<float>(GetMaxDesiredLockAngleDiff());
}

double UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredLockAngleDiff() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxDesiredLockAngleDiff, GetMaxDesiredLockAngleDiff);
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredRangeAngleDiff_BP(
	float InMaxDesiredRangeAngleDiff)
{
	SetMaxDesiredRangeAngleDiff(static_cast<double>(InMaxDesiredRangeAngleDiff));
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredRangeAngleDiff(
	double InMaxDesiredRangeAngleDiff)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaxDesiredRangeAngleDiff, InMaxDesiredRangeAngleDiff, SetMaxDesiredRangeAngleDiff);
}

float UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredRangeAngleDiff_BP() const
{
	return static_cast<float>(GetMaxDesiredRangeAngleDiff());
}

double UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredRangeAngleDiff() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxDesiredRangeAngleDiff, GetMaxDesiredRangeAngleDiff);
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredSpeedDiff_BP(float InMaxDesiredSpeedDiff)
{
	SetMaxDesiredSpeedDiff(static_cast<double>(InMaxDesiredSpeedDiff));
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxDesiredSpeedDiff(double InMaxDesiredSpeedDiff)
{
	AGX_ASSET_SETTER_IMPL_VALUE(MaxDesiredSpeedDiff, InMaxDesiredSpeedDiff, SetMaxDesiredSpeedDiff);
}

float UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredSpeedDiff_BP() const
{
	return static_cast<float>(GetMaxDesiredSpeedDiff());
}

double UAGX_ConstraintMergeSplitThresholds::GetMaxDesiredSpeedDiff() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxDesiredSpeedDiff, GetMaxDesiredSpeedDiff);
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxRelativeSpeed_BP(float InMaxRelativeSpeed)
{
	SetMaxRelativeSpeed(static_cast<double>(InMaxRelativeSpeed));
}

void UAGX_ConstraintMergeSplitThresholds::SetMaxRelativeSpeed(double InMaxRelativeSpeed)
{
	AGX_ASSET_SETTER_IMPL_VALUE(MaxRelativeSpeed, InMaxRelativeSpeed, SetMaxRelativeSpeed);
}

float UAGX_ConstraintMergeSplitThresholds::GetMaxRelativeSpeed_BP() const
{
	return static_cast<float>(GetMaxRelativeSpeed());
}

double UAGX_ConstraintMergeSplitThresholds::GetMaxRelativeSpeed() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxRelativeSpeed, GetMaxRelativeSpeed);
}

#if WITH_EDITOR
void UAGX_ConstraintMergeSplitThresholds::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ConstraintMergeSplitThresholds::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ConstraintMergeSplitThresholds::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholds, MaxDesiredForceRangeDiff),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxDesiredForceRangeDiff, SetMaxDesiredForceRangeDiff)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholds, MaxDesiredLockAngleDiff),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxDesiredLockAngleDiff, SetMaxDesiredLockAngleDiff) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholds, MaxDesiredRangeAngleDiff),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxDesiredRangeAngleDiff, SetMaxDesiredRangeAngleDiff)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholds, MaxDesiredSpeedDiff),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxDesiredSpeedDiff, SetMaxDesiredSpeedDiff) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintMergeSplitThresholds, MaxRelativeSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxRelativeSpeed, SetMaxRelativeSpeed) });
}
#endif

UAGX_ConstraintMergeSplitThresholds* UAGX_ConstraintMergeSplitThresholds::GetOrCreateInstance(
	UWorld* PlayingWorld, bool bIsRotational)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_ConstraintMergeSplitThresholds* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ConstraintMergeSplitThresholds::CreateFromAsset(
			PlayingWorld, *this, bIsRotational);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_ConstraintMergeSplitThresholds* UAGX_ConstraintMergeSplitThresholds::GetInstance()
{
	if (IsInstance())
	{
		return this;
	}

	return Instance.Get();
}

void UAGX_ConstraintMergeSplitThresholds::CreateNative(bool bIsRotational)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_ConstraintMergeSplitThresholds "
					 "'%s' who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called "
					 "prior "
					 "to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateNative(bIsRotational);
		return;
	}

	AGX_CHECK(IsInstance());
	NativeBarrier.AllocateNative(bIsRotational);
	AGX_CHECK(HasNative());

	SetNativeProperties();
}

FConstraintMergeSplitThresholdsBarrier* UAGX_ConstraintMergeSplitThresholds::GetOrCreateNative(
	UWorld* PlayingWorld, bool bIsRotational)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_ConstraintMergeSplitThresholds '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative(PlayingWorld, bIsRotational);
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative(bIsRotational);
	}

	return &NativeBarrier;
}

FConstraintMergeSplitThresholdsBarrier* UAGX_ConstraintMergeSplitThresholds::GetNative()
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

const FConstraintMergeSplitThresholdsBarrier* UAGX_ConstraintMergeSplitThresholds::GetNative() const
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

bool UAGX_ConstraintMergeSplitThresholds::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

UAGX_ConstraintMergeSplitThresholds* UAGX_ConstraintMergeSplitThresholds::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholds& Source, bool bIsRotational)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());
	AGX_CHECK(!Source.IsInstance());

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_ConstraintMergeSplitThresholds>(
		GetTransientPackage(), UAGX_ConstraintMergeSplitThresholds::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative(bIsRotational);
	return NewInstance;
}

bool UAGX_ConstraintMergeSplitThresholds::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage())
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

void UAGX_ConstraintMergeSplitThresholds::CopyFrom(const FMergeSplitThresholdsBarrier& Barrier)
{
	const auto CmstBarrier = static_cast<const FConstraintMergeSplitThresholdsBarrier*>(&Barrier);
	MaxDesiredForceRangeDiff = CmstBarrier->GetMaxDesiredForceRangeDiff();
	MaxDesiredLockAngleDiff = CmstBarrier->GetMaxDesiredLockAngleDiff();
	MaxDesiredRangeAngleDiff = CmstBarrier->GetMaxDesiredRangeAngleDiff();
	MaxDesiredSpeedDiff = CmstBarrier->GetMaxDesiredSpeedDiff();
	MaxRelativeSpeed = CmstBarrier->GetMaxRelativeSpeed();
	ImportGuid = CmstBarrier->GetGuid();
}

void UAGX_ConstraintMergeSplitThresholds::CopyFrom(
	const UAGX_ConstraintMergeSplitThresholds& Source)
{
	MaxDesiredForceRangeDiff = Source.MaxDesiredForceRangeDiff;
	MaxDesiredLockAngleDiff = Source.MaxDesiredLockAngleDiff;
	MaxDesiredRangeAngleDiff = Source.MaxDesiredRangeAngleDiff;
	MaxDesiredSpeedDiff = Source.MaxDesiredSpeedDiff;
	MaxRelativeSpeed = Source.MaxRelativeSpeed;
}

void UAGX_ConstraintMergeSplitThresholds::CopyTo(FConstraintMergeSplitThresholdsBarrier& Barrier)
{
	Barrier.SetMaxDesiredForceRangeDiff(MaxDesiredForceRangeDiff);
	Barrier.SetMaxDesiredLockAngleDiff(MaxDesiredLockAngleDiff);
	Barrier.SetMaxDesiredRangeAngleDiff(MaxDesiredRangeAngleDiff);
	Barrier.SetMaxDesiredSpeedDiff(MaxDesiredSpeedDiff);
	Barrier.SetMaxRelativeSpeed(MaxRelativeSpeed);
}

void UAGX_ConstraintMergeSplitThresholds::SetNativeProperties()
{
	if (HasNative())
	{
		CopyTo(NativeBarrier);
	}
}
