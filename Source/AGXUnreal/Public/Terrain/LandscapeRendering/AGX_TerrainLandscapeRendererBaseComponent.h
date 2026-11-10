// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainPagerBarrier.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"

#include "AGX_TerrainLandscapeRendererBaseComponent.generated.h"

class AAGX_Terrain;

/** Common lifecycle for Landscape deformation renderers owned by an AGX Terrain. */
UCLASS(Abstract, ClassGroup = "AGX_Terrain")
class AGXUNREAL_API UAGX_TerrainLandscapeRendererBaseComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	/** Enable Landscape deformation rendering. May be changed during Play; physics continues while
	 * disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Landscape Rendering")
	bool bEnabled = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Landscape Rendering")
	void SetEnabled(bool bInEnabled);
	UFUNCTION(BlueprintPure, Category = "AGX Landscape Rendering")
	bool GetEnabled() const;

	bool IsRendererInitialized() const;
	bool Initialize(AAGX_Terrain& Terrain);
	void Update(const TArray<FTerrainPagerHeightSample>& Samples);
	void Shutdown();

#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
protected:
	virtual void BeginPlay() override;
	virtual bool InitializeRenderer()
		PURE_VIRTUAL(UAGX_TerrainLandscapeRendererBaseComponent::InitializeRenderer, return false;);
	virtual void UpdateRenderer(const TArray<FTerrainPagerHeightSample>& Samples)
		PURE_VIRTUAL(UAGX_TerrainLandscapeRendererBaseComponent::UpdateRenderer, );
	virtual void ShutdownRenderer()
		PURE_VIRTUAL(UAGX_TerrainLandscapeRendererBaseComponent::ShutdownRenderer, );
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UPROPERTY(Transient)
	TObjectPtr<AAGX_Terrain> Terrain;

private:
	bool bInitialized = false;
};
