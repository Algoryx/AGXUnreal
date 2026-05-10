// Copyright 2026, Algoryx Simulation AB.

#include "PCG/AGX_ShapeSpawner.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Helpers/PCGActorHelpers.h"
#include "Helpers/PCGHelpers.h"
#include "Metadata/PCGMetadata.h"
#include "Metadata/PCGMetadataAttribute.h"
#include "Metadata/PCGMetadataAttributeTraits.h"
#include "Metadata/PCGMetadataAttributeTpl.h"
#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGManagedResource.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

#define LOCTEXT_NAMESPACE "AGX_ShapeSpawner"

namespace AGX_ShapeSpawner_helpers
{
	const FName NodeActorTag(TEXT("AGX_PCGShapeSpawner"));
	const FName NodeComponentTag(TEXT("AGX_PCGShapeSpawner"));

	FName GetSettingsTag(const UAGX_ShapeSpawnerSettings& Settings)
	{
		return FName(*FString::Printf(
			TEXT("AGX_PCGShapeSpawner_%llu"),
			static_cast<unsigned long long>(Settings.GetStableUID())));
	}

	USceneComponent* EnsureActorRootComponent(AActor& Actor)
	{
		if (USceneComponent* RootComponent = Actor.GetRootComponent())
		{
			return RootComponent;
		}

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
		UPCGComponent& SourceComponent, const UAGX_ShapeSpawnerSettings& Settings)
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
				TEXT("Failed to attach generated AGX Shape Component '%s' to spawned PCG Actor "
					 "'%s'."),
				*ShapeComponent.GetName(), *SpawnActor.GetName());
		}

		ShapeComponent.ComponentTags.AddUnique(NodeComponentTag);
		ShapeComponent.ComponentTags.AddUnique(SettingsTag);
		ShapeComponent.ComponentTags.AddUnique(SourceComponent.GetFName());
		ShapeComponent.ComponentTags.AddUnique(PCGHelpers::DefaultPCGTag);
	}

	// Carries resolved per-point shape dimensions after applying scale and attribute overrides.
	struct FShapeDimensions
	{
		float Radius = 50.f;
		FVector HalfExtent {50.f, 50.f, 50.f};
		float Height = 100.f;
	};

	// Returns a transform with the point's position and rotation but scale stripped to one.
	// Physics shapes express size through their own properties, not component scale.
	FTransform PointPositionAndRotation(const FPCGPoint& Point)
	{
		return FTransform(Point.Transform.GetRotation(), Point.Transform.GetTranslation());
	}

	// Resolves final shape dimensions for a single point. Attribute overrides have precedence;
	// scale-based scaling applies when bScaleByPointTransform is true and no override is present.
	FShapeDimensions ResolveShapeDimensions(
		const FPCGPoint& Point, const UAGX_ShapeSpawnerSettings& Settings,
		const UPCGPointData* PointData)
	{
		FShapeDimensions Dims;
		const FVector Scale = Settings.bScaleByPointTransform ? Point.Transform.GetScale3D()
															  : FVector::OneVector;

		Dims.Radius = Settings.BaseRadius * Scale.X;
		Dims.HalfExtent = Settings.BaseHalfExtent * Scale;
		Dims.Height = Settings.BaseHeight * Scale.Z;

		if (PointData == nullptr)
			return Dims;
		const UPCGMetadata* Metadata = PointData->ConstMetadata();
		if (Metadata == nullptr || Point.MetadataEntry == PCGInvalidEntryKey)
			return Dims;

		if (Settings.RadiusAttribute != NAME_None)
		{
			const FPCGMetadataAttributeBase* AttrBase =
				Metadata->GetConstAttribute(Settings.RadiusAttribute);
			if (AttrBase != nullptr &&
				AttrBase->GetTypeId() == PCG::Private::MetadataTypes<float>::Id)
			{
				Dims.Radius = static_cast<const FPCGMetadataAttribute<float>*>(AttrBase)
								  ->GetValueFromItemKey(Point.MetadataEntry);
			}
			else if (AttrBase != nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("AGX Shape Spawner: RadiusAttribute '%s' is not a float attribute; "
						 "ignoring."),
					*Settings.RadiusAttribute.ToString());
			}
		}

		if (Settings.HalfExtentAttribute != NAME_None)
		{
			const FPCGMetadataAttributeBase* AttrBase =
				Metadata->GetConstAttribute(Settings.HalfExtentAttribute);
			if (AttrBase != nullptr &&
				AttrBase->GetTypeId() == PCG::Private::MetadataTypes<FVector>::Id)
			{
				Dims.HalfExtent = static_cast<const FPCGMetadataAttribute<FVector>*>(AttrBase)
									  ->GetValueFromItemKey(Point.MetadataEntry);
			}
			else if (AttrBase != nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("AGX Shape Spawner: HalfExtentAttribute '%s' is not an FVector attribute; "
						 "ignoring."),
					*Settings.HalfExtentAttribute.ToString());
			}
		}

		if (Settings.HeightAttribute != NAME_None)
		{
			const FPCGMetadataAttributeBase* AttrBase =
				Metadata->GetConstAttribute(Settings.HeightAttribute);
			if (AttrBase != nullptr &&
				AttrBase->GetTypeId() == PCG::Private::MetadataTypes<float>::Id)
			{
				Dims.Height = static_cast<const FPCGMetadataAttribute<float>*>(AttrBase)
								  ->GetValueFromItemKey(Point.MetadataEntry);
			}
			else if (AttrBase != nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("AGX Shape Spawner: HeightAttribute '%s' is not a float attribute; "
						 "ignoring."),
					*Settings.HeightAttribute.ToString());
			}
		}

		return Dims;
	}

	void SpawnSphereAtPoint(
		AActor& SpawnActor, USceneComponent& RootComponent, UPCGComponent& SourceComponent,
		const FName SettingsTag, const FPCGPoint& Point, const FShapeDimensions& Dims,
		EObjectFlags ObjectFlags)
	{
		UAGX_SphereShapeComponent* Sphere = NewObject<UAGX_SphereShapeComponent>(
			&SpawnActor, UAGX_SphereShapeComponent::StaticClass(), NAME_None, ObjectFlags);
		if (Sphere == nullptr)
			return;

		Sphere->SetWorldTransform(PointPositionAndRotation(Point));
		Sphere->SetRadius(Dims.Radius);
		RegisterGeneratedComponent(*Sphere, SpawnActor, RootComponent, SourceComponent, SettingsTag);

		// Trying my hardest to make the shape not render.
		Sphere->SetVisibleFlag(false);
		Sphere->SetVisibility(false);
		Sphere->SetHiddenInGame(true);

	}

	void SpawnBoxAtPoint(
		AActor& SpawnActor, USceneComponent& RootComponent, UPCGComponent& SourceComponent,
		const FName SettingsTag, const FPCGPoint& Point, const FShapeDimensions& Dims,
		EObjectFlags ObjectFlags)
	{
		UAGX_BoxShapeComponent* Box = NewObject<UAGX_BoxShapeComponent>(
			&SpawnActor, UAGX_BoxShapeComponent::StaticClass(), NAME_None, ObjectFlags);
		if (Box == nullptr)
			return;

		Box->SetWorldTransform(PointPositionAndRotation(Point));
		Box->SetHalfExtent(Dims.HalfExtent);
		RegisterGeneratedComponent(*Box, SpawnActor, RootComponent, SourceComponent, SettingsTag);


		// Trying my hardest to make the shape not render.
		Box->SetVisibleFlag(false);
		Box->SetVisibility(false);
		Box->SetHiddenInGame(true);
	}

	void SpawnCapsuleAtPoint(
		AActor& SpawnActor, USceneComponent& RootComponent, UPCGComponent& SourceComponent,
		const FName SettingsTag, const FPCGPoint& Point, const FShapeDimensions& Dims,
		EObjectFlags ObjectFlags)
	{
		UAGX_CapsuleShapeComponent* Capsule = NewObject<UAGX_CapsuleShapeComponent>(
			&SpawnActor, UAGX_CapsuleShapeComponent::StaticClass(), NAME_None, ObjectFlags);
		if (Capsule == nullptr)
			return;

		// AGX Dynamics capsules align along Y; PCG/Unreal point transforms use Z-up.
		// The same 90° correction is used in AGX_SpawnShapeComponents for sphyl elements.
		FTransform CapsuleTransform = PointPositionAndRotation(Point);
		const FQuat AxisCorrection = FQuat(FRotator(0.0, 0.0, 90.0));
		CapsuleTransform.SetRotation(CapsuleTransform.GetRotation() * AxisCorrection);

		Capsule->SetWorldTransform(CapsuleTransform);
		Capsule->SetRadius(Dims.Radius);
		Capsule->SetHeight(Dims.Height);
		RegisterGeneratedComponent(
			*Capsule, SpawnActor, RootComponent, SourceComponent, SettingsTag);

		// Trying my hardest to make the shape not render.
		Capsule->SetVisibleFlag(false);
		Capsule->SetVisibility(false);
		Capsule->SetHiddenInGame(true);
	}
}

