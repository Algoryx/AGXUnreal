// Copyright 2024, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_Environment.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_TrackProperties.h"
#include "Vehicle/TrackPropertiesBarrier.h"

// Unreal Engine includes.
#include "Components/InstancedStaticMeshComponent.h"
#include "CoreGlobals.h"
#include "Engine/GameInstance.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Math/Quat.h"

#define LOCTEXT_NAMESPACE "AGX_TrackRenderer"

UAGX_TrackComponent::UAGX_TrackComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bWantsOnUpdateTransform = true;

	SetComponentReferencesLocalScope();

	static const TCHAR* DefaultMeshPath =
		TEXT("StaticMesh'/AGXUnreal/Track/StaticMeshes/SM_TrackShoeCube.SM_TrackShoeCube'");
	if (RenderMesh == nullptr)
		RenderMesh = FAGX_ObjectUtilities::GetAssetFromPath<UStaticMesh>(DefaultMeshPath);

	static const TCHAR* DefaultMatPath =
		TEXT("Material'/AGXUnreal/Track/Materials/MI_TrackDefault.MI_TrackDefault'");
	if (RenderMaterials.Num() == 0)
		RenderMaterials.Add(
			FAGX_ObjectUtilities::GetAssetFromPath<UMaterialInterface>(DefaultMatPath));
}

FAGX_TrackPreviewData* UAGX_TrackComponent::GetTrackPreview(bool bForceUpdate) const
{
	// Avoid getting Track Preview if no valid license is available since this will spam license
	// errors in the log.
	if (!MayAttemptTrackPreview)
	{
		MayAttemptTrackPreview = FAGX_Environment::GetInstance().EnsureAGXDynamicsLicenseValid();
		if (!MayAttemptTrackPreview)
		{
			return nullptr;
		}
	}

	if (IsBeingDestroyed() || !bEnabled)
	{
		return nullptr;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		return nullptr; // no preview while playing
	}

	if (!TrackPreview.IsValid() || bTrackPreviewNeedsUpdate || bForceUpdate)
	{
		// Generate track preview data, without actually creating a real track or bodies.

		if (!TrackPreview.IsValid())
		{
			TrackPreview = MakeShared<FAGX_TrackPreviewData>();
		}

		// Create AGX barrier wheel descs.
		TArray<FTrackBarrier::FTrackWheelDescription> WheelDescs;
		WheelDescs.Reserve(Wheels.Num());
		for (const auto& Wheel : Wheels)
		{
			UAGX_RigidBodyComponent* Body = Wheel.RigidBody.GetRigidBody();
			if (!Body)
				continue;

			// Make sure the world transform is up-to-date.
			Body->ConditionalUpdateComponentToWorld();

			// Create wheel data.
			FTrackBarrier::FTrackWheelDescription Desc;
			Desc.Model = static_cast<decltype(Desc.Model)>(Wheel.Model);
			Desc.Radius = Wheel.Radius;
			Desc.RigidBodyTransform = Body->GetComponentTransform();
			Wheel.GetTransformRelativeToBody(Desc.RelativePosition, Desc.RelativeRotation);
			WheelDescs.Add(Desc);
		}

		// Let AGX generate track nodes preview data.
		FTrackBarrier::GetPreviewData(
			TrackPreview->NodeTransforms, TrackPreview->NodeHalfExtents, NumberOfNodes, Width,
			Thickness, InitialDistanceTension, WheelDescs);

		bTrackPreviewNeedsUpdate = false;
	}

	check(TrackPreview.IsValid());
	return TrackPreview.Get();
}

UInstancedStaticMeshComponent* UAGX_TrackComponent::GetVisualMeshes()
{
	return VisualMeshes;
}

void UAGX_TrackComponent::SetRenderMesh(UStaticMesh* Mesh)
{
	for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*this))
	{
		if (Instance->RenderMesh == RenderMesh)
		{
			Instance->SetRenderMesh(Mesh);
		}
	}

	RenderMesh = Mesh;
	RenderMaterials.Empty();
	for (const FStaticMaterial& SM : Mesh->GetStaticMaterials())
		RenderMaterials.Add(SM.MaterialInterface);

	if (VisualMeshes != nullptr)
	{
		VisualMeshes->SetStaticMesh(RenderMesh);
		WriteRenderMaterialsToVisualMesh();
	}
}

void UAGX_TrackComponent::SetRenderMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	if (!RenderMaterials.IsValidIndex(ElementIndex))
		return;

	RenderMaterials[ElementIndex] = Material;
	WriteRenderMaterialsToVisualMesh();
}

