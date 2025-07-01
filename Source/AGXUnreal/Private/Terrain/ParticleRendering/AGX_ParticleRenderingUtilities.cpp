// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/AGX_ParticleRenderingUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

void AGX_ParticleRenderingUtilities::AssignDefaultNiagaraAsset(
	auto*& AssetRefProperty, const TCHAR* AssetPath)
{
	if (AssetRefProperty != nullptr)
		return;

	using Type = typename std::remove_reference<decltype(*AssetRefProperty)>::type;
	auto AssetFinder = ConstructorHelpers::FObjectFinder<Type>(AssetPath);
	if (!AssetFinder.Succeeded())
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Expected to find asset '%s' but it was not found."), AssetPath);
		return;
	}

	AssetRefProperty = AssetFinder.Object;
}

AAGX_Terrain* AGX_ParticleRenderingUtilities::GetParentTerrainActor(
	UActorComponent* ActorComponent)
{
	// First get parent actor.
	AActor* Owner = ActorComponent->GetOwner();
	if (!Owner)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle Renderer '%s', unable to fetch the parent actor. "
				"No particles will be rendered."),
			*ActorComponent->GetName());
		return nullptr;
	}

	// Then cast it to the AGX Terrain actor.
	AAGX_Terrain* ParentTerrainActor = Cast<AAGX_Terrain>(Owner);
	if (!ParentTerrainActor)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle Renderer '%s', unable to cast its parent '%s' to a 'AGX Terrain' actor. "
				"No particles will be rendered."),
			*ActorComponent->GetName(), *GetLabelSafe(ActorComponent->GetOwner()));
		return nullptr;
	}

	return ParentTerrainActor;
}

UNiagaraComponent* AGX_ParticleRenderingUtilities::InitializeNiagaraParticleSystemComponent(
	UNiagaraSystem* ParticleSystemAsset, UActorComponent* ActorComponent)
{
	if (!ParticleSystemAsset)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Particle renderer '%s' in Actor '%s', no particle system assigned in the details panel. "
				 "No particles will be rendered."),
			*ActorComponent->GetName(), *GetLabelSafe(ActorComponent->GetOwner()));
		return nullptr;
	}

	// It is important that we attach the ParticleSystemComponent using "KeepRelativeOffset" so that
	// it's world position becomes the same as the Terrain's. Otherwise it will be spawned at
	// the world origin which in turn may result in particles being culled and not rendered if the
	// terrain is located far away from the world origin (see Fixed Bounds in the Particle System).
	UNiagaraComponent* ParticleSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
		ParticleSystemAsset, ActorComponent->GetOwner()->GetRootComponent(), NAME_None,
		FVector::ZeroVector, FRotator::ZeroRotator, FVector::OneVector,
		EAttachLocation::Type::KeepRelativeOffset, false,
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

	return ParticleSystemComponent;
}
