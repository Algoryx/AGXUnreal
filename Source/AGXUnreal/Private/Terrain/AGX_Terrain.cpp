// Copyright 2025, Algoryx Simulation AB.

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
#include "Terrain/AGX_TerrainSpriteComponent.h"
#include "Terrain/AGX_TopEdgeComponent.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_RenderUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Containers/Ticker.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeStreamingProxy.h"
#include "Misc/AssertionMacros.h"
#include "Misc/EngineVersionComparison.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UObjectIterator.h"
#include "WorldPartition/WorldPartition.h"

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

	AssignDefault(
		ParticleSystemAsset, TEXT("NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/"
								  "PS_SoilParticleSystem.PS_SoilParticleSystem'"));
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
	return ParticleSystemComponent;
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
		ShovelComponents, Shovel, TEXT("Set Preload Radius"), *GetName());
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
		ShovelComponents, Shovel, TEXT("Set Required Radius"), *GetName());
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
		ShovelComponents, Shovel, TEXT("Set Required Radius"), *GetName());
	if (Element == nullptr)
	{
		return false;
	}

	Element->PreloadRadius = InPreloadRadius;
	Element->RequiredRadius = InRequiredRadius;
	return AGX_Terrain_helpers::WriteBarrierRadii(NativeTerrainPagerBarrier, *Element);
}

void AAGX_Terrain::SetCreateParticles(bool CreateParticles)
{
	if (HasNative())
	{
		NativeBarrier.SetCreateParticles(CreateParticles);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	bCreateParticles = CreateParticles;
}

void AAGX_Terrain::SetEnableTerrainPaging(bool bEnabled)
{
	bEnableTerrainPaging = bEnabled;
}

bool AAGX_Terrain::GetEnableTerrainPaging() const
{
	return bEnableTerrainPaging;
}

bool AAGX_Terrain::GetCreateParticles() const
{
	if (HasNative())
	{
		return NativeBarrier.GetCreateParticles();
	}

	return bCreateParticles;
}

void AAGX_Terrain::SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds)
{
	if (HasNative())
	{
		NativeBarrier.SetDeleteParticlesOutsideBounds(DeleteParticlesOutsideBounds);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	bDeleteParticlesOutsideBounds = DeleteParticlesOutsideBounds;
}

bool AAGX_Terrain::GetDeleteParticlesOutsideBounds() const
{
	if (HasNative())
	{
		return NativeBarrier.GetDeleteParticlesOutsideBounds();
	}

	return bDeleteParticlesOutsideBounds;
}

void AAGX_Terrain::SetPenetrationForceVelocityScaling(double InPenetrationForceVelocityScaling)
{
	if (HasNative())
	{
		NativeBarrier.SetPenetrationForceVelocityScaling(InPenetrationForceVelocityScaling);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	PenetrationForceVelocityScaling = InPenetrationForceVelocityScaling;
}

double AAGX_Terrain::GetPenetrationForceVelocityScaling() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPenetrationForceVelocityScaling();
	}

	return PenetrationForceVelocityScaling;
}

void AAGX_Terrain::SetPenetrationForceVelocityScaling_BP(float InPenetrationForceVelocityScaling)
{
	SetPenetrationForceVelocityScaling(static_cast<double>(InPenetrationForceVelocityScaling));
}

float AAGX_Terrain::GetPenetrationForceVelocityScaling_BP() const
{
	return static_cast<float>(GetPenetrationForceVelocityScaling());
}

void AAGX_Terrain::SetMaximumParticleActivationVolume(double InMaximumParticleActivationVolume)
{
	if (HasNative())
	{
		NativeBarrier.SetMaximumParticleActivationVolume(InMaximumParticleActivationVolume);
		if (HasNativeTerrainPager())
		{
			NativeTerrainPagerBarrier.OnTemplateTerrainChanged();
		}
	}

	MaximumParticleActivationVolume = InMaximumParticleActivationVolume;
}

double AAGX_Terrain::GetMaximumParticleActivationVolume() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMaximumParticleActivationVolume();
	}

	return MaximumParticleActivationVolume;
}

