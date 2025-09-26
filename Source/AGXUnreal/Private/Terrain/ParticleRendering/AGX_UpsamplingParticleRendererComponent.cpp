// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_UpsamplingParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDataHandler.h"
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/AGX_ParticleUpsamplingDI.h"
#include "Terrain/ParticleRendering/AGX_ParticleRenderingUtilities.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystemInstanceController.h"
#include "NiagaraRenderGraphUtils.h"
#include "SphereTypes.h"

UAGX_UpsamplingParticleRendererComponent::UAGX_UpsamplingParticleRendererComponent()
{
	AGX_ParticleRenderingUtilities::AssignDefaultNiagaraAsset(
		ParticleSystemAsset,
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		TEXT("NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/UpsamplingParticleSystem"
			 "/PS_UpsamplingParticleSystem_UE_53_54.PS_UpsamplingParticleSystem_UE_53_54'"));
#else
		TEXT("NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/UpsamplingParticleSystem"
			 "/PS_UpsamplingParticleSystem.PS_UpsamplingParticleSystem'"));
#endif
}

void UAGX_UpsamplingParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();

	AAGX_Terrain* ParentTerrainActor = AGX_ParticleRenderingUtilities::GetParentTerrainActor(this);
	if (!ParentTerrainActor)
	{
		return;
	}
	
	ParticleSystemComponent =
		AGX_ParticleRenderingUtilities::InitializeNiagaraParticleSystemComponent(
			ParticleSystemAsset, this);
	if (!ParticleSystemComponent)
	{
		return;
	}

	ParticleSystemComponent->SetActive(bEnableParticleRendering);

	// Try to get the correct data interface.
	UpsamplingDataInterface =
		static_cast<UAGX_ParticleUpsamplingDI*>(UNiagaraFunctionLibrary::GetDataInterface(
			UAGX_ParticleUpsamplingDI::StaticClass(), ParticleSystemComponent,
			"PUDI"));

	if (!UpsamplingDataInterface)
	{
		UE_LOG(
			LogTemp, Warning,
			TEXT("Particle renderer '%s' in Actor '%s', unable to to find Niagara Data Interface with name 'PUDI' "
				"in loaded Niagara system with name '%s'. No particles will be rendered."),
			*GetName(), *GetLabelSafe(ParentTerrainActor), *ParticleSystemComponent->GetName());
		return;
	}

	ElementSize = ParentTerrainActor->SourceLandscape->GetActorScale().X;
	
	// Bind function to terrain delegate to handle particle data.
	ParentTerrainActor->OnParticleData.AddDynamic(
		this, &UAGX_UpsamplingParticleRendererComponent::HandleParticleData);
}

void UAGX_UpsamplingParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->DeactivateImmediate();
		ParticleSystemComponent->SetActive(bEnabled);
	}
	
	bEnableParticleRendering = bEnabled; 
}

bool UAGX_UpsamplingParticleRendererComponent::GetEnableParticleRendering() const
{
	return bEnableParticleRendering;
}

UNiagaraComponent* UAGX_UpsamplingParticleRendererComponent::GetParticleSystemComponent()
{
	return ParticleSystemComponent;
}

const UNiagaraComponent* UAGX_UpsamplingParticleRendererComponent::GetParticleSystemComponent()
	const
{
	return ParticleSystemComponent;
}

void UAGX_UpsamplingParticleRendererComponent::SetUpsampling(int32 InUpsampling)
{
	Upsampling = InUpsampling;
}

int32 UAGX_UpsamplingParticleRendererComponent::GetUpsampling() const
{
	return Upsampling;
}

void UAGX_UpsamplingParticleRendererComponent::SetOverrideVoxelSize(bool bEnabled)
{
	bOverrideVoxelSize = bEnabled;
}

bool UAGX_UpsamplingParticleRendererComponent::GetOverrideVoxelSize() const
{
	return bOverrideVoxelSize;
}

void UAGX_UpsamplingParticleRendererComponent::SetVoxelSize(double InVoxelSize)
{
	VoxelSize = InVoxelSize;
}

double UAGX_UpsamplingParticleRendererComponent::GetVoxelSize() const
{
	return VoxelSize;
}

void UAGX_UpsamplingParticleRendererComponent::SetEaseStepSize(double InEaseStepSize)
{
	EaseStepSize = InEaseStepSize;
}

double UAGX_UpsamplingParticleRendererComponent::GetEaseStepSize() const
{
	return EaseStepSize;
}

