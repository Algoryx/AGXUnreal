// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CableComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerSceneComponentInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Cable/CableNodeBarrier.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/InstancedStaticMeshComponent.h"


UAGX_CableComponent::UAGX_CableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UAGX_CableComponent::SetRenderMaterial(UMaterialInterface* Material)
{
	RenderMaterial = Material;

	if (VisualCylinders != nullptr)
		VisualCylinders->SetMaterial(0, RenderMaterial);

	if (VisualSpheres != nullptr)
		VisualSpheres->SetMaterial(0, RenderMaterial);
}

FCableBarrier* UAGX_CableComponent::GetOrCreateNative()
{
	if (!HasNative())
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
				TEXT(
					"A request for the AGX Dynamics instance for Cable '%s' in '%s' was "
					"made but we are in the middle of a Blueprint Reconstruction and the requested "
					"instance has not yet been restored. The instance cannot be returned, which "
					"may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}

	return GetNative();
}

FCableBarrier* UAGX_CableComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FCableBarrier* UAGX_CableComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

bool UAGX_CableComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_CableComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_CableComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

void UAGX_CableComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
		CreateNative();
}

void UAGX_CableComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_CableComponent will inherit this one's Native, so don't wreck it.
		// The call to NativeBarrier.ReleaseNative below is safe because the AGX Dynamics Simulation
		// will retain a reference counted pointer to the AGX Dynamics Observer Frame.
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

TStructOnScope<FActorComponentInstanceData> UAGX_CableComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<
		FActorComponentInstanceData, FAGX_NativeOwnerSceneComponentInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

void UAGX_CableComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateVisuals();
}

namespace AGX_CableComponent_helpers
{
	std::tuple<FRigidBodyBarrier*, FVector> GetBodyAndLocalLocation(
		const FAGX_CableRouteNode& RouteNode, const FTransform& CableTransform)
	{
		UAGX_RigidBodyComponent* BodyComponent = RouteNode.RigidBody.GetRigidBody();
		if (BodyComponent == nullptr)
		{
			return {nullptr, FVector::ZeroVector};
		}
		FRigidBodyBarrier* NativeBody = BodyComponent->GetOrCreateNative();
		check(NativeBody);
		const FVector LocalLocation =
			RouteNode.Frame.GetLocationRelativeTo(*BodyComponent, CableTransform);
		return {NativeBody, LocalLocation};
	}

	void SetLocalScope(UAGX_CableComponent& Component)
	{
		AActor* Owner = FAGX_ObjectUtilities::GetRootParentActor(Component);
		for (auto& Node : Component.RouteNodes)
			Node.RigidBody.LocalScope = Owner;
	}
}

void UAGX_CableComponent::CreateNative()
{
	if (HasNative())
		return;

	check(!GIsReconstructingBlueprintInstances);
	NativeBarrier.AllocateNative(Radius, ResolutionPerUnitLength);

	TArray<FString> ErrorMessages;
	for (int32 I = 0; I < RouteNodes.Num(); ++I)
	{
		FAGX_CableRouteNode& Node = RouteNodes[I];
		FCableNodeBarrier NodeBarrier;

		switch (Node.NodeType)
		{
			case EAGX_CableNodeType::Free:
			{
				const FVector WorldLocation = Node.Frame.GetWorldLocation(*this);
				NodeBarrier.AllocateNativeFreeNode(WorldLocation);
				break;
			}
			case EAGX_CableNodeType::BodyFixed:
			{
				FRigidBodyBarrier* Body {nullptr};
				FVector Location;
				std::tie(Body, Location) = AGX_CableComponent_helpers::GetBodyAndLocalLocation(
					Node, GetComponentTransform());
				if (Body == nullptr)
				{
					ErrorMessages.Add(FString::Printf(
						TEXT("Cable node at index %d has invalid body. Creating Free Node "
							 "instead of Body Fixed Node."),
						I));
					const FVector WorldLocation = Node.Frame.GetWorldLocation(*this);
					NodeBarrier.AllocateNativeFreeNode(WorldLocation);
					break;
				}
				NodeBarrier.AllocateNativeBodyFixedNode(*Body, Location);
				break;
			}
		}
		NativeBarrier.Add(NodeBarrier);
	}

	if (ErrorMessages.Num() > 0)
	{
		FString Message = FString::Printf(
			TEXT("Errors detected during initialization of Cable '%s' in '%s':\n"), *GetName(),
			*GetLabelSafe(GetOwner()));
		for (const FString& Line : ErrorMessages)
		{
			Message += Line + '\n';
		}
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
	}

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Observer Frame Component '%s' in '%s' tried to get Simulation, but "
				 "UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Simulation->Add(*this);
}

