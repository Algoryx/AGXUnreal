// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"
#include "Framework/Commands/UICommandList.h"

class UAGX_WireComponent;

/**
 * The Wire Component Visualizer uses lines and points to produce a simple visualization of a wire,
 * both its route nodes, while editing, and the simulation nodes, during a Play In Editor session.
 *
 * While in edit mode the Wire Component Visualizer provides functionality to duplicate, move, and
 * delete nodes, collectively called editing a wire. Editing mode is enabled for at most one wire at
 * a time.
 *
 * The Wire Component Visualizer maintains a node selection state for the currently edited wire,
 * which can be used to provide additional manipulation operations on the node, such as from a
 * Details Customization.
 *
 * To avoid ambiguity for the word "selected" we use "selected" to refer to something selected
 * within the Unreal Editor at large, either in the Level Editor or the Blueprint Editor. The wire
 * and node tracking done by the Wire Component Visualizer is instead called "edited" or "editing",
 * as in "The currently edited wire must be one of the selected wires.".
 */
class AGXUNREALEDITOR_API FAGX_WireComponentVisualizer : public FComponentVisualizer
{
	using Super = FComponentVisualizer;

public:
	FAGX_WireComponentVisualizer();
	~FAGX_WireComponentVisualizer();

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
	bool HasValidEditWinch() const;
	UAGX_WireComponent* GetEditWire() const;
	int32 GetEditNodeIndex() const;
	void SetEditNodeIndex(int32 InIndex);
	void ClearEdit();

private:
	void OnDeleteKey();
	bool CanDeleteKey() const;

private:
	/// The index of the node currently selected for editing, if any. INDEX_NONE otherwise.
	int32 EditNodeIndex = INDEX_NONE;

	EWireSide EditWinch = EWireSide::None;
	EWinchSide EditWinchSide = EWinchSide::None;

	/**
	 * Property path from the owning Actor to the Wire Component of the wire currently selected for
	 * editing. We must use a path instead of a pointer because during Blueprint Reconstruction the
	 * Wire Component will be replaced by a new instance.
	 */
	FComponentPropertyPath EditWirePropertyPath;

	FProperty* RouteNodesProperty {nullptr};
	FProperty* BeginWinchProperty {nullptr};
	FProperty* EndWinchProperty {nullptr};

	/// True while a node duplication move is in progress, so that we don't create a new each frame.
	bool bIsDuplicatingNode = false;

	TSharedPtr<FUICommandList> CommandList;

	/// A library of helper function manipulating the private state of FAGX_WireComponentVisualizer.
	friend class FWireVisualizerOperations;
};
