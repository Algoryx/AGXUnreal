// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/ShovelBarrier.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

UAGX_ShovelProperties::UAGX_ShovelProperties()
	: bOverride_ContactRegionThreshold(false)
	, bOverride_ContactRegionVerticalLimit(false)
{
}

template <typename StorageT, typename ParameterT>
void SetAndPropagateShovelProperty(
	UAGX_ShovelProperties& Properties, TArray<TWeakObjectPtr<UAGX_ShovelComponent>>& Shovels,
	StorageT* Storage, ParameterT NewValue, bool PropagateToNative,
	void (UAGX_ShovelProperties::*ComponentSetter)(ParameterT),
	void (FShovelBarrier::*BarrierSetter)(ParameterT))
{
	/*
	This implementation should follow the pattern set by AGX_ASSET_SETTER_IMPL_INTERNAL, but
	adapted due to the fact that we don't have a single Barrier object but instead a collection
	of Shovel Components that own the Barriers.

	The general idea is that changes made to an instance should propagate to Barriers, and changes
	made to an asset should be applied on the instance instead, if there is one.
	*/
	if (Properties.IsInstance())
	{
		*Storage = NewValue;
		if (PropagateToNative)
		{
			for (TWeakObjectPtr<UAGX_ShovelComponent>& Shovel : Shovels)
			{
				if (Shovel.IsValid() && Shovel->HasNative())
				{
					(Shovel->GetNative()->*BarrierSetter)(NewValue);
				}
			}
		}
	}
	else
	{
		if (Properties.GetInstance() != nullptr)
		{
			(Properties.GetInstance()->*ComponentSetter)(NewValue);
		}
		else
		{
			*Storage = NewValue;
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(Properties);
#endif
		}
	}
}

