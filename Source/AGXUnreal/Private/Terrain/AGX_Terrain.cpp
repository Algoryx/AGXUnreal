// Copyright 2022, Algoryx Simulation AB.


#include "Terrain/AGX_Terrain.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "AGX_RigidBodyComponent.h"
#include "Materials/AGX_TerrainMaterialInstance.h"
#include "Materials/AGX_ShapeMaterialInstance.h"
#include "Materials/AGX_MaterialBase.h"
#include "Terrain/AGX_CuttingDirectionComponent.h"
#include "Terrain/AGX_CuttingEdgeComponent.h"
#include "Terrain/AGX_LandscapeSizeInfo.h"
#include "Terrain/AGX_TopEdgeComponent.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_TextureUtilities.h"

// AGXUnrealBarrier includes.
#include "Terrain/TerrainBarrier.h"
#include "Shapes/HeightFieldShapeBarrier.h"
#include "Terrain/ShovelBarrier.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeComponent.h"
#include "Misc/AssertionMacros.h"
#include "Misc/EngineVersionComparison.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

// Standard library includes.
#include <algorithm>

AAGX_Terrain::AAGX_Terrain()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;

	// Create a root SceneComponent so that this Actor has a transform
	// which can be modified in the Editor.
	{
		USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(
			USceneComponent::GetDefaultSceneRootVariableName());

		Root->Mobility = EComponentMobility::Static;
		Root->SetFlags(Root->GetFlags() | RF_Transactional); /// \todo What does this mean?

#if WITH_EDITORONLY_DATA
		Root->bVisualizeComponent = true;
#endif

		SetRootComponent(Root);
	}
}

void AAGX_Terrain::SetCreateParticles(bool CreateParticles)
{
	if (HasNative())
	{
		NativeBarrier.SetCreateParticles(CreateParticles);
	}

	bCreateParticles = CreateParticles;
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

void AAGX_Terrain::SetPenetrationForceVelocityScaling(float InPenetrationForceVelocityScaling)
{
	if (HasNative())
	{
		NativeBarrier.SetPenetrationForceVelocityScaling(
			static_cast<double>(InPenetrationForceVelocityScaling));
	}

	PenetrationForceVelocityScaling = InPenetrationForceVelocityScaling;
}

float AAGX_Terrain::GetPenetrationForceVelocityScaling() const
{
	if (HasNative())
	{
		return static_cast<float>(NativeBarrier.GetPenetrationForceVelocityScaling());
	}

	return PenetrationForceVelocityScaling;
}

bool AAGX_Terrain::HasNative() const
{
	return NativeBarrier.HasNative();
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

#if WITH_EDITOR
void AAGX_Terrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != nullptr)
							 ? PropertyChangedEvent.Property->GetFName()
							 : NAME_None;
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(AAGX_Terrain, SourceLandscape)))
	{
		if (SourceLandscape == nullptr)
		{
			return;
		}

		FAGX_LandscapeSizeInfo LandscapeSizeInfo(*SourceLandscape);
		const int32 QuadCountX = LandscapeSizeInfo.NumQuadsSideX;
		const int32 QuadCountY = LandscapeSizeInfo.NumQuadsSideY;
		const float QuadSizeX = LandscapeSizeInfo.QuadSideSizeX;
		const float QuadSizeY = LandscapeSizeInfo.QuadSideSizeY;
		const float SizeX = QuadSizeX * QuadCountX;
		const float SizeY = QuadSizeY * QuadCountY;
		UE_LOG(
			LogAGX, Display, TEXT("Selected %fcm x %fcm Landscape containing %d x %d quads."),
			SizeX, SizeY, QuadCountX, QuadCountY);
	}
}
#endif

void AAGX_Terrain::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative())
	{
		InitializeNative();
	}
}

void AAGX_Terrain::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	ClearDisplacementMap();
	ClearParticlesMap();

	if (HasNative() && Reason != EEndPlayReason::EndPlayInEditor && Reason != EEndPlayReason::Quit)
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
	UpdateDisplacementMap();
	UpdateParticlesMap();
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
		auto RecursiveFind = [](const TArray<USceneComponent*>& Components, auto& recurse) {
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
		return RecursiveFind(Body.GetAttachChildren(), RecursiveFind);
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
		UE_LOG(LogAGX, Error, TEXT("No source landscape selected for terrain %s."), *GetName());
		return;
	}

	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("BeginPlay called on a Terrain that has already been initialized."));
		return;
	}

	CreateNativeTerrain();
	CreateNativeShovels();
	InitializeRendering();
	CreateTerrainMaterial();
}

