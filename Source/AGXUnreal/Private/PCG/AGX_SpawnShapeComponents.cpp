// Copyright 2026, Algoryx Simulation AB.

#include "PCG/AGX_SpawnShapeComponents.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Elements/PCGAddComponent.h"
#include "Helpers/PCGActorHelpers.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/Accessors/PCGAttributeAccessorHelpers.h"
#include "Metadata/PCGAttributePropertySelector.h"
#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGManagedResource.h"
#include "PCGParamData.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/BoxElem.h"
#include "PhysicsEngine/SphereElem.h"
#include "PhysicsEngine/SphylElem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "AGX_SpawnShapeComponents"

namespace AGX_SpawnShapeComponents_helpers
{
	const FName NodeActorTag(TEXT("AGX_PCGSpawnShapeComponents"));
	const FName NodeComponentTag(TEXT("AGX_PCGSpawnShapeComponents"));

	FName GetSettingsTag(const UAGX_SpawnShapeComponentsSettings& Settings)
	{
		return FName(*FString::Printf(
			TEXT("AGX_PCGSpawnShapeComponents_%llu"),
			static_cast<unsigned long long>(Settings.GetStableUID())));
	}

	FTransform GetMeshRotationAndTranslation(const UStaticMeshComponent& MeshComponent)
	{
		return FTransform(MeshComponent.GetComponentQuat(), MeshComponent.GetComponentLocation());
	}