void AAGX_Terrain::SetMaximumParticleActivationVolume_BP(float InMaximumParticleActivationVolume)
{
	SetMaximumParticleActivationVolume(static_cast<double>(InMaximumParticleActivationVolume));
}

float AAGX_Terrain::GetMaximumParticleActivationVolume_BP() const
{
	return static_cast<float>(GetMaximumParticleActivationVolume());
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
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, ParticleSystemAsset))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, LandscapeDisplacementMap))
		return false;
	else if (Prop == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, bEnableTerrainPaging))
		return false;
	else
		return SuperCanEditChange;
}

namespace AGX_Terrain_helpers
{
	void EnsureUseDynamicMaterialInstance(AAGX_Terrain& Terrain)
	{
		if (!IsValid(Terrain.SourceLandscape))
			return;

		TArray<ALandscapeProxy*> StreamingProxies;
		if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(*Terrain.SourceLandscape))
		{
			for (TObjectIterator<ALandscapeStreamingProxy> It; It; ++It)
			{
				if (It->GetLandscapeActor() != Terrain.SourceLandscape)
					continue;

				StreamingProxies.Add(*It);
			}
		}

		auto IsUsingDynamicMaterialInstance = [&StreamingProxies](ALandscape& Landscape)
		{
			bool Res = Landscape.bUseDynamicMaterialInstance;
			for (auto Proxy : StreamingProxies)
				Res &= Proxy->bUseDynamicMaterialInstance;

			return Res;
		};

		if (IsUsingDynamicMaterialInstance(*Terrain.SourceLandscape))
		{
			return;
		}

		FText AskEnableDynamicMaterial = LOCTEXT(
			"EnableDynamicMaterial?",
			"The selected Landscape does not have Use Dynamic Material Instance enabled, "
			"meaning that the material parameters for Landsacpe size and position cannot "
			"be set automatically. Should Use Dynamic Material Instance be enabled on the "
			"Landscape?");
		if (FAGX_NotificationUtilities::YesNoQuestion(AskEnableDynamicMaterial))
		{
			auto SetUseDynamicMaterialInstance = [](ALandscapeProxy& Proxy)
			{
				Proxy.bUseDynamicMaterialInstance = true;
				Proxy.Modify();
				Proxy.PostEditChange();
			};

			SetUseDynamicMaterialInstance(*Terrain.SourceLandscape);

			if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(*Terrain.SourceLandscape))
			{
				for (auto Proxy : StreamingProxies)
				{
					SetUseDynamicMaterialInstance(*Proxy);
				}
			}
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
		GET_MEMBER_NAME_CHECKED(ThisClass, bCanCollide),
		[](ThisClass* This) { This->SetCanCollide(This->bCanCollide); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, SourceLandscape),
		[](ThisClass* This) { AGX_Terrain_helpers::EnsureUseDynamicMaterialInstance(*This); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, bCreateParticles),
		[](ThisClass* This) { This->SetCreateParticles(This->bCreateParticles); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, bDeleteParticlesOutsideBounds), [](ThisClass* This)
		{ This->SetDeleteParticlesOutsideBounds(This->bDeleteParticlesOutsideBounds); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, PenetrationForceVelocityScaling), [](ThisClass* This)
		{ This->SetPenetrationForceVelocityScaling(This->PenetrationForceVelocityScaling); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(AAGX_Terrain, MaximumParticleActivationVolume), [](ThisClass* This)
		{ This->SetMaximumParticleActivationVolume(This->MaximumParticleActivationVolume); });

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ParticleSystemAsset),
		[](ThisClass* This)
		{
			if (This->ParticleSystemAsset != nullptr)
			{
				This->ParticleSystemAsset->RequestCompile(true);
			}
		});

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bEnableTerrainPaging),
		[](ThisClass* This) { This->SetEnableTerrainPaging(This->bEnableTerrainPaging); });
}

void AAGX_Terrain::PostLoad()
{
	Super::PostLoad();
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
}
#endif // WITH_EDITOR