#define AGX_SHOVEL_SETTER_IMPL(PropertyName)                   \
	SetAndPropagateShovelProperty(                             \
		*this, Shovels, &PropertyName, In##PropertyName, true, \
		&UAGX_ShovelProperties::Set##PropertyName, &FShovelBarrier::Set##PropertyName)

#define AGX_SHOVEL_SETTER_OVERRIDE_IMPL(PropertyName, Overriden)    \
	SetAndPropagateShovelProperty(                                  \
		*this, Shovels, &PropertyName, In##PropertyName, Overriden, \
		&UAGX_ShovelProperties::Set##PropertyName, &FShovelBarrier::Set##PropertyName)

template <typename StorageT, typename ParameterT>
void SetAndPropagateShovelExcavationProperty(
	UAGX_ShovelProperties& Properties, TArray<TWeakObjectPtr<UAGX_ShovelComponent>>& Shovels,
	EAGX_ExcavationMode ExcavationMode, StorageT* Storage, ParameterT NewValue,
	bool PropagateToNative,
	void (UAGX_ShovelProperties::*ComponentSetter)(EAGX_ExcavationMode, ParameterT),
	void (FShovelBarrier::*BarrierSetter)(EAGX_ExcavationMode, ParameterT))
{
	/*
	This implementation should follow the pattern set by SetAndPropagateShovelProperty, but
	adapted due to the fact that the setter functions must specify which excavation mode is being
	modified.

	The general idea is that changes made to an instance should propagate to Barriers, and changes
	made to an asset should be applied on the instance instead, if there is one.
	*/
	if (Properties.IsInstance())
	{
		*Storage = NewValue;
		if (PropagateToNative)
		{
			for (TWeakObjectPtr<UAGX_ShovelComponent>& Shovel : Shovels)
			{
				if (Shovel.IsValid() && Shovel->HasNative())
				{
					(Shovel->GetNative()->*BarrierSetter)(ExcavationMode, NewValue);
				}
			}
		}
	}
	else
	{
		if (Properties.GetInstance() != nullptr)
		{
			(Properties.GetInstance()->*ComponentSetter)(ExcavationMode, NewValue);
		}
		else
		{
			*Storage = NewValue;
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(Properties);
#endif
		}
	}
}

#define AGX_SHOVEL_SETTER_EXCAVATION_IMPL(                                              \
	PropertyName, MemberName, ExcavationMode, ComponentSetter, BarrierSetter)           \
	SetAndPropagateShovelExcavationProperty(                                            \
		*this, Shovels, ExcavationMode, &PropertyName.MemberName, In##MemberName, true, \
		&UAGX_ShovelProperties::ComponentSetter, &FShovelBarrier::BarrierSetter)

void UAGX_ShovelProperties::SetToothLength(double InToothLength)
{
	AGX_SHOVEL_SETTER_IMPL(ToothLength);
}

void UAGX_ShovelProperties::SetToothMinimumRadius(double InToothMinimumRadius)
{
	AGX_SHOVEL_SETTER_IMPL(ToothMinimumRadius);
}

void UAGX_ShovelProperties::SetToothMaximumRadius(double InToothMaximumRadius)
{
	AGX_SHOVEL_SETTER_IMPL(ToothMaximumRadius);
}

void UAGX_ShovelProperties::SetNumberOfTeeth(int32 InNumberOfTeeth)
{
	AGX_SHOVEL_SETTER_IMPL(NumberOfTeeth);
}

void UAGX_ShovelProperties::SetNoMergeExtensionDistance(double InNoMergeExtensionDistance)
{
	AGX_SHOVEL_SETTER_IMPL(NoMergeExtensionDistance);
}

void UAGX_ShovelProperties::SetMinimumSubmergedContactLengthFraction(
	double InMinimumSubmergedContactLengthFraction)
{
	AGX_SHOVEL_SETTER_IMPL(MinimumSubmergedContactLengthFraction);
}

void UAGX_ShovelProperties::SetVerticalBladeSoilMergeDistance(
	double InVerticalBladeSoilMergeDistance)
{
	AGX_SHOVEL_SETTER_IMPL(VerticalBladeSoilMergeDistance);
}

void UAGX_ShovelProperties::SetSecondarySeparationDeadloadLimit(
	double InSecondarySeparationDeadloadLimit)
{
	AGX_SHOVEL_SETTER_IMPL(SecondarySeparationDeadloadLimit);
}

void UAGX_ShovelProperties::SetPenetrationDepthThreshold(double InPenetrationDepthThreshold)
{
	AGX_SHOVEL_SETTER_IMPL(PenetrationDepthThreshold);
}

void UAGX_ShovelProperties::SetPenetrationForceScaling(double InPenetrationForceScaling)
{
	AGX_SHOVEL_SETTER_IMPL(PenetrationForceScaling);
}

void UAGX_ShovelProperties::SetEnableParticleFreeDeformers(bool InbEnableParticleFreeDeformers)
{
	AGX_SHOVEL_SETTER_IMPL(bEnableParticleFreeDeformers);
}

void UAGX_ShovelProperties::SetAlwaysRemoveShovelContacts(bool InbAlwaysRemoveShovelContacts)
{
	AGX_SHOVEL_SETTER_IMPL(bAlwaysRemoveShovelContacts);
}

void UAGX_ShovelProperties::SetMaximumPenetrationForce(double InMaximumPenetrationForce)
{
	AGX_SHOVEL_SETTER_IMPL(MaximumPenetrationForce);
}

void UAGX_ShovelProperties::SetContactRegionThreshold(double InContactRegionThreshold)
{
	AGX_SHOVEL_SETTER_OVERRIDE_IMPL(ContactRegionThreshold, bOverride_ContactRegionThreshold);
}

void UAGX_ShovelProperties::SetContactRegionVerticalLimit(double InContactRegionVerticalLimit)
{
	AGX_SHOVEL_SETTER_OVERRIDE_IMPL(
		ContactRegionVerticalLimit, bOverride_ContactRegionVerticalLimit);
}

void UAGX_ShovelProperties::SetEnableInnerShapeCreateDynamicMass(
	bool InbEnableInnerShapeCreateDynamicMass)
{
	AGX_SHOVEL_SETTER_IMPL(bEnableInnerShapeCreateDynamicMass);
}

void UAGX_ShovelProperties::SetEnableParticleForceFeedback(bool InbEnableParticleForceFeedback)
{
	AGX_SHOVEL_SETTER_IMPL(bEnableParticleForceFeedback);
}

void UAGX_ShovelProperties::SetParticleInclusionMultiplier(double InParticleInclusionMultiplier)
{
	AGX_SHOVEL_SETTER_IMPL(ParticleInclusionMultiplier);
}

FAGX_ShovelExcavationSettings* UAGX_ShovelProperties::GetExcavationSettings(
	EAGX_ExcavationMode InExcavationMode)
{
	switch (InExcavationMode)
	{
		case EAGX_ExcavationMode::Primary:
			return &PrimaryExcavationSettings;
		case EAGX_ExcavationMode::DeformBack:
			return &DeformBackExcavationSettings;
		case EAGX_ExcavationMode::DeformRight:
			return &DeformRightExcavationSettings;
		case EAGX_ExcavationMode::DeformLeft:
			return &DeformLeftExcavationSettings;
	}
	return nullptr;
}

void UAGX_ShovelProperties::SetExcavationEnabled(
	EAGX_ExcavationMode InExcavationMode, bool InbEnabled)
{
#define AGX_SHOVEL_SETTER_EXCAVATION_ENABLED(ExcavationSettings)              \
	AGX_SHOVEL_SETTER_EXCAVATION_IMPL(                                        \
		ExcavationSettings, bEnabled, InExcavationMode, SetExcavationEnabled, \
		SetExcavationSettingsEnabled)

	switch (InExcavationMode)
	{
		case EAGX_ExcavationMode::Primary:
			AGX_SHOVEL_SETTER_EXCAVATION_ENABLED(PrimaryExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformBack:
			AGX_SHOVEL_SETTER_EXCAVATION_ENABLED(DeformBackExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformRight:
			AGX_SHOVEL_SETTER_EXCAVATION_ENABLED(DeformRightExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformLeft:
			AGX_SHOVEL_SETTER_EXCAVATION_ENABLED(DeformLeftExcavationSettings);
			break;
	}
#undef AGX_SHOVEL_SETTER_EXCAVATION_ENABLED
}

void UAGX_ShovelProperties::SetExcavationCreateDynamicMassEnabled(
	EAGX_ExcavationMode InExcavationMode, bool InbEnableCreateDynamicMass)
{
#define AGX_SHOVEL_SETTER_EXCAVATION_DYNAMIC_MASS(ExcavationMode)   \
	AGX_SHOVEL_SETTER_EXCAVATION_IMPL(                              \
		ExcavationMode, bEnableCreateDynamicMass, InExcavationMode, \
		SetExcavationCreateDynamicMassEnabled, SetExcavationSettingsEnableCreateDynamicMass)

	switch (InExcavationMode)
	{
		case EAGX_ExcavationMode::Primary:
			AGX_SHOVEL_SETTER_EXCAVATION_DYNAMIC_MASS(PrimaryExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformBack:
			AGX_SHOVEL_SETTER_EXCAVATION_DYNAMIC_MASS(DeformBackExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformRight:
			AGX_SHOVEL_SETTER_EXCAVATION_DYNAMIC_MASS(DeformRightExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformLeft:
			AGX_SHOVEL_SETTER_EXCAVATION_DYNAMIC_MASS(DeformLeftExcavationSettings);
			break;
	}

#undef AGX_SHOVEL_SETTER_EXCAVATION_DYNAMIC_MASS
}

void UAGX_ShovelProperties::SetExcavationForceFeedbackEnabled(
	EAGX_ExcavationMode InExcavationMode, bool InbEnableForceFeedback)
{
#define AGX_SHOVEL_SETTER_EXCAVATION_FORCE_FEEDBACK(ExcavationMode)                                \
	AGX_SHOVEL_SETTER_EXCAVATION_IMPL(                                                             \
		ExcavationMode, bEnableForceFeedback, InExcavationMode, SetExcavationForceFeedbackEnabled, \
		SetExcavationSettingsEnableForceFeedback)

	switch (InExcavationMode)
	{
		case EAGX_ExcavationMode::Primary:
			AGX_SHOVEL_SETTER_EXCAVATION_FORCE_FEEDBACK(PrimaryExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformBack:
			AGX_SHOVEL_SETTER_EXCAVATION_FORCE_FEEDBACK(DeformBackExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformRight:
			AGX_SHOVEL_SETTER_EXCAVATION_FORCE_FEEDBACK(DeformRightExcavationSettings);
			break;
		case EAGX_ExcavationMode::DeformLeft:
			AGX_SHOVEL_SETTER_EXCAVATION_FORCE_FEEDBACK(DeformLeftExcavationSettings);
			break;
	}

#undef AGX_SHOVEL_SETTER_EXCAVATION_FORCE_FEEDBACK
}

#if WITH_EDITOR
void UAGX_ShovelProperties::PostInitProperties()
{
	UObject::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShovelProperties::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif

UAGX_ShovelProperties* UAGX_ShovelProperties::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}
	if (Instance.IsValid())
	{
		return Instance.Get();
	}
	if (PlayingWorld == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create runtime instance for Shovel Properties asset '%s' because no "
				 "world to create it in was given."),
			*GetPathName());
		return nullptr;
	}
	if (!PlayingWorld->IsGameWorld())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create Shovel Properties runtime instance for asset '%s' because the "
				 "given world is not a game world."),
			*GetPathName());
		return nullptr;
	}

	const FString InstanceName = GetName() + TEXT("_Instance");
	UAGX_ShovelProperties* NewInstance =
		NewObject<UAGX_ShovelProperties>(GetTransientPackage(), *InstanceName, RF_Transient, this);
	NewInstance->Asset = this;
	Instance = NewInstance;
	return NewInstance;
}

bool UAGX_ShovelProperties::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	return Asset != nullptr;
}

UAGX_ShovelProperties* UAGX_ShovelProperties::GetInstance()
{
	return Instance.Get();
}

UAGX_ShovelProperties* UAGX_ShovelProperties::GetAsset()
{
	return Asset.Get();
}

void UAGX_ShovelProperties::CommitToAsset()
{
	if (IsInstance())
	{
		if (Asset == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Shovel Properties '%s' could not commit to asset because it does not have an "
					 "associtated asset to commit to."),
				*GetName());
			return;
		}

		UEngine::CopyPropertiesForUnrelatedObjects(this, Asset.Get());

		// We have modified an asset. We must mark it dirty so that it gets the asterisk marker
		// in the icon in the Content Browser, is saved if the user does Save All, and so that it
		// is included in the Save? dialog when Unreal Editor is closed.
		//
		// We would like to use UPackage::MarkPackageDirty, but that is rejected during Play In
		// Editor sessions, which is when Commit To Asset is typically called. I hope doing what
		// we do below doesn't break anything. An alternative is to save the package immediately
		// with FAGX_ObjectUtilities::SaveAsset(*Asset);.
		UPackage* Package = Asset->GetPackage();
		if (Package == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Shovel Properties '%s' could not commit to asset because the associated "
					 "asset '%s' does not have a package."),
				*GetName(), *Asset->GetPathName());
			return;
		}
		Package->SetDirtyFlag(true);
		Package->PackageMarkedDirtyEvent.Broadcast(Package, true);
	}
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
}

