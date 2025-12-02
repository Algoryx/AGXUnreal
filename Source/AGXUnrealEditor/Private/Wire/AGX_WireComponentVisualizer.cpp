// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RuntimeStyle.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireHitProxies.h"
#include "Wire/AGX_WireNode.h"
#include "Wire/AGX_WireUtilities.h"
#include "Wire/AGX_WireWinch.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "SceneManagement.h"
#include "ScopedTransaction.h"
#include "Selection.h"
#include "SSubobjectEditor.h"
#include "UnrealEngine.h"

#define LOCTEXT_NAMESPACE "AGX_WireComponentVisualizer"

class FWireVisualizerOperations
{
public:
	static bool NodeProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		const bool bNewWire, HNodeProxy& Proxy)
	{
		if (Wire.IsInitialized())
		{
			// Node editing is currently only for route nodes. All node manipulation operations
			// operate on the route nodes, but when the wire is initialized what we're seeing are
			// the simulation nodes.
			Visualizer.ClearEdit();
			return false;
		}
		if (!bNewWire && Proxy.NodeIndex == Visualizer.EditNodeIndex)
		{
			// Clicking a selected node deselects it.
			Visualizer.ClearEdit();
			return true;
		}

		// A new node became selected.
		SetEditNode(Visualizer, Wire, Proxy.NodeIndex);
		return true;
	}

	static void SetEditNode(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire, int32 NodeIndex)
	{
		// We must handle editor selection before our wire edit selection because if another wire
		// was selected then the editor selection change will clear the Wire Component Visualizer
		// and we want the edit selection we're about to make to persist after the return from this
		// function.

		// Make sure the Wire Component that the node is part of is selected. Deselect any other
		// selected Component.
		if (FActorEditorUtils::IsAPreviewOrInactiveActor(Wire.GetOwner()))
		{
			// We are in a Blueprint Editor. Find the Blueprint Editor instance and select the
			// SCS node for the wire.
			TSharedPtr<IBlueprintEditor> BlueprintEditor =
				FKismetEditorUtilities::GetIBlueprintEditorForObject(&Wire, /*Open*/ false);
			if (BlueprintEditor.IsValid())
			{
				TArray<TSharedPtr<FSubobjectEditorTreeNode>> Selection =
					BlueprintEditor->GetSelectedSubobjectEditorTreeNodes();
				// Determine if the wire already is the only selected SCS node. If not select it.
				if (Selection.Num() != 1 || Selection[0]->GetComponentTemplate() == &Wire)
				{
					BlueprintEditor->FindAndSelectSubobjectEditorTreeNode(&Wire, false);
				}
			}
		}
		else
		{
			// We are in the Level Editor. Handle selection directly through the GEditor instance.
			TArray<UActorComponent*> SelectedComponents;
			GEditor->GetSelectedComponents()->GetSelectedObjects(SelectedComponents);
			bool bWireAlreadySelected {false};
			// Unselect everything not the Wire Component.
			for (UActorComponent* Component : SelectedComponents)
			{
				if (Component == &Wire)
				{
					bWireAlreadySelected = true;
				}
				else
				{
					GEditor->SelectComponent(Component, /*Selected*/ false, /*Notify*/ true);
				}
			}
			if (!bWireAlreadySelected)
			{
				GEditor->SelectComponent(const_cast<UAGX_WireComponent*>(&Wire), true, true);
			}
		}

		Visualizer.EditWinch = EWireSide::None;
		Visualizer.EditWinchSide = EWinchSide::None;
		Visualizer.EditNodeIndex = NodeIndex;
		Visualizer.EditWirePropertyPath = FComponentPropertyPath(&Wire);
	}

	static bool WinchLocationProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		HWinchLocationProxy& Proxy)
	{
		return WinchProxyClicked(Visualizer, Wire, Proxy.Side, EWinchSide::Location);
	}

	static bool WinchDirectionProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		HWinchDirectionProxy& Proxy)
	{
		return WinchProxyClicked(Visualizer, Wire, Proxy.Side, EWinchSide::Rotation);
	}

	static bool WinchProxyClicked(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		EWireSide& WireSide, EWinchSide WinchSide)
	{
		if (Wire.IsInitialized())
		{
			/// @todo Figure out how, if possible, one can move/orient an AGX Dynamics Wire Winch
			/// Controller during runtime. For now we don't allow editing a Wire Winch after the
			/// simulation has started.
			Visualizer.ClearEdit();
			return false;
		}
		if (Visualizer.EditWinch == WireSide && Visualizer.EditWinchSide == WinchSide)
		{
			// Clicking a selected winch deselects it.
			Visualizer.ClearEdit();
			return true;
		}
		// A new winch became selected.
		SetEditWinch(Visualizer, Wire, WireSide, WinchSide);
		return true;
	}

	static void SetEditWinch(
		FAGX_WireComponentVisualizer& Visualizer, const UAGX_WireComponent& Wire,
		EWireSide WireSide, EWinchSide WinchSide)
	{
		Visualizer.EditWinch = WireSide;
		Visualizer.EditWinchSide = WinchSide;
		Visualizer.EditNodeIndex = INDEX_NONE;
		Visualizer.EditWirePropertyPath = FComponentPropertyPath(&Wire);
	}

	static void NodeProxyDragged(
		FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire,
		FEditorViewportClient& ViewportClient, const FVector& DeltaTranslate)
	{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		if (DeltaTranslate.IsZero() || ViewportClient.GetWidgetMode() != FWidget::WM_Translate)
#else
		if (DeltaTranslate.IsZero() || ViewportClient.GetWidgetMode() != UE::Widget::WM_Translate)
#endif
		{
			return;
		}

		// TODO Is this Modify necessary? Compare with SplineComponentVisualizer.
		Wire.Modify();

		if (ViewportClient.IsAltPressed())
		{
			// A drag with Alt held down means that the current node should be duplicated and the
			// copy selected.

			if (!Visualizer.bIsDuplicatingNode)
			{
				// This is the start of a duplication drag. Create the duplicate and select it.
				DuplicateNode(Visualizer, Wire);
			}
			else
			{
				// This is a continuation of a previously started duplication drag. Move the
				// selected node, i.e., the copy.
				MoveNode(Visualizer, Wire, DeltaTranslate);
			}
		}
		else
		{
			// This is a regular drag, move the selected node.
			MoveNode(Visualizer, Wire, DeltaTranslate);
		}
	}

	static void DuplicateNode(FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire)
	{
		Visualizer.bIsDuplicatingNode = true;
		int32 NewNodeIndex = Visualizer.EditNodeIndex + 1;
		FWireRoutingNode Clone = FWireRoutingNode(Wire.RouteNodes[Visualizer.EditNodeIndex]);
		Wire.RouteNodes.Insert(Clone, NewNodeIndex);
		Visualizer.EditNodeIndex = NewNodeIndex;
		Visualizer.NotifyPropertyModified(&Wire, Visualizer.RouteNodesProperty);
		Wire.MarkVisualsDirty();
	}

	static void MoveNode(
		FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire,
		const FVector& DeltaTranslate)
	{
		FWireRoutingNode& SelectedNode = Wire.RouteNodes[Visualizer.EditNodeIndex];
		const FVector CurrentWorldLocation = SelectedNode.Frame.GetWorldLocation(Wire);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		SelectedNode.Frame.SetWorldLocation(NewWorldLocation, Wire);

		Visualizer.NotifyPropertyModified(&Wire, Visualizer.RouteNodesProperty);
		Wire.MarkVisualsDirty();
	}

	static void WinchProxyDragged(
		FAGX_WireComponentVisualizer& Visualizer, UAGX_WireComponent& Wire,
		const FVector& DeltaTranslate, const FRotator& DeltaRotate)
	{
		FAGX_WireWinch& Winch = *Wire.GetWinch(Visualizer.EditWinch);
		const FTransform& WinchToWorld =
			FAGX_WireUtilities::GetWinchLocalToWorld(Wire, Visualizer.EditWinch);
		AGX_WireVisualization_helpers::TransformWinch(
			Winch, WinchToWorld, Visualizer.EditWinchSide, DeltaTranslate, DeltaRotate);

		FProperty* EditedProperty = Visualizer.EditWinch == EWireSide::Begin
										? Visualizer.BeginWinchProperty
										: Visualizer.EndWinchProperty;
		Visualizer.NotifyPropertyModified(&Wire, EditedProperty);
		Wire.MarkVisualsDirty();
	}
};

