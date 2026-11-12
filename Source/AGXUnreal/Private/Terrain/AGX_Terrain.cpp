// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_Terrain.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_CustomVersion.h"
#include "AGX_InternalDelegateAccessor.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Terrain/AGX_CuttingDirectionComponent.h"
#include "Terrain/AGX_CuttingEdgeComponent.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_TerrainLandscapeRendererComponent.h"
#include "Terrain/AGX_TerrainProperties.h"
#include "Terrain/AGX_TerrainRVTHeightSource.h"
#include "Terrain/AGX_TerrainRVTLandscapeRendererComponent.h"
#include "Terrain/AGX_TerrainSpriteComponent.h"
#include "Terrain/AGX_TopEdgeComponent.h"
#include "Terrain/ParticleRendering/AGX_SoilParticleRendererComponent.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Containers/Ticker.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "Misc/AssertionMacros.h"
#include "Misc/EngineVersionComparison.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"

#ifdef LOCTEXT_NAMESPACE
#error "LOCTEXT_NAMESPACE leakage."
#endif
#define LOCTEXT_NAMESPACE "AAGX_Terrain"

AAGX_Terrain::AAGX_Terrain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

#if WITH_EDITOR && UE_VERSION_OLDER_THAN(5, 0, 0) == false
	// Actors that are spatially loaded (streamed in/out via world partitioning) may not reference
	// actors that are not. Since the ALanscape is not spatially loaded, the AGX_Terrain cannot be
	// either since we reference an ALandscape from it. Default value for all actors in OpenWorld is
	// true.
	bIsSpatiallyLoaded = false;
#endif

	SpriteComponent = CreateDefaultSubobject<UAGX_TerrainSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());
	RootComponent = SpriteComponent;

	TerrainBounds = CreateDefaultSubobject<UAGX_HeightFieldBoundsComponent>(TEXT("TerrainBounds"));
	
	DefaultParticleRenderer =
		CreateDefaultSubobject<UAGX_SoilParticleRendererComponent>(
		TEXT("DefaultParticleRenderer"));

	DefaultLandscapeRenderer = CreateDefaultSubobject<UAGX_TerrainLandscapeRendererComponent>(
		TEXT("DefaultLandscapeRenderer"));

	// Set render targets and niagara system from plugin by default to reduce manual steps when
	// using Terrain.
	auto AssignDefault = [](auto*& AssetRefProperty, const TCHAR* Path)
	{
		if (AssetRefProperty != nullptr)
			return;

		using Type = typename std::remove_reference<decltype(*AssetRefProperty)>::type;
		auto AssetFinder = ConstructorHelpers::FObjectFinder<Type>(Path);
		if (!AssetFinder.Succeeded())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Expected to find asset '%s' but it was not found."), Path);
			return;
		}

		AssetRefProperty = AssetFinder.Object;
	};

	AssignDefault(
		LandscapeDisplacementMap,
		TEXT("TextureRenderTarget2D'/AGXUnreal/Terrain/Rendering/HeightField/"
			 "RT_LandscapeDisplacementMap.RT_LandscapeDisplacementMap'"));
	LastDeprecatedDisplacementMap = LandscapeDisplacementMap;
}

void AAGX_Terrain::SetEnabled(bool InEnabled)
{
	if (HasNative())
		NativeBarrier.SetEnabled(InEnabled);

	bIsEnabled = InEnabled;
}

bool AAGX_Terrain::IsEnabled() const
{
	if (HasNative())
		return NativeBarrier.GetEnabled();

	return bIsEnabled;
}

void AAGX_Terrain::SetCanCollide(bool bInCanCollide)
{
	// CanCollide is set on the Terrain Pager native if using Terrain Paging, otherwise set on the
	// regular Terrain native. TerrainPager.OnTemplateTerrainChanged has no effect when disabling
	// collision on a template Terrain.
	if (HasNativeTerrainPager())
	{
		NativeTerrainPagerBarrier.SetCanCollide(bInCanCollide);
	}
	else if (HasNative())
	{
		NativeBarrier.SetCanCollide(bInCanCollide);
	}

	bCanCollide = bInCanCollide;
}

bool AAGX_Terrain::GetCanCollide() const
{
	// There is no clean CanCollide check for a Terrain pager, in that case we fall back on
	// the member in the Terrain actor.
	if (HasNative() && !bEnableTerrainPaging)
	{
		return NativeBarrier.GetCanCollide();
	}

	return bCanCollide;
}

bool AAGX_Terrain::SetTerrainProperties(UAGX_TerrainProperties* InTerrainProperties)
{
	UAGX_TerrainProperties* TerrainPropertiesOrig = TerrainProperties;
	if (TerrainPropertiesOrig != nullptr && HasNativeTerrainPager())
		TerrainPropertiesOrig->UnregisterTerrainPager(*this);

	TerrainProperties = InTerrainProperties;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeTerrainProperties is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeTerrainProperties())
	{
		// Something went wrong, restore original TerrainProperties.
		TerrainProperties = TerrainPropertiesOrig;
		if (HasNativeTerrainPager())
			TerrainProperties->RegisterTerrainPager(*this);
		return false;
	}

	return true;
}

bool AAGX_Terrain::SetTerrainMaterial(UAGX_TerrainMaterial* InTerrainMaterial)
{
	UAGX_TerrainMaterial* TerrainMaterialOrig = TerrainMaterial;
	TerrainMaterial = InTerrainMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeTerrainMaterial is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeTerrainMaterial())
	{
		// Something went wrong, restore original TerrainMaterial.
		TerrainMaterial = TerrainMaterialOrig;
		return false;
	}

	return true;
}

bool AAGX_Terrain::SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial)
{
	UAGX_ShapeMaterial* ShapeMaterialOrig = ShapeMaterial;
	ShapeMaterial = InShapeMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeShapeMaterial is responsible to create an instance if none exists and do the
	// asset/instance swap.
	if (!UpdateNativeShapeMaterial())
	{
		// Something went wrong, restore original ShapeMaterial.
		ShapeMaterial = ShapeMaterialOrig;
		return false;
	}

	return true;
}

void AAGX_Terrain::AddCollisionGroup(FName GroupName)
{
	if (GroupName.IsNone())
	{
		return;
	}

	if (CollisionGroups.Contains(GroupName))
		return;

	CollisionGroups.Add(GroupName);
	if (HasNative())
		NativeBarrier.AddCollisionGroup(GroupName);
}