void AAGX_Terrain::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}

	if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
	{
		// Update the Displacement Map on each PostStepForward
		PostStepForwardHandle =
			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.AddLambda(
					[this](double)
					{
						if (bEnableDisplacementRendering)
						{
							UpdateDisplacementMap();
						}
					});
	}
}

void AAGX_Terrain::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	ClearDisplacementMap();
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

			FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(*Simulation)
				.Remove(PostStepForwardHandle);
		}
	}

	if (HasNativeTerrainPager())
	{
		NativeTerrainPagerBarrier.ReleaseNative();
	}
	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

// Called every frame
void AAGX_Terrain::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_Terrain::Tick"));
	Super::Tick(DeltaTime);
	if (bEnableParticleRendering)
	{
		UpdateParticlesArrays();
	}
}

bool AAGX_Terrain::FetchHeights(
	const FVector& WorldPosStart, int32 VertsX, int32 VertsY, TArray<float>& OutHeights)
{
	/*
	 * This function will be called by the native Terrain Pager from a worker thread, meaning we
	 * have to make sure that what we do here is thread safe. For example, we protect the
	 * OriginalHeights array here since it may be read from the main thread in
	 * UpdateDisplacementMap.
	 */

	if (SourceLandscape == nullptr || !HasNative())
		return false;

	const double QuadSizeX = SourceLandscape->GetActorScale().X;
	const double QuadSizeY = SourceLandscape->GetActorScale().Y;
	const FVector PosStartLocal =
		SourceLandscape->GetTransform().InverseTransformPositionNoScale(WorldPosStart);
	const int32 StartVertX = FMath::RoundToInt(PosStartLocal.X / QuadSizeX);
	const int32 StartVertY = FMath::RoundToInt(PosStartLocal.Y / QuadSizeY);

	const FVector NativePosLocal = SourceLandscape->GetTransform().InverseTransformPositionNoScale(
		GetNativeTransform().GetLocation());
	const int32 BoundsCornerMinX =
		FMath::RoundToInt(NativePosLocal.X / QuadSizeX) - NumVerticesX / 2;
	const int32 BoundsCornerMinY =
		FMath::RoundToInt(NativePosLocal.Y / QuadSizeY) - NumVerticesY / 2;
	const int32 BoundsCornerMaxX =
		FMath::RoundToInt(NativePosLocal.X / QuadSizeX) + NumVerticesX / 2;
	const int32 BoundsCornerMaxY =
		FMath::RoundToInt(NativePosLocal.Y / QuadSizeY) + NumVerticesY / 2;

	// Check that we are not asked to read outside the bounds.
	if (StartVertX < BoundsCornerMinX || StartVertY < BoundsCornerMinY ||
		StartVertX + VertsX - 1 > BoundsCornerMaxX || StartVertY + VertsY - 1 > BoundsCornerMaxY)
	{
		return false;
	}

	OutHeights.Reserve(VertsX * VertsY);

	{
		std::lock_guard<std::mutex> ScopedOrigHeightsLock(OriginalHeightsMutex);

		// AGX Dynamics coordinate systems are mapped with Y-axis flipped.
		for (int Y = StartVertY + VertsY - 1; Y >= StartVertY; Y--)
		{
			for (int X = StartVertX; X < StartVertX + VertsX; X++)
			{
				const FVector SamplePosLocal = FVector(
					static_cast<double>(X) * QuadSizeX, static_cast<double>(Y) * QuadSizeY, 0.0);
				const FVector SamplePosGlobal =
					SourceLandscape->GetTransform().TransformPositionNoScale(SamplePosLocal);

				if (auto Height = SourceLandscape->GetHeightAtLocation(SamplePosGlobal))
				{
					FVector HeightPointLocal =
						SourceLandscape->GetTransform().InverseTransformPositionNoScale(
							FVector(SamplePosGlobal.X, SamplePosGlobal.Y, *Height));
					OutHeights.Add(HeightPointLocal.Z);
					OriginalHeights
						[(X - BoundsCornerMinX) + (Y - BoundsCornerMinY) * NumVerticesX] =
							HeightPointLocal.Z;
				}
				else
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("Height read unsuccessful in Terrain. World sample pos: %s"),
						*SamplePosGlobal.ToString());
					OutHeights.Add(SourceLandscape->GetActorLocation().Z);
					OriginalHeights
						[(X - BoundsCornerMinX) + (Y - BoundsCornerMinY) * NumVerticesX] =
							SourceLandscape->GetActorLocation().Z;
				}
			}
		}
	}

	return true;
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