	USceneComponent* EnsureActorRootComponent(AActor& Actor)
	{
		if (USceneComponent* RootComponent = Actor.GetRootComponent())
		{
			return RootComponent;
		}

		// The node spawns a plain AActor as a visible container for generated AGX shapes.
		// AActor does not guarantee a root component, so we create one explicitly. Doing this
		// makes later AttachToComponent calls well-defined and keeps the component hierarchy easy
		// to inspect in the editor outliner.
		USceneComponent* RootComponent = NewObject<USceneComponent>(
			&Actor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
		Actor.SetRootComponent(RootComponent);
		Actor.AddInstanceComponent(RootComponent);
		RootComponent->RegisterComponent();
		return RootComponent;
	}

	void ClearPreviouslyGeneratedShapes(AActor& Actor, const FName SettingsTag)
	{
		TInlineComponentArray<UAGX_ShapeComponent*> ShapeComponents(&Actor);
		for (UAGX_ShapeComponent* ShapeComponent : ShapeComponents)
		{
			if (ShapeComponent != nullptr && ShapeComponent->ComponentHasTag(SettingsTag))
			{
				Actor.RemoveInstanceComponent(ShapeComponent);
				ShapeComponent->DestroyComponent();
			}
		}
	}

	UPCGManagedActors* FindManagedActorResource(
		UPCGComponent& SourceComponent, const FName SettingsTag, AActor*& OutActor)
	{
		UPCGManagedActors* ManagedActorsResource = nullptr;
		OutActor = nullptr;

		SourceComponent.ForEachManagedResource(
			[&ManagedActorsResource, &OutActor, SettingsTag](UPCGManagedResource* Resource)
			{
				if (ManagedActorsResource != nullptr)
				{
					return;
				}

				UPCGManagedActors* Candidate = Cast<UPCGManagedActors>(Resource);
				if (Candidate == nullptr || !Candidate->CanBeUsed())
				{
					return;
				}

				for (const TSoftObjectPtr<AActor>& ManagedActorPtr :
					 Candidate->GetConstGeneratedActors())
				{
					AActor* ManagedActor = ManagedActorPtr.Get();
					if (ManagedActor != nullptr && ManagedActor->ActorHasTag(SettingsTag))
					{
						ManagedActorsResource = Candidate;
						OutActor = ManagedActor;
						return;
					}
				}
			});

		return ManagedActorsResource;
	}

	AActor* GetOrCreateSpawnActor(
		UPCGComponent& SourceComponent, const UAGX_SpawnShapeComponentsSettings& Settings)
	{
		AActor* SpawnActor = nullptr;
		const FName SettingsTag = GetSettingsTag(Settings);
		UPCGManagedActors* ManagedActorsResource =
			FindManagedActorResource(SourceComponent, SettingsTag, SpawnActor);

		if (SpawnActor != nullptr)
		{
			ManagedActorsResource->MarkAsReused();
			return SpawnActor;
		}

		AActor* SourceOwner = SourceComponent.GetOwner();
		if (SourceOwner == nullptr)
		{
			return nullptr;
		}

		UWorld* World = SourceOwner->GetWorld();
		if (World == nullptr)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = SourceOwner;
		SpawnParameters.OverrideLevel = SourceOwner->GetLevel();
		SpawnParameters.Name = MakeUniqueObjectName(
			SpawnParameters.OverrideLevel ? static_cast<UObject*>(SpawnParameters.OverrideLevel)
										  : static_cast<UObject*>(World),
			AActor::StaticClass(), FName(*Settings.SpawnedActorName));

		UPCGActorHelpers::FSpawnDefaultActorParams SpawnDefaultActorParams(
			World, AActor::StaticClass(), SourceOwner->GetTransform(), SpawnParameters);
		SpawnDefaultActorParams.bForceStaticMobility = false;
		SpawnDefaultActorParams.bIsPreviewActor = SourceComponent.IsInPreviewMode();
#if WITH_EDITOR
		SpawnDefaultActorParams.DataLayerInstances = SourceOwner->GetDataLayerInstances();
		SpawnDefaultActorParams.HLODLayer = SourceOwner->GetHLODLayer();
#endif

		SpawnActor = UPCGActorHelpers::SpawnDefaultActor(SpawnDefaultActorParams);
		if (SpawnActor == nullptr)
		{
			return nullptr;
		}

		// These tags make the generated Actor visible to both humans and Unreal's PCG cleanup
		// conventions. The node-specific tag is what this implementation uses to find "its" Actor
		// on reevaluation, while the default PCG tag keeps the actor identifiable as PCG-generated.
		SpawnActor->Tags.AddUnique(NodeActorTag);
		SpawnActor->Tags.AddUnique(SettingsTag);
		SpawnActor->Tags.AddUnique(SourceComponent.GetFName());
		SpawnActor->Tags.AddUnique(PCGHelpers::DefaultPCGTag);

		if (ManagedActorsResource == nullptr)
		{
			ManagedActorsResource = NewObject<UPCGManagedActors>(&SourceComponent);
			SourceComponent.AddToManagedResources(ManagedActorsResource);
		}
		else
		{
			// I don't think this can ever happen. We can only ever get a ManagedActorResource from
			// FindManagedActorResource if we also get an Actor. And if we got an Actor then we
			// would have already returned.
			ManagedActorsResource->MarkAsUsed();
		}

		ManagedActorsResource->GetMutableGeneratedActors().Reset();
		ManagedActorsResource->GetMutableGeneratedActors().Add(SpawnActor);
		return SpawnActor;
	}

	EObjectFlags GetGeneratedComponentFlags(const UPCGComponent& SourceComponent)
	{
		return SourceComponent.IsInPreviewMode() ? RF_Transient | RF_NonPIEDuplicateTransient
												 : RF_NoFlags;
	}

	void RegisterGeneratedComponent(
		UAGX_ShapeComponent& ShapeComponent, AActor& SpawnActor, USceneComponent& RootComponent,
		UPCGComponent& SourceComponent, const FName SettingsTag)
	{
		SpawnActor.AddInstanceComponent(&ShapeComponent);
		ShapeComponent.RegisterComponent();

		if (!ShapeComponent.AttachToComponent(
				&RootComponent, FAttachmentTransformRules::KeepWorldTransform))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Failed to attach generated AGX Shape Component '%s' to spawned PCG Actor "
					"'%s'."),
				*ShapeComponent.GetName(), *SpawnActor.GetName());
		}

		ShapeComponent.ComponentTags.AddUnique(NodeComponentTag);
		ShapeComponent.ComponentTags.AddUnique(SettingsTag);
		ShapeComponent.ComponentTags.AddUnique(SourceComponent.GetFName());
		ShapeComponent.ComponentTags.AddUnique(PCGHelpers::DefaultPCGTag);
	}

	UAGX_SphereShapeComponent* SpawnSphereComponent(
		AActor& SpawnActor, USceneComponent& RootComponent, UPCGComponent& SourceComponent,
		const FName SettingsTag, const FKSphereElem& SphereElem,
		const UStaticMeshComponent& MeshComponent, EObjectFlags ObjectFlags)
	{
		UAGX_SphereShapeComponent* SphereComponent = NewObject<UAGX_SphereShapeComponent>(
			&SpawnActor, UAGX_SphereShapeComponent::StaticClass(), NAME_None, ObjectFlags);
		if (SphereComponent == nullptr)
		{
			return nullptr;
		}

		const FTransform MeshRotationAndTranslation = GetMeshRotationAndTranslation(MeshComponent);
		const FKSphereElem ScaledSphere = SphereElem.GetFinalScaled(
			MeshComponent.GetComponentTransform().GetScale3D(), FTransform::Identity);

		SphereComponent->SetWorldTransform(
			ScaledSphere.GetTransform() * MeshRotationAndTranslation);
		SphereComponent->SetRadius(ScaledSphere.Radius);

		RegisterGeneratedComponent(
			*SphereComponent, SpawnActor, RootComponent, SourceComponent, SettingsTag);
		return SphereComponent;
	}

	UAGX_BoxShapeComponent* SpawnBoxComponent(
		AActor& SpawnActor, USceneComponent& RootComponent, UPCGComponent& SourceComponent,
		const FName SettingsTag, const FKBoxElem& BoxElem,
		const UStaticMeshComponent& MeshComponent, EObjectFlags ObjectFlags)
	{
		UAGX_BoxShapeComponent* BoxComponent = NewObject<UAGX_BoxShapeComponent>(
			&SpawnActor, UAGX_BoxShapeComponent::StaticClass(), NAME_None, ObjectFlags);
		if (BoxComponent == nullptr)
		{
			return nullptr;
		}

		const FTransform MeshRotationAndTranslation = GetMeshRotationAndTranslation(MeshComponent);
		const FKBoxElem ScaledBox = BoxElem.GetFinalScaled(
			MeshComponent.GetComponentTransform().GetScale3D(), FTransform::Identity);

		BoxComponent->SetWorldTransform(ScaledBox.GetTransform() * MeshRotationAndTranslation);
		BoxComponent->SetHalfExtent(FVector(ScaledBox.X, ScaledBox.Y, ScaledBox.Z) * 0.5f);

		RegisterGeneratedComponent(
			*BoxComponent, SpawnActor, RootComponent, SourceComponent, SettingsTag);
		return BoxComponent;
	}

	UAGX_CapsuleShapeComponent* SpawnCapsuleComponent(
		AActor& SpawnActor, USceneComponent& RootComponent, UPCGComponent& SourceComponent,
		const FName SettingsTag, const FKSphylElem& SphylElem,
		const UStaticMeshComponent& MeshComponent, EObjectFlags ObjectFlags)
	{
		UAGX_CapsuleShapeComponent* CapsuleComponent = NewObject<UAGX_CapsuleShapeComponent>(
			&SpawnActor, UAGX_CapsuleShapeComponent::StaticClass(), NAME_None, ObjectFlags);
		if (CapsuleComponent == nullptr)
		{
			return nullptr;
		}

		const FTransform MeshRotationAndTranslation = GetMeshRotationAndTranslation(MeshComponent);
		const FKSphylElem ScaledSphyl = SphylElem.GetFinalScaled(
			MeshComponent.GetComponentTransform().GetScale3D(), FTransform::Identity);

		// Unreal sphyl primitives point along +Z while AGX Dynamics capsules are aligned along Y.
		const FQuat CapsuleAxisCorrection = FQuat(FRotator(0.0, 0.0, 90.0));
		FTransform CapsuleTransform = ScaledSphyl.GetTransform();
		CapsuleTransform.SetRotation(CapsuleTransform.GetRotation() * CapsuleAxisCorrection);

		CapsuleComponent->SetWorldTransform(CapsuleTransform * MeshRotationAndTranslation);
		CapsuleComponent->SetRadius(ScaledSphyl.Radius);
		CapsuleComponent->SetHeight(ScaledSphyl.Length);

		RegisterGeneratedComponent(
			*CapsuleComponent, SpawnActor, RootComponent, SourceComponent, SettingsTag);
		return CapsuleComponent;
	}

	void SpawnShapesForMeshComponent(
		const UAGX_SpawnShapeComponentsSettings& Settings, UPCGComponent& SourceComponent,
		AActor& SpawnActor, USceneComponent& RootComponent, UStaticMeshComponent& MeshComponent)
	{
		const UBodySetup* BodySetup = MeshComponent.GetBodySetup();
		if (BodySetup == nullptr)
		{
			return;
		}

		const EObjectFlags ObjectFlags = GetGeneratedComponentFlags(SourceComponent);
		const FName SettingsTag = GetSettingsTag(Settings);
		const FKAggregateGeom& AggGeom = BodySetup->AggGeom;

		if (Settings.ShouldSpawn(EAGX_FilterAggGeomTypes::Sphere))
		{
			for (const FKSphereElem& SphereElem : AggGeom.SphereElems)
			{
				SpawnSphereComponent(
					SpawnActor, RootComponent, SourceComponent, SettingsTag, SphereElem,
					MeshComponent, ObjectFlags);
			}
		}

		if (Settings.ShouldSpawn(EAGX_FilterAggGeomTypes::Box))
		{
			for (const FKBoxElem& BoxElem : AggGeom.BoxElems)
			{
				SpawnBoxComponent(
					SpawnActor, RootComponent, SourceComponent, SettingsTag, BoxElem, MeshComponent,
					ObjectFlags);
			}
		}

		if (Settings.ShouldSpawn(EAGX_FilterAggGeomTypes::Sphyl))
		{
			for (const FKSphylElem& SphylElem : AggGeom.SphylElems)
			{
				SpawnCapsuleComponent(
					SpawnActor, RootComponent, SourceComponent, SettingsTag, SphylElem,
					MeshComponent, ObjectFlags);
			}
		}

		if (!AggGeom.ConvexElems.IsEmpty())
		{
			UE_LOG(
				LogAGX, Verbose,
				TEXT("AGX PCG spawn node skipped unsupported AggGeom primitive types on '%s'."),
				*MeshComponent.GetName());
		}
	}
}