void AAGX_Terrain::RemoveCollisionGroupIfExists(FName GroupName)
{
	if (GroupName.IsNone())
		return;

	auto Index = CollisionGroups.IndexOfByKey(GroupName);
	if (Index == INDEX_NONE)
		return;

	CollisionGroups.RemoveAt(Index);
	if (HasNative())
		NativeBarrier.RemoveCollisionGroup(GroupName);
}

UNiagaraComponent* AAGX_Terrain::GetSpawnedParticleSystemComponent()
{
	return DefaultParticleRenderer->GetParticleSystemComponent();
}

int32 AAGX_Terrain::GetNumParticles() const
{
	if (!HasNative())
		return 0;

	if (HasNativeTerrainPager())
	{
		return static_cast<int32>(NativeTerrainPagerBarrier.GetNumParticles());
	}
	else
	{
		check(HasNative());
		return static_cast<int32>(NativeBarrier.GetNumParticles());
	}
}

namespace AGX_Terrain_helpers
{
	FShovelReferenceWithSettings* FindShovelSettings(
		TArray<FShovelReferenceWithSettings>& Shovels, UAGX_ShovelComponent* Shovel,
		const TCHAR* FunctionName, const TCHAR* TerrainName)
	{
		FShovelReferenceWithSettings* Element =
			Shovels.FindByPredicate([Shovel](const FShovelReferenceWithSettings& Element)
									{ return Element.Shovel.GetShovelComponent() == Shovel; });
		if (Element == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("%s called for a shovel that the AGX Terrain '%s' doesn't know about."),
				FunctionName, TerrainName);
			return nullptr;
		}

		return Element;
	}

	bool WriteBarrierRadii(FTerrainPagerBarrier& Barrier, FShovelReferenceWithSettings& Element)
	{
		if (!Barrier.HasNative())
		{
			return false;
		}

		UAGX_ShovelComponent* Shovel = Element.Shovel.GetShovelComponent();
		if (Shovel == nullptr)
		{
			return false;
		}

		UAGX_RigidBodyComponent* Body = Shovel->RigidBody.GetRigidBody();
		if (Body == nullptr || !Body->HasNative())
		{
			return false;
		}

		return Barrier.SetTileLoadRadii(
			*Body->GetNative(), Element.PreloadRadius, Element.RequiredRadius);
	}
}

bool AAGX_Terrain::SetPreloadRadius(UAGX_ShovelComponent* Shovel, double InPreloadRadius)
{
	FShovelReferenceWithSettings* Element = AGX_Terrain_helpers::FindShovelSettings(
		TerrainPagingSettings.TrackedShovels, Shovel, TEXT("Set Preload Radius"), *GetName());
	if (Element == nullptr)
	{
		return false;
	}

	Element->PreloadRadius = InPreloadRadius;
	return AGX_Terrain_helpers::WriteBarrierRadii(NativeTerrainPagerBarrier, *Element);
}

bool AAGX_Terrain::SetRequiredRadius(UAGX_ShovelComponent* Shovel, double InRequiredRadius)
{
	FShovelReferenceWithSettings* Element = AGX_Terrain_helpers::FindShovelSettings(
		TerrainPagingSettings.TrackedShovels, Shovel, TEXT("Set Required Radius"), *GetName());
	if (Element == nullptr)
	{
		return false;
	}

	Element->RequiredRadius = InRequiredRadius;
	return AGX_Terrain_helpers::WriteBarrierRadii(NativeTerrainPagerBarrier, *Element);
}

bool AAGX_Terrain::SetTerrainPagerRadii(
	UAGX_ShovelComponent* Shovel, double InPreloadRadius, double InRequiredRadius)
{
	FShovelReferenceWithSettings* Element = AGX_Terrain_helpers::FindShovelSettings(
		TerrainPagingSettings.TrackedShovels, Shovel, TEXT("Set Required Radius"), *GetName());
	if (Element == nullptr)
	{
		return false;
	}

	Element->PreloadRadius = InPreloadRadius;
	Element->RequiredRadius = InRequiredRadius;
	return AGX_Terrain_helpers::WriteBarrierRadii(NativeTerrainPagerBarrier, *Element);
}

void AAGX_Terrain::SetEnableTerrainPaging(bool bEnabled)
{
	bEnableTerrainPaging = bEnabled;
}

bool AAGX_Terrain::GetEnableTerrainPaging() const
{
	return bEnableTerrainPaging;
}

bool AAGX_Terrain::HasNative() const
{
	return NativeBarrier.HasNative() && (!bEnableTerrainPaging || HasNativeTerrainPager());
}

bool AAGX_Terrain::HasNativeTerrainPager() const
{
	return NativeTerrainPagerBarrier.HasNative();
}

FTerrainBarrier* AAGX_Terrain::GetOrCreateNative()
{
	if (!HasNative())
		InitializeNative();

	return GetNative();
}

FTerrainBarrier* AAGX_Terrain::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

const FTerrainBarrier* AAGX_Terrain::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

FTerrainPagerBarrier* AAGX_Terrain::GetOrCreateNativeTerrainPager()
{
	if (!HasNative())
		InitializeNative();

	return GetNativeTerrainPager();
}

FTerrainPagerBarrier* AAGX_Terrain::GetNativeTerrainPager()
{
	if (!NativeTerrainPagerBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeTerrainPagerBarrier;
}

const FTerrainPagerBarrier* AAGX_Terrain::GetNativeTerrainPager() const
{
	if (!NativeTerrainPagerBarrier.HasNative())
	{
		return nullptr;
	}

	return &NativeTerrainPagerBarrier;
}

namespace
{
	/**
	Calculates and returns the smallest base size of a square sized texture,
	such that the base size is evenly divisible by pixelsPerItem and has a square
	that is at least minNumItems x pixelsPerItem.
	*/
	int32 CalculateTextureBaseSize(int32 MinNumItems, int32 PixelsPerItem)
	{
		// Max size taken from UTextureRenderTarget2D::PostEditChangeProperty in
		// Engine/Source/Runtime/Engine/Private/TextureRenderTarget2D.cpp. Update here
		// if future versions of Unreal Engine allow larger render targets.
		const int32 MaxSide = 8192;
		const int32 NumPixels = MinNumItems * PixelsPerItem;
		int32 Side =
			FMath::Clamp(FMath::CeilToInt(FMath::Sqrt(static_cast<double>(NumPixels))), 0, MaxSide);
		// We might not get a good side length on the first attempt, so search upwards until we
		// find one.
		for (; Side <= MaxSide; ++Side)
		{
			if ((Side % PixelsPerItem == 0) && (Side * Side >= NumPixels))
			{
				return Side;
			}
		}
		AGX_CHECK(!"CalculateTextureBaseSize failed");
		return 0;
	}
}

#if WITH_EDITOR

void AAGX_Terrain::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	ValidateLandscapeBounds();
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void AAGX_Terrain::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

bool AAGX_Terrain::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (!HasNative())
		return SuperCanEditChange;

	const FName Prop = InProperty->GetFName();

	// Properties that should never be edited during Play.
	if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, SourceLandscape))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, Shovels))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, LandscapeDisplacementMap))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, bEnableTerrainPaging))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, SimulationResolution))
		return false;
	else
		return SuperCanEditChange;
}

