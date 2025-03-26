// Copyright 2024, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "UObject/Package.h"

void FAGX_ConstraintMergeSplitProperties::OnBeginPlay(UAGX_ConstraintComponent& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());

	// Only allocate native if either EnableMerge or EnableSplit is true.
	// Not having a native is a perfectly valid and regular thing for this class.
	if (bEnableMerge || bEnableSplit)
	{
		FAGX_NotificationUtilities::LogWarningIfAmorDisabled("Constraint");
		CreateNative(Owner);
		CreateNativeThresholds(Owner);
		UpdateNativeProperties();
	}
}

#if WITH_EDITOR
void FAGX_ConstraintMergeSplitProperties::OnPostEditChangeProperty(UAGX_ConstraintComponent& Owner)
{
	if (bEnableMerge || bEnableSplit)
	{
		FAGX_NotificationUtilities::LogWarningIfAmorDisabled("Constraint");
		if (Owner.HasNative() && !HasNative())
		{
			// If we have not yet allocated a native, and we are in Play, and EnableMerge or
			// EnableSplit is true, then we should now allocate a Native.
			CreateNative(Owner);
			CreateNativeThresholds(Owner);
		}
	}

	if (HasNative())
	{
		UpdateNativeProperties();
	}
}
#endif

void FAGX_ConstraintMergeSplitProperties::CreateNative(UAGX_ConstraintComponent& Owner)
{
	AGX_CHECK(Owner.HasNative());
	AGX_CHECK(!HasNative());

	NativeBarrier.AllocateNative(*Owner.GetNative());
}

void FAGX_ConstraintMergeSplitProperties::BindBarrierToOwner(FConstraintBarrier& NewOwner)
{
	if (!NewOwner.HasNative())
	{
		NativeBarrier.ReleaseNative();
		return;
	}

	NativeBarrier.BindToNewOwner(NewOwner);
}

void FAGX_ConstraintMergeSplitProperties::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	NativeBarrier.SetEnableMerge(bEnableMerge);
	NativeBarrier.SetEnableSplit(bEnableSplit);

	UpdateNativeThresholds();
}

void FAGX_ConstraintMergeSplitProperties::CreateNativeThresholds(UAGX_ConstraintComponent& Owner)
{
	if (Thresholds == nullptr)
	{
		return;
	}

	UWorld* PlayingWorld = Owner.GetWorld();
	UAGX_ConstraintMergeSplitThresholds* ThresholdsInstance =
		Thresholds->GetOrCreateInstance(PlayingWorld, Owner.IsRotational());
	if (ThresholdsInstance == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to create a Merge Split Thresholds instance from the "
				 "asset '%s'."),
			*Thresholds->GetName());
		return;
	}

	if (!ThresholdsInstance->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to create a Merge Split Thresholds Native from the "
				 "instance '%s'."),
			*ThresholdsInstance->GetName());
	}
}

void FAGX_ConstraintMergeSplitProperties::UpdateNativeThresholds()
{
	AGX_CHECK(HasNative());
	if (Thresholds == nullptr)
	{
		NativeBarrier.SetConstraintMergeSplitThresholds(nullptr);
		return;
	}

	UAGX_ConstraintMergeSplitThresholds* ThresholdsInstance = Thresholds->GetInstance();
	if (ThresholdsInstance == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeThresholds called on Thresholds '%s' but it does not have an "
				 "instance. Has CreateNativeThresholds been called?"),
			*Thresholds->GetName());
		return;
	}

	if (Thresholds != ThresholdsInstance)
	{
		Thresholds = ThresholdsInstance;
	}

	FConstraintMergeSplitThresholdsBarrier* Barrier = ThresholdsInstance->GetNative();
	AGX_CHECK(Barrier);
	NativeBarrier.SetConstraintMergeSplitThresholds(Barrier);
}

UAGX_MergeSplitThresholdsBase* FAGX_ConstraintMergeSplitProperties::GetThresholds()
{
	return Thresholds;
}

void FAGX_ConstraintMergeSplitProperties::CopyFrom(
	const FMergeSplitPropertiesBarrier& Barrier, FAGX_ImportContext* Context)
{
	FAGX_MergeSplitPropertiesBase::CopyFrom(Barrier, Context);

	if (Context == nullptr || Context->MSThresholds == nullptr)
		return;

	// Get or create Merge Split Threashold from Context.
	FConstraintMergeSplitThresholdsBarrier ThresholdsBarrier =
		Barrier.GetConstraintMergeSplitThresholds();
	if (!ThresholdsBarrier.HasNative())
		return;

	const auto MSTGuid = ThresholdsBarrier.GetGuid();
	if (auto MST = Context->MSThresholds->FindRef(MSTGuid))
	{
		Thresholds = Cast<UAGX_ConstraintMergeSplitThresholds>(MST);
		return;
	}

	Thresholds = NewObject<UAGX_ConstraintMergeSplitThresholds>(
		Context->Outer, NAME_None, RF_Public | RF_Standalone);
	FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(*Thresholds, Context->SessionGuid);
	const FString THName = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		Thresholds->GetOuter(), FString::Printf(TEXT("AGX_CMST_%s"), *MSTGuid.ToString()), nullptr);
	Thresholds->Rename(*THName);
	Thresholds->CopyFrom(ThresholdsBarrier);
	Context->MSThresholds->Add(MSTGuid, Thresholds);
}