#if WITH_EDITOR

FName UAGX_SpawnShapeComponentsSettings::GetDefaultNodeName() const
{
	static const FName Name(TEXT("SpawnShapeComponents"));
	return Name;
}

FText UAGX_SpawnShapeComponentsSettings::GetDefaultNodeTitle() const
{
	static const FText Title(LOCTEXT("NodeTitle", "Spawn AGX Shape Components"));
	return Title;
}

FText UAGX_SpawnShapeComponentsSettings::GetNodeTooltipText() const
{
	static const FText Tooltip(LOCTEXT(
		"NodeTooltip",
		"Spawns AGX Shape Components from the AggGeom primitive collision in Static Mesh "
		"Components referenced by an Attribute Set. Intended input: Get Actor Data in Get "
		"Component Reference mode, optionally filtered by Filter by AggGeom."));
	return Tooltip;
}

EPCGSettingsType UAGX_SpawnShapeComponentsSettings::GetType() const
{
	return EPCGSettingsType::Spawner;
}

EPCGChangeType UAGX_SpawnShapeComponentsSettings::GetChangeTypeForProperty(
	const FName& InPropertyName) const
{
	return (InPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_SpawnShapeComponentsSettings, ToSpawn) ||
			InPropertyName ==
				GET_MEMBER_NAME_CHECKED(UAGX_SpawnShapeComponentsSettings, SpawnedActorName))
			   ? EPCGChangeType::Settings
			   : Super::GetChangeTypeForProperty(InPropertyName);
}

