// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_SoilParticleRendererComponent.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/AGX_ParticleRenderingUtilities.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraDataInterfaceArray.h"
#include "NiagaraSystemInstance.h"
#include "NiagaraEmitterInstance.h"
#include "NiagaraSystemInstanceController.h"

UAGX_SoilParticleRendererComponent::UAGX_SoilParticleRendererComponent()
{
	AGX_ParticleRenderingUtilities::AssignDefaultNiagaraAsset(
		ParticleSystemAsset,
		TEXT("NiagaraSystem'/AGXUnreal/Terrain/Rendering/Particles/SoilParticleSystem/"
			 "PS_SoilParticleSystem.PS_SoilParticleSystem'"));
}

void UAGX_SoilParticleRendererComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!bEnableParticleRendering)
	{
		return;
	}

	AAGX_Terrain* ParentTerrainActor =
		AGX_ParticleRenderingUtilities::InitializeParentTerrainActor(this);
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

	// Bind function to terrain delegate to handle particle data.
	ParentTerrainActor->OnParticleData.AddDynamic(
		this, &UAGX_SoilParticleRendererComponent::HandleParticleData);
}

bool UAGX_SoilParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->DeactivateImmediate();
		ParticleSystemComponent->SetActive(bEnabled);
	}
	return bEnableParticleRendering = bEnabled;
}

bool UAGX_SoilParticleRendererComponent::GetEnableParticleRendering()
{
	return bEnableParticleRendering;
}

UNiagaraComponent* UAGX_SoilParticleRendererComponent::GetParticleSystemComponent()
{
	return ParticleSystemComponent;
}

void UAGX_SoilParticleRendererComponent::HandleParticleData(FDelegateParticleData data)
{
	if (ParticleSystemComponent == nullptr)
	{
		return;
	}

	const FNiagaraParameterStore& UserParams = ParticleSystemComponent->GetOverrideParameters();
	TArray<FNiagaraVariable> Params;
	UserParams.GetParameters(Params);

	// Check for the default particle system parameters.
	// Otherwise the engine throws warnings each time we set them...
	for (FNiagaraVariable& Param : Params)
	{
		FString ParamName = Param.GetName().ToString();
		FString ParamType = Param.GetType().GetName();

		if (ParamType == "NiagaraDataInterfaceArrayFloat4")
		{
			if (ParamName == "User.Positions And Scales")
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, "Positions And Scales", data.PositionsAndScale);
			}
			else if (ParamName == "User.Orientations")
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, "Orientations", data.Orientations);
			}
			else if (ParamName == "User.Velocities And Masses")
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, "Velocities And Masses", data.VelocitiesAndMasses);
			}
		}
		else if (
			ParamName == "User.Exists" && 
			ParamType == "NiagaraDataInterfaceArrayBool")
		{
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
				ParticleSystemComponent, "Exists", data.Exists);
		}
		else if (
			ParamName == "User.Target Particle Count" && 
			ParamType == "NiagaraInt32")
		{

#if UE_VERSION_OLDER_THAN(5, 3, 0)
			ParticleSystemComponent->SetNiagaraVariableInt(
				"User.Target Particle Count", data.TargetParticleCount);
#else
			ParticleSystemComponent->SetVariableInt(
				FName("User.Target Particle Count"), data.TargetParticleCount);
#endif
		}
	}
}

#if WITH_EDITOR

void UAGX_SoilParticleRendererComponent::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_SoilParticleRendererComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_SoilParticleRendererComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(bEnableParticleRendering),
		[](ThisClass* This) { This->SetEnableParticleRendering(This->bEnableParticleRendering); });

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(ParticleSystemAsset),
		[](ThisClass* This)
		{
			if (This->ParticleSystemAsset != nullptr)
			{
				This->ParticleSystemAsset->RequestCompile(true);
			}
		});

}

#endif