/**
 * A collection of commands that can be triggered through the Wire Component Visualizer.
 */
class FAGX_WireComponentVisualizerCommands : public TCommands<FAGX_WireComponentVisualizerCommands>
{
public:
	FAGX_WireComponentVisualizerCommands()
		: TCommands<FAGX_WireComponentVisualizerCommands>(
			  "AGX_WireComponentVisualizer",
			  LOCTEXT("AGX_WireComponentVisualizer", "AGX Wire Component Visualizer"), NAME_None,
			  FAGX_RuntimeStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(
			DeleteKey, "Delete wire node.", "Delete the currently selected wire node",
			EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
	}

	TSharedPtr<FUICommandInfo> DeleteKey;
};

FAGX_WireComponentVisualizer::FAGX_WireComponentVisualizer()
{
	FAGX_WireComponentVisualizerCommands::Register();
	UClass* Class = UAGX_WireComponent::StaticClass();
	RouteNodesProperty =
		FindFProperty<FProperty>(Class, GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes));
	BeginWinchProperty = FindFProperty<FProperty>(
		Class, GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch));
	EndWinchProperty =
		FindFProperty<FProperty>(Class, GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch));

	CommandList = MakeShareable(new FUICommandList());
}

FAGX_WireComponentVisualizer::~FAGX_WireComponentVisualizer()
{
	FAGX_WireComponentVisualizerCommands::Unregister();
}