namespace AGX_ShovelProperties_helpers
{
	/*
	 *  The intention is that shovels should always unregister themselves before being destroyed.
	 *  If we ever find a nullptr shovel in a Shovel Properties then that is a sign that we've
	 * either misunderstood something or that there is something we don't know. Either way,
	 * investigation required.
	 */
	bool CheckNoNullptr(const TArray<TWeakObjectPtr<UAGX_ShovelComponent>>& Shovels)
	{
		for (const TWeakObjectPtr<UAGX_ShovelComponent>& Shovel : Shovels)
		{
			if (Shovel.Get() == nullptr)
			{
				return false;
			}
		}
		return true;
	}
}

void UAGX_ShovelProperties::RegisterShovel(UAGX_ShovelComponent& Shovel)
{
	if (!IsInstance())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shovel Component '%s' in '%s' called Register Shovel on non-instance Shovel "
				 "Properties '%s'."),
			*Shovel.GetName(), *GetLabelSafe(Shovel.GetOwner()), *GetPathName());
		return;
	}
	AGX_CHECK(AGX_ShovelProperties_helpers::CheckNoNullptr(Shovels));
	Shovels.AddUnique(&Shovel);
}

void UAGX_ShovelProperties::UnregisterShovel(UAGX_ShovelComponent& Shovel)
{
	if (!IsInstance())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shovel Component '%s' in '%s' called Unregister Shovel on non-instance Shovel "
				 "Properties '%s'."),
			*Shovel.GetName(), *GetLabelSafe(Shovel.GetOwner()), *GetPathName());
		return;
	}
	AGX_CHECK(AGX_ShovelProperties_helpers::CheckNoNullptr(Shovels));
	Shovels.RemoveSwap(&Shovel);
}