namespace AGX_Terrain_helpers
{
// Since ALandscapeStreamingProxy::GetIsSpatiallyLoaded and GetStreamingBounds is guarded by
// WITH_EDITOR, WarnIfStreamingLandscape must be as well.
#if WITH_EDITOR
	void WarnIfStreamingLandscape(const ALandscape& Landscape, AAGX_Terrain& Terrain)
	{
		const UWorld* const World = Landscape.GetWorld();
		if (World == nullptr)
			return;
		UWorldPartition* Partition = World->GetWorldPartition();
		if (Partition == nullptr)
			return;
		if (!Partition->IsStreamingEnabled())
			return;
		ULandscapeInfo* Info = Landscape.GetLandscapeInfo();
		if (Info == nullptr)
			return;
		const TArray<TWeakObjectPtr<ALandscapeStreamingProxy>>& Proxies = Info->StreamingProxies;
		if (Proxies.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX Terrain '%s' detected that the source Landscape '%s' doesn't have any "
					 "Streaming Proxies in a level with World Partition enabled and World Settings "
					 "> Enable Streaming ticked. This is a sign that AGX Terrain may not be able "
					 "to initialize itself. If initialization fails either disable World Partition "
					 "Streaming in the World Settings panel or disable Is Spatially Loaded on the "
					 "Landscape Streaming Proxies overlapping the AGX Terrain bounds"),
				*Terrain.GetActorLabel(), *Landscape.GetActorLabel());
			return;
		}

		// If the Terrain has infinite bounds then all Landscape Streaming Proxies must have Is
		// Spatially Loaded unticked. If the Terrain is bounded then overlapping proxies must have
		// Is Spatially Loaded unticked.
		const bool bBounded = !Terrain.TerrainBounds->bInfiniteBounds;

		// If not infinite, then any Landscape Streaming Proxy that overlaps the Terrain Bounds
		// must have Is Spatially Loaded unticked.
		const FBox TerrainBounds {
			Terrain.GetActorLocation() - Terrain.TerrainBounds->HalfExtent,
			Terrain.GetActorLocation() + Terrain.TerrainBounds->HalfExtent};

		// Determine if there are any Streaming Proxies with Is Spatially Loaded enabled overlapping
		// the Terrain bounds. Log a note to disable Is Spatially Loaded for each found.
		bool bFoundIntersectingStreamingProxy {false};
		for (const auto& Proxy : Info->StreamingProxies)
		{
			if (!Proxy->GetIsSpatiallyLoaded())
				continue;

#if UE_VERSION_OLDER_THAN(5, 5, 0)
			if (bBounded && !Proxy->GetStreamingBounds().Intersect(TerrainBounds))
				continue;
#else
			if (bBounded)
			{
				FBox EditorBound;
				FBox RuntimeBound;
				Proxy->GetStreamingBounds(RuntimeBound, EditorBound);
				if (EditorBound.Intersect(TerrainBounds))
					continue;
			}
#endif

			bFoundIntersectingStreamingProxy = true;
			UE_LOG(
				LogAGX, Warning,
				TEXT("Found Proxy '%s' for which Is Spatially Loaded should be disabled."),
				*Proxy->GetActorLabel());
		}
		if (!bFoundIntersectingStreamingProxy)
			return;

		const FString Message = FString::Printf(
			TEXT("AGX Terrain '%s' detected that the source Landscape '%s' uses World Partition "
				 "streaming. This is only supported by AGX Terrain if the Landscape Streaming "
				 "Proxies that overlap the Terrain bounds are loaded on Begin Play. Either untick "
				 "Enable Streaming in the World Settings or untick Is Spatially Loaded on all "
				 "Landscape Streaming Proxies near and around the Terrain. The Output Log contains "
				 "a list of proxies that need to have Is Spatially Loaded disabled."),
			*GetLabelSafe(&Terrain), *Landscape.GetActorLabel());
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
	}
