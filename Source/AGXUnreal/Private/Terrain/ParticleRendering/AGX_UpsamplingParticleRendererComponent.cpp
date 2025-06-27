// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_UpsamplingParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"
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

UAGX_UpsamplingParticleRendererComponent::UAGX_UpsamplingParticleRendererComponent()
{
	AGX_ParticleRenderingUtilities::AssignDefaultNiagaraAsset(
		ParticleSystemAsset,
		TEXT("NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/UpsamplingParticleSystem"
			 "/PS_MeshPUSystem.PS_MeshPUSystem'"));
}

void UAGX_UpsamplingParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!bEnableParticleRendering)
	{
		return;
	}

	AAGX_Terrain* ParentTerrainActor = AGX_ParticleRenderingUtilities::InitializeParentTerrainActor(this);
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

	// Try to get the correct data interface.
	UpsamplingDataInterface =
		static_cast<UAGX_ParticleUpsamplingDI*>(UNiagaraFunctionLibrary::GetDataInterface(
			UAGX_ParticleUpsamplingDI::StaticClass(), ParticleSystemComponent,
			"PUDI"));

	if (!UpsamplingDataInterface)
	{
		UE_LOG(
			LogTemp, Warning,
			TEXT("Particle renderer '%s' could not find Niagara Data Interface with name 'PUDI' "
				"in loaded Niagara system with name '%s', cannot render particles..."),
			*GetName(), *ParticleSystemComponent->GetName());
		return;
	}

	ElementSize = ParentTerrainActor->SourceLandscape->GetActorScale().X;
	
	// Bind function to terrain delegate to handle particle data.
	ParentTerrainActor->OnParticleData.AddDynamic(
		this, &UAGX_UpsamplingParticleRendererComponent::HandleParticleData);
}

bool UAGX_UpsamplingParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->DeactivateImmediate();
		ParticleSystemComponent->SetActive(bEnabled);
	}
	return bEnableParticleRendering = bEnabled; 
}

bool UAGX_UpsamplingParticleRendererComponent::GetEnableParticleRendering()
{
	return bEnableParticleRendering;
}

UNiagaraComponent* UAGX_UpsamplingParticleRendererComponent::GetParticleSystemComponent()
{
	return ParticleSystemComponent;
}

int32 UAGX_UpsamplingParticleRendererComponent::SetUpsampling(int32 InUpsampling)
{
	return Upsampling = InUpsampling;
}

int32 UAGX_UpsamplingParticleRendererComponent::GetUpsampling()
{
	return Upsampling;
}

bool UAGX_UpsamplingParticleRendererComponent::SetEnableVoxelSize(bool bEnabled)
{
	return bEnableVoxelSize = bEnabled;
}

bool UAGX_UpsamplingParticleRendererComponent::GetEnableVoxelSize()
{
	return bEnableVoxelSize;
}

double UAGX_UpsamplingParticleRendererComponent::SetVoxelSize(double InVoxelSize)
{
	return VoxelSize = InVoxelSize;
}

double UAGX_UpsamplingParticleRendererComponent::GetVoxelSize()
{
	return VoxelSize;
}

double UAGX_UpsamplingParticleRendererComponent::SetEaseStepSize(double InEaseStepSize)
{
	return EaseStepSize = InEaseStepSize;
}

double UAGX_UpsamplingParticleRendererComponent::GetEaseStepSize()
{
	return EaseStepSize;
}

void UAGX_UpsamplingParticleRendererComponent::HandleParticleData(FDelegateParticleData data)
{
	if (!ParticleSystemComponent || !UpsamplingDataInterface)
	{
		return;
	}

	TArray<FCoarseParticle> NewCoarseParticles;
	TSet<FIntVector> ActiveVoxelSet;
	float ParticleDensity = 0.0f;
	for (int32 I = 0; I < data.TargetParticleCount; ++I)
	{
		if (data.Exists[I])
		{
			float Radius = data.PositionsAndScale[I].W / 2 * 100;
			float Volume = (4.0 / 3.0) * PI * FMath::Pow(Radius, 3);
			ParticleDensity = data.VelocitiesAndMasses[I].W / Volume;

			FVector Position = FVector(
				data.PositionsAndScale[I].X, 
				data.PositionsAndScale[I].Y,
				data.PositionsAndScale[I].Z);

			AppendIfActiveVoxel(ActiveVoxelSet, Position, Radius);

			FCoarseParticle CP;
			CP.PositionAndRadius = FVector4f(Position.X, Position.Y, Position.Z, Radius);
			CP.VelocityAndMass = FVector4f(
				data.VelocitiesAndMasses[I].X, 
				data.VelocitiesAndMasses[I].Y, 
				data.VelocitiesAndMasses[I].Z, 
				data.VelocitiesAndMasses[I].W);
			NewCoarseParticles.Add(CP);
		}
	}

	if (ParticleDensity == 0.0f)
	{
		return;
	}

	TArray<FIntVector4> ActiveVoxelIndices = GetActiveVoxelsFromSet(ActiveVoxelSet);
	UpsamplingDataInterface->SetCoarseParticles(NewCoarseParticles);
	UpsamplingDataInterface->SetActiveVoxelIndices(ActiveVoxelIndices);
	UpsamplingDataInterface->RecalculateFineParticleProperties(Upsampling, ElementSize, ParticleDensity);
	UpsamplingDataInterface->SetStaticVariables(VoxelSize, EaseStepSize);
	int HashTableSize = UpsamplingDataInterface->GetElementsInActiveVoxelBuffer();

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	ParticleSystemComponent->SetNiagaraVariableInt(
		"User.Active Voxels Count", ActiveVoxelIndices.Num());
	ParticleSystemComponent->SetNiagaraVariableInt("User.HashTable Size", HashTableSize);
	ParticleSystemComponent->SetNiagaraVariableFloat("User.Voxel Size", VoxelSize);
#else
	ParticleSystemComponent->SetVariableInt(
		FName("User.Active Voxels Count"), ActiveVoxelIndices.Num());
	ParticleSystemComponent->SetVariableInt(FName("User.HashTable Size"), HashTableSize);
	ParticleSystemComponent->SetVariableFloat(FName("User.Voxel Size"), VoxelSize);
#endif
}

void UAGX_UpsamplingParticleRendererComponent::AppendIfActiveVoxel(
	TSet<FIntVector>& ActiveVoxelIndices, FVector CPPosition, float CPRadius)
{
	float AABBRadius = 1.6119919540164696407169668466392849389446140723238615 * CPRadius / 2;
	FVector VSPosition = CPPosition / VoxelSize;

	FVector OffsetPosition(
		FMath::Sign(VSPosition.X), FMath::Sign(VSPosition.Y), FMath::Sign(VSPosition.Z));

	OffsetPosition *= 0.5;
	OffsetPosition += VSPosition;

	FVector VSParticleVoxelPos(
		(double) (int) OffsetPosition.X, (double) (int) OffsetPosition.Y,
		(double) (int) OffsetPosition.Z);

	int n = (int) (AABBRadius / VoxelSize + 1);
	for (int x = -n; x <= n; x++)
	{
		for (int y = -n; y <= n; y++)
		{
			for (int z = -n; z <= n; z++)
			{
				FVector VSVoxelPos = VSParticleVoxelPos + FVector(x, y, z);
				FVector VSVoxelToParticle = VSPosition - VSVoxelPos;

				FVector VSToEdge =
					FVector::Max(VSVoxelToParticle, -VSVoxelToParticle) - AABBRadius / VoxelSize;
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
}

#endif