void AAGX_Terrain::CreateNativeTerrain()
{
	FHeightFieldShapeBarrier HeightField =
		AGX_HeightFieldUtilities::CreateHeightField(*SourceLandscape);
	NativeBarrier.AllocateNative(HeightField, MaxDepth);
	check(HasNative());

	SetInitialTransform();
	OriginalHeights = NativeBarrier.GetHeights();
	NativeBarrier.SetCreateParticles(bCreateParticles);
	NativeBarrier.SetDeleteParticlesOutsideBounds(bDeleteParticlesOutsideBounds);
	NativeBarrier.SetPenetrationForceVelocityScaling(
		static_cast<double>(PenetrationForceVelocityScaling));

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
		return;
	}

	int32 NumIterations = Simulation->GetNumPpgsIterations();
	Simulation->Add(*this);
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

	for (FAGX_Shovel& Shovel : Shovels)
	{
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
		FTransform const WorldToBody = Body->GetComponentTransform().Inverse();
		FTwoVectors TopEdgeLine = TopEdge->GetInLocal(WorldToBody);
		FTwoVectors CuttingEdgeLine = CuttingEdge->GetInLocal(WorldToBody);
		FVector CuttingDirectionVector =
			WorldToBody.TransformVector(CuttingDirection->GetVectorDirectionNormalized());
		ShovelBarrier.AllocateNative(
			*BodyBarrier, TopEdgeLine, CuttingEdgeLine, CuttingDirectionVector);

		FAGX_Shovel::UpdateNativeShovelProperties(ShovelBarrier, Shovel);

		bool Added = NativeBarrier.AddShovel(ShovelBarrier);
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
			Added = NativeBarrier.AddShovel(ShovelBarrier);
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
}

void AAGX_Terrain::SetInitialTransform()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("SetInitialTransform called on Terrain '%s' which doesn't have a native "
				 "representation."),
			*GetName());
		return;
	}

	// Apply the same rotation to the native terrain as the landscape.
	const FQuat LandscapeRotation = SourceLandscape->GetActorQuat();
	NativeBarrier.SetRotation(LandscapeRotation);

	// The Unreal landscape has its origin in the bottom left corner.
	// The native terrain has its origin at the center of the terrain, when there are an even number
	// of tiles in the x and y direction. If there are an odd number of tiles in the x-direction,
	// the origins x-coordinate is the same as the x-coordinate of the left edge of the center tile.
	// If there are an odd number of tiles in the y-direction, the origins y-coordinate is the same
	// as the y-coordinate of the top edge of the center tile.
	const FAGX_LandscapeSizeInfo LandscapeSizeInfo(*SourceLandscape);
	const float SideSizeX = LandscapeSizeInfo.NumQuadsSideX * LandscapeSizeInfo.QuadSideSizeX;
	const float SideSizeY = LandscapeSizeInfo.NumQuadsSideY * LandscapeSizeInfo.QuadSideSizeY;
	const float TerrainTileCenterOffsetX =
		(LandscapeSizeInfo.NumQuadsSideX % 2 == 0) ? 0 : LandscapeSizeInfo.QuadSideSizeX / 2;
	const float TerrainTileCenterOffsetY =
		(LandscapeSizeInfo.NumQuadsSideY % 2 == 0) ? 0 : -LandscapeSizeInfo.QuadSideSizeY / 2;

	// Calculate the offset from landscape origin to terrain origin expressed in landscapes local
	// coordinate system.
	const FVector LandscapeToTerrainOffsetLocal = FVector(
		SideSizeX / 2.0f + TerrainTileCenterOffsetX, SideSizeY / 2.0f + TerrainTileCenterOffsetY,
		0);

	// Transform the offset from landscape local coordinate system to the global coordinate system.
	const FTransform LandscapeTransform = SourceLandscape->GetTransform();
	const FVector LandscapeToTerrainOffsetGlobal =
		LandscapeTransform.TransformPositionNoScale(LandscapeToTerrainOffsetLocal);

	NativeBarrier.SetPosition(LandscapeToTerrainOffsetGlobal);
}