void UAGX_TrackComponent::RaiseTrackPreviewNeedsUpdate(bool bDoNotBroadcastIfAlreadyRaised)
{
	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		return; // Track preview is only relevant when not playing.
	}

	bool bShouldBroadcastEvent = !bTrackPreviewNeedsUpdate || !bDoNotBroadcastIfAlreadyRaised;
	bTrackPreviewNeedsUpdate = true;
	UpdateVisuals();

	if (bShouldBroadcastEvent)
	{
		TrackPreviewNeedsUpdateEvent.Broadcast(this);
	}
}

void UAGX_TrackComponent::CopyFrom(const FTrackBarrier& Barrier)
{
	NumberOfNodes = Barrier.GetNumNodes();
	Width = static_cast<float>(Barrier.GetWidth());
	Thickness = static_cast<float>(Barrier.GetThickness());
	InitialDistanceTension = static_cast<float>(Barrier.GetInitialDistanceTension());
	CollisionGroups = Barrier.GetCollisionGroups();
	ImportGuid = Barrier.GetGuid();

	if (Barrier.GetNumNodes() > 0)
	{
		FRigidBodyBarrier FirstBodyBarrier = Barrier.GetNodeBody(0);
		if (FirstBodyBarrier.HasNative())
		{
			const FMassPropertiesBarrier& MassProperties = FirstBodyBarrier.GetMassProperties();

			bAutoGenerateMass = MassProperties.GetAutoGenerateMass();
			bAutoGenerateCenterOfMassOffset = MassProperties.GetAutoGenerateCenterOfMassOffset();
			bAutoGeneratePrincipalInertia = MassProperties.GetAutoGeneratePrincipalInertia();
			NodeMass = MassProperties.GetMass();
			NodeCenterOfMassOffset = FirstBodyBarrier.GetCenterOfMassOffset();
			NodePrincipalInertia = MassProperties.GetPrincipalInertia();
		}
	}
}

int32 UAGX_TrackComponent::GetNumNodes() const
{
	if (HasNative())
	{
		return GetNative()->GetNumNodes();
	}
	else if (TrackPreview.IsValid())
	{
		return TrackPreview->NodeTransforms.Num();
	}
	else
	{
		return NumberOfNodes;
	}
}

void UAGX_TrackComponent::GetNodeTransforms(
	TArray<FTransform>& OutTransforms, const FVector& LocalScale, const FVector& LocalOffset,
	const FQuat& LocalRotation) const
{
	if (HasNative())
	{
		GetNative()->GetNodeTransforms(OutTransforms, LocalScale, LocalOffset, LocalRotation);
	}
	else if (TrackPreview.IsValid())
	{
		const int32 NumNodes = TrackPreview->NodeTransforms.Num();
		// Retain the container buffer so that the same transform cache can be reused for multiple
		// tracks without reallocation every time.
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutTransforms.SetNum(NumNodes, /*bAllowShrinking*/ false);
#else
		OutTransforms.SetNum(NumNodes, EAllowShrinking::No);
#endif
		for (int32 I = 0; I < NumNodes; ++I)
		{
			const FTransform& Source = TrackPreview->NodeTransforms[I];
			FTransform& Target = OutTransforms[I];
			Target.SetScale3D(LocalScale);
			Target.SetRotation(Source.GetRotation() * LocalRotation);
			const FVector WorldOffset = Source.GetRotation().RotateVector(LocalOffset);
			Target.SetLocation(Source.GetLocation() + WorldOffset);
		}
	}
	else
	{
		// Retain the container buffer so that the same transform cache can be reused for multiple
		// tracks without reallocation every time.
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutTransforms.SetNum(0, /*bAllowShrinking*/ false);
#else
		OutTransforms.SetNum(0, EAllowShrinking::No);
#endif
	}
}

void UAGX_TrackComponent::GetNodeSizes(TArray<FVector>& OutNodeSizes) const
{
	if (!HasNative())
	{
		return;
	}

	GetNative()->GetNodeSizes(OutNodeSizes);
}

FVector UAGX_TrackComponent::GetNodeSize(int32 Index) const
{
	if (!HasNative())
	{
		return FVector::ZeroVector;
	}

	return GetNative()->GetNodeSize(Index);
}

FTrackBarrier* UAGX_TrackComponent::GetOrCreateNative()
{
	if (!HasNative() && bEnabled)
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Track '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}

	return GetNative();
}

FTrackBarrier* UAGX_TrackComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FTrackBarrier* UAGX_TrackComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

bool UAGX_TrackComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_TrackComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_TrackComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