#if WITH_EDITOR

FName UAGX_ShapeSpawnerSettings::GetDefaultNodeName() const
{
	static const FName Name(TEXT("AGXShapeSpawner"));
	return Name;
}

FText UAGX_ShapeSpawnerSettings::GetDefaultNodeTitle() const
{
	static const FText Title(LOCTEXT("NodeTitle", "AGX Shape Spawner"));
	return Title;
}

FText UAGX_ShapeSpawnerSettings::GetNodeTooltipText() const
{
	static const FText Tooltip(LOCTEXT(
		"NodeTooltip",
		"Spawns one AGX Shape Component per input PCG point into a PCG-managed container actor. "
		"Use in parallel with a Static Mesh Spawner to create matching visual meshes and AGX "
		"physics shapes from the same point set."));
	return Tooltip;
}

EPCGSettingsType UAGX_ShapeSpawnerSettings::GetType() const
{
	return EPCGSettingsType::Spawner;
}

EPCGChangeType UAGX_ShapeSpawnerSettings::GetChangeTypeForProperty(
	const FName& InPropertyName) const
{
	if (InPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeSpawnerSettings, ShapeType) ||
		InPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeSpawnerSettings, BaseRadius) ||
		InPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeSpawnerSettings, BaseHalfExtent) ||
		InPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeSpawnerSettings, BaseHeight) ||
		InPropertyName ==
			GET_MEMBER_NAME_CHECKED(UAGX_ShapeSpawnerSettings, bScaleByPointTransform) ||
		InPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_ShapeSpawnerSettings, SpawnedActorName))
	{
		return EPCGChangeType::Settings;
	}
	return Super::GetChangeTypeForProperty(InPropertyName);
}