void AAGX_Terrain::InitializeRendering()
{
	InitializeDisplacementMap();
	ParticleSystemInitialized = InitializeParticleSystem();
}

void AAGX_Terrain::CreateTerrainMaterial()
{
	if (!HasNative())
		return;

	if (TerrainMaterial)
	{
		// Both an UAGX_TerrainMaterialInstance and a UAGX_ShapeMaterialInstance
		// are set for the terrain. The former is the native agxTerrain::TerrainMaterial
		// counterpart and the latter is the native agx::Material counterpart.

		// Set TerrainMaterial
		UAGX_TerrainMaterialInstance* TerrainMaterialInstance =
			static_cast<UAGX_TerrainMaterialInstance*>(
				TerrainMaterial->GetOrCreateInstance(GetWorld()));

		check(TerrainMaterialInstance);

		FTerrainMaterialBarrier* TerrainMaterialBarrier =
			TerrainMaterialInstance->GetOrCreateTerrainMaterialNative(GetWorld());
		check(TerrainMaterialBarrier);

		GetNative()->SetTerrainMaterial(*TerrainMaterialBarrier);

		// Set ShapeMaterial
		FShapeMaterialBarrier* MaterialBarrier =
			TerrainMaterialInstance->GetOrCreateShapeMaterialNative(GetWorld());
		check(MaterialBarrier);

		GetNative()->SetShapeMaterial(*MaterialBarrier);

		// Swap properties
		UWorld* PlayingWorld = GetWorld();

		if (TerrainMaterialInstance != TerrainMaterial && PlayingWorld &&
			PlayingWorld->IsGameWorld())
		{
			TerrainMaterial = TerrainMaterialInstance;
		}
	}
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

	// "Grid" in the Terrain is what the Landscape calls "vertices". There is
	// one more "grid" element than there is "quads" per side. There is one
	// displacement map texel per vertex.
	int32 GridSizeX = NativeBarrier.GetGridSizeX();
	int32 GridSizeY = NativeBarrier.GetGridSizeY();
	if (LandscapeDisplacementMap->SizeX != GridSizeX ||
		LandscapeDisplacementMap->SizeY != GridSizeY)
	{
		UE_LOG(
			LogAGX, Verbose,
			TEXT("The size of the Displacement Map render target (%dx%d) for "
				 "AGX Terrain '%s' does not match the vertices in the terrain (%dx%d). "
				 "Resizing the displacement map."),
			LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY, *GetName(), GridSizeX,
			GridSizeY);

		LandscapeDisplacementMap->ResizeTarget(GridSizeX, GridSizeY);
	}
	if (LandscapeDisplacementMap->SizeX != GridSizeX ||
		LandscapeDisplacementMap->SizeY != GridSizeY)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Landscape displacement map for terrain '%s' could not be resized. "
				 "There may be rendering issues."),
			*GetName(), LandscapeDisplacementMap->SizeX, LandscapeDisplacementMap->SizeY);
	}

	DisplacementData.SetNum(GridSizeX * GridSizeY);
	DisplacementMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, GridSizeX, GridSizeY));

	/// \todo I'm not sure why we need this. Does the texture sampler "fudge the
	/// values" when using non-linear gamma?
	LandscapeDisplacementMap->bForceLinearGamma = true;

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

	const int32 NumVerticesX = NativeBarrier.GetGridSizeX();
	const int32 NumVerticesY = NativeBarrier.GetGridSizeY();
	const int32 NumPixels = NumVerticesX * NumVerticesY;
	check(DisplacementData.Num() == NumPixels);
	check(DisplacementMapRegions.Num() == 1);

	TArray<float> CurrentHeights = NativeBarrier.GetHeights();
	for (int32 PixelIndex = 0; PixelIndex < NumPixels; ++PixelIndex)
	{
		const float HeightChange = CurrentHeights[PixelIndex] - OriginalHeights[PixelIndex];
		DisplacementData[PixelIndex] = static_cast<FFloat16>(HeightChange);
	}

	uint32 BytesPerPixel = sizeof(FFloat16);
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
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

	const int32 NumVerticesX = NativeBarrier.GetGridSizeX();
	const uint32 BytesPerPixel = sizeof(FFloat16);
	for (FFloat16& Displacement : DisplacementData)
	{
		Displacement = FFloat16();
	}
	uint8* PixelData = reinterpret_cast<uint8*>(DisplacementData.GetData());
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
		*LandscapeDisplacementMap, 1, DisplacementMapRegions.GetData(),
		NumVerticesX * BytesPerPixel, BytesPerPixel, PixelData, false);
}