namespace AGX_Terrain_helpers
{
	void EnsureUseDynamicMaterialInstance(AAGX_Terrain& Terrain)
	{
		ALandscape* Landscape = Terrain.SourceLandscape;
		if (!IsValid(Landscape) ||
			(Landscape->GetWorld() != nullptr && Landscape->GetWorld()->IsPartitionedWorld()) ||
			AGX_HeightFieldUtilities::IsOpenWorldLandscape(*Landscape) ||
			Landscape->bUseDynamicMaterialInstance)
			return;

		const FText AskEnableDynamicMaterial = LOCTEXT(
			"EnableDynamicMaterial?",
			"The selected Landscape does not have Use Dynamic Material Instance enabled, "
			"meaning that the material parameters for Landscape size and position cannot "
			"be set automatically. Should Use Dynamic Material Instance be enabled on the "
			"Landscape?");
		if (FAGX_NotificationUtilities::YesNoQuestion(AskEnableDynamicMaterial))
		{
			Landscape->Modify();
			Landscape->bUseDynamicMaterialInstance = true;
			Landscape->PostEditChange();
		}
	}
}

void AAGX_Terrain::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bIsEnabled),
		[](ThisClass* This) { This->SetEnabled(This->bIsEnabled); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bCanCollide),
		[](ThisClass* This) { This->SetCanCollide(This->bCanCollide); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, TerrainProperties),
		[](ThisClass* This) { This->SetTerrainProperties(This->TerrainProperties); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, SourceLandscape),
		[](ThisClass* This)
		{
			if (!This->UsesRVTLandscapeRendering())
				AGX_Terrain_helpers::EnsureUseDynamicMaterialInstance(*This);
		});

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bEnableTerrainPaging),
		[](ThisClass* This) { This->SetEnableTerrainPaging(This->bEnableTerrainPaging); });
}

#endif // WITH_EDITOR

void AAGX_Terrain::PostActorCreated()
{
	Super::PostActorCreated();
	bLastDeprecatedRenderingEnabled = bEnableDisplacementRendering;
	LastDeprecatedDisplacementMap = LandscapeDisplacementMap;
}

void AAGX_Terrain::PostLoad()
{
	Super::PostLoad();
	// Blueprint CDO defaults use UClass::SerializeDefaultObject, bypassing this actor's
	// Serialize override. Read the package version after subobjects have been instanced.
	if (GetLinkerCustomVersion(FAGX_CustomVersion::GUID) <
			FAGX_CustomVersion::TerrainLandscapeRenderingComponents &&
		DefaultLandscapeRenderer != nullptr)
	{
		DefaultLandscapeRenderer->bEnabled = bEnableDisplacementRendering;
		DefaultLandscapeRenderer->LandscapeDisplacementMap = LandscapeDisplacementMap;
#if WITH_EDITOR
		MarkPackageDirty();
#endif
	}
	bLastDeprecatedRenderingEnabled = bEnableDisplacementRendering;
	LastDeprecatedDisplacementMap = LandscapeDisplacementMap;
	ValidateLandscapeBounds();
#if WITH_EDITOR
	if (bNeedsShapeMaterialWarning)
	{
		// We do this late to avoid issue with Editor not starting after showing the dialog.
		FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateLambda(
				[this](float DeltaTime)
				{
					if (!IsValid(this) || !bNeedsShapeMaterialWarning)
						return false; // Returning false means run only once.

					ShowShapeMaterialWarning();
					bNeedsShapeMaterialWarning = false;
					return false; // Returning false means run only once.
				}),
			/*delay*/ 5.f);
	}
#endif
}

void AAGX_Terrain::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		PreStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPreStepForwardInternal(*Simulation)
				.AddLambda(
					[this](double)
					{
						if (bEnableTerrainPaging && SourceLandscape != nullptr)
							HeightFetcher->ServiceRequests(*SourceLandscape);
					});
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda([this](double) { UpdateLandscapeRendering(); });
	}
}

void AAGX_Terrain::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (HasNativeTerrainPager() && TerrainProperties != nullptr)
		TerrainProperties->UnregisterTerrainPager(*this);

	TInlineComponentArray<UAGX_TerrainLandscapeRendererComponent*> LegacyRenderers(this);
	for (auto* Renderer : LegacyRenderers)
		Renderer->Shutdown();
	TInlineComponentArray<UAGX_TerrainRVTLandscapeRendererComponent*> RVTRenderers(this);
	for (auto* Renderer : RVTRenderers)
		Renderer->Shutdown();
	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		FAGX_InternalDelegateAccessor::GetOnPreStepForwardInternal(*Simulation)
			.Remove(PreStepForwardHandle);
		FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
			.Remove(PostStepForwardHandle);
	}
	if (HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			// @todo Figure out how to handle Terrain Materials. A Terrain Material can be
			// shared between many Terrains in theory. We only want to remove the Terrain
			// Material from the simulation if this Terrain is the last one using it. Some
			// reference counting may be needed.
			Simulation->Remove(*this);

		}
	}

	if (HasNativeTerrainPager())
	{
		NativeTerrainPagerBarrier.ReleaseNative();
	}
	if (NativeBarrier.HasNative())
		NativeBarrier.ReleaseNative();
	HeightFetcher->ResetSnapshots();
	RVTHeightSource.Reset();
	HeightFetcher->SetTerrain(nullptr);
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_Terrain::Tick"));
	Super::Tick(DeltaTime);

	UpdateParticlesArrays();
	if (bEnableTerrainPaging && SourceLandscape != nullptr)
		HeightFetcher->ServiceRequests(*SourceLandscape);
}