void UAGX_UpsamplingParticleRendererComponent::HandleParticleData(FDelegateParticleData& Data)
{
	if (!ParticleSystemComponent || !UpsamplingDataInterface)
	{
		return;
	}

	TArray<FCoarseParticle> NewCoarseParticles;
	TSet<FIntVector> ActiveVoxelSet;
	float ParticleDensity = 0.0f;
	float UsedVoxelSize = (bOverrideVoxelSize ? VoxelSize : ElementSize);

	for (int32 I = 0; I < Data.ParticleCount; ++I)
	{
		if (!Data.Exists[I])
			continue;



		float Radius = Data.PositionsAndRadii[I].W;
		
		/**
		 * TODO: The density of the particles should be fetched by the terrain and not 
		 * computed here. Maybe particles have seperate densities that could be used 
		 * for improved motion of the upsampling simulations, maybe worth investigating.
		 */
		ParticleDensity =
			Data.VelocitiesAndMasses[I].W / UE::Geometry::TSphere3<float>::Volume(Radius);

		FVector Position = FVector(
			Data.PositionsAndRadii[I].X, 
			Data.PositionsAndRadii[I].Y, 
			Data.PositionsAndRadii[I].Z);

		AppendIfActiveVoxel(ActiveVoxelSet, Position, Radius, UsedVoxelSize);

		FCoarseParticle CP;
		CP.PositionAndRadius = FVector4f(Position.X, Position.Y, Position.Z, Radius);
		CP.VelocityAndMass = FVector4f(
			Data.VelocitiesAndMasses[I].X, 
			Data.VelocitiesAndMasses[I].Y, 
			Data.VelocitiesAndMasses[I].Z, 
			Data.VelocitiesAndMasses[I].W);
		NewCoarseParticles.Add(CP);
	}

	if (ParticleDensity == 0.0f)
	{
		return;
	}


	TArray<FIntVector4> ActiveVoxelIndices = GetActiveVoxelsFromSet(ActiveVoxelSet);
	UpsamplingDataInterface->SetCoarseParticles(NewCoarseParticles);
	UpsamplingDataInterface->SetActiveVoxelIndices(ActiveVoxelIndices);
	UpsamplingDataInterface->RecalculateFineParticleProperties(
		Upsampling, ElementSize, ParticleDensity);
	UpsamplingDataInterface->SetStaticVariables(UsedVoxelSize, EaseStepSize);
	int HashTableSize = UpsamplingDataInterface->GetHashTableCapacity();


#if UE_VERSION_OLDER_THAN(5, 3, 0)
	ParticleSystemComponent->SetNiagaraVariableInt(
		"User.Active Voxels Count", ActiveVoxelIndices.Num());
	ParticleSystemComponent->SetNiagaraVariableInt("User.HashTable Size", HashTableSize);
	ParticleSystemComponent->SetNiagaraVariableFloat("User.Voxel Size", UsedVoxelSize);
#else
	ParticleSystemComponent->SetVariableInt(
		FName("User.Active Voxels Count"), ActiveVoxelIndices.Num());
	ParticleSystemComponent->SetVariableInt(FName("User.HashTable Size"), HashTableSize);
	ParticleSystemComponent->SetVariableFloat(FName("User.Voxel Size"), UsedVoxelSize);
#endif
}

void UAGX_UpsamplingParticleRendererComponent::AppendIfActiveVoxel(
	TSet<FIntVector>& ActiveVoxelIndices, FVector Position, float Radius, float SizeOfVoxel)
{

	float AABBRadius = VOLUME_MOD * Radius / 2;
	FVector VSPosition = Position / SizeOfVoxel;

	FVector OffsetPosition(
		FMath::Sign(VSPosition.X), FMath::Sign(VSPosition.Y), FMath::Sign(VSPosition.Z));

	OffsetPosition *= 0.5;
	OffsetPosition += VSPosition;

	FVector VSParticleVoxelPos(
		(double) (int) OffsetPosition.X, (double) (int) OffsetPosition.Y,
		(double) (int) OffsetPosition.Z);

	int n = (int) (AABBRadius / SizeOfVoxel + 1);
	for (int x = -n; x <= n; x++)
	{
		for (int y = -n; y <= n; y++)
		{
			for (int z = -n; z <= n; z++)
			{
				FVector VSVoxelPos = VSParticleVoxelPos + FVector(x, y, z);
				FVector VSVoxelToParticle = VSPosition - VSVoxelPos;

				FVector VSToEdge =
					FVector::Max(VSVoxelToParticle, -VSVoxelToParticle) - AABBRadius / SizeOfVoxel;
				FVector VSVoxelSize2(0.5);
				if (FVector::Max(VSToEdge, VSVoxelSize2) == VSVoxelSize2)
				{
					VSVoxelPos += FVector(
									  FMath::Sign(VSVoxelPos.X), FMath::Sign(VSVoxelPos.Y),
									  FMath::Sign(VSVoxelPos.Z)) *
								  0.5;
					ActiveVoxelIndices.Add(FIntVector(VSVoxelPos));
				}
			}
		}
	}
}

TArray<FIntVector4> UAGX_UpsamplingParticleRendererComponent::GetActiveVoxelsFromSet(
	TSet<FIntVector> VoxelSet)
{
	TArray<FIntVector4> ActiveVoxels;
	for (FIntVector Voxel : VoxelSet)
	{
		ActiveVoxels.Add(FIntVector4(Voxel, 0));
	}
	return ActiveVoxels;
}

#if WITH_EDITOR

void UAGX_UpsamplingParticleRendererComponent::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_UpsamplingParticleRendererComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_UpsamplingParticleRendererComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

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
		AGX_MEMBER_NAME(bEnableParticleRendering),
		[](ThisClass* This) { This->SetEnableParticleRendering(This->bEnableParticleRendering); });
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bOverrideVoxelSize),
		[](ThisClass* This) { This->SetOverrideVoxelSize(This->bOverrideVoxelSize); });
}

#endif
