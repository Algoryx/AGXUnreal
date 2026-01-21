// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Terrain/AGX_TerrainWheelMaterial.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

#define LOCTEXT_NAMESPACE "AGX_TerrainWheelComponent"

namespace AGX_TerrainWheelComponent_helpers
{
	void SetLocalScope(UAGX_TerrainWheelComponent& TerrainWheel)
	{
		AActor* Owner = FAGX_ObjectUtilities::GetRootParentActor(TerrainWheel);
		TerrainWheel.RigidBody.LocalScope = Owner;
	}
}

UAGX_TerrainWheelComponent::UAGX_TerrainWheelComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	AGX_TerrainWheelComponent_helpers::SetLocalScope(*this);
}

bool UAGX_TerrainWheelComponent::SetTerrainWheelMaterial(UAGX_TerrainWheelMaterial* Material)
{
	UAGX_TerrainWheelMaterial* TerrainWheelMaterialOrig = TerrainWheelMaterial;
	TerrainWheelMaterial = Material;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeMaterial is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeTerrainWheelMaterial())
	{
		// Something went wrong, restore original TerrainWheelMaterial.
		TerrainWheelMaterial = TerrainWheelMaterialOrig;
		return false;
	}

	return true;
}

void UAGX_TerrainWheelComponent::SetTerrainDeformationEnabled(bool InEnable)
{
	bEnableTerrainDeformation = InEnable;

	if (HasNative())
		NativeBarrier.SetEnableTerrainDeformation(InEnable);
}

bool UAGX_TerrainWheelComponent::IsTerrainDeformationEnabled() const
{
	if (HasNative())
		return NativeBarrier.GetEnableTerrainDeformation();

	return bEnableTerrainDeformation;
}

void UAGX_TerrainWheelComponent::SetTerrainDisplacementEnabled(bool InEnable)
{
	bEnableTerrainDisplacement = InEnable;

	if (HasNative())
		NativeBarrier.SetEnableTerrainDisplacement(InEnable);
}

bool UAGX_TerrainWheelComponent::IsTerrainDisplacementEnabled() const
{
	if (HasNative())
		return NativeBarrier.GetEnableTerrainDisplacement();

	return bEnableTerrainDisplacement;
}

void UAGX_TerrainWheelComponent::SetAGXDebugRenderingEnabled(bool InEnable)
{
	bEnableAGXDebugRendering = InEnable;
	if (HasNative())
		NativeBarrier.SetEnableAGXDebugRendering(InEnable);
}

void UAGX_TerrainWheelComponent::SetEnableTerrainDeformation(bool InEnable)
{
	SetTerrainDeformationEnabled(InEnable);
}

void UAGX_TerrainWheelComponent::SetEnableTerrainDisplacement(bool InEnable)
{
	SetTerrainDisplacementEnabled(InEnable);
}

void UAGX_TerrainWheelComponent::SetEnableAGXDebugRendering(bool InEnable)
{
	SetAGXDebugRenderingEnabled(InEnable);
}

void UAGX_TerrainWheelComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// This code is run after the constructor and after Init Properties, where property values are
	// copied from the Class Default Object, but before deserialization in cases where this object
	// is created from another, such as at the start of a Play-in-Editor session or when loading
	// a map in a cooked build (I hope).
	AGX_TerrainWheelComponent_helpers::SetLocalScope(*this);

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_TerrainWheelComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif

void UAGX_TerrainWheelComponent::OnRegister()
{
	Super::OnRegister();
	AGX_TerrainWheelComponent_helpers::SetLocalScope(*this);
}

void UAGX_TerrainWheelComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
		return;

	if (!HasNative())
		CreateNative();

	if (!HasNative())
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Unable to create Native TerrainWheel for '%s' in '%s', Output Log may "
					 "contain more information."),
				*GetName(), *GetNameSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return;
	}
}

void UAGX_TerrainWheelComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_TerrainWheelComponent will inherit this one's Native, so don't wreck it.
		// The call to NativeBarrier.ReleaseNative below is safe because the AGX Dynamics
		// Simulation will retain a reference counted pointer to the AGX Dynamics Observer
		// Frame.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
			Sim->Remove(*this);
	}

	if (HasNative())
		NativeBarrier.ReleaseNative();
}

TStructOnScope<FActorComponentInstanceData> UAGX_TerrainWheelComponent::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

bool UAGX_TerrainWheelComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_TerrainWheelComponent::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	NativeBarrier.IncrementRefCount();
	return NativeBarrier.GetNativeAddress();
}

void UAGX_TerrainWheelComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(NativeAddress);
	NativeBarrier.DecrementRefCount();
}