void FAGX_WireComponentVisualizer::OnRegister()
{
	const auto& Commands = FAGX_WireComponentVisualizerCommands::Get();

	CommandList->MapAction(
		Commands.DeleteKey,
		FExecuteAction::CreateSP(this, &FAGX_WireComponentVisualizer::OnDeleteKey),
		FCanExecuteAction::CreateSP(this, &FAGX_WireComponentVisualizer::CanDeleteKey));
}

namespace AGX_WireComponentVisualizer_helpers
{
	constexpr uint32 NumNodeColors = (uint32) EWireNodeType::NUM_NODE_TYPES;

	TStaticArray<FLinearColor, NumNodeColors> CreateWireNodeColors()
	{
		TStaticArray<FLinearColor, (uint32) EWireNodeType::NUM_NODE_TYPES> WireNodeColors;
		for (uint32 I = 0; I < NumNodeColors; ++I)
		{
			// Fallback color for any node type not assigned below.
			WireNodeColors[I] = FLinearColor::Gray;
		}
		WireNodeColors[(int) EWireNodeType::Free] = FLinearColor::Red;
		WireNodeColors[(int) EWireNodeType::Eye] = FLinearColor::Green;
		WireNodeColors[(int) EWireNodeType::BodyFixed] = FLinearColor::Blue;
		WireNodeColors[(int) EWireNodeType::Other] = FLinearColor::White;
		return WireNodeColors;
	}

	FLinearColor WireNodeTypeToColor(EWireNodeType Type)
	{
		static const TStaticArray<FLinearColor, NumNodeColors> WireNodeColors =
			CreateWireNodeColors();
		const uint32 I = static_cast<uint32>(Type);
		return WireNodeColors[I];
	}

	/**
	 * Draw the route nodes in a wire, with hit proxies if selected.
	 */
	template <typename FNodeColorFunc>
	void DrawRouteNodes(
		const UAGX_WireComponent& Wire, FPrimitiveDrawInterface* PDI,
		const FLinearColor& LineColor, FNodeColorFunc NodeColorFunc)
	{
		const FTransform& LocalToWorld = Wire.GetComponentTransform();
		const TArray<FWireRoutingNode>& Nodes = Wire.RouteNodes;
		const int32 NumNodes = Nodes.Num();

		FVector PrevLocation;

		for (int32 I = 0; I < NumNodes; ++I)
		{
			const FWireRoutingNode& Node = Nodes[I];
			const FLinearColor NodeColor = NodeColorFunc(I, Node.NodeType);
			const FVector Location = Node.Frame.GetWorldLocation(Wire);

			PDI->SetHitProxy(new HNodeProxy(&Wire, I));
			PDI->DrawPoint(
				Location, NodeColor, FAGX_WireUtilities::NodeHandleSize, SDPG_Foreground);
			PDI->SetHitProxy(nullptr);

			if (I > 0)
			{
				PDI->DrawLine(PrevLocation, Location, LineColor, SDPG_Foreground);
			}

			PrevLocation = Location;
		}
	}