#endif

TArray<FPCGPinProperties> UAGX_SpawnShapeComponentsSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	FPCGPinProperties& InputPin =
		PinProperties.Emplace_GetRef(PCGPinConstants::DefaultInputLabel, EPCGDataType::Param);
	InputPin.SetRequiredPin();
	return PinProperties;
}

TArray<FPCGPinProperties> UAGX_SpawnShapeComponentsSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Param);
	return PinProperties;
}

FPCGElementPtr UAGX_SpawnShapeComponentsSettings::CreateElement() const
{
	return MakeShared<FAGX_SpawnShapeComponentsElement>();
}

EAGX_FilterAggGeomTypes UAGX_SpawnShapeComponentsSettings::GetToSpawn() const
{
	return static_cast<EAGX_FilterAggGeomTypes>(ToSpawn);
}

bool UAGX_SpawnShapeComponentsSettings::ShouldSpawn(EAGX_FilterAggGeomTypes Type) const
{
	const uint32 TypeValue = static_cast<uint32>(Type);
	return (ToSpawn & TypeValue) != 0;
}

bool FAGX_SpawnShapeComponentsElement::ExecuteInternal(FPCGContext* Context) const
{
	const UAGX_SpawnShapeComponentsSettings* Settings =
		Context->GetInputSettings<UAGX_SpawnShapeComponentsSettings>();
	UPCGComponent* SourceComponent = Cast<UPCGComponent>(Context->ExecutionSource.Get());

	// This node is intentionally world-mutating: it takes Attribute Set rows that merely describe
	// component references and turns them into real AActor/UActorComponent instances in the level.
	// Because of that we pass through the original data unchanged for downstream nodes and keep the
	// world object lifecycle separate through PCG managed resources.
	Context->OutputData = Context->InputData;

	if (Settings == nullptr || SourceComponent == nullptr)
	{
		return true;
	}

	AActor* SpawnActor =
		AGX_SpawnShapeComponents_helpers::GetOrCreateSpawnActor(*SourceComponent, *Settings);
	if (SpawnActor == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX PCG spawn node could not create or reuse its spawned-shapes actor."));
		return true;
	}

	USceneComponent* RootComponent =
		AGX_SpawnShapeComponents_helpers::EnsureActorRootComponent(*SpawnActor);
	if (RootComponent == nullptr)
	{
		return true;
	}

	AGX_SpawnShapeComponents_helpers::ClearPreviouslyGeneratedShapes(
		*SpawnActor, AGX_SpawnShapeComponents_helpers::GetSettingsTag(*Settings));

	FPCGAttributePropertyInputSelector Selector;
	Selector.SetAttributeName(PCGAddComponentConstants::ComponentReferenceAttribute);

	TArray<FPCGTaggedData> Inputs =
		Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGParamData* ParamData = Cast<UPCGParamData>(Input.Data);
		if (ParamData == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX PCG spawn node received non-attribute-set input and skipped it."));
			continue;
		}

		TArray<FSoftObjectPath> ComponentReferences;
		if (!PCGAttributeAccessorHelpers::ExtractAllValues<FSoftObjectPath>(
				ParamData, Selector, ComponentReferences, Context,
				EPCGAttributeAccessorFlags::AllowBroadcastAndConstructible))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"AGX PCG spawn node could not read ComponentReference values from an input "
					"Attribute Set."));
			continue;
		}

		for (const FSoftObjectPath& ComponentReference : ComponentReferences)
		{
			UStaticMeshComponent* MeshComponent =
				Cast<UStaticMeshComponent>(ComponentReference.ResolveObject());
			if (MeshComponent == nullptr)
			{
				continue;
			}

			AGX_SpawnShapeComponents_helpers::SpawnShapesForMeshComponent(
				*Settings, *SourceComponent, *SpawnActor, *RootComponent, *MeshComponent);
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
