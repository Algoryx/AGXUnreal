// Copyright 2024, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackInternalMergeProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"

void UAGX_TrackInternalMergeProperties::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR

void UAGX_TrackInternalMergeProperties::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

#ifdef ADD_DISPATCH
#error "ADD_DISPATCH already defined. Chose another name here.
#endif
#define ADD_DISPATCH(Name, SetFunc)               \
	Dispatcher.Add(                               \
		GET_MEMBER_NAME_CHECKED(ThisClass, Name), \
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Name, SetFunc) })

	ADD_DISPATCH(bEnableMerge, SetMergeEnabled);
	ADD_DISPATCH(NumNodesPerMergeSegment, SetNumNodesPerMergeSegment);
	ADD_DISPATCH(ContactReduction, SetContactReduction);
	ADD_DISPATCH(bEnableLockToReachMergeCondition, SetLockToReachMergeConditionEnabled);
	ADD_DISPATCH(LockToReachMergeConditionCompliance, SetLockToReachMergeConditionCompliance);
	ADD_DISPATCH(LockToReachMergeConditionSpookDamping, SetLockToReachMergeConditionSpookDamping);
	ADD_DISPATCH(MaxAngleMergeCondition, SetMaxAngleMergeCondition);
#undef ADD_DISPATCH
}

void UAGX_TrackInternalMergeProperties::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	// UAGX_TrackInternalMergeProperties is not a Component and will not be destroyed and recreated
	// during RerunConstructionScript. It is therefore safe to call the base class
	// implementation immediately.
	Super::PostEditChangeProperty(Event);

	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
}

#endif

namespace AGX_TrackInternalMergeProperties_helpers
{
	bool IsValidTarget(
		const UAGX_TrackInternalMergeProperties* This, const UAGX_TrackComponent* Track)
	{
		return IsValid(Track) && Track->HasNative() && Track->InternalMergeProperties != nullptr &&
			   Track->InternalMergeProperties->GetInstance() == This;
	}

	/*
	 * This function should be functionally equivalent to the AGX_ASSET_SETTER_IMPL.+ macros in
	 * AGX_AssetGetterSetterImpl.h, except for the more complicated relationship with the Native
	 * that this asset type has.
	 */
	template <typename PropertyT, typename BarrierSetFuncT, typename ComponentSetFuncT>
	void AssetSetter(
		UAGX_TrackInternalMergeProperties* This, PropertyT& Property, PropertyT InValue,
		BarrierSetFuncT BarrierSetFunc, ComponentSetFuncT ComponentSetFunc)
	{
		if (This->IsInstance())
		{
			Property = InValue;
			for (const TWeakObjectPtr<UAGX_TrackComponent>& Track : This->GetTargetTracks())
			{
				if (Track.IsValid() && IsValidTarget(This, Track.Get()))
				{
					(Track->GetNative()->*BarrierSetFunc)(InValue);
				}
			}
		}
		else
		{
			if (This->GetInstance() != nullptr)
			{
				(This->GetInstance()->*ComponentSetFunc)(InValue);
			}
			else
			{
				Property = InValue;
			}
		}
	}

	template <typename FProperty, typename FGetFunc>
	FProperty AssetGetter(
		const UAGX_TrackInternalMergeProperties* This, const FProperty& Property, FGetFunc GetFunc)
	{
		if (This->IsInstance())
		{
			return Property;
		}
		else if (This->GetInstance() != nullptr)
		{
			return (This->GetInstance()->*GetFunc)();
		}
		else
		{
			return Property;
		}
	}

}

void UAGX_TrackInternalMergeProperties::SetMergeEnabled(bool bInEnabled)
{
	AGX_TrackInternalMergeProperties_helpers::AssetSetter(
		this, bEnableMerge, bInEnabled, &FTrackBarrier::InternalMergeProperties_SetEnableMerge,
		&UAGX_TrackInternalMergeProperties::SetMergeEnabled);
}

bool UAGX_TrackInternalMergeProperties::GetMergeEnabled() const
{
	return AGX_TrackInternalMergeProperties_helpers::AssetGetter(
		this, bEnableMerge, &UAGX_TrackInternalMergeProperties::GetMergeEnabled);
}

