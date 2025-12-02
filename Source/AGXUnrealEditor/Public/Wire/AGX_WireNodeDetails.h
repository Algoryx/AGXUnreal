// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "IDetailCustomNodeBuilder.h"
#include "SceneOutlinerFilters.h"
#include "Widgets/Input/SComboBox.h"

class FAGX_WireComponentVisualizer;
class UAGX_WireComponent;

class IDetailLayoutBuilder;

/**
 * A Slate builder that produces the widgets exposing the properties for a single selected wire
 * route node. Created by AGX_WireDetails and controlled by the node selections handled by
 * AGX_WireComponentVisualizer.
 */
class FAGX_WireNodeDetails : public IDetailCustomNodeBuilder,
							 public TSharedFromThis<FAGX_WireNodeDetails>
{
	/*
	 * Slate functionality is organized in layers held and orchestrated by the owning class. The
	 * layers are:
	 * - Widgets. The pixels on the screen. Rendered every frame.
	 * - Callbacks. Functions called by the widget. Either every frame or when interacted with.
	 * - Storage. In-class backing storage read by the widget renderers through the callbacks.
	 * - Objects. The actual objects that the widgets and storage represents. May be more than one.
	 *
	 * When the widgets are created, function pointers are passed and registered as callbacks. There
	 * are callbacks both for getting the current value to be rendered, called a read callback, and
	 * for performing some action when the end-user interacts with the widget, called a write
	 * callback.
	 *
	 * Crucially, the widget is NOT regenerated/recreated when the state changes so these callbacks
	 * must be able to accommodate any possible change to the underlying objects.
	 *
	 * The storage layer consists of TOptionals, for simple values, and TArray<TSharedPtr>s, for
	 * selection lists, that the widget uses for its function. Sometimes the widget knows about
	 * the storage directly, e.g., SComboBox and its source, and sometimes the read callback returns
	 * a value to be rendered by reading from the storage, e.g., SVectorInputBox.
	 *
	 * It is the responsibility of the Tick virtual member function to synchronize the storage layer
	 * with the current object state. If that is not possible in some cases then call ExecuteIfBound
	 * on the FSimpleDelegate passed to SetOnRebuildChildren during initialization. This will
	 * recreate everything from scratch so consider this a last fallback.
	 */
public:
	FAGX_WireNodeDetails(IDetailLayoutBuilder& InDetailBuilder);

	//~ Begin IDetailCustomNodeBuilder.
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;
	virtual bool InitiallyCollapsed() const override;
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override;
	virtual FName GetName() const override;
	virtual bool RequiresTick() const override;
	virtual void Tick(float DeltaTime) override;
	//~ End IDetailCustomNodeBuilder.

private:
	/*
	 * WIDGETS
	 * Only widgets for which we need functionality beyond its supported callbacks are stored.
	 */

	/// Used to select between Free, Eye, and BodyFixed.
	/// Stored because we need to change the selected item when switching between nodes.
	TSharedPtr<SComboBox<TSharedPtr<FString>>> NodeTypeComboBox;

	/// Used to select which body among those in an Actor that an Eye or BodyFixed should attach to.
	/// Stored because it must be rebuilt when a new node is selected or the Rigid Body is changed.
	TSharedPtr<SComboBox<TSharedPtr<FString>>> BodyNameComboBox;

	/*
	 * CALLBACKS
	 * I use OnSet for all write callbacks and OnGet for all read callbacks.
	 * Button callbacks uses OnClicked.
	 */

	/* Selection. */

	// Getters.

	FText OnGetSelectedNodeIndexText() const;

	// Setters.

	FReply OnClickedSelectFirstNode();

	/* Location. */

	// Getters.

	/** Callbacks called when the Location widget is rendered. */
	TOptional<float> OnGetLocationX() const;
	TOptional<float> OnGetLocationY() const;
	TOptional<float> OnGetLocationZ() const;

	// Setters.

	/// Callback called when the Location widget is edited.
	void OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis);

	/* Node type. */

	// Getters.

	/// Called for each entry in the WireNodeTypes combo box, to generate the list entries.
	TSharedRef<SWidget> OnGetNodeTypeEntryWidget(TSharedPtr<FString> InComboString);

	/// Called to generate the default (non-opened) view of the node type combo box.
	FText OnGetNodeTypeLabel() const;

	// Setters.

	/// Called when the end-user selects an entry in the node type combo box.
	void OnSetNodeType(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

	/* Rigid body. */

	// Getters.

	/// Called to generate the default (non-opened) view of the rigid body combo box.
	FText OnGetRigidBodyLabel() const;

	/// Called to get the color of the Rigid Body name. Used to highlight invalid selections.
	FSlateColor OnGetRigidBodyNameColor() const;

	/// Called to generated the label identifying the currently selected body owning Actor.
	FText OnGetRigidBodyOwnerLabel() const;

	/// Called to test if the Actor can be used as the body owner, i.e., if it has a rigid body.
	bool OnGetHasRigidBody(const AActor* Actor);

	/// Called to limit the set of types that can be picked when selecting rigid body owner.
	void OnGetAllowedClasses(TArray<const UClass*>& AllowedClasses);

	/// Called to limit the set of Actors that can be selected as the rigid body owner.
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	void OnGetActorFilters(TSharedPtr<SceneOutliner::FOutlinerFilters>& Filters);
#else
	void OnGetActorFilters(TSharedPtr<FSceneOutlinerFilters>& OutFilters);
#endif

	/// Called for each entry in the RigidBody combo box, to generate the list entries.
	TSharedRef<SWidget> OnGetRigidBodyEntryWidget(TSharedPtr<FString> InComboString);

	// Setters.

	/// Called when the end-user picks a rigid body owner Actor.
	void OnSetRigidBodyOwner(AActor* Actor);

	/// Called when the end-user selects an entry in the rigid body combo box.
	void OnSetRigidBody(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);

	/*
	 * STORAGE
	 * The getter callbacks, prefixed with OnGet, should only read these values, not from the object
	 * directly. Not entirely sure why, but that's how it's done in the engine code. See
	 * FSplinePointDetails in SplineComponentDetails.cpp. I guess it's for performance reasons.
	 */

	FText SelectedNodeIndexText;

	// Backing storage for the selected node's location.
	TOptional<float> LocationX;
	TOptional<float> LocationY;
	TOptional<float> LocationZ;

	// Backing storage for the selected node's type.
	TOptional<EWireNodeType> NodeType;
	TArray<TSharedPtr<FString>> WireNodeTypes;
	FText NodeTypeText;

	/// @todo Consider making this a FName array instead.
	// Backing storage for the rigid body name.
	TArray<TSharedPtr<FString>> RigidBodyNames;
	FText RigidBodyNameText;
	FText RigidBodyOwnerLabelText;

	void ClearStorage();

public:
	/*
	 * OBJECTS
	 * Functions and variables related to getting the actual object we're displaying the state for
	 * and making changes on. Not storing the object itself because it can change at any time.
	 */

	/**
	 * Get the currently selected wire. A wire will only be returned if there is agreement on which
	 * wire is selected between the Wire Visualizer and the Detail Builder. So selecting one wire in
	 * the Component List and another in the Viewport will return nullptr here.
	 *
	 * @return The currently selected wire.
	 */
	UAGX_WireComponent* GetWire() const;

	/**
	 * Get the currently selected node index from the Wire Visualizer. This is the node whose state
	 * should be shown in the widgets, and edits made in the widgets should be applied to this node.
	 *
	 * Will return INDEX_NONE if no node is selected or if the Wire Visualizer couldn't be found.
	 *
	 * @return The index of the route node that the user most recently clicked on in the Viewport.
	 */
	int32 GetNodeIndex() const;

	/**
	 * @return The Wire that was selected during the most recent call to UpdateValues.
	 */
	UAGX_WireComponent* GetPreviousWire() const;

	/**
	 * @return  The node index that was selected during the most recent call to UpdateValues.
	 */
	int32 GetPreviousNodeIndex() const;

private:
	/**
	 * The currently active Wire Component Visualizer. Used to fetch the wire that owns the
	 * currently selected route node.
	 *
	 * Not sure how this works with multiple Detail Panels, selections, and Viewports.
	 */
	FAGX_WireComponentVisualizer* WireVisualizer = nullptr;

	/**
	 * The builder for the currently active Details Panel. Used to fetch the selected wire.
	 */
	IDetailLayoutBuilder& DetailBuilder;

private:
	/**
	 * @return true if we have a wire and a node index that is valid for that wire.
	 */
	bool HasWireAndNodeSelection() const;

	/**
	 * Update the Details Panel state from the selected wire and node.
	 *
	 * Resets the wire and node selection if incompatible edits has been made elsewhere.
	 *
	 * Copy node properties from the selected node, if any, into the backing storage.
	 */
	void UpdateValues();

	/// Widgets with this visibility is only shown when a wire node is selected.
	EVisibility WithSelection() const;

	/// Widgets with this visibility is only shown when no wire node is selected.
	EVisibility WithoutSelection() const;

	/// Widgets with this visibility is only shown when the selected wire node type needs a body.
	EVisibility NodeHasRigidBody() const;

private:
	/// Delegate that the Slate system provides for us. When executed a rebuild of all widgets is
	/// triggered.
	FSimpleDelegate OnRegenerateChildren;

	FProperty* RouteNodesProperty = nullptr;

	/// The wire that was used in the previous (most recent) UpdateValues. Used to detect when we
	/// must rebuild combo box sources and such.
	///
	/// Should not be dereferenced outside of UpdateValues.
	UAGX_WireComponent* PreviousWire = nullptr;

	/// The node index that was used in the previous (most recent) UpdateValues. Used to detect when
	/// we must rebuild combo box sources and such.
	int32 PreviousNodeIndex = INDEX_NONE;

	/// Some of the callbacks manipulate other widgets. In these cases the original callback is the
	/// one responsible for maintaining all invariants and the transitive callbacks should not do
	/// anything since that will interfere with the original callback. We signal that we already
	/// have a callback on the stack by setting this flag, which is tested at the top of each
	/// callback.
	bool bIsRunningCallback = false;

	/**
	 * Rebuilt the Rigid Body combo box.
	 *
	 * Selects the ToSelect entry if it exists. Selects the first non-empty entry if ToSelect
	 * doesn't exist. Selects the empty string entry if there is no non-empty entry.
	 *
	 * Call this when the Details Panel is being updated in response to an edit.
	 * @see RebuildRigidBodyComboBox_View
	 *
	 * The returned string should be assigned to the node's Rigid Body name.
	 *
	 * @param ToSelect The contents of the entry which we wish to select in the Combo Box.
	 * @param Actor The Actor in which we search for Rigid Body Components.
	 */
	FString RebuildRigidBodyComboBox_Edit(const FString& ToSelect, AActor* Actor);

	/**
	 * Rebuild the Rigid Body combo box.
	 *
	 * Selects the ToSelect entry if it exists. Selects the empty string entry if ToSelect doesn't
	 * exist in the updated list.
	 *
	 * Call this when the Details Panel is being updated without changing the Node.
	 * @see RebuildRigidBodyComboBox_Edit
	 *
	 * @param ToSelect The contents of the entry which we wish to select in the Combo Box.
	 * @param Actor The Actor in which we search for Rigid Body Components.
	 */
	void RebuildRigidBodyComboBox_View(const FString& ToSelect, AActor* Actor);

	/**
	 * Replace the contents of RigidBodyNames with the names of the Rigid Body Components in Actor.
	 * An empty string is added before the names to make it possible to select "nothing".
	 * A nullptr Actor produces a list containing only the empty string.
	 *
	 * @param Actor The Actor to search for Rigid Body Components in.
	 */
	void RebuildRigidBodyNames(AActor* Actor);

	void RebuildRigidBodyNamesFromBlueprint();
};