#if WITH_EDITOR
void UAGX_ShovelProperties::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_ASSET_DEFAULT_DISPATCHER(ToothLength);
	AGX_ASSET_DEFAULT_DISPATCHER(ToothMinimumRadius);
	AGX_ASSET_DEFAULT_DISPATCHER(ToothMaximumRadius);
	AGX_ASSET_DEFAULT_DISPATCHER(NumberOfTeeth);
	AGX_ASSET_DEFAULT_DISPATCHER(NoMergeExtensionDistance);
	AGX_ASSET_DEFAULT_DISPATCHER(MinimumSubmergedContactLengthFraction);
	AGX_ASSET_DEFAULT_DISPATCHER(VerticalBladeSoilMergeDistance);
	AGX_ASSET_DEFAULT_DISPATCHER(SecondarySeparationDeadloadLimit);
	AGX_ASSET_DEFAULT_DISPATCHER(PenetrationDepthThreshold);
	AGX_ASSET_DEFAULT_DISPATCHER(PenetrationForceScaling);
	AGX_ASSET_DEFAULT_DISPATCHER(bEnableParticleFreeDeformers);
	AGX_ASSET_DEFAULT_DISPATCHER(bAlwaysRemoveShovelContacts);
	AGX_ASSET_DEFAULT_DISPATCHER(MaximumPenetrationForce);
	AGX_ASSET_DEFAULT_DISPATCHER(ContactRegionThreshold);
	AGX_ASSET_DEFAULT_DISPATCHER(ContactRegionVerticalLimit);
	AGX_ASSET_DEFAULT_DISPATCHER(bEnableInnerShapeCreateDynamicMass);
	AGX_ASSET_DEFAULT_DISPATCHER(bEnableParticleForceFeedback);
	AGX_ASSET_DEFAULT_DISPATCHER(ParticleInclusionMultiplier);

	// When the user toggles an override flag then call the corresponding Set function. The
	// associated value will be propagated if it should. Disabling the override does not mean that
	// the default value is restored, only that new values will not be propagated.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bOverride_ContactRegionThreshold),
		[](ThisClass* This) { This->SetContactRegionThreshold(This->ContactRegionThreshold); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bOverride_ContactRegionVerticalLimit),
		[](ThisClass* This)
		{ This->SetContactRegionVerticalLimit(This->ContactRegionVerticalLimit); });

	// The excavation settings are multiple instances of a struct with multiple members. This lambda
	// function registers property dispatchers for one instance of the struct.
	auto RegisterExcavationSettings =
		[&](const FName& PropertyName, EAGX_ExcavationMode ExcavationMode)
	{
		PropertyDispatcher.Add(
			PropertyName, GET_MEMBER_NAME_CHECKED(FAGX_ShovelExcavationSettings, bEnabled),
			[ExcavationMode](ThisClass* This)
			{
				This->SetExcavationEnabled(
					ExcavationMode, This->GetExcavationSettings(ExcavationMode)->bEnabled);
			});

		PropertyDispatcher.Add(
			PropertyName,
			GET_MEMBER_NAME_CHECKED(FAGX_ShovelExcavationSettings, bEnableCreateDynamicMass),
			[ExcavationMode](ThisClass* This)
			{
				This->SetExcavationCreateDynamicMassEnabled(
					ExcavationMode,
					This->GetExcavationSettings(ExcavationMode)->bEnableCreateDynamicMass);
			});

		PropertyDispatcher.Add(
			PropertyName,
			GET_MEMBER_NAME_CHECKED(FAGX_ShovelExcavationSettings, bEnableForceFeedback),
			[ExcavationMode](ThisClass* This)
			{
				This->SetExcavationForceFeedbackEnabled(
					ExcavationMode,
					This->GetExcavationSettings(ExcavationMode)->bEnableForceFeedback);
			});
	};

	// Call the register lambda for each of the excavation modes.
	RegisterExcavationSettings(
		GET_MEMBER_NAME_CHECKED(ThisClass, PrimaryExcavationSettings),
		EAGX_ExcavationMode::Primary);
	RegisterExcavationSettings(
		GET_MEMBER_NAME_CHECKED(ThisClass, DeformBackExcavationSettings),
		EAGX_ExcavationMode::DeformBack);
	RegisterExcavationSettings(
		GET_MEMBER_NAME_CHECKED(ThisClass, DeformRightExcavationSettings),
		EAGX_ExcavationMode::DeformRight);
	RegisterExcavationSettings(
		GET_MEMBER_NAME_CHECKED(ThisClass, DeformLeftExcavationSettings),
		EAGX_ExcavationMode::DeformLeft);
}
#endif