void UAGX_TrackInternalMergeProperties::SetNumNodesPerMergeSegment(int32 InNumNodesPerMergeSegment)
{
	AGX_TrackInternalMergeProperties_helpers::AssetSetter(
		this, NumNodesPerMergeSegment, InNumNodesPerMergeSegment,
		&FTrackBarrier::InternalMergeProperties_SetNumNodesPerMergeSegment,
		&UAGX_TrackInternalMergeProperties::SetNumNodesPerMergeSegment);
}

int32 UAGX_TrackInternalMergeProperties::GetNumNodesPerMergeSegment() const
{
	return AGX_TrackInternalMergeProperties_helpers::AssetGetter(
		this, NumNodesPerMergeSegment,
		&UAGX_TrackInternalMergeProperties::GetNumNodesPerMergeSegment);
}

void UAGX_TrackInternalMergeProperties::SetContactReduction(
	EAGX_MergedTrackNodeContactReduction InContactReduction)
{
	AGX_TrackInternalMergeProperties_helpers::AssetSetter(
		this, ContactReduction, InContactReduction,
		&FTrackBarrier::InternalMergeProperties_SetContactReduction,
		&UAGX_TrackInternalMergeProperties::SetContactReduction);
}

EAGX_MergedTrackNodeContactReduction UAGX_TrackInternalMergeProperties::GetContactReduction() const
{
	return AGX_TrackInternalMergeProperties_helpers::AssetGetter(
		this, ContactReduction, &UAGX_TrackInternalMergeProperties::GetContactReduction);
}

void UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionEnabled(bool bEnabled)
{
	AGX_TrackInternalMergeProperties_helpers::AssetSetter(
		this, bEnableLockToReachMergeCondition, bEnabled,
		&FTrackBarrier::InternalMergeProperties_SetEnableLockToReachMergeCondition,
		&UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionEnabled);
}

bool UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionEnabled() const
{
	return AGX_TrackInternalMergeProperties_helpers::AssetGetter(
		this, bEnableLockToReachMergeCondition,
		&UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionEnabled);
}

void UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionCompliance(double Compliance)
{
	AGX_TrackInternalMergeProperties_helpers::AssetSetter(
		this, LockToReachMergeConditionCompliance.GetValue(), Compliance,
		&FTrackBarrier::InternalMergeProperties_SetLockToReachMergeConditionCompliance,
		&UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionCompliance);
}

double UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionCompliance() const
{
	return AGX_TrackInternalMergeProperties_helpers::AssetGetter(
		this, LockToReachMergeConditionCompliance,
		&UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionCompliance);
}

void UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionCompliance_BP(float Compliance)
{
	SetLockToReachMergeConditionCompliance(static_cast<double>(Compliance));
}

float UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionCompliance_BP() const
{
	return static_cast<float>(GetLockToReachMergeConditionCompliance());
}

void UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionSpookDamping(double Damping)
{
	AGX_TrackInternalMergeProperties_helpers::AssetSetter(
		this, LockToReachMergeConditionSpookDamping.GetValue(), Damping,
		&FTrackBarrier::InternalMergeProperties_SetLockToReachMergeConditionSpookDamping,
		&UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionSpookDamping);
}

double UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionSpookDamping() const
{
	return AGX_TrackInternalMergeProperties_helpers::AssetGetter(
		this, LockToReachMergeConditionSpookDamping,
		&UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionSpookDamping);
}

void UAGX_TrackInternalMergeProperties::SetLockToReachMergeConditionSpookDamping_BP(float Damping)
{
	SetLockToReachMergeConditionSpookDamping(static_cast<double>(Damping));
}

float UAGX_TrackInternalMergeProperties::GetLockToReachMergeConditionSpookDamping_BP() const
{
	return static_cast<float>(GetLockToReachMergeConditionSpookDamping());
}