#endif
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

#if WITH_EDITOR
	AGX_Terrain_helpers::WarnIfStreamingLandscape(*SourceLandscape, *this);
#endif

	HeightFetcher.SetTerrain(this);

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
	CreateNativeShovels();
	AddTerrainPagerBodies();
	InitializeRendering();

	if (!UpdateNativeTerrainMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeTerrainMaterial returned false in AGX_Terrain '%s'. "
				 "Ensure the selected Terrain Material is valid."),
			*GetName());
	}

	if (!UpdateNativeShapeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeShapeMaterial returned false in AGX_Terrain '%s'. "
				 "Ensure the selected Shape Material is valid."),
			*GetName());
	}
}

bool AAGX_Terrain::CreateNative()
{
	TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo> Bounds =
		TerrainBounds->GetLandscapeAdjustedBounds();
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

	NumVerticesX =
		FMath::RoundToInt(Bounds->HalfExtent.X * 2.0 / SourceLandscape->GetActorScale().X) + 1;
	NumVerticesY =
		FMath::RoundToInt(Bounds->HalfExtent.Y * 2.0 / SourceLandscape->GetActorScale().Y) + 1;

	NativeBarrier.AddCollisionGroups(CollisionGroups);

	if (bEnableTerrainPaging)
	{
		OriginalHeights.SetNumZeroed(NumVerticesX * NumVerticesY);
	}
	else
	{
		AGX_CHECK(NumVerticesX == NativeBarrier.GetGridSizeX());
		AGX_CHECK(NumVerticesY == NativeBarrier.GetGridSizeY());
		OriginalHeights.Reserve(NumVerticesX * NumVerticesY);
		NativeBarrier.GetHeights(OriginalHeights, false);
	}

	// We must initialize CurrentHeights since we will only read height changes during runtime.
	CurrentHeights.Reserve(OriginalHeights.Num());
	CurrentHeights = OriginalHeights;

	NativeBarrier.SetCreateParticles(bCreateParticles);
	NativeBarrier.SetDeleteParticlesOutsideBounds(bDeleteParticlesOutsideBounds);
	NativeBarrier.SetPenetrationForceVelocityScaling(PenetrationForceVelocityScaling);
	NativeBarrier.SetMaximumParticleActivationVolume(MaximumParticleActivationVolume);

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
	if (bDeleteParticlesOutsideBounds)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("DeleteParticlesOutsideBounds was set to true while using Terrain Paging. This "
				 "combination is not supported. DeleteParticlesOutsideBounds will be set to "
				 "false."));
		SetDeleteParticlesOutsideBounds(false);
	}

	const auto QuadSize = SourceLandscape->GetActorScale().X;
	const int32 TileNumVerticesSide =
		FMath::RoundToInt(TerrainPagingSettings.TileSize / QuadSize) + 1;
	const int32 TileOverlapVertices =
		FMath::RoundToInt(TerrainPagingSettings.TileOverlap / QuadSize);

	NativeTerrainPagerBarrier.AllocateNative(
		&HeightFetcher, NativeBarrier, TileNumVerticesSide, TileOverlapVertices, QuadSize,
		MaxDepth);

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
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("CreateNativeShovels called on Terrain '%s' which doesn't have a native "
				 "representation."),
			*GetName());
	}

	auto AddShovel =
		[this](FShovelBarrier& ShovelBarrier, double RequiredRadius, double PreloadRadius) -> bool
	{
		if (bEnableTerrainPaging)
		{
			return NativeTerrainPagerBarrier.AddShovel(
				ShovelBarrier, RequiredRadius, PreloadRadius);
		}
		else
		{
			return NativeBarrier.AddShovel(ShovelBarrier);
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
			*BodyBarrier, TopEdgeLine, CuttingEdgeLine, CuttingDirectionVector);

		FAGX_Shovel::UpdateNativeShovelProperties(ShovelBarrier, Shovel);

		bool Added = AddShovel(ShovelBarrier, Shovel.RequiredRadius, Shovel.PreloadRadius);
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
			Added = AddShovel(ShovelBarrier, Shovel.RequiredRadius, Shovel.PreloadRadius);
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

	// Create and register Shovel Components.
	for (FShovelReferenceWithSettings& ShovelRef : ShovelComponents)
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

		bool Added = AddShovel(*ShovelBarrier, RequiredRadius, PreloadRadius);
		if (!Added)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Terrain '%s' rejected shovel '%s' in '%s'. Reversing edge directions and "
					 "trying again."),
				*GetLabelSafe(this), *ShovelComponent->GetName(),
				*GetLabelSafe(ShovelComponent->GetOwner()));

			ShovelComponent->SwapEdgeDirections();
			Added = AddShovel(*ShovelBarrier, RequiredRadius, PreloadRadius);
			if (!Added)
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Terrain '%s' rejected shovel '%s' in '%s' after edge directions flip. "
						 "Abandoning shovel."),
					*GetLabelSafe(this), *GetNameSafe(ShovelComponent),
					*GetLabelSafe(ShovelComponent->GetOwner()));
			}
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
	if (bEnableDisplacementRendering)
	{
		InitializeDisplacementMap();
	}
	if (bEnableParticleRendering)
	{
		InitializeParticleSystem();
	}

	UpdateLandscapeMaterialParameters();
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