void UAGX_TrackComponent::PostInitProperties()
{
	Super::PostInitProperties();
	SetComponentReferencesLocalScope();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR

bool UAGX_TrackComponent::CanEditChange(const FProperty* InProperty) const
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
			GET_MEMBER_NAME_CHECKED(ThisClass, bEnabled),
			GET_MEMBER_NAME_CHECKED(ThisClass, NumberOfNodes),
			GET_MEMBER_NAME_CHECKED(ThisClass, Width),
			GET_MEMBER_NAME_CHECKED(ThisClass, Thickness),
			GET_MEMBER_NAME_CHECKED(ThisClass, InitialDistanceTension)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
}

void UAGX_TrackComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TrackComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// \note We trigger dispatch both here and from PostEditChangeChainProperty, because
	// for example when editing a mass property while playing on a BP Actor Instance the
	// PostEditChangeChainProperty appears to be called too late in the reconstruction.
	//
	// \todo Check if this problem was "fixed" with merge request !583 - Call the Property Changed
	// callback on all selected objects.
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(PropertyChangedEvent);

	// Track Preview needs update if any of NumberOfNodes, Width, Thickness, or
	// InitialDistanceTension has changed.
	if (bAutoUpdateTrackPreview)
	{
		RaiseTrackPreviewNeedsUpdate();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}

#endif

void UAGX_TrackComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	SetComponentReferencesLocalScope();
}

void UAGX_TrackComponent::PostLoad()
{
	Super::PostLoad();

	// During load the Wheels array may be been modified with data restored from serialization or an
	// archetype. Make sure all Component References in those array elements have the correct Local
	// Scope.
	SetComponentReferencesLocalScope();

	RaiseTrackPreviewNeedsUpdate();
	UpdateVisuals();
}

void UAGX_TrackComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!HasNative() && bEnabled && !GIsReconstructingBlueprintInstances)
	{
		// Do not create a native AGX Dynamics object if GIsReconstructingBlueprintInstances is set.
		// That means that we're being created as part of a Blueprint Reconstruction and we will
		// soon be assigned the native that the reconstructed Track Component had, if any, in
		// ApplyComponentInstanceData.
		CreateNative();
		check(HasNative()); /// @todo Consider better error handling than check.
	}
}

void UAGX_TrackComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_TrackComponent will inherit this one's Native, so don't wreck it.
		// The call to NativeBarrier.ReleaseNative below is safe because the AGX Dynamics Simulation
		// will retain a reference counted pointer to the AGX Dynamics Track.
		//
		// But what if the Track isn't currently part of any Simulation? Can we guarantee that
		// something will keep the Track instance alive? Should we do explicit incref/decref
		// on the Track in GetNativeAddress / SetNativeAddress?
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		// This object is being destroyed / removed from a Play session that will continue without
		// it, so there will be no global cleanup of everything, so we must cleanup after ourself.
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			if (!Sim->HasNative())
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Track '%s' in '%s' tried to get Simulation, but returned simulation has "
						 "no native."),
					*GetName(), *GetNameSafe(GetOwner()));
				return;
			}

			// \todo Want to use AAGX_Simulation::Remove, but there's no overload taking a
			// TrackComponent
			//       (or LinkedStructure or Assembly), so we use a work-around in the TrackBarrier.
			const bool Result = GetNative()->RemoveFromSimulation(*Sim->GetNative());
		}
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_TrackComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_TrackComponentInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

void UAGX_TrackComponent::ApplyComponentInstanceData(
	const FActorComponentInstanceData* Data, ECacheApplyPhase CacheApplyPhase)
{
	if (CacheApplyPhase == ECacheApplyPhase::PostUserConstructionScript)
	{
		// In the case of BP Actor Instances, there can be wheels added to the instance
		// in the level in addition to the wheels on the CDO. During BP instance reconstruction,
		// those additional wheels are not added until instance has been fully deserialized.
		// Therefore, we here make sure that the resolving of OwningActor in RigidBodyReference
		// and SceneComponentReference is done for those additional wheels.
		SetComponentReferencesLocalScope();

		// Call this to re-register this reconstructed track component to
		// UAGX_TrackInternalMergeProperties.
		WriteInternalMergePropertiesToNative();

		// Mark the Track Preview Data for update after all BP instance data has been deserialized.
		if (bAutoUpdateTrackPreview)
		{
			RaiseTrackPreviewNeedsUpdate();
		}
	}
}

void UAGX_TrackComponent::OnRegister()
{
	Super::OnRegister();

	// If a Blueprint Construction Script modifies the Wheels array then this is the first chance
	// we have to catch any new elements and set the correct Local Scope on the Component
	// References.
	SetComponentReferencesLocalScope();

	if (VisualMeshes == nullptr)
		CreateVisuals();
	UpdateVisuals();
}