namespace
{
	/**
	Calculates and returns the smallest base size of a square sized texture,
	such that the base size is evenly divisible by pixelsPerItem and has a square
	that is at least minNumItems x pixelsPerItem.
	*/
	int32 CalculateTextureBaseSize(int32 minNumItems, int32 PixelsPerItem)
	{
		const int32 maxSize = 8192;
		for (int32 base = FMath::Sqrt(minNumItems * PixelsPerItem); base <= maxSize; ++base)
		{
			if ((base % PixelsPerItem == 0) && (base * base >= minNumItems * PixelsPerItem))
				return base;
		}
		check(!"CalculateTextureBaseSize failed");
		return 0;
	}
}

bool AAGX_Terrain::InitializeParticleSystem()
{
	return InitializeParticleSystemComponent() && InitializeParticlesMap();
}

bool AAGX_Terrain::InitializeParticleSystemComponent()
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Terrain '%s' does not have a particle system, cannot render particles"),
			*GetName());
		return false;
	}

	ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, RootComponent, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
		FVector::OneVector, EAttachLocation::Type::KeepWorldPosition, false,
#if UE_VERSION_OLDER_THAN(4, 24, 0)
		EPSCPoolMethod::None
#else
		ENCPoolMethod::None
#endif
	);
#if WITH_EDITORONLY_DATA
	ParticleSystemComponent->bVisualizeComponent = true;
#endif

	return true;
}

bool AAGX_Terrain::InitializeParticlesMap()
{
	if (TerrainParticlesDataMap == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("No particles data map configured for terrain '%s'. Terrain rendering will not "
				 "include particle."),
			*GetName());
		return false;
	}

	if (TerrainParticlesDataMap->GetFormat() != EPixelFormat::PF_A32B32G32R32F)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The particle data map pixel format for the terrain '%s' must be RGBA32F."),
			*GetName());
		return false;
	}

	// Finds the closest fitting base size of a square sized texture such that it has room for all
	// particles data and such that the base size is a multiple of number of pixels per particle
	// (because we do not want a row-break in the middle of a particle element, since that increases
	// complexity of the Niagara Module Script).
	const int32 TextureBaseSize =
		CalculateTextureBaseSize(MaxNumRenderParticles, NumPixelsPerParticle);
	check(TextureBaseSize % NumPixelsPerParticle == 0);
	check(TextureBaseSize * TextureBaseSize >= MaxNumRenderParticles * NumPixelsPerParticle);

	const bool TargetLargeEnough =
		TerrainParticlesDataMap->SizeX * TerrainParticlesDataMap->SizeY >=
		TextureBaseSize * TextureBaseSize;
	const bool TargetSquare = TerrainParticlesDataMap->SizeX == TerrainParticlesDataMap->SizeY;
	const bool TargetSizeMultipleOfPpp = TerrainParticlesDataMap->SizeX % NumPixelsPerParticle == 0;

	if (!TargetLargeEnough || !TargetSquare || !TargetSizeMultipleOfPpp)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("The size of the particle data render target (%dx%d) for "
				 "AGX Terrain '%s' has a size that does not meet the requirements to hold the \n"
				 "terrain particles data. It must be square, be at least (%dx%d) in size and have "
				 "side size that is a multiple of %d. Please resize the displacement map. The \n"
				 "recommended size given current 'Max Num Render Particles' is (%dx%d). Note \n"
				 "that you may have to recompile the Niagara Particle System after changing the "
				 "size of the particle data render taget."),
			TerrainParticlesDataMap->SizeX, TerrainParticlesDataMap->SizeY, *GetName(),
			TextureBaseSize, TextureBaseSize, NumPixelsPerParticle, TextureBaseSize,
			TextureBaseSize);

		return false;
	}

	return true;
}