void AAGX_Terrain::InitializeDisplacementMap()
{
	if (LandscapeDisplacementMap == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("No landscape displacement map configured for terrain '%s'. Terrain rendering "
				 "will not include height updates."),
			*GetName());
		return;
	}

	if (LandscapeDisplacementMap->GetFormat() != EPixelFormat::PF_R16F)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The displacement map pixel format for the terrain '%s' must be R16F."),
			*GetName());
		return;
	}

	if (LandscapeDisplacementMap->SizeX != NumVerticesX ||
		LandscapeDisplacementMap->SizeY != NumVerticesY)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("The size of the Displacement Map render target (%dx%d) for "
				 "AGX Terrain '%s' does not match the vertices in the Terrain (%dx%d). "
				 "Resizing the displacement map."),
			LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY, *GetLabelSafe(this),
			NumVerticesX, NumVerticesY);

		LandscapeDisplacementMap->ResizeTarget(NumVerticesX, NumVerticesY);
	}
	if (LandscapeDisplacementMap->SizeX != NumVerticesX ||
		LandscapeDisplacementMap->SizeY != NumVerticesY)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Landscape displacement map for terrain '%s' could not be resized. "
				 "There may be rendering issues."),
			*GetName(), LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY);
	}

	DisplacementData.SetNum(NumVerticesX * NumVerticesY);
	DisplacementMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, NumVerticesX, NumVerticesY));

	/// \todo I'm not sure why we need this. Does the texture sampler "fudge the
	/// values" when using non-linear gamma?
	LandscapeDisplacementMap->bForceLinearGamma = true;

	if (LandscapeDisplacementMap->GetResource() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not allocate resource for Landscape Displacement Map for AGX Terrain '%s'. "
				 "There may be rendering issues."),
			*GetLabelSafe(this));
		return;
	}

	DisplacementMapInitialized = true;
}