FTransform AAGX_Terrain::GetNativeTransform() const
{
	check(HasNative());

	if (bEnableTerrainPaging)
		return FTransform(
			NativeTerrainPagerBarrier.GetReferenceRotation(),
			NativeTerrainPagerBarrier.GetReferencePoint());
	else
		return FTransform(NativeBarrier.GetRotation(), NativeBarrier.GetPosition());
}

namespace
{
	UAGX_RigidBodyComponent* GetBodyComponent(
		AActor* OwningActor, const FString& BodyName, const TCHAR* TerrainName)
	{
		TArray<UAGX_RigidBodyComponent*> Bodies;
		OwningActor->GetComponents(Bodies, false);
		UAGX_RigidBodyComponent** It = Bodies.FindByPredicate(
			[BodyName](UAGX_RigidBodyComponent* Body) { return BodyName == Body->GetName(); });
		if (It == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot create shovel: Shovel Actor '%s' in terrain '%s' could not be created "
					 "because the configured shovel body '%s' does not exist in the shovel Actor."),
				*OwningActor->GetName(), TerrainName, *BodyName);

			return nullptr;
		}
		return *It;
	}

	template <typename TPtr>
	TPtr GetShovelComponent(UAGX_RigidBodyComponent& Body, const TCHAR* TerrainName)
	{
		auto RecursiveFind = [](const TArray<USceneComponent*>& Components, auto& recurse)
		{
			for (USceneComponent* Component : Components)
			{
				if (TPtr Match = Cast<std::remove_pointer_t<TPtr>>(Component))
				{
					return Match;
				}
				if (TPtr Match = recurse(Component->GetAttachChildren(), recurse))
				{
					return Match;
				}
			}
			return TPtr(nullptr);
		};

		TPtr Result = RecursiveFind(Body.GetAttachChildren(), RecursiveFind);
		if (Result == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to find Shovel Component '%s' in Rigid Body '%s'. Make sure it has "
					 "been added as a child to the Rigid Body."),
				*std::remove_pointer<TPtr>::type::StaticClass()->GetName(), *Body.GetName());
		}

		return Result;
	}

	template <typename TPtr>
	TPtr GetShovelComponent(AActor* Owner, const TCHAR* TerrainName)
	{
		using TType = typename std::remove_pointer<TPtr>::type;
		TArray<TPtr> Components;
		Owner->GetComponents<TType>(Components);
		if (Components.Num() != 1)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("The shovel '%s' in the AGX Terrain '%s' is invalid because it doesn't have "
					 "exactly one '%s'"),
				*Owner->GetName(), TerrainName, *TType::StaticClass()->GetName());
			return nullptr;
		}
		return Components[0];
	}

}

void AAGX_Terrain::InitializeNative()
{
	if (SourceLandscape == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("No source landscape selected for terrain '%s'."),
			*GetLabelSafe(this));
		return;
	}

	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Initialize Native called on AGX Terrain '%s' that has already been initialized."),
			*GetLabelSafe(this));
		return;
	}

	ValidateLandscapeBounds();
	const bool bPartitionedSource = GetWorld() != nullptr && GetWorld()->IsPartitionedWorld();
	const double Spacing = GetSimulationResolution();
	const FVector Scale = SourceLandscape->GetActorScale3D();
	const FRotator Rotation = SourceLandscape->GetActorRotation();
	if (!FMath::IsFinite(Spacing) || Spacing <= 0.0 || !FMath::IsFinite(SimulationResolution) ||
		SimulationResolution < 0.0 || Scale.ContainsNaN() || Scale.X <= 0.0 || Scale.Y <= 0.0 ||
		Scale.Z <= 0.0 || !FMath::IsNearlyZero(Rotation.Pitch) ||
		!FMath::IsNearlyZero(Rotation.Roll) || (bPartitionedSource && !bEnableTerrainPaging) ||
		(SimulationResolution > 0.0 && !bEnableTerrainPaging))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Terrain '%s' requires positive finite resolution and scale, an upright Landscape, "
				"and Terrain Paging for partitioned sources or custom simulation resolution."),
			*GetName());
		return;
	}
	HeightFetcher->SetTerrain(this);

	if (!CreateNative())
	{
		return; // Logging done in CreateNative.
	}

	if (bEnableTerrainPaging)
	{
		if (!CreateNativeTerrainPager())
		{
			return; // Logging done in CreateNativeTerrainPager.
		}
	}

	SetCanCollide(bCanCollide);
	SetEnabled(bIsEnabled);
	CreateNativeShovels();
	AddTerrainPagerBodies();
	InitializeRendering();
	UpdateNativeTerrainProperties();

	if (!UpdateNativeTerrainMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeTerrainMaterial returned false in AGX_Terrain '%s'. "
				 "Ensure the selected Terrain Material is valid."),
			*GetName());
	}
	if (HasNativeTerrainPager())
	{
		FTerrainMaterialBarrier* DefaultTerrainMaterial =
			TerrainMaterial != nullptr ? TerrainMaterial->GetTerrainMaterialNative() : nullptr;
		NativeTerrainPagerBarrier.CreateTerrainDataSource(HeightFetcher, DefaultTerrainMaterial);
	}

	if (!UpdateNativeShapeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeShapeMaterial returned false in AGX_Terrain '%s'. "
				 "Ensure the selected Shape Material is valid."),
			*GetName());
	}

	if (HasNativeTerrainPager() && ShapeMaterial != nullptr && TerrainMaterial != nullptr &&
		ShapeMaterial->IsInstance() && TerrainMaterial->IsInstance())
	{
		auto SMBarrier = ShapeMaterial->GetOrCreateShapeMaterialNative(GetWorld());
		auto TMBarrier = TerrainMaterial->GetOrCreateTerrainMaterialNative(GetWorld());
		if (SMBarrier != nullptr && TMBarrier != nullptr)
			NativeTerrainPagerBarrier.SetAssociatedMaterial(*TMBarrier, *SMBarrier);
	}
}