void AAGX_Terrain::UpdateParticlesMap()
{
	if (!ParticleSystemInitialized)
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AGXUnreal:AAGX_Terrain::UpdateParticlesMap"));

	const int32 ResolutionX = TerrainParticlesDataMap->SizeX;
	const int32 ResolutionY = TerrainParticlesDataMap->SizeY;
	const int32 NumPixels = ResolutionX * ResolutionY;
	const int32 NumComponentsPerPixel = 4;
	const int32 NumBytesPerPixel = NumComponentsPerPixel * sizeof(FFloat32);
	const int32 NumComponentsPerParticle = NumComponentsPerPixel * NumPixelsPerParticle;
	const int32 NumBytes = NumPixels * NumBytesPerPixel;
	const int32 MaxNumParticles = NumPixels / NumPixelsPerParticle;

	if (TerrainParticlesData.Num() == 0)
	{
		TerrainParticlesData.SetNum(NumComponentsPerPixel * NumPixels);
	}

	if (ParticlesDataMapRegions.Num() == 0)
	{
		ParticlesDataMapRegions.Add(FUpdateTextureRegion2D(0, 0, 0, 0, ResolutionX, ResolutionY));
	}

	TArray<FVector> Positions = NativeBarrier.GetParticlePositions();
	TArray<float> Radii = NativeBarrier.GetParticleRadii();
	TArray<FQuat> Rotations = NativeBarrier.GetParticleRotations();

	check(Positions.Num() == Radii.Num());
	check(Positions.Num() == Rotations.Num());

	int32 NumParticles = FMath::Min(Positions.Num(), MaxNumParticles);
	ParticleSystemComponent->SetNiagaraVariableInt("User.TargetParticleCount", NumParticles);

	for (int32 ParticleIndex = 0, PixelIndex = 0; ParticleIndex < NumParticles;
		 ++ParticleIndex, PixelIndex += NumComponentsPerParticle)
	{
		// Multiply position by 0.01 because it seems we need to pack floats to
		// smaller range. The position floats are unpacked in the
		// `GetTerrainParticleData` Niagara Module Script.
		/// \todo Investigate!
		const float PackingScale = 0.01f;
		TerrainParticlesData[PixelIndex + 0] = Positions[ParticleIndex].X * PackingScale;
		TerrainParticlesData[PixelIndex + 1] = Positions[ParticleIndex].Y * PackingScale;
		TerrainParticlesData[PixelIndex + 2] = Positions[ParticleIndex].Z * PackingScale;

		// The particle size slot in the render target is a scale, not the
		// actual size. The scale is relative to a SI unit cube, meaning that a
		// scale of 1.0 should render a particle that is 1x1x1 m large, or
		// 100x100x100 Unreal units. We multiply by 2.0 to convert from radius
		// to full width.
		float UnitCubeScale = (Radii[ParticleIndex] * 2.0f) / 100.0f;
		TerrainParticlesData[PixelIndex + 3] = UnitCubeScale;

		TerrainParticlesData[PixelIndex + 4] = Rotations[ParticleIndex].X;
		TerrainParticlesData[PixelIndex + 5] = Rotations[ParticleIndex].Y;
		TerrainParticlesData[PixelIndex + 6] = Rotations[ParticleIndex].Z;
		TerrainParticlesData[PixelIndex + 7] = Rotations[ParticleIndex].W;
	}

	uint8* PixelData = reinterpret_cast<uint8*>(TerrainParticlesData.GetData());
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
		*TerrainParticlesDataMap, 1, ParticlesDataMapRegions.GetData(),
		ResolutionX * NumBytesPerPixel, NumBytesPerPixel, PixelData, false);
}

void AAGX_Terrain::ClearParticlesMap()
{
	if (!ParticleSystemInitialized)
	{
		return;
	}
	if (TerrainParticlesDataMap == nullptr)
	{
		return;
	}
	if (!HasNative())
	{
		return;
	}
	if (ParticlesDataMapRegions.Num() == 0)
	{
		return;
	}

	const int32 ResolutionX = TerrainParticlesDataMap->SizeX;
	const int32 NumComponentsPerPixel = 4;
	const int32 NumBytesPerPixel = NumComponentsPerPixel * sizeof(FFloat32);
	for (FFloat32& Pixel : TerrainParticlesData)
	{
		Pixel = FFloat32();
	}
	uint8* PixelData = reinterpret_cast<uint8*>(TerrainParticlesData.GetData());
	FAGX_TextureUtilities::UpdateRenderTextureRegions(
		*TerrainParticlesDataMap, 1, ParticlesDataMapRegions.GetData(),
		ResolutionX * NumBytesPerPixel, NumBytesPerPixel, PixelData, false);
}