void UAGX_ShovelProperties::SetbAlwaysRemoveShovelContacts(bool InbAlwaysRemoveShovelContacts)
{
	SetAlwaysRemoveShovelContacts(InbAlwaysRemoveShovelContacts);
}
void UAGX_ShovelProperties::SetbEnableParticleFreeDeformers(bool InbEnableParticleFreeDeformers)
{
	SetEnableParticleFreeDeformers(InbEnableParticleFreeDeformers);
}
void UAGX_ShovelProperties::SetbEnableInnerShapeCreateDynamicMass(
	bool InbEnableInnerShapeCreateDynamicMass)
{
	SetEnableInnerShapeCreateDynamicMass(InbEnableInnerShapeCreateDynamicMass);
}
void UAGX_ShovelProperties::SetbEnableParticleForceFeedback(bool InbEnableParticleForceFeedback)
{
	SetEnableParticleForceFeedback(InbEnableParticleForceFeedback);
}

namespace AGX_ShovelProperties_helpers
{
	FString CreatePropertiesName(const FShovelBarrier& Barrier, FAGX_ImportContext& Context)
	{
		auto Shovel = Context.Shovels->FindRef(Barrier.GetGuid());
		const FString BaseName = Shovel != nullptr ? Shovel->GetName() : "Unknown";
		const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			Context.Outer, FString::Printf(TEXT("AGX_SP_%s"), *BaseName),
			UAGX_ShovelProperties::StaticClass());