bool AAGX_Terrain::CreateNative()
{
	TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo> Bounds =
		((bEnableTerrainPaging && UsesRVTLandscapeRendering()) ||
		 (GetWorld() != nullptr && GetWorld()->IsPartitionedWorld()))
			? TerrainBounds->GetUserSetBounds()
			: TerrainBounds->GetLandscapeAdjustedBounds();
	if (!Bounds.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to create Terrain native for '%s'; the given Terrain Bounds was "
				 "invalid."),
			*GetName());
		return false;
	}

	const FVector StartPos = Bounds->Transform.TransformPositionNoScale(-Bounds->HalfExtent);

	FHeightFieldShapeBarrier HeightField = [this, &StartPos, &Bounds]()
	{
		if (bEnableTerrainPaging)
		{
			// For the TerrainPaging case, this Terrain is the "Template Terrain" used by the
			// Terrain Pager. In this case, we can set small "dummy" values for resolution, size
			// and heights which will save memory usage. See AGX Dynamics example
			// 'basic_paging_example.agxPy' for reference.
			FHeightFieldShapeBarrier HeightField;
			HeightField.AllocateNative(4, 4, 1.0, 1.0);
			return HeightField;
		}
		else
		{
			return AGX_HeightFieldUtilities::CreateHeightField(
				*SourceLandscape, StartPos, Bounds->HalfExtent.X * 2.0, Bounds->HalfExtent.Y * 2.0);
		}
	}();

	NativeBarrier.AllocateNative(HeightField, MaxDepth);

	if (!NativeBarrier.HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to create Terrain native for '%s'. The Output log may include more "
				 "details."),
			*GetName());
		return false;
	}

	NativeBarrier.SetRotation(Bounds->Transform.GetRotation());
	NativeBarrier.SetPosition(Bounds->Transform.GetLocation());

	const double SpacingX = GetSimulationResolution();
	const double SpacingY = bEnableTerrainPaging ? SpacingX : SourceLandscape->GetActorScale3D().Y;
	const double GridX = Bounds->HalfExtent.X * 2.0 / SpacingX;
	const double GridY = Bounds->HalfExtent.Y * 2.0 / SpacingY;
	if (GridX > MAX_int32 - 1 || GridY > MAX_int32 - 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Terrain '%s' coverage exceeds the supported grid coordinate range."), *GetName());
		return false;
	}
	NumVerticesX = FMath::RoundToInt(GridX) + 1;
	NumVerticesY = FMath::RoundToInt(GridY) + 1;

	NativeBarrier.AddCollisionGroups(CollisionGroups);

	if (bEnableTerrainPaging)
		HeightFetcher->ConfigureSnapshots(
			Bounds->Transform, FVector2D(Bounds->HalfExtent.X, Bounds->HalfExtent.Y),
			GetSimulationResolution());

	// Create the AGX Dynamics instance for the terrain.
	// Note that the AGX Dynamics Terrain messes with the solver parameters on add, parameters that
	// our user may have set explicitly. If so, re-set the user-provided settings.
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Terrain '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	int32 NumIterations = Simulation->GetNumPpgsIterations();

	if (!bEnableTerrainPaging)
	{
		if (UsesRVTLandscapeRendering())
			RVTHeightSource =
				MakeShared<FAGX_TerrainRVTHeightSource>(NativeBarrier, GetNativeTransform());
		// We add this Terrain to the Simulation here only if we are not using TerrainPaging.
		Simulation->Add(*this);
	}

	if (Simulation->bOverridePPGSIterations)
	{
		// We must check the override flag and not blindly re-set the value we read a few lines up
		// because when not overriding one should get the number of iterations set by the terrain,
		// not the number of iterations that is the default in the solver.
		Simulation->SetNumPpgsIterations(NumIterations);
	}
	else
	{
		// Not overriding the number of iterations, which means that the UAGX_Simulation instance
		// should be notified of the new current number of iterations set by the AGX Dynamics
		// terrain. Not using SetNumPpgsIterations because this code fixes a broken class invariant,
		// it does not move from one valid state to another, so lower-level fiddling is required.
		//
		// I don't like it.
		Simulation->NumPpgsIterations = Simulation->GetNative()->GetNumPpgsIterations();
	}

	return true;
}

bool AAGX_Terrain::CreateNativeTerrainPager()
{
	check(NativeBarrier.HasNative());
	check(!HasNativeTerrainPager());

	if (!bEnableTerrainPaging)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeTerrainPager called on Terrain '%s' which doesn't use Terrain "
				 "Paging."),
			*GetName());
		return false;
	}

	// Always set DeleteParticlesOutsideBounds to false if we are using Terrain Paging, otherwise
	// particles may be deleted when tiles are loaded and unloaded in an unexpected way. This will
	// be handled automatically by AGX Dynamics in the future.
	if (TerrainProperties != nullptr && TerrainProperties->bDeleteParticlesOutsideBounds)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("DeleteParticlesOutsideBounds was set to true while using Terrain Paging. This "
				 "combination is not supported. DeleteParticlesOutsideBounds will be set to "
				 "false."));
		TerrainProperties->SetDeleteParticlesOutsideBounds(false);
	}

	const double QuadSize = GetSimulationResolution();
	if (!FMath::IsFinite(TerrainPagingSettings.TileSize) ||
		!FMath::IsFinite(TerrainPagingSettings.TileOverlap) ||
		TerrainPagingSettings.TileSize / QuadSize < 1.5 ||
		TerrainPagingSettings.TileSize / QuadSize > 1023.0 ||
		TerrainPagingSettings.TileOverlap / QuadSize > 1023.0 ||
		TerrainPagingSettings.TileOverlap < 0.0)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Invalid Terrain paging size or overlap on '%s'. A tile may contain at most 1024 "
				"vertices per side."),
			*GetName());
		return false;
	}
	const int32 TileNumVerticesSide =
		FMath::RoundToInt(TerrainPagingSettings.TileSize / QuadSize) + 1;
	const int32 TileOverlapVertices =
		FMath::RoundToInt(TerrainPagingSettings.TileOverlap / QuadSize);

	if (!FMath::IsFinite(TerrainPagingSettings.TileSize) ||
		!FMath::IsFinite(TerrainPagingSettings.TileOverlap) || TileNumVerticesSide < 3 ||
		TileNumVerticesSide > 1024 || TileOverlapVertices < 0 ||
		TileOverlapVertices >= TileNumVerticesSide - 1)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Invalid Terrain paging grid on '%s'. Tile side must contain 3..1024 vertices; "
				"overlap must be nonnegative and smaller than the tile."),
			*GetName());
		return false;
	}

	NativeTerrainPagerBarrier.AllocateNative(
		NativeBarrier, TileNumVerticesSide, TileOverlapVertices, QuadSize, MaxDepth);

	if (!HasNativeTerrainPager())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to create Terrain Pager native for '%s'. The Output log may include more "
				 "details."),
			*GetName());
		return false;
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Terrain '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	Simulation->Add(*this);
	return true;
}