	/**
	 * Draw the route nodes in a wire that is not selected.
	 */
	void DrawRouteNodes(const UAGX_WireComponent& Wire, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor LineColor = FLinearColor::White;
		auto NodeColorFunc = [](int32 I, EWireNodeType NodeType)
		{ return WireNodeTypeToColor(NodeType); };
		DrawRouteNodes(Wire, PDI, LineColor, NodeColorFunc);
	}

	/**
	 * Draw the route nodes in a wire that is selected.
	 */
	void DrawRouteNodes(
		const UAGX_WireComponent& Wire, int32 SelectedNodeIndex, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor LineColor = GEngine->GetSelectionOutlineColor();
		auto NodeColorFunc = [SelectedNodeIndex](int32 I, EWireNodeType NodeType)
		{
			return I == SelectedNodeIndex ? GEditor->GetSelectionOutlineColor()
										  : WireNodeTypeToColor(NodeType);
		};
		DrawRouteNodes(Wire, PDI, LineColor, NodeColorFunc);
	}

	/**
	 * Draw the simulation nodes of the given Wire, including lines between them. Hit proxies
	 * are not created when drawing simulation nodes.
	 */
	void DrawSimulationNodes(const UAGX_WireComponent& Wire, FPrimitiveDrawInterface* PDI)
	{
		int32 I = 0;
		TOptional<FVector> PrevLocation;

		for (auto It = Wire.GetRenderBeginIterator(), End = Wire.GetRenderEndIterator(); It != End;
			 It.Inc())
		{
			const FAGX_WireNode Node = It.Get();
			const EWireNodeType NodeType = Node.GetType();
			const FLinearColor Color = WireNodeTypeToColor(NodeType);
			const FVector Location = Node.GetWorldLocation();

			PDI->DrawPoint(Location, Color, FAGX_WireUtilities::NodeHandleSize, SDPG_Foreground);
			if (PrevLocation.IsSet())
			{
				PDI->DrawLine(*PrevLocation, Location, FLinearColor::White, SDPG_Foreground);
			}

			PrevLocation = Location;
		}
	}
}

// Called by Unreal Editor when it's time to draw the visualization.
void FAGX_WireComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_WireComponentVisualizer_helpers;
	using namespace AGX_WireVisualization_helpers;

	Super::DrawVisualization(Component, View, PDI);

	const UAGX_WireComponent* Wire = Cast<UAGX_WireComponent>(Component);
	if (Wire == nullptr)
	{
		return;
	}

	const bool bSelected = FAGX_EditorUtilities::IsSelected(*Wire);
	const bool bEditing = Wire == GetEditWire();

	if (Wire->HasBeginWinch())
	{
		const FVector WinchLocation = DrawWinch(*Wire, EWireSide::Begin, bSelected, PDI);
		if (Wire->RouteNodes.Num() > 0 && !Wire->IsInitialized())
		{
			// Do not render the implicit begin-winch-to-first-node line because the render iterator
			// does provide that line along with all the other lines. The route nodes does not.
			const FVector WorldLocation = Wire->RouteNodes[0].Frame.GetWorldLocation(*Wire);
			PDI->DrawLine(WinchLocation, WorldLocation, FLinearColor::White, SDPG_Foreground);
		}
	}

	if (Wire->IsInitialized())
	{
		DrawSimulationNodes(*Wire, PDI);
	}
	else
	{
		if (bSelected && bEditing)
		{
			DrawRouteNodes(*Wire, EditNodeIndex, PDI);
		}
		else
		{
			DrawRouteNodes(*Wire, PDI);
		}
	}

	if (Wire->HasEndWinch())
	{
		const FVector& WinchLocation = DrawWinch(*Wire, EWireSide::End, bSelected, PDI);
		if (Wire->RouteNodes.Num() > 0 && !Wire->IsInitialized())
		{
			// Do not render the implicit end-winch-to-first-node line because the render iterator
			// does provide that line along with all the other lines. The route nodes does not.
			const FVector WorldLocation = Wire->RouteNodes.Last().Frame.GetWorldLocation(*Wire);
			PDI->DrawLine(WorldLocation, WinchLocation, FLinearColor::White, SDPG_Foreground);
		}
	}

	// Don't know where to put this. Don't want to miss a deselection if we don't get any more
	// calls to DrawVisualization after the deselection. Is there a delegate for editor selection
	// changes?
	if (HasValidEditNode() || HasValidEditWinch())
	{
		if (!FAGX_EditorUtilities::IsSelected(*GetEditWire()))
		{
			// Do not maintain a node or winch selection if the selected Wire isn't selected
			// anymore. This is so that the transform widget is placed at the newly selected
			// Component instead of at the now no longer selected node or winch.
			ClearEdit();
		}
	}
}

