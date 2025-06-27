// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AAGX_Terrain;
class UNiagaraSystem;
class UNiagaraComponent;

class AGXUNREAL_API AGX_ParticleRenderingUtilities
{
public:
	/**
	 * Finds the parent terrain of the given component and returns it. Cannot
	 * render particles if the terrain is not found since we cannot bind to the
	 * particle data delegate.
	 */
	static AAGX_Terrain* InitializeParentTerrainActor(UActorComponent* ActorComponent);

	/**
	 * Initializes the Niagara VFX System and attaches to the parent of the given component.
	 */
	static UNiagaraComponent* InitializeNiagaraParticleSystemComponent(
		UNiagaraSystem* ParticleSystemAsset, UActorComponent* ActorComponent);

	/**
	 * Assigns the default Niagara VFX System asset when adding the component to an actor.
	 */
	static void AssignDefaultNiagaraAsset(auto*& AssetRefProperty, const TCHAR* AssetPath);
};
