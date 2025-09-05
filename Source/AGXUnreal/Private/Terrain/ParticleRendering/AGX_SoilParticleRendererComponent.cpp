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

	// Bind function to terrain delegate to handle particle data.
	ParentTerrainActor->OnParticleData.AddDynamic(
		this, &UAGX_SoilParticleRendererComponent::HandleParticleData);
}

void UAGX_SoilParticleRendererComponent::SetEnableParticleRendering(bool bEnabled)
{
	if (ParticleSystemComponent)
	{
		ParticleSystemComponent->DeactivateImmediate();
		ParticleSystemComponent->SetActive(bEnabled);
	}

	bEnableParticleRendering = bEnabled;
}

bool UAGX_SoilParticleRendererComponent::GetEnableParticleRendering() const
{
	return bEnableParticleRendering;
}

UNiagaraComponent* UAGX_SoilParticleRendererComponent::GetParticleSystemComponent()
{
	return ParticleSystemComponent;
}

const UNiagaraComponent* UAGX_SoilParticleRendererComponent::GetParticleSystemComponent() const
{
	return ParticleSystemComponent;
}

void UAGX_SoilParticleRendererComponent::HandleParticleData(FDelegateParticleData& Data)
{
	if (ParticleSystemComponent == nullptr)
	{
		return;
	}

	static const FName PositionsAndRadiiName {TEXT("User.Positions And Radii")};
	static const FName VelocitiesAndMassesName {TEXT("User.Velocities And Masses")};
	static const FName OrientationsName {TEXT("User.Orientations")};
	static const FName ExistsName {TEXT("User.Exists")};
	static const FName ParticleCountName {TEXT("User.Particle Count")};

	static const FName Vector4ArrayName {TEXT("NiagaraDataInterfaceArrayFloat4")};
	static const FName BoolArrayName {TEXT("NiagaraDataInterfaceArrayBool")};
	static const FName Int32Name {TEXT("NiagaraInt32")};

	const FNiagaraParameterStore& UserParams = ParticleSystemComponent->GetOverrideParameters();
	TArray<FNiagaraVariable> Params;
	UserParams.GetParameters(Params);

	// Check for the default particle system parameters.
	// Otherwise the engine throws warnings each time we set them...
	for (FNiagaraVariable& Param : Params)
	{
		const FName ParamName = Param.GetName();
		const FName ParamType = Param.GetType().GetFName();

		if (ParamType == Vector4ArrayName)
		{
			if (ParamName == PositionsAndRadiiName)
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, ParamName, Data.PositionsAndRadii);
			}
			else if (ParamName == OrientationsName)
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, ParamName, Data.Orientations);
			}
			else if (ParamName == VelocitiesAndMassesName)
			{
				UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector4(
					ParticleSystemComponent, ParamName, Data.VelocitiesAndMasses);
			}
		}
		else if (ParamName == ExistsName && ParamType == BoolArrayName)
		{
			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayBool(
				ParticleSystemComponent, ParamName, Data.Exists);
		}
		else if (ParamName == ParticleCountName && ParamType == Int32Name)
		{
#if UE_VERSION_OLDER_THAN(5, 3, 0)
			ParticleSystemComponent->SetNiagaraVariableInt(ParamName, Data.ParticleCount);
#else
			ParticleSystemComponent->SetVariableInt(ParamName, Data.ParticleCount);
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
				// TODO If we are in Play, replace the `ParticleSystemComponent` with a new one that uses the new particle system asset.
			}
		});

}

#endif