#endif

TArray<FPCGPinProperties> UAGX_ShapeSpawnerSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	FPCGPinProperties& InputPin =
		PinProperties.Emplace_GetRef(PCGPinConstants::DefaultInputLabel, EPCGDataType::Point);
	InputPin.SetRequiredPin();
	return PinProperties;
}

TArray<FPCGPinProperties> UAGX_ShapeSpawnerSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
	return PinProperties;
}

FPCGElementPtr UAGX_ShapeSpawnerSettings::CreateElement() const
{
	return MakeShared<FAGX_ShapeSpawnerElement>();
}

bool FAGX_ShapeSpawnerElement::ExecuteInternal(FPCGContext* Context) const
{
	const UAGX_ShapeSpawnerSettings* Settings =
		Context->GetInputSettings<UAGX_ShapeSpawnerSettings>();
	UPCGComponent* SourceComponent = Cast<UPCGComponent>(Context->ExecutionSource.Get());

	// Pass through point data first so downstream nodes receive it regardless of world-object
	// creation success or failure.
	Context->OutputData = Context->InputData;

	if (Settings == nullptr || SourceComponent == nullptr)
	{
		return true;
	}

	AActor* SpawnActor =
		AGX_ShapeSpawner_helpers::GetOrCreateSpawnActor(*SourceComponent, *Settings);
	if (SpawnActor == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX Shape Spawner: could not create or reuse the container actor."));
		return true;
	}

	USceneComponent* RootComponent =
		AGX_ShapeSpawner_helpers::EnsureActorRootComponent(*SpawnActor);
	if (RootComponent == nullptr)
	{
		return true;
	}

	const FName SettingsTag = AGX_ShapeSpawner_helpers::GetSettingsTag(*Settings);
	AGX_ShapeSpawner_helpers::ClearPreviouslyGeneratedShapes(*SpawnActor, SettingsTag);

	const EObjectFlags ObjFlags =
		AGX_ShapeSpawner_helpers::GetGeneratedComponentFlags(*SourceComponent);

	const TArray<FPCGTaggedData> Inputs =
		Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);

	for (const FPCGTaggedData& Input : Inputs)
	{
		const UPCGSpatialData* Spatial = Cast<UPCGSpatialData>(Input.Data);
		if (Spatial == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX Shape Spawner: received non-spatial input on the default pin; "
					 "skipping."));
			continue;
		}

		const UPCGPointData* PointData = Spatial->ToPointData(Context);
		if (PointData == nullptr)
		{
			continue;
		}

		const TArray<FPCGPoint>& Points = PointData->GetPoints();
		for (int32 I = 0; I < Points.Num(); ++I)
		{
			const FPCGPoint& Point = Points[I];

			if (Point.Density <= 0.f)
			{
				continue;
			}

			const AGX_ShapeSpawner_helpers::FShapeDimensions Dims =
				AGX_ShapeSpawner_helpers::ResolveShapeDimensions(Point, *Settings, PointData);

			switch (Settings->ShapeType)
			{
			case EAGX_ShapeSpawnerType::Sphere:
				AGX_ShapeSpawner_helpers::SpawnSphereAtPoint(
					*SpawnActor, *RootComponent, *SourceComponent, SettingsTag, Point, Dims,
					ObjFlags);
				break;
			case EAGX_ShapeSpawnerType::Box:
				AGX_ShapeSpawner_helpers::SpawnBoxAtPoint(
					*SpawnActor, *RootComponent, *SourceComponent, SettingsTag, Point, Dims,
					ObjFlags);
				break;
			case EAGX_ShapeSpawnerType::Capsule:
				AGX_ShapeSpawner_helpers::SpawnCapsuleAtPoint(
					*SpawnActor, *RootComponent, *SourceComponent, SettingsTag, Point, Dims,
					ObjFlags);
				break;
			}
		}
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