// Called by Unreal Editor when an element with a hit proxy of the visualization is clicked.
bool FAGX_WireComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_WireComponent* Wire = Cast<const UAGX_WireComponent>(VisProxy->Component);
	if (Wire == nullptr)
	{
		// Clicked something not a wire, deselect whatever we had selected before.
		ClearEdit();
		return false;
	}

	const AActor* OldOwningActor = EditWirePropertyPath.GetParentOwningActor();
	const AActor* NewOwningActor = Wire->GetOwner();
	if (NewOwningActor != OldOwningActor)
	{
		// Don't reuse selection data between Actors. It's completely different wires.
		ClearEdit();
	}

	const bool bNewWire = Wire != GetEditWire();

	if (HNodeProxy* Proxy = HitProxyCast<HNodeProxy>(VisProxy))
	{
		return FWireVisualizerOperations::NodeProxyClicked(*this, *Wire, bNewWire, *Proxy);
	}
	if (HWinchLocationProxy* Proxy = HitProxyCast<HWinchLocationProxy>(VisProxy))
	{
		return FWireVisualizerOperations::WinchLocationProxyClicked(*this, *Wire, *Proxy);
	}
	if (HWinchDirectionProxy* Proxy = HitProxyCast<HWinchDirectionProxy>(VisProxy))
	{
		return FWireVisualizerOperations::WinchDirectionProxyClicked(*this, *Wire, *Proxy);
	}

	// Add additional Proxy types here when needed.
	// Or add a virtual function, that could work as well.

	// The give proxy isn't one of ours, return false to pass on to the next handler in line.
	return false;
}

// Called by Unreal Editor to decide where the transform widget should be rendered. We place it on
// the selected edit node, if there is one, or on the selected edit winch handle, if there is one.
bool FAGX_WireComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UAGX_WireComponent* Wire = GetEditWire();
	if (Wire == nullptr)
	{
		return false;
	}
	if (!FAGX_EditorUtilities::IsSelected(*Wire))
	{
		// Is this const-case safe?
		// If not, how do we clear the node selection when the Wire Component becomes unselected?
		const_cast<FAGX_WireComponentVisualizer*>(this)->ClearEdit();
		return false;
	}
	if (HasValidEditNode())
	{
		OutLocation = Wire->RouteNodes[EditNodeIndex].Frame.GetWorldLocation(*Wire);
		return true;
	}
	else if (HasValidEditWinch())
	{
		const FAGX_WireWinch* Winch = Wire->GetWinch(EditWinch);
		checkf(
			Winch != nullptr,
			TEXT("HasValidWinchSelection has been checked but we still didn't get a winch."));

		FAGX_WireWinchPose WinchPose = FAGX_WireUtilities::GetWireWinchPose(*Wire, EditWinch);
		return AGX_WireVisualization_helpers::GetWidgetLocation(
			WinchPose, EditWinchSide, OutLocation);
	}
	return false;
}

bool FAGX_WireComponentVisualizer::GetCustomInputCoordinateSystem(
	const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	// TODO Implement FAGX_WireComponentVisualizer::GetCustomInputCoordinateSystem.
	// See AGX_ShovelComponentVisualizer.cpp for an example.
	return Super::GetCustomInputCoordinateSystem(ViewportClient, OutMatrix);
}