void AAGX_Terrain::UpdateDisplacementMap()
{
	if (!DisplacementMapInitialized)
	{
		return;
	}
	if (LandscapeDisplacementMap == nullptr)
	{
		return;
	}
	if (!HasNative())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_Terrain::UpdateDisplacementMap"));

	TArray<std::tuple<int32, int32>> ModifiedVertices;
	if (bEnableTerrainPaging)
	{
		ModifiedVertices = NativeTerrainPagerBarrier.GetModifiedHeights(
			CurrentHeights, NumVerticesX, NumVerticesY);
	}
	else
	{
		NativeBarrier.GetHeights(CurrentHeights, true);
		ModifiedVertices = NativeBarrier.GetModifiedVertices();
	}

	{
		std::lock_guard<std::mutex> ScopedOrigHeightsLock(OriginalHeightsMutex);
		for (const auto& VertexTuple : ModifiedVertices)
		{
			const int32 VertX = std::get<0>(VertexTuple);
			const int32 VertY = std::get<1>(VertexTuple);
			const int32 Index = VertX + VertY * NumVerticesX;
			const float HeightChange = CurrentHeights[Index] - OriginalHeights[Index];
			DisplacementData[Index] = static_cast<FFloat16>(HeightChange);
		}
	}

	const uint32 BytesPerPixel = sizeof(FFloat16);
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	FAGX_RenderUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
}

void AAGX_Terrain::ClearDisplacementMap()
{
	if (!DisplacementMapInitialized)
	{
		return;
	}
	if (LandscapeDisplacementMap == nullptr)
	{
		return;
	}
	if (!HasNative())
	{
		return;
	}
	if (DisplacementMapRegions.Num() == 0)
	{
		return;
	}

	const uint32 BytesPerPixel = sizeof(FFloat16);
	for (FFloat16& Displacement : DisplacementData)
	{
		Displacement = FFloat16();
	}
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	FAGX_RenderUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
}

bool AAGX_Terrain::InitializeParticleSystem()
{
	return InitializeParticleSystemComponent();
}

bool AAGX_Terrain::InitializeParticleSystemComponent()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain '%s' does not have a particle system, cannot render particles"),
			*GetName());
		return false;
	}

	// It is important that we attach the ParticleSystemComponent using "KeepRelativeOffset" so that
	// it's world position becomes the same as the Terrain's. Otherwise it will be spawned at
	// the world origin which in turn may result in particles being culled and not rendered if the
	// terrain is located far away from the world origin (see Fixed Bounds in the Particle System).
	ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
		FVector::OneVector, EAttachLocation::Type::KeepRelativeOffset, false,
#if UE_VERSION_OLDER_THAN(4, 24, 0)
		EPSCPoolMethod::None
#else
		ENCPoolMethod::None
#endif
	);
#if WITH_EDITORONLY_DATA
	// Must check for nullptr here because no particle system component is created with running
	// as a unit test without graphics, i.e. with our run_unit_tests script in GitLab CI.
	if (ParticleSystemComponent != nullptr)
	{
		ParticleSystemComponent->bVisualizeComponent = true;
	}
#endif

	return ParticleSystemComponent != nullptr;
}

void AAGX_Terrain::UpdateParticlesArrays()
{
	if (!NativeBarrier.HasNative())
	{
		return;
	}
	if (ParticleSystemComponent == nullptr)
	{
		return;
	}

	// Copy data with holes.
	EParticleDataFlags ToInclude = EParticleDataFlags::Positions | EParticleDataFlags::Rotations |
								   EParticleDataFlags::Radii | EParticleDataFlags::Velocities;
	const FParticleDataById ParticleData =
		bEnableTerrainPaging ? NativeTerrainPagerBarrier.GetParticleDataById(ToInclude)
							 : NativeBarrier.GetParticleDataById(ToInclude);

	const TArray<FVector>& Positions = ParticleData.Positions;
	const TArray<FQuat>& Rotations = ParticleData.Rotations;
	const TArray<float>& Radii = ParticleData.Radii;
	const TArray<bool>& Exists = ParticleData.Exists;
	const TArray<FVector>& Velocities = ParticleData.Velocities;

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	ParticleSystemComponent->SetNiagaraVariableInt("User.Target Particle Count", Exists.Num());
#else
	ParticleSystemComponent->SetVariableInt(FName("User.Target Particle Count"), Exists.Num());
#endif

	const int32 NumParticles = Positions.Num();

	TArray<FVector4> PositionsAndScale;
	PositionsAndScale.SetNum(NumParticles);
	TArray<FVector4> Orientations;
	Orientations.SetNum(NumParticles);

	for (int32 I = 0; I < NumParticles; ++I)
	{
		// The particle size slot in the PositionAndScale buffer is a scale and not the
		// actual size. The scale is relative to a SI unit cube, meaning that a
		// scale of 1.0 should render a particle that is 1x1x1 m large, or
		// 100x100x100 Unreal units. We multiply by 2.0 to convert from radius
		// to full width.
		float UnitCubeScale = (Radii[I] * 2.0f) / 100.0f;
		PositionsAndScale[I] = FVector4(Positions[I], UnitCubeScale);
		Orientations[I] = FVector4(Rotations[I].X, Rotations[I].Y, Rotations[I].Z, Rotations[I].W);
	}

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
		ParticleSystemComponent, "Positions And Scales", PositionsAndScale);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
		ParticleSystemComponent, "Orientations", Orientations);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
		ParticleSystemComponent, "Exists", Exists);
	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
		ParticleSystemComponent, TEXT("Velocities"), Velocities);
}