void UAGX_TrackInternalMergeProperties::SetMaxAngleMergeCondition(double MaxAngleToMerge)
{
	AGX_TrackInternalMergeProperties_helpers::AssetSetter(
		this, MaxAngleMergeCondition.GetValue(), MaxAngleToMerge,
		&FTrackBarrier::InternalMergeProperties_SetMaxAngleMergeCondition,
		&UAGX_TrackInternalMergeProperties::SetMaxAngleMergeCondition);
}

double UAGX_TrackInternalMergeProperties::GetMaxAngleMergeCondition() const
{
	return AGX_TrackInternalMergeProperties_helpers::AssetGetter(
		this, MaxAngleMergeCondition,
		&UAGX_TrackInternalMergeProperties::GetMaxAngleMergeCondition);
}

void UAGX_TrackInternalMergeProperties::SetMaxAngleMergeCondition_BP(float MaxAngleToMerge)
{
	SetMaxAngleMergeCondition(static_cast<double>(MaxAngleToMerge));
}

float UAGX_TrackInternalMergeProperties::GetMaxAngleMergeCondition_BP() const
{
	return static_cast<float>(GetMaxAngleMergeCondition());
}

void UAGX_TrackInternalMergeProperties::CopyFrom(const UAGX_TrackInternalMergeProperties& Source)
{
#ifdef COPY_PROPERTY
#error "COPY_PROPERTY already defined. Chose another name here."
#endif
#define COPY_PROPERTY(Name) Name = Source.Name
	/// \todo Is there a way to make this in a more implicit way? Easy to forget these when
	/// adding properties.
	COPY_PROPERTY(bEnableMerge);
	COPY_PROPERTY(NumNodesPerMergeSegment);
	COPY_PROPERTY(ContactReduction);
	COPY_PROPERTY(bEnableLockToReachMergeCondition);
	COPY_PROPERTY(LockToReachMergeConditionCompliance);
	COPY_PROPERTY(LockToReachMergeConditionSpookDamping);
	COPY_PROPERTY(MaxAngleMergeCondition);
#undef COPY_PROPERTY
}

void UAGX_TrackInternalMergeProperties::CopyFrom(const FTrackBarrier& Source)
{
	using namespace AGX_TrackInternalMergeProperties_helpers;
#ifdef COPY_PROPERTY
#error "COPY_PROPERTY already defined. Chose another name here."
#endif
#define COPY_PROPERTY(Name) Name = Source.InternalMergeProperties_Get##Name()
	/// \todo Is there a way to make this in a more implicit way? Easy to forget these when
	/// adding properties.
	bEnableMerge = Source.InternalMergeProperties_GetEnableMerge();
	COPY_PROPERTY(NumNodesPerMergeSegment);
	ContactReduction = Source.InternalMergeProperties_GetContactReduction();
	bEnableLockToReachMergeCondition =
		Source.InternalMergeProperties_GetEnableLockToReachMergeCondition();
	COPY_PROPERTY(LockToReachMergeConditionCompliance);
	COPY_PROPERTY(LockToReachMergeConditionSpookDamping);
	COPY_PROPERTY(MaxAngleMergeCondition);
#undef COPY_PROPERTY
}

void UAGX_TrackInternalMergeProperties::CommitToAsset()
{
	if (IsInstance())
	{
		/// \todo What should we do here?
		if (TargetTracks.Num() > 0)
		{
			Asset->CopyFrom(*TargetTracks[0]->GetNative());
		}
	}
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
}