// Called by Unreal Editor when the transform widget is moved, rotated, or scaled.
bool FAGX_WireComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	using namespace AGX_WireVisualization_helpers;
	using namespace AGX_WireComponentVisualizer_helpers;

	UAGX_WireComponent* Wire = GetEditWire();
	if (Wire == nullptr)
	{
		return false;
	}
	if (Wire->HasNative())
	{
		// Currently only allow direct node editing at edit time, i.e., when not having a native.
		return false;
	}

	if (HasValidEditNode())
	{
		FWireVisualizerOperations::NodeProxyDragged(*this, *Wire, *ViewportClient, DeltaTranslate);
	}
	else if (HasValidEditWinch())
	{
		FWireVisualizerOperations::WinchProxyDragged(*this, *Wire, DeltaTranslate, DeltaRotate);
	}
	else
	{
		// We got a move request but we have no valid selection so don't know what to move.
		// Something's wrong, so reset the selection state.
		ClearEdit();
		return false;
	}

	GEditor->RedrawLevelEditingViewports();

	return true;
}

// Called by Unreal Editor when a key is pressed or released while this Visualizer is active.
bool FAGX_WireComponentVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	if (Key == EKeys::LeftMouseButton && Event == IE_Released)
	{
		bIsDuplicatingNode = false;
		// Not returning here. We're just detecting the event, not performing an action.
	}

	// Pass press events on to our Command List.
	if (Event == IE_Pressed)
	{
		return CommandList->ProcessCommandBindings(
			Key, FSlateApplication::Get().GetModifierKeys(), false);
	}

	return false;
}

bool FAGX_WireComponentVisualizer::IsVisualizingArchetype() const
{
	/*
	 * I have no idea what this is or why it's even a thing but for route node editing to work in
	 * the Blueprint Editor it must be here because otherwise
	 * FSCSEditorViewportClient::GetWidgetLocation never calls our GetWidgetLocation.
	 */
	UAGX_WireComponent* Wire = GetEditWire();
	if (Wire == nullptr)
	{
		return false;
	}
	AActor* Owner = Wire->GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}
	return FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
}

// I assume this is called by Unreal Editor, but not sure when, or what we should do here.
void FAGX_WireComponentVisualizer::EndEditing()
{
	ClearEdit();
}

bool FAGX_WireComponentVisualizer::HasValidEditNode() const
{
	return GetEditWire() != nullptr &&
		   // Node selection is currently only for route nodes, i.e. non-initialized wires.
		   !GetEditWire()->IsInitialized() && GetEditWire()->RouteNodes.IsValidIndex(EditNodeIndex);
}

bool FAGX_WireComponentVisualizer::HasValidEditWinch() const
{
	/// \note Should we allow any type of winch or only owned winches? This will also accept Wire
	/// Winch owned winches and freestanding borrowed winches. Can this be handled properly in
	/// HandleInputDelta?
	return GetEditWire() != nullptr && !GetEditWire()->IsInitialized() &&
		   EditWinch != EWireSide::None && EditWinchSide != EWinchSide::None &&
		   GetEditWire()->HasWinch(EditWinch);
}

UAGX_WireComponent* FAGX_WireComponentVisualizer::GetEditWire() const
{
	return Cast<UAGX_WireComponent>(EditWirePropertyPath.GetComponent());
}

int32 FAGX_WireComponentVisualizer::GetEditNodeIndex() const
{
	return EditNodeIndex;
}

void FAGX_WireComponentVisualizer::SetEditNodeIndex(int32 InIndex)
{
	EditNodeIndex = InIndex;
}

void FAGX_WireComponentVisualizer::ClearEdit()
{
	bIsDuplicatingNode = false;
	EditNodeIndex = INDEX_NONE;
	EditWinch = EWireSide::None;
	EditWirePropertyPath.Reset();
}

void FAGX_WireComponentVisualizer::OnDeleteKey()
{
	if (!HasValidEditNode())
	{
		ClearEdit();
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteWireNode", "Delete wire node"));

	UAGX_WireComponent* Wire = GetEditWire();
	Wire->Modify();
	Wire->RouteNodes.RemoveAt(EditNodeIndex);
	Wire->MarkVisualsDirty();
	EditNodeIndex = INDEX_NONE;
	bIsDuplicatingNode = false;

	NotifyPropertyModified(Wire, RouteNodesProperty);

	GEditor->RedrawLevelEditingViewports(true);
}

bool FAGX_WireComponentVisualizer::CanDeleteKey() const
{
	return HasValidEditNode();
}

#undef LOCTEXT_NAMESPACE