void AAGX_Terrain::UpdateLandscapeMaterialParameters()
{
	if (!IsValid(SourceLandscape) || GetWorld() == nullptr || !GetWorld()->IsGameWorld())
	{
		return;
	}

	// Set scalar material parameters for Landscape size and position.
	// It is the Landscape material's responsibility to declare and implement displacement map
	// sampling and passing on to World Position Offset.

	const auto QuadSideSizeX = SourceLandscape->GetActorScale().X;
	const auto QuadSideSizeY = SourceLandscape->GetActorScale().Y;

	// This assumes that the Terrain and Landscape resolution (quad size) is the same.
	const double TerrainSizeX = static_cast<double>(NumVerticesX - 1) * QuadSideSizeX;
	const double TerrainSizeY = static_cast<double>(NumVerticesY - 1) * QuadSideSizeY;

	const FVector TerrainCenterGlobal = NativeBarrier.GetPosition();

	const FVector TerrainCenterLocal =
		SourceLandscape->GetActorTransform().InverseTransformPositionNoScale(TerrainCenterGlobal);
	const FVector TerrainCornerLocal =
		TerrainCenterLocal - FVector(TerrainSizeX / 2.0, TerrainSizeY / 2.0, 0.0);
	const FVector TerrainCornerGlobal =
		SourceLandscape->GetActorTransform().TransformPositionNoScale(TerrainCornerLocal);

	const double PositionX = TerrainCornerGlobal.X;
	const double PositionY = TerrainCornerGlobal.Y;

	auto SetLandscapeMaterialParameters = [=](ALandscapeProxy& Proxy)
	{
		// Parameter for materials supporting only square Landscape.
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSize", static_cast<float>(TerrainSizeX));
		// Parameters for materials supporting rectangular Landscape.
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSizeX", static_cast<float>(TerrainSizeX));
		Proxy.SetLandscapeMaterialScalarParameterValue(
			"TerrainSizeY", static_cast<float>(TerrainSizeY));
		// Parameters for Landscape position.
		Proxy.SetLandscapeMaterialScalarParameterValue("TerrainPositionX", PositionX);
		Proxy.SetLandscapeMaterialScalarParameterValue("TerrainPositionY", PositionY);
	};

	SetLandscapeMaterialParameters(*SourceLandscape);

	if (AGX_HeightFieldUtilities::IsOpenWorldLandscape(*SourceLandscape))
	{
		// There might be a better way to get all LandscapeStreamingProxies directly from the
		// SourceLandscape, but I have not found any. This is likely slower than any such methods,
		// but this is not extremely time critical since this function is called only once on Play.
		// If a better way if getting them is found in the future, this can be replaced.
		for (TObjectIterator<ALandscapeStreamingProxy> It; It; ++It)
		{
			if (It->GetLandscapeActor() != SourceLandscape)
				continue;

			SetLandscapeMaterialParameters(**It);
		}
	}
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
}

#if WITH_EDITOR
void AAGX_Terrain::ShowShapeMaterialWarning() const
{
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

	FAGX_NotificationUtilities::ShowDialogBoxWithWarningLog(Msg);
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
