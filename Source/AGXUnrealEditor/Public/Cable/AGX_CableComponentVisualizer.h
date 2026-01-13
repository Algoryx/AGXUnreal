// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "Framework/Commands/UICommandList.h"

class FPrimitiveDrawInterface;
class FSceneView;
class UActorComponent;
class UAGX_CableComponent;

class AGXUNREALEDITOR_API FAGX_CableComponentVisualizer : public FComponentVisualizer
{
public:
	FAGX_CableComponentVisualizer();
	~FAGX_CableComponentVisualizer();

	//~ Begin FComponentVisualizer Interface.
	virtual void OnRegister() override;
	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
	virtual bool VisProxyHandleClick(
		FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
		const FViewportClick& Click) override;
	virtual bool GetWidgetLocation(
		const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;
	virtual bool GetCustomInputCoordinateSystem(
		const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const override;
	virtual bool HandleInputDelta(
		FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
		FRotator& DeltaRotate, FVector& DeltaScale) override;
	virtual bool HandleInputKey(
		FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key,
		EInputEvent Event) override;
	virtual bool IsVisualizingArchetype() const override;
	virtual void EndEditing() override;
	//~ End FComponentVisualizer Interface.

	bool HasValidEditNode() const;
	UAGX_CableComponent* GetEditCable() const;
	int32 GetEditNodeIndex() const;
	void SetEditNodeIndex(int32 InIndex);
	void ClearEdit();

private:
	void OnDeleteKey();
	bool CanDeleteKey() const;

private:
	/// The index of the node currently selected for editing, if any. INDEX_NONE otherwise.
	int32 EditNodeIndex {INDEX_NONE};

	/**
	 * Property path from the owning Actor to the Cable Component of the Cable currently selected
	 * for editing. We must use a path instead of a pointer because during Blueprint Reconstruction
	 * the Cable Component will be replaced by a new instance.
	 */
	FComponentPropertyPath EditCablePropertyPath;

	FProperty* RouteNodesProperty {nullptr};

	/// True while a node duplication move is in progress, so that we don't create a new each frame.
	bool bIsDuplicatingNode {false};

	TSharedPtr<FUICommandList> CommandList;

	/// A library of helper function manipulating the private state of
	/// FAGX_CableComponentVisualizer.
	friend class FCableVisualizerOperations;
};
