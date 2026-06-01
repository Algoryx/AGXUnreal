// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainEnums.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class UAGX_ShovelComponent;
struct FAGX_Frame;

/**
 * The Shovel Visualizer uses lines and points to visualize the various edges and directions that
 * characterizes an AGX Dynamics Shovel. The Shovel Visualizer maintains a frame selection state
 * that can be used to provide additional operations on the frame, such as from a Details
 * Customization, and for interactive editing in the viewport.
 */
class AGXUNREALEDITOR_API FAGX_ShovelComponentVisualizer : public FComponentVisualizer
{
public:
	FAGX_ShovelComponentVisualizer();
	virtual ~FAGX_ShovelComponentVisualizer();

	//~ Begin FComponentVisualizer interface.
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
	//~ End FComponentVisualizer interface.

	bool HasValidFrameSection() const;
	FAGX_Frame* GetSelectedFrame() const;
	EAGX_ShovelFrame GetSelectedFrameSource() const;
	FProperty* GetSelectedFrameProperty() const;
	void ClearSelection();

	UAGX_ShovelComponent* GetSelectedShovel() const;

private:
	/**
	 * The type of frame (TopEdgeBegin, ToothDirection, etc) that was most recently clicked by
	 * the user.
	 */
	EAGX_ShovelFrame SelectedFrame {EAGX_ShovelFrame::None};

	/**
	 * Property path from the owning Actor to the Shovel Component of the currently selected shovel.
	 * We must use a path instead of a pointer because during Blueprint Reconstruction the Shovel
	 * Component will be replaced by a new instance and a raw pointer would be left dangling.
	 *
	 * By "selected" we mean the Shovel owning the selected edge or direction. Note that a single
	 * Visualizer may be rendering multiple shovels in the same frame but only a single one of them
	 * may be selected by the Shovel Visualizer. Note that the Shovel Visualizer selection need not
	 * match the selected Component in any Components panel.
	 */
	FComponentPropertyPath ShovelPropertyPath;

	// Properties used when triggering modify events. Note that we only trigger events for the
	// top-level properties, i.e. entire edges or directions, not the leaf properties within those
	// structs.
	FProperty* TopEdgeProperty {nullptr};
	FProperty* CuttingEdgeProperty {nullptr};
	FProperty* ToothDirectionProperty {nullptr};

	FQuat CachedRotation {ForceInit};

	/// A library of helper functions manipulating the private state of
	/// FAGX_WireComponentVisualizer.
	friend struct FShovelVisualizerOperations;
};