void AAGX_Terrain::CreateNativeShovels()
{
	// Todo: most of this logic can be moved to ShovelComponent. Since AGX Dynamics 2.40, Shovels
	// are added to the Simulation instead of the Terrains.

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeShovels called on Terrain '%s' which doesn't have a native "
				 "representation."),
			*GetName());
	}

	auto AddLegacyShovel =
		[this](FShovelBarrier& ShovelBarrier, double RequiredRadius, double PreloadRadius) -> bool
	{
		if (bEnableTerrainPaging)
		{
			return NativeTerrainPagerBarrier.AddShovel(
				ShovelBarrier, RequiredRadius, PreloadRadius);
		}
		else
		{
			UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
			if (Simulation == nullptr)
				return false;
			return Simulation->GetNative()->Add(ShovelBarrier);
		}
	};

	// Create and register legacy shovels.
	for (FAGX_Shovel& Shovel : Shovels)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Deprecation warning: AGX Terrain '%s': AAGX_Terrain::Shovels has been deprecated "
				 "and will be removed in a future release. Use AAGX_Terrain::ShovelComponents "
				 "instead."),
			*GetLabelSafe(this));

		if (Shovel.RigidBodyActor == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("A shovel in the AGX Terrain '%s' is invalid because it does not reference "
					 "any Actor."),
				*GetName());
			continue;
		}

		AActor* Actor = Shovel.RigidBodyActor;
		UAGX_RigidBodyComponent* Body = ::GetBodyComponent(Actor, Shovel.BodyName, *GetName());
		if (Body == nullptr)
		{
			// Error message printed by GetBodyComponent.
			continue;
		}
		UAGX_TopEdgeComponent* TopEdge = GetShovelComponent<decltype(TopEdge)>(*Body, *GetName());
		UAGX_CuttingEdgeComponent* CuttingEdge =
			GetShovelComponent<decltype(CuttingEdge)>(*Body, *GetName());
		UAGX_CuttingDirectionComponent* CuttingDirection =
			GetShovelComponent<decltype(CuttingDirection)>(*Body, *GetName());

		if (TopEdge == nullptr || CuttingEdge == nullptr || CuttingDirection == nullptr)
		{
			// GetShovelComponent is responsible for printing the error message.
			continue;
		}

		FShovelBarrier ShovelBarrier;
		FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
		const FTransform WorldToBody = Body->GetComponentTransform().Inverse();
		FTwoVectors TopEdgeLine = TopEdge->GetInLocal(WorldToBody);
		FTwoVectors CuttingEdgeLine = CuttingEdge->GetInLocal(WorldToBody);

		// AGX Dynamics always expects a normalized Cutting Direction vector.
		const FVector CuttingDirectionVector =
			WorldToBody.TransformVector(CuttingDirection->GetVectorDirection()).GetSafeNormal();

		ShovelBarrier.AllocateNative(
			*BodyBarrier, TopEdgeLine, CuttingEdgeLine, CuttingDirectionVector, Shovel.ToothLength);

		FAGX_Shovel::UpdateNativeShovelProperties(ShovelBarrier, Shovel);

		bool Added = AddLegacyShovel(ShovelBarrier, Shovel.RequiredRadius, Shovel.PreloadRadius);
		if (!Added)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Terrain '%s' rejected shovel '%s'. Reversing edge directions and trying "
					 "again."),
				*GetName(), *Actor->GetName());
			std::swap(TopEdgeLine.v1, TopEdgeLine.v2);
			std::swap(CuttingEdgeLine.v1, CuttingEdgeLine.v2);
			ShovelBarrier.SetTopEdge(TopEdgeLine);
			ShovelBarrier.SetCuttingEdge(CuttingEdgeLine);
			Added = AddLegacyShovel(ShovelBarrier, Shovel.RequiredRadius, Shovel.PreloadRadius);
			if (!Added)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Terrain '%s' rejected shovel '%s' after edge direction flip. Abandoning "
						 "shovel."),
					*GetName(), *Actor->GetName());
				continue;
			}
			UE_LOG(
				LogAGX, Warning,
				TEXT("Shovel with reversed edges added successfully. Consider flipping the edges "
					 "in the editor."));
		}

		UE_LOG(
			LogAGX, Log, TEXT("Created shovel '%s' for terrain '%s'."), *Actor->GetName(),
			*GetName());
	}

	// Register Terrain Paging Shovel Components (the Shovel Components add themselves to the
	// Simulation).
	if (bEnableTerrainPaging)
	{
		for (FShovelReferenceWithSettings& ShovelRef : TerrainPagingSettings.TrackedShovels)
		{
			UAGX_ShovelComponent* ShovelComponent = ShovelRef.Shovel.GetShovelComponent();
			if (ShovelComponent == nullptr)
			{
				const FString Message = FString::Printf(
					TEXT("AGX Terrain '%s' have a Shovel reference to '%s' in '%s' that does not "
						 "reference a valid Shovel."),
					*GetLabelSafe(this), *ShovelRef.Shovel.Name.ToString(),
					*GetLabelSafe(ShovelRef.Shovel.OwningActor));
				FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
				continue;
			}

			FShovelBarrier* ShovelBarrier = ShovelComponent->GetOrCreateNative();
			if (ShovelBarrier == nullptr)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Shovel '%s' in AGX Terrain '%s' could not create AGX Dynamics "
						 "representation. Ignoring this shovel. It will not be able to deform the "
						 "Terrain."),
					*ShovelComponent->GetName(), *GetLabelSafe(this));
				continue;
			}
			check(ShovelBarrier->HasNative());

			const double RequiredRadius = ShovelRef.RequiredRadius;
			const double PreloadRadius = ShovelRef.PreloadRadius;
			NativeTerrainPagerBarrier.AddShovel(*ShovelBarrier, RequiredRadius, PreloadRadius);
		}
	}
}

void AAGX_Terrain::AddTerrainPagerBodies()
{
	if (!HasNativeTerrainPager())
		return;

	for (FAGX_TerrainPagingBodyReference& TrackedBody : TerrainPagingSettings.TrackedRigidBodies)
	{
		UAGX_RigidBodyComponent* Body = TrackedBody.RigidBody.GetRigidBody();
		if (Body == nullptr)
			continue;

		FRigidBodyBarrier* BodyBarrier = Body->GetOrCreateNative();
		if (BodyBarrier == nullptr)
			continue;

		NativeTerrainPagerBarrier.AddRigidBody(
			*Body->GetNative(), TrackedBody.RequiredRadius, TrackedBody.PreloadRadius);
	}
}

