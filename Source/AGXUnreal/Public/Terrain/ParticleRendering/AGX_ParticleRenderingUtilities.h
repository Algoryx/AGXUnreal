// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AAGX_Terrain;
class UAGX_MovableTerrainComponent;
class UActorComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USceneComponent;

class AGXUNREAL_API AGX_ParticleRenderingUtilities
{
public:
	/**
	 * Finds the parent terrain of the given component and returns it. Cannot
	 * render particles if the terrain is not found since we cannot bind to the
	 * particle data delegate.
	 */
	static AAGX_Terrain* GetParentTerrainActor(
		UActorComponent* ActorComponent, bool SkipWarnings = false);

	/**
	 * Finds and returns the attach parent Movable Terrain Component for the given scene component.
	 * Returns nullptr if none is found.
	 * Only looks at the immediate parent component.
	 */
	static UAGX_MovableTerrainComponent* GetParentMovableTerrainComponent(
		const USceneComponent& SceneComponent, bool SkipWarnings = false);

	/**
	 * Initializes the Niagara VFX System and attaches to the parent of the given component.
	 */
	static UNiagaraComponent* InitializeNiagaraParticleSystemComponent(
		UNiagaraSystem* ParticleSystemAsset, UActorComponent* ActorComponent);

	/**
	 * Assigns the default Niagara VFX System asset when adding the component to an actor.
	 */
	static void AssignDefaultNiagaraAsset(
		UNiagaraSystem*& AssetRefProperty, const TCHAR* AssetPath);
};