UAGX_TrackInternalMergeProperties* UAGX_TrackInternalMergeProperties::CreateInstanceFromAsset(
	const UWorld& PlayingWorld, UAGX_TrackInternalMergeProperties& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld.IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_TrackInternalMergeProperties* NewInstance = NewObject<UAGX_TrackInternalMergeProperties>(
		GetTransientPackage(), UAGX_TrackInternalMergeProperties::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyFrom(Source);

	return NewInstance;
}

UAGX_TrackInternalMergeProperties* UAGX_TrackInternalMergeProperties::GetInstance()
{
	if (IsInstance())
	{
		return this;
	}
	else
	{
		return Instance.Get();
	}
}

const UAGX_TrackInternalMergeProperties* UAGX_TrackInternalMergeProperties::GetInstance() const
{
	if (IsInstance())
	{
		return this;
	}
	else
	{
		return Instance.Get();
	}
}

UAGX_TrackInternalMergeProperties* UAGX_TrackInternalMergeProperties::GetOrCreateInstance(
	const UWorld& PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}
	else
	{
		UAGX_TrackInternalMergeProperties* InstancePtr = Instance.Get();
		if (InstancePtr == nullptr && PlayingWorld.IsGameWorld())
		{
			InstancePtr =
				UAGX_TrackInternalMergeProperties::CreateInstanceFromAsset(PlayingWorld, *this);
			Instance = InstancePtr;
		}

		return InstancePtr;
	}
}

UAGX_TrackInternalMergeProperties* UAGX_TrackInternalMergeProperties::GetAsset()
{
	if (IsInstance())
	{
		return Asset.Get();
	}
	else
	{
		return this;
	}
}

bool UAGX_TrackInternalMergeProperties::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage())
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;

	// Internal testing the hypothesis that UObject::IsAsset is a valid inverse of this function.
	// @todo Consider removing this function and instead use UObject::IsAsset if the below check
	// has never failed for some period of time.
	AGX_CHECK(bIsInstance != IsAsset());

	return bIsInstance;
}

void UAGX_TrackInternalMergeProperties::UpdateNativeProperties()
{
	for (TWeakObjectPtr<UAGX_TrackComponent>& Track : TargetTracks)
	{
		UpdateNativeProperties(Track.Get());
	}
}

void UAGX_TrackInternalMergeProperties::UpdateNativeProperties(UAGX_TrackComponent* Track)
{
	using namespace AGX_TrackInternalMergeProperties_helpers;

	if (!IsValidTarget(this, Track))
	{
		return;
	}

	/// \note Be aware that this method will probably be called from
	/// UAGX_TrackComponent::CreateNative(), which means that the native is potentially under
	/// construction and that it is therefore a bit sensitive to exactly where from within that
	/// function that this function is called.

	FTrackBarrier* TrackBarrier = Track->GetOrCreateNative();
	if (TrackBarrier == nullptr || !TrackBarrier->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to update native properties of TrackInternalMergeProperties '%s' on "
				 "Track '%s' in '%s' because a valid native could not be found."),
			*GetName(), *Track->GetName(), *GetLabelSafe(Track->GetOwner()));
		return;
	}
	check(TrackBarrier->HasNative());

	TrackBarrier->InternalMergeProperties_SetEnableMerge(bEnableMerge);
	TrackBarrier->InternalMergeProperties_SetNumNodesPerMergeSegment(NumNodesPerMergeSegment);
	TrackBarrier->InternalMergeProperties_SetContactReduction(ContactReduction);
	TrackBarrier->InternalMergeProperties_SetEnableLockToReachMergeCondition(
		bEnableLockToReachMergeCondition);
	TrackBarrier->InternalMergeProperties_SetLockToReachMergeConditionCompliance(
		LockToReachMergeConditionCompliance);
	TrackBarrier->InternalMergeProperties_SetLockToReachMergeConditionSpookDamping(
		LockToReachMergeConditionSpookDamping);
	TrackBarrier->InternalMergeProperties_SetMaxAngleMergeCondition(MaxAngleMergeCondition);
}

const TArray<TWeakObjectPtr<UAGX_TrackComponent>>&
UAGX_TrackInternalMergeProperties::GetTargetTracks() const
{
	return TargetTracks;
}

void UAGX_TrackInternalMergeProperties::RegisterTargetTrack(UAGX_TrackComponent* Track)
{
	TargetTracks.AddUnique(Track);

	/// \todo Remove no longer valid TargetTargets? (i.e. tracks that has become null, for example
	///       due to BP instance reconstruction, or tracks that do no longer use this asset).

	UpdateNativeProperties(Track);
}

void UAGX_TrackInternalMergeProperties::UnregisterTargetTrack(UAGX_TrackComponent* Track)
{
	TargetTracks.Remove(Track);
}