void AAGX_Terrain::InitializeRendering()
{
	UpdateLandscapeRendering();
}

double AAGX_Terrain::GetSimulationResolution() const
{
	return SimulationResolution > 0.0
			   ? SimulationResolution
			   : (SourceLandscape != nullptr ? SourceLandscape->GetActorScale3D().X : 0.0);
}

bool AAGX_Terrain::UsesRVTLandscapeRendering() const
{
	TInlineComponentArray<UAGX_TerrainRVTLandscapeRendererComponent*> Renderers(this);
	return !Renderers.IsEmpty();
}

void AAGX_Terrain::ValidateLandscapeBounds()
{
	if (TerrainBounds != nullptr && SourceLandscape != nullptr &&
		((GetWorld() != nullptr && GetWorld()->IsPartitionedWorld()) ||
		 UsesRVTLandscapeRendering()))
		TerrainBounds->bInfiniteBounds = false;
}

TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe> AAGX_Terrain::GetSourceTileSnapshot(
	const FVector& Corner, int32 Side) const
{
	if (RVTHeightSource.IsValid())
		return RVTHeightSource->GetSourceTileSnapshot(Corner, Side);
	return HeightFetcher->GetOrRequestSnapshot(Corner, Side, Side);
}

TArray<FTerrainTileHeights> AAGX_Terrain::GetLandscapeRenderUpdates(
	TMap<FIntPoint, uint64>& KnownTiles)
{
	if (HasNativeTerrainPager())
		return NativeTerrainPagerBarrier.GetTileHeightUpdates(KnownTiles);
	return RVTHeightSource.IsValid()
			   ? RVTHeightSource->GetTileHeightUpdates(NativeBarrier, KnownTiles)
			   : TArray<FTerrainTileHeights>();
}

int64 AAGX_Terrain::GetLandscapeSourceHeightBytes() const
{
	return RVTHeightSource.IsValid() ? RVTHeightSource->GetRetainedBytes() : 0;
}

void AAGX_Terrain::UpdateLandscapeRendering()
{
	if (!HasNative())
		return;
	// Preserve writes through the released C++ properties without making them the new
	// component's authority. Component changes are independent until an old property changes.
	if (DefaultLandscapeRenderer != nullptr)
	{
		if (bLastDeprecatedRenderingEnabled != bEnableDisplacementRendering)
			DefaultLandscapeRenderer->SetEnabled(bEnableDisplacementRendering);
		if (LastDeprecatedDisplacementMap.Get() != LandscapeDisplacementMap)
		{
			DefaultLandscapeRenderer->Shutdown();
			DefaultLandscapeRenderer->LandscapeDisplacementMap = LandscapeDisplacementMap;
		}
	}
	bLastDeprecatedRenderingEnabled = bEnableDisplacementRendering;
	LastDeprecatedDisplacementMap = LandscapeDisplacementMap;
	TInlineComponentArray<UAGX_TerrainLandscapeRendererComponent*> Legacy(this);
	TInlineComponentArray<UAGX_TerrainRVTLandscapeRendererComponent*> RVT(this);
	int32 EnabledCount = 0;
	for (auto* Renderer : Legacy)
		EnabledCount += Renderer->bEnabled ? 1 : 0;
	for (auto* Renderer : RVT)
		EnabledCount += Renderer->bEnabled ? 1 : 0;
	const bool bConflict = EnabledCount > 1;
	if (bConflict && !bRendererConflictReported)
		UE_LOG(
			LogAGX, Error,
			TEXT(
				"Terrain '%s' has multiple enabled Landscape renderers. Enable exactly one. "
				"Physics and paging continue."),
			*GetName());
	bRendererConflictReported = bConflict;
	for (auto* Renderer : Legacy)
	{
		if (Renderer->bEnabled && !bConflict)
		{
			Renderer->Initialize(*this);
			Renderer->Update();
		}
		else
			Renderer->Shutdown();
	}
	for (auto* Renderer : RVT)
	{
		if (Renderer->bEnabled && !bConflict)
		{
			Renderer->Initialize(*this);
			Renderer->Update();
		}
		else
			Renderer->Shutdown();
	}
}

bool AAGX_Terrain::UpdateNativeTerrainMaterial()
{
	if (!HasNative())
		return false;

	if (TerrainMaterial == nullptr)
	{
		GetNative()->ClearTerrainMaterial();
		return true;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot update native Terrain material because don't have a world to create "
				 "the material instance in."));
		return false;
	}

	UAGX_TerrainMaterial* Instance = TerrainMaterial->GetOrCreateInstance(World);
	check(Instance);

	if (TerrainMaterial != Instance)
		TerrainMaterial = Instance;

	FTerrainMaterialBarrier* TerrainMaterialBarrier =
		Instance->GetOrCreateTerrainMaterialNative(World);
	check(TerrainMaterialBarrier);

	GetNative()->SetTerrainMaterial(*TerrainMaterialBarrier);

	return true;
}

bool AAGX_Terrain::UpdateNativeTerrainProperties()
{
	if (!HasNative())
		return false;

	if (TerrainProperties == nullptr)
		return false; // Nullptr TerrainProperties not allowed.

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot update native Terrain Properties because don't have a world to create "
				 "the Properties instance in."));
		return false;
	}

	UAGX_TerrainProperties* Instance = TerrainProperties->GetOrCreateInstance(World);
	check(Instance);
	if (HasNativeTerrainPager())
		Instance->RegisterTerrainPager(*this);

	TerrainProperties = Instance;

	FTerrainPropertiesBarrier* TerrainPropertiesBarrier = Instance->GetOrCreateNative();
	check(TerrainPropertiesBarrier);

	GetNative()->SetTerrainProperties(*TerrainPropertiesBarrier);

	return true;
}

bool AAGX_Terrain::UpdateNativeShapeMaterial()
{
	if (!HasNative())
		return false;

	if (ShapeMaterial == nullptr)
	{
		GetNative()->ClearShapeMaterial();
		return true;
	}

	UAGX_ShapeMaterial* Instance =
		static_cast<UAGX_ShapeMaterial*>(ShapeMaterial->GetOrCreateInstance(GetWorld()));
	check(Instance);

	if (ShapeMaterial != Instance)
		ShapeMaterial = Instance;

	FShapeMaterialBarrier* MaterialBarrier = Instance->GetOrCreateShapeMaterialNative(GetWorld());
	check(MaterialBarrier);

	GetNative()->SetShapeMaterial(*MaterialBarrier);
	return true;
}