#if WITH_EDITOR
bool UAGX_CableComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, Radius),
			GET_MEMBER_NAME_CHECKED(ThisClass, ResolutionPerUnitLength),
			GET_MEMBER_NAME_CHECKED(ThisClass, RouteNodes)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
			return false;
	}

	return SuperCanEditChange;
}

void UAGX_CableComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	// TODO: add properties here.
}
#endif // WITH_EDITOR

void UAGX_CableComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	NativeBarrier.SetName(!ImportName.IsEmpty() ? ImportName : GetName());
}

#if WITH_EDITOR
void UAGX_CableComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_CableComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_CableComponent::OnRegister()
{
	Super::OnRegister();
	AGX_CableComponent_helpers::SetLocalScope(*this);

	if (VisualCylinders == nullptr || VisualSpheres == nullptr)
		CreateVisuals();

	UpdateVisuals();
}
#endif // WITH_EDITOR

void UAGX_CableComponent::CreateVisuals()
{
	VisualCylinders =
		NewObject<UInstancedStaticMeshComponent>(this, FName(TEXT("VisualCylinders")));
	VisualCylinders->SetCanEverAffectNavigation(false);
	VisualCylinders->RegisterComponent();
	VisualCylinders->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	static const TCHAR* CylinderAssetPath =
		TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualCylinder.SM_WireVisualCylinder'");
	VisualCylinders->SetStaticMesh(
		FAGX_ObjectUtilities::GetAssetFromPath<UStaticMesh>(CylinderAssetPath));
	VisualCylinders->SetMaterial(0, RenderMaterial);

	VisualSpheres = NewObject<UInstancedStaticMeshComponent>(this, FName(TEXT("VisualSpheres")));
	VisualSpheres->SetCanEverAffectNavigation(false);
	VisualSpheres->RegisterComponent();
	VisualSpheres->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	static const TCHAR* SphereAssetPath =
		TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualSphere.SM_WireVisualSphere'");
	VisualSpheres->SetStaticMesh(
		FAGX_ObjectUtilities::GetAssetFromPath<UStaticMesh>(SphereAssetPath));
	VisualSpheres->SetMaterial(0, RenderMaterial);
}

bool UAGX_CableComponent::ShouldRenderSelf() const
{
	return VisualCylinders != nullptr && VisualSpheres != nullptr && ShouldRender();
}

void UAGX_CableComponent::UpdateVisuals()
{
	if (!ShouldRenderSelf())
	{
		const bool bHasVisualCylinders =
			VisualCylinders != nullptr && VisualCylinders->GetInstanceCount() > 0;
		const bool bHasVisualSpheres =
			VisualSpheres != nullptr && VisualSpheres->GetInstanceCount() > 0;

		if (bHasVisualCylinders || bHasVisualSpheres)
			SetVisualsInstanceCount(0);

		return;
	}

	// Workaround, the RenderMaterial does not propagate properly in SetRenderMaterial() in
	// Blueprints, so we assign it here.
	if (VisualCylinders->GetMaterial(0) != RenderMaterial)
		VisualCylinders->SetMaterial(0, RenderMaterial);

	if (VisualSpheres->GetMaterial(0) != RenderMaterial)
		VisualSpheres->SetMaterial(0, RenderMaterial);

	// TODO:
	// TArray<FVector> NodeLocations = GetNodesForRendering();
	// RenderSelf(NodeLocations);
}

void UAGX_CableComponent::SetVisualsInstanceCount(int32 Num)
{
	Num = std::max(0, Num);

	auto SetNum = [](UInstancedStaticMeshComponent& C, int32 N)
	{
		while (C.GetInstanceCount() < N)
		{
			C.AddInstance(FTransform());
		}
		while (C.GetInstanceCount() > N)
		{
			C.RemoveInstance(C.GetInstanceCount() - 1);
		}
	};

	if (VisualCylinders != nullptr)
		SetNum(*VisualCylinders, Num);

	if (VisualSpheres != nullptr)
		SetNum(*VisualSpheres, Num);
}