void UAGX_TrackComponent::DestroyComponent(bool bPromoteChildren)
{
	if (VisualMeshes != nullptr)
		VisualMeshes->DestroyComponent();

	Super::DestroyComponent(bPromoteChildren);
}

void UAGX_TrackComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateVisuals();
}

#if WITH_EDITOR

void UAGX_TrackComponent::OnUpdateTransform(
	EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);

	// \todo This event does not seem to be called when drag-moving an actor/component,
	//       but not when writing values directly in the Detail Panel transform input fields.

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (!bIsPlaying)
	{
		// \note Actually moving the TransformComponent does not itself means that track preview
		//       needs update, but it's likely that this happened because the owning actor was
		//       moved, which usually means that all wheels moved.
		if (bAutoUpdateTrackPreview)
		{
			RaiseTrackPreviewNeedsUpdate(true);
		}
	}
}

void UAGX_TrackComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, Wheels),
		[](ThisClass* Self) { Self->SetComponentReferencesLocalScope(); });

	// Assets.

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, ShapeMaterial),
		[](ThisClass* Self) { Self->UpdateNativeMaterial(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, TrackProperties),
		[](ThisClass* Self) { Self->WriteTrackPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, InternalMergeProperties),
		[](ThisClass* Self) { Self->WriteInternalMergePropertiesToNative(); });

	// Mass Properties.

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, NodeMass),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, bAutoGenerateMass),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, NodeCenterOfMassOffset),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, bAutoGenerateCenterOfMassOffset),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, NodePrincipalInertia),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, bAutoGeneratePrincipalInertia),
		[](ThisClass* Self) { Self->WriteMassPropertiesToNative(); });

	// Visuals

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, RenderMaterials),
		[](ThisClass* Track) { Track->WriteRenderMaterialsToVisualMeshWithCheck(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_TrackComponent, RenderMesh),
		[](ThisClass* Track) { Track->SetRenderMesh(Track->RenderMesh); });
}

#endif

void UAGX_TrackComponent::SetComponentReferencesLocalScope()
{
	AActor* Owner = FAGX_ObjectUtilities::GetRootParentActor(this);
	for (FAGX_TrackWheel& Wheel : Wheels)
	{
		Wheel.RigidBody.LocalScope = Owner;
		Wheel.FrameDefiningComponent.LocalScope = Owner;
	}
}

void UAGX_TrackComponent::CreateNative()
{
	// Check preconditions.
	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNative called on Track Component '%s' in '%s' even though a Native already "
				 "exists. Doing nothing"),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}
	if (!bEnabled)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNative called on disabled Track Component '%s' in '%s'. Doing nothing."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}
	if (GIsReconstructingBlueprintInstances)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNative called on a Track Component while a Blueprint Reconstruction is in "
				 "progress. This is not allowed because during Blueprint Reconstruction AGX "
				 "Dynamic objects are stashed in an Actor Component Instance Data and will be "
				 "reused in the new AGX Actor Components. The Track Component is '%s' in '%s'. "
				 "Doing nothing."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	NativeBarrier.AllocateNative(NumberOfNodes, Width, Thickness, InitialDistanceTension);
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Track Component '%s' in '%s' could not allocate native AGX Dynamics instance."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this);
	if (!IsValid(Sim) || !Sim->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Track '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned a nullptr or a simulation without a native."),
			*GetName(), *GetNameSafe(GetOwner()));
		return;
	}

	SetComponentReferencesLocalScope();
	for (FAGX_TrackWheel& Wheel : Wheels)
	{
		// Validate and get the Rigid Body Component.
		UAGX_RigidBodyComponent* Body = Wheel.RigidBody.GetRigidBody();
		if (!IsValid(Body) || Body->GetOrCreateNative() == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Track initialization for '%s' in '%s'' encountered a track wheel with a "
					 "Rigid Body Component or native that is null or invalid. The wheel will be "
					 "ignored."),
				*GetName(), *GetNameSafe(GetOwner()));
			continue;
		}

		// Compute wheel position and rotation relative to the body.
		FVector RelPos;
		FQuat RelRot;
		bool transformOK = Wheel.GetTransformRelativeToBody(RelPos, RelRot);
		check(transformOK);

		// Add native wheel to native Track.
		NativeBarrier.AddTrackWheel(
			static_cast<uint8>(Wheel.Model), Wheel.Radius, *Body->GetOrCreateNative(), RelPos,
			RelRot, Wheel.bSplitSegments, Wheel.bMoveNodesToRotationPlane, Wheel.bMoveNodesToWheel);
	}

	// Set TrackProperties BEFORE adding track to simulation (i.e. triggering track initialization),
	// because some properties in TrackProperties affects the track initialization algorithm.
	WriteTrackPropertiesToNative();

	// \todo Want to use AAGX_Simulation::Add, but there's no overload taking a Track Component
	//       (or LinkedStructure or Assembly), so we use a work-around in the TrackBarrier.
	const bool Result = GetNative()->AddToSimulation(*Sim->GetNative());
	if (!Result)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to add '%s' in '%s' to Simulation. Add() returned false. "
				 "The Log category AGXDynamicsLog may contain more information about the failure."),
			*GetName(), *GetNameSafe(GetOwner()));
	}

	UpdateNativeProperties();
}