		if (Shovel == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to get Shovel from FAGX_ImportContext in "
					 "AGX_ShovelProperties_helpers::CreatePropertiesName. The ShovelProperties "
					 "will get the name '%s'."), *Name);
		}

		return Name;
	}
}

void UAGX_ShovelProperties::CopyFrom(const FShovelBarrier& Barrier, FAGX_ImportContext* Context)
{
	ImportGuid = Barrier.GetGuid();
	ToothLength = Barrier.GetToothLength();
	ToothMinimumRadius = Barrier.GetToothMinimumRadius();
	ToothMaximumRadius = Barrier.GetToothMaximumRadius();
	NumberOfTeeth = Barrier.GetNumberOfTeeth();
	NoMergeExtensionDistance = Barrier.GetNoMergeExtensionDistance();
	MinimumSubmergedContactLengthFraction = Barrier.GetMinimumSubmergedContactLengthFraction();
	VerticalBladeSoilMergeDistance = Barrier.GetVerticalBladeSoilMergeDistance();
	SecondarySeparationDeadloadLimit = Barrier.GetSecondarySeparationDeadloadLimit();
	PenetrationDepthThreshold = Barrier.GetPenetrationDepthThreshold();
	PenetrationForceScaling = Barrier.GetPenetrationForceScaling();
	bEnableParticleFreeDeformers = Barrier.GetEnableParticleFreeDeformers();
	bAlwaysRemoveShovelContacts = Barrier.GetAlwaysRemoveShovelContacts();
	MaximumPenetrationForce = Barrier.GetMaximumPenetrationForce();
	bOverride_ContactRegionThreshold = true;
	ContactRegionThreshold = Barrier.GetContactRegionThreshold();
	bOverride_ContactRegionVerticalLimit = true;
	ContactRegionVerticalLimit = Barrier.GetContactRegionVerticalLimit();
	bEnableInnerShapeCreateDynamicMass = Barrier.GetEnableInnerShapeCreateDynamicMass();
	bEnableParticleForceFeedback = Barrier.GetEnableParticleForceFeedback();
	ParticleInclusionMultiplier = Barrier.GetParticleInclusionMultiplier();

	auto CopyExcavationSettings =
		[&Barrier](EAGX_ExcavationMode Mode, FAGX_ShovelExcavationSettings& Settings)
	{
		Settings.bEnabled = Barrier.GetExcavationSettingsEnabled(Mode);
		Settings.bEnableCreateDynamicMass =
			Barrier.GetExcavationSettingsEnableCreateDynamicMass(Mode);
		Settings.bEnableForceFeedback = Barrier.GetExcavationSettingsEnableForceFeedback(Mode);
	};

	CopyExcavationSettings(EAGX_ExcavationMode::Primary, PrimaryExcavationSettings);
	CopyExcavationSettings(EAGX_ExcavationMode::DeformBack, DeformBackExcavationSettings);
	CopyExcavationSettings(EAGX_ExcavationMode::DeformRight, DeformRightExcavationSettings);
	CopyExcavationSettings(EAGX_ExcavationMode::DeformLeft, DeformLeftExcavationSettings);

	if (Context == nullptr || Context->Shovels == nullptr || Context->ShovelProperties == nullptr)
		return; // We are done.

	Rename(*AGX_ShovelProperties_helpers::CreatePropertiesName(Barrier, *Context));
	AGX_CHECK(!Context->ShovelProperties->Contains(ImportGuid));
	Context->ShovelProperties->Add(ImportGuid, this);
}