FTerrainWheelBarrier* UAGX_TerrainWheelComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're
			// in the middle of a RerunConstructionScripts and this Component haven't been given
			// its Native yet then there isn't much we can do. We can't create a new one since
			// we will be given the actual Native soon, but we also can't return the actual
			// Native right now because it hasn't been restored from the Component Instance Data
			// yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will
			// behave as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for TerrainWheel '%s' in '%s' was "
					 "made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, "
					 "which may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}
		CreateNative();
	}

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("'%s' in '%s': Could not allocate AGX Dynamics TerrainWheel in "
				 "UAGX_TerrainWheelComponent::GetOrCreateNative, nullptr will be returned to "
				 "caller."),
			*GetName(), *GetLabelSafe(GetOwner()));
	}

	return GetNative();
}

FTerrainWheelBarrier* UAGX_TerrainWheelComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FTerrainWheelBarrier* UAGX_TerrainWheelComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

#if WITH_EDITOR
void UAGX_TerrainWheelComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableTerrainDeformation);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableTerrainDisplacement);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableAGXDebugRendering);

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TerrainWheelComponent, TerrainWheelMaterial),
		[](ThisClass* Self) { Self->UpdateNativeTerrainWheelMaterial(); });
}
bool UAGX_TerrainWheelComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, RigidBody)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
}
#endif // WITH_EDITOR

void UAGX_TerrainWheelComponent::CreateNative()
{
	if (HasNative())
		return;

	auto ShowFailNotification = [this]()
	{
		const FString Msg = FString::Printf(
			TEXT("Unable to create Terrain Wheel for '%s' in '%s'. The Output Log "
				 "may contain more details."),
			*GetName(), *GetNameSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
	};

	UAGX_RigidBodyComponent* Body = RigidBody.GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Rigid Body selected for '%s' in '%s'."), *GetName(),
			*GetNameSafe(GetOwner()));
		ShowFailNotification;
		return;
	}

	const TArray<UAGX_CylinderShapeComponent*> Cylinders =
		FAGX_ObjectUtilities::GetChildrenOfType<UAGX_CylinderShapeComponent>(*Body, false);
	if (Cylinders.Num() != 1)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Expected excactly 1 Cylinder Shape in RigidBody for '%s' in '%s', but found %d."), *GetName(),
			*GetNameSafe(GetOwner()), Cylinders.Num());
		ShowFailNotification;
		return;
	}

	FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
	if (BodyBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to get Rigid Body Barrier for RigidBody in '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		ShowFailNotification;
		return;
	}

	FCylinderShapeBarrier* CylinderBarrier =
		static_cast<FCylinderShapeBarrier*>(Cylinders[0]->GetNative());
	if (CylinderBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to get Cylinder Shape Barrier from Cylinder in Body selected in '%s' in '%s'."), *GetName(),
			*GetNameSafe(GetOwner()));
		ShowFailNotification;
		return;
	}

	NativeBarrier.AllocateNative(*BodyBarrier, *CylinderBarrier);
	if (!HasNative())
	{
		ShowFailNotification;
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to create Native Terrain Wheel for '%s' in '%s'. The Output Log may "
				 "contain more details."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	NativeBarrier.SetEnableTerrainDeformation(bEnableTerrainDeformation);
	NativeBarrier.SetEnableTerrainDisplacement(bEnableTerrainDisplacement);
	NativeBarrier.SetEnableAGXDebugRendering(bEnableAGXDebugRendering);
	UpdateNativeTerrainWheelMaterial();

	if (auto Sim = UAGX_Simulation::GetFrom(this))
		Sim->Add(*this);
}

bool UAGX_TerrainWheelComponent::UpdateNativeTerrainWheelMaterial()
{
	if (!HasNative())
		return false;

	if (TerrainWheelMaterial == nullptr)
	{
		if (HasNative())
			GetNative()->SetTerrainWheelMaterialToDefault();

		return true;
	}

	UWorld* World = GetWorld();
	UAGX_TerrainWheelMaterial* Instance =
		static_cast<UAGX_TerrainWheelMaterial*>(TerrainWheelMaterial->GetOrCreateInstance(World));
	check(Instance);

	if (TerrainWheelMaterial != Instance)
		TerrainWheelMaterial = Instance;

	FTerrainWheelMaterialBarrier* MaterialBarrier = Instance->GetOrCreateNative();
	check(MaterialBarrier);
	GetNative()->SetTerrainWheelMaterial(*MaterialBarrier);
	return true;
}

#undef LOCTEXT_NAMESPACE