void UAGX_TrackComponent::UpdateNativeMaterial()
{
	if (!HasNative() || !GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	if (ShapeMaterial == nullptr)
	{
		GetNative()->ClearMaterial();
		return;
	}

	// Create instance if necessary.
	UAGX_ShapeMaterial* MaterialInstance =
		static_cast<UAGX_ShapeMaterial*>(ShapeMaterial->GetOrCreateInstance(GetWorld()));
	check(MaterialInstance);
	// Replace asset reference with instance reference.
	if (ShapeMaterial != MaterialInstance)
	{
		ShapeMaterial = MaterialInstance;
	}

	const FShapeMaterialBarrier* MaterialBarrier =
		MaterialInstance->GetOrCreateShapeMaterialNative(GetWorld());
	check(MaterialBarrier);

	// Assign native.
	GetNative()->SetMaterial(*MaterialBarrier);
}

void UAGX_TrackComponent::WriteTrackPropertiesToNative()
{
	if (!HasNative() || GetWorld() == nullptr || !GetWorld()->IsGameWorld())
	{
		return;
	}

	if (TrackProperties)
	{
		// Create instance if necessary.
		UAGX_TrackProperties* TrackPropertiesInstance =
			TrackProperties->GetOrCreateInstance(GetWorld());
		check(TrackPropertiesInstance);

		// Replace asset reference with instance reference.
		if (TrackPropertiesInstance != TrackProperties)
		{
			TrackProperties = TrackPropertiesInstance;
		}

		// Assign native.
		const FTrackPropertiesBarrier* TrackPropertiesBarrier =
			TrackPropertiesInstance->GetOrCreateNative();
		check(TrackPropertiesBarrier);
		GetNative()->SetProperties(*TrackPropertiesBarrier);
	}
	else
	{
		GetNative()->ClearProperties();
	}
}

void UAGX_TrackComponent::WriteInternalMergePropertiesToNative()
{
	if (!HasNative() || !GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	const UWorld& World = *GetWorld();

	// \todo If InternalMergeProperties was switched out or set to None during Play, unregister this
	//       track from the previously set InternalMergePropertiesInstance. It is not strictly
	//       required though because UAGX_TrackInternalMergeProperties checks if registered target
	//       tracks point back to itself before applying any changes to the native.

	//// \todo Do this in a more performance friendly way. For example we could keep a reference
	////       to the previous InternalMergeProperties, or unregister from within PreEditChange()?
	// for (TObjectIterator<UAGX_TrackInternalMergeProperties> It; It; ++It)
	//{
	//	if (It->GetWorld() == GetWorld())
	//	{
	//		It->UnregisterTargetTrack(this);
	//	}
	//}

	if (InternalMergeProperties)
	{
		// Create instance if necessary.
		UAGX_TrackInternalMergeProperties* InternalMergePropertiesInstance =
			static_cast<UAGX_TrackInternalMergeProperties*>(
				InternalMergeProperties->GetOrCreateInstance(World));
		check(InternalMergePropertiesInstance);

		// Replace asset reference with instance reference.
		if (InternalMergePropertiesInstance != InternalMergeProperties)
		{
			InternalMergeProperties = InternalMergePropertiesInstance;
		}

		// Register this track as one of the target tracks.
		InternalMergePropertiesInstance->RegisterTargetTrack(this);
	}
	else
	{
		// \todo Want to call TrackInternalMergeProperties::resetToDefault(), but doing so gives
		//       very strange dynamics behaviour. It seems merge is supposed to become disbled but
		//       it still remains enabled somehow.
		//       Because of this, instead of reset we just set disabled here, which from a
		//       user perspective should lead to desired behaviour.
		GetNative()->InternalMergeProperties_SetEnableMerge(false);
	}
}

void UAGX_TrackComponent::WriteMassPropertiesToNative()
{
	if (!HasNative() || !GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	const int NumNodes = NativeBarrier.GetNumNodes();
	for (int i = 0; i < NumNodes; ++i)
	{
		FRigidBodyBarrier BodyBarrier = NativeBarrier.GetNodeBody(i);
		check(BodyBarrier.HasNative());

		FMassPropertiesBarrier& MassProperties = BodyBarrier.GetMassProperties();

		MassProperties.SetAutoGenerateMass(bAutoGenerateMass);
		MassProperties.SetAutoGenerateCenterOfMassOffset(bAutoGenerateCenterOfMassOffset);
		MassProperties.SetAutoGeneratePrincipalInertia(bAutoGeneratePrincipalInertia);

		if (!bAutoGenerateMass)
		{
			MassProperties.SetMass(NodeMass);
		}
		if (!bAutoGenerateCenterOfMassOffset)
		{
			BodyBarrier.SetCenterOfMassOffset(NodeCenterOfMassOffset);
		}
		if (!bAutoGeneratePrincipalInertia)
		{
			MassProperties.SetPrincipalInertia(NodePrincipalInertia);
		}

		// Make sure mass properties are up-to-date with respect to the auto-generate
		// options set above. This is important because merely setting auto-generate flags
		// does not trigger an update of mass properties.
		BodyBarrier.UpdateMassProperties();
	}

	// Update UI with auto-generated values.
	if (NumNodes > 0)
	{
		FRigidBodyBarrier FirstBodyBarrier = NativeBarrier.GetNodeBody(0);
		check(FirstBodyBarrier.HasNative());

		const FMassPropertiesBarrier& MassProperties = FirstBodyBarrier.GetMassProperties();

		if (bAutoGenerateMass)
		{
			NodeMass = MassProperties.GetMass();
		}
		if (bAutoGenerateCenterOfMassOffset)
		{
			NodeCenterOfMassOffset = FirstBodyBarrier.GetCenterOfMassOffset();
		}
		if (bAutoGeneratePrincipalInertia)
		{
			NodePrincipalInertia = MassProperties.GetPrincipalInertia();
		}
	}
}

void UAGX_TrackComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		return;
	}

	NativeBarrier.SetName(GetName());

	// Set shape material on all native geometries.
	UpdateNativeMaterial();

	// Set track properties.
	WriteTrackPropertiesToNative();

	// Set track internal merge properties.
	WriteInternalMergePropertiesToNative();

	// Set collision groups on native.
	for (const FName& Group : CollisionGroups)
	{
		NativeBarrier.AddCollisionGroup(Group);
	}

	//  Set mass, center of mass, inertia tensor, and auto-gen properties on native rigid bodies.
	WriteMassPropertiesToNative();
}

void UAGX_TrackComponent::CreateVisuals()
{
	VisualMeshes = NewObject<UInstancedStaticMeshComponent>(this, FName(TEXT("VisualMeshes")));
	VisualMeshes->SetCanEverAffectNavigation(false);
	VisualMeshes->RegisterComponent();
	VisualMeshes->AttachToComponent(this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	VisualMeshes->SetStaticMesh(RenderMesh);
	for (int32 I = 0; I < RenderMaterials.Num(); I++)
		VisualMeshes->SetMaterial(I, RenderMaterials[I]);
}

void UAGX_TrackComponent::UpdateVisuals()
{
	if (!ShouldRenderSelf())
	{
		if (VisualMeshes != nullptr && VisualMeshes->GetInstanceCount() > 0)
			SetVisualsInstanceCount(0);

		return;
	}

	{
		// Workaround, the RenderMaterial and Mesh does not propagate properly in
		// SetRenderMaterial() and SetRenderMesh() in Blueprints, so we assign it here.
		if (VisualMeshes->GetStaticMesh() != RenderMesh)
			VisualMeshes->SetStaticMesh(RenderMesh);

		WriteRenderMaterialsToVisualMesh();
	}

	// Get the mesh instance transforms, either from the native if playing or
	// from the preview data if not playing.
	if (!ComputeNodeTransforms(NodeTransformsCache))
	{
		NodeTransformsCache.Empty(); // if failed, do not render anything.
	}

	// Make sure there is one mesh instance per track node.
	const int32 NumNodes = NodeTransformsCache.Num();
	SetVisualsInstanceCount(NumNodes);

	if (VisualMeshes->PerInstancePrevTransform.Num() != NumNodes)
		VisualMeshes->PerInstancePrevTransform.SetNum(NumNodes);

	if (NodeTransformsCachePrev.Num() != NumNodes)
		NodeTransformsCachePrev.SetNum(NumNodes);

	// Because UInstancedStaticMeshComponent::UpdateInstanceTransform() converts instance transforms
	// from World to Local Transform Space, make sure our local transform space is up-to-date.
	VisualMeshes->UpdateComponentToWorld();

	// Update transforms of the track node mesh instances.
	VisualMeshes->BatchUpdateInstancesTransforms(
		0, NodeTransformsCache, NodeTransformsCachePrev, /*bWorldSpace*/ true,
		/*bMarkRenderStateDirty*/ true);

	NodeTransformsCachePrev = NodeTransformsCache;
}

bool UAGX_TrackComponent::ShouldRenderSelf() const
{
	return VisualMeshes != nullptr && ShouldRender();
}

void UAGX_TrackComponent::SetVisualsInstanceCount(int32 Num)
{
	if (VisualMeshes == nullptr)
		return;

	Num = std::max(0, Num);

	while (VisualMeshes->GetInstanceCount() < Num)
	{
		VisualMeshes->AddInstance(FTransform());
	}

	while (VisualMeshes->GetInstanceCount() > Num)
	{
		VisualMeshes->RemoveInstance(VisualMeshes->GetInstanceCount() - 1);
	}
}

bool UAGX_TrackComponent::ComputeNodeTransforms(TArray<FTransform>& OutTransforms)
{
	// Get node transforms either from the actual track when playing,
	// or from a generated preview if not playing.
	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// Get mesh instance transforms from the native.
		if (!HasNative())
			return false;

		FVector VisualScale, VisualOffset;
		if (bAutoScaleAndOffset)
		{
			ComputeVisualScaleAndOffset(VisualScale, VisualOffset, GetNodeSize(0));
		}
		else
		{
			VisualScale = Scale;
			VisualOffset = Offset;
		}

		GetNodeTransforms(OutTransforms, VisualScale, VisualOffset, Rotation.Quaternion());
	}
	else
	{
		// Get mesh instance transforms from preview data.
		FAGX_TrackPreviewData* Preview = GetTrackPreview(/*bForceUpdate*/ false);

		if (!Preview || Preview->NodeTransforms.Num() <= 0)
		{
			return false;
		}
		check(Preview->NodeTransforms.Num() == Preview->NodeHalfExtents.Num());

		const FVector PhysicsNodeSize = 2 * Preview->NodeHalfExtents[0];
		const FVector BodyFrameToNodeCenter = FVector(0, 0, 0.5f * PhysicsNodeSize.Z);
		FVector VisualScale, VisualOffset;
		if (bAutoScaleAndOffset)
		{
			ComputeVisualScaleAndOffset(VisualScale, VisualOffset, PhysicsNodeSize);
		}
		else
		{
			VisualScale = Scale;
			VisualOffset = Offset;
		}

#if UE_VERSION_OLDER_THAN(5, 5, 0)
		OutTransforms.SetNum(Preview->NodeTransforms.Num(), /*bAllowShrinking*/ true);
#else
		OutTransforms.SetNum(Preview->NodeTransforms.Num(), EAllowShrinking::Yes);
#endif
		
		for (int i = 0; i < Preview->NodeTransforms.Num(); ++i)
		{
			const FVector WorldOffset = Preview->NodeTransforms[i].GetRotation().RotateVector(
				VisualOffset + BodyFrameToNodeCenter);

			OutTransforms[i].SetScale3D(VisualScale);
			OutTransforms[i].SetRotation(
				Preview->NodeTransforms[i].GetRotation() * Rotation.Quaternion());
			OutTransforms[i].SetLocation(Preview->NodeTransforms[i].GetTranslation() + WorldOffset);
		}
	}

	return true;
}

bool UAGX_TrackComponent::ComputeVisualScaleAndOffset(
	FVector& OutVisualScale, FVector& OutVisualOffset, const FVector& PhysicsNodeSize) const
{
	const FVector LocalMeshBoundsSize = LocalMeshBoundsMax - LocalMeshBoundsMin;
	const FVector LocalBoundsCenter = LocalMeshBoundsMin + LocalMeshBoundsSize * 0.5f;

	if (FMath::IsNearlyZero(LocalMeshBoundsSize.X) || FMath::IsNearlyZero(LocalMeshBoundsSize.Y) ||
		FMath::IsNearlyZero(LocalMeshBoundsSize.Z))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Failed to compute visual Scale and Offset for '%s' in '%s' because "
				 "LocalMeshBoundsMax is too close too LocalMeshBoundsMin. Render mesh will use "
				 "identity transformation."),
			*GetName(), *GetNameSafe(GetOwner()));
		OutVisualScale = FVector::OneVector;
		OutVisualOffset = FVector::ZeroVector;
		return false;
	}

	OutVisualScale = PhysicsNodeSize / LocalMeshBoundsSize;
	OutVisualOffset =
		-LocalBoundsCenter * Scale; // times scale to convert offset to post-scale coordinates
	return true;
}