void AAGX_Terrain::UpdateParticlesArrays()
{
	if (!NativeBarrier.HasNative() || !OnParticleData.IsBound())
	{
		return;
	}


	// Copy data with holes.
	// 
	// TODO: Change the way this data broadcasting is done by letting the user
	// request only the subset of data that the user wants.This is to avoid fetching, 
	// converting, packing, and passing data that will never be looked at. Right now
	// we fetch everything but may not look at everything. 
	EParticleDataFlags ToInclude = EParticleDataFlags::Positions | EParticleDataFlags::Rotations |
								   EParticleDataFlags::Radii | EParticleDataFlags::Velocities | EParticleDataFlags::Masses;
	const FParticleDataById ParticleData =
		bEnableTerrainPaging ? NativeTerrainPagerBarrier.GetParticleDataById(ToInclude)
							 : NativeBarrier.GetParticleDataById(ToInclude);

	const TArray<FVector>& Positions = ParticleData.Positions;
	const TArray<FQuat>& Rotations = ParticleData.Rotations;
	const TArray<float>& Radii = ParticleData.Radii;
	const TArray<bool>& Exists = ParticleData.Exists;
	const TArray<FVector>& Velocities = ParticleData.Velocities;
	const TArray<float>& Masses = ParticleData.Masses;

	const int32 NumParticles = Positions.Num();

	FDelegateParticleData data;
	data.PositionsAndRadii.SetNum(NumParticles);
	data.Orientations.SetNum(NumParticles);
	data.VelocitiesAndMasses.SetNum(NumParticles);
	data.Exists.SetNum(NumParticles);
	data.ParticleCount = NumParticles;

	for (int32 I = 0; I < NumParticles; ++I)
	{
		data.PositionsAndRadii[I] = FVector4(Positions[I], Radii[I]);
		data.Orientations[I] = FVector4(Rotations[I].X, Rotations[I].Y, Rotations[I].Z, Rotations[I].W);
		data.VelocitiesAndMasses[I] =
			FVector4(Velocities[I].X, Velocities[I].Y, Velocities[I].Z, Masses[I]);
		data.Exists[I] = Exists[I];
	}

	OnParticleData.Broadcast(data);
}

void AAGX_Terrain::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::HeightFieldUsesBounds))
	{
		TerrainBounds->bInfiniteBounds = true;
	}

	if (SpriteComponent == nullptr && RootComponent == nullptr &&
		ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainCGDisablerCMRegistrarViewporIcons))
	{
		SpriteComponent = CreateDefaultSubobject<UAGX_TerrainSpriteComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());
		RootComponent = SpriteComponent;
	}

	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainMaterialShapeMaterialSplit) &&
		TerrainMaterial != nullptr && ShapeMaterial == nullptr)
	{
		bNeedsShapeMaterialWarning = true;
	}

	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::ShovelComponentAddsSelf) &&
		bEnableTerrainPaging)
	{
		for (auto& Shovel : ShovelComponents_DEPRECATED)
		{
			// Terrain Pager still needs to know about the shovels in order to track them.
			TerrainPagingSettings.TrackedShovels.Add(Shovel);
		}
	}

	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainPropertiesAsset) &&
		TerrainProperties == nullptr)
	{
		TerrainProperties = GetOrCreateTerrainPropertiesForOldTerrain();
	}

	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::ParticleRenderingByRenderingComponents))
	{
		if (DefaultParticleRenderer != nullptr)
		{
			DefaultParticleRenderer->bEnableParticleRendering = bEnableParticleRendering_DEPRECATED;
			if (ParticleSystemAsset_DEPRECATED != nullptr)
				DefaultParticleRenderer->ParticleSystemAsset = ParticleSystemAsset_DEPRECATED;
		}
	}
}

#if WITH_EDITOR
void AAGX_Terrain::ShowShapeMaterialWarning() const
{
	if (!IsValid(this))
		return;

	const FString Msg = FString::Printf(
		TEXT("Important!\n\nIt was detected that the AGX Terrain Actor '%s' references an AGX "
			 "Terrain Material but no Shape Material. The surface properties of a Terrain is "
			 "no longer described by the Terrain Material, but instead is described by a "
			 "separate Shape Material that can be assigned from the Terrain Actor's Details "
			 "Panel.\n\nIt is recommended to open the Terrain Material and use the 'Create "
			 "Shape Material' button to generate a Shape Material containing the Terrain "
			 "surface properties of the Terrain Material and then assign it to the Terrain "
			 "Actor. Note that this also affects all Contact Materials referencing a Terrain "
			 "Material; these should be updated to point to a Shape Material generated from "
			 "the previously pointed to Terrain Material.\n\nThis information is also "
			 "available in the Changelog in the User Manual.\n\nTo disable this warning, "
			 "simply re-save the Level that contains this Terrain Actor."),
		*GetName());

	FAGX_NotificationUtilities::ShowDialogBoxWithWarning(Msg);
}
#endif // WITH_EDITOR

UAGX_TerrainProperties* AAGX_Terrain::GetOrCreateTerrainPropertiesForOldTerrain()
{
	const FString AssetName = FString::Printf(TEXT("AGX_TEP_%s"), *GetName());
	FString PackagePath = FString::Printf(TEXT("/Game/%s"), *AssetName);
	if (auto Existing = FAGX_ObjectUtilities::GetAssetFromPath<UObject>(*PackagePath))
		return Cast<UAGX_TerrainProperties>(Existing);

	// We have an old Terrain that did not have Terrain Properties.
	// Create a new Terrain Properties asset, set it's values and assign it to the Terrain.
	UPackage* Package = CreatePackage(*PackagePath);
	UAGX_TerrainProperties* Props =
		NewObject<UAGX_TerrainProperties>(Package, *AssetName, RF_Public | RF_Standalone);

	Props->bCreateParticles = bCreateParticles_DEPRECATED;
	Props->bDeleteParticlesOutsideBounds = bDeleteParticlesOutsideBounds_DEPRECATED;
	Props->PenetrationForceVelocityScaling = PenetrationForceVelocityScaling_DEPRECATED;
	Props->MaximumParticleActivationVolume = MaximumParticleActivationVolume_DEPRECATED;
	Props->SoilParticleSizeScaling = SoilParticleSizeScaling_DEPRECATED;

	Package->SetDirtyFlag(true);
	return Props;
}

#undef LOCTEXT_NAMESPACE