void UAGX_TrackComponent::WriteRenderMaterialsToVisualMesh()
{
	if (VisualMeshes == nullptr)
		return;

	if (RenderMesh != nullptr && RenderMesh->GetStaticMaterials().Num() != RenderMaterials.Num())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("WriteRenderMaterialsToVisualMesh was called for Track '%s' in '%s' but the "
				 "Render Material slot numbers does not match the Track's Render Mesh slot "
				 "numbers. Doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	for (int32 Elem = 0; Elem < RenderMaterials.Num(); Elem++)
	{
		if (VisualMeshes->GetMaterial(Elem) != RenderMaterials[Elem])
			VisualMeshes->SetMaterial(Elem, RenderMaterials[Elem]);
	}
}

#if WITH_EDITOR
void UAGX_TrackComponent::WriteRenderMaterialsToVisualMeshWithCheck()
{
	EnsureValidRenderMaterials();
	WriteRenderMaterialsToVisualMesh();
}

void UAGX_TrackComponent::EnsureValidRenderMaterials()
{
	for (int32 Elem = 0; Elem < RenderMaterials.Num(); Elem++)
	{
		if (RenderMaterials[Elem] == nullptr)
			continue;

		UMaterial* Material = RenderMaterials[Elem]->GetMaterial();
		if (Material == nullptr || Material->bUsedWithInstancedStaticMeshes)
			continue;

		if (Material->GetPathName().StartsWith("/Game/"))
		{
			// This is a material part of the UE project itself. We can therefore be a bit more
			// helpful and offer to fix the material setting and save the asset so that the user
			// does not have to manually do it.
			const FText AskEnableUseWithInstancedSM = LOCTEXT(
				"EnableUseWithInstancedStaticMeshes",
				"The selected Material does not have Use With Instanced Static Meshes enabled, "
				"meaning that it cannot be used to visualize the Track. Would you like this "
				"setting to be automatically enabled? The Material asset will be re-saved.");
			if (FAGX_NotificationUtilities::YesNoQuestion(AskEnableUseWithInstancedSM))
			{
				Material->Modify();
				Material->bUsedWithInstancedStaticMeshes = true;
				Material->PostEditChange();
				FAGX_ObjectUtilities::SaveAsset(*Material);
			}
			else
			{
				// Clear the material selection.
				for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*this))
				{
					if (Instance->RenderMaterials.IsValidIndex(Elem) &&
						Instance->RenderMaterials[Elem] == RenderMaterials[Elem])
					{
						Instance->RenderMaterials[Elem] = nullptr;
					}
				}

				RenderMaterials[Elem] = nullptr;
			}
		}
		else
		{
			// This is a material not part of the UE project itself. It may reside in the
			// installed Unreal Editor itself, or some plugin. We are not comfortable making
			// permanent changes to such materials, so we will prompt the user to do it
			// themselves.
			const FString Message =
				"The selected Material does not have Use With Instanced Static Meshes enabled, "
				"meaning that it cannot be used to visualize the Track. You can enable this "
				"setting from the Material editor. The Material needs to be saved after these "
				"changes. It is recommended to make a copy and place the "
				"material within the project Contents, that way the behavior will be the same "
				"on any computer opening this project.";
			FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(Message);

			// Clear the material selection.
			for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*this))
			{
				if (Instance->RenderMaterials.IsValidIndex(Elem) &&
					Instance->RenderMaterials[Elem] == RenderMaterials[Elem])
				{
					Instance->RenderMaterials[Elem] = nullptr;
				}
			}

			RenderMaterials[Elem] = nullptr;
		}
	}
}
#endif

FAGX_TrackComponentInstanceData::FAGX_TrackComponentInstanceData(
	const IAGX_NativeOwner* NativeOwner, const USceneComponent* SourceComponent,
	TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster)
	: FAGX_NativeOwnerInstanceData(NativeOwner, SourceComponent, InDowncaster)
{
}

void FAGX_TrackComponentInstanceData::ApplyToComponent(
	UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	Super::ApplyToComponent(Component, CacheApplyPhase);

	CastChecked<UAGX_TrackComponent>(Component)->ApplyComponentInstanceData(this, CacheApplyPhase);
}

#undef LOCTEXT_NAMESPACE
