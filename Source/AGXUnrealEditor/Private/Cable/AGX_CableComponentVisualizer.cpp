// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CableComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Cable/AGX_CableComponent.h"
#include "Cable/AGX_CableHitProxies.h"
#include "AGX_RuntimeStyle.h"
#include "Utilities/AGX_SlateUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Selection.h"
#include "SSubobjectEditor.h"

#define LOCTEXT_NAMESPACE "FAGX_CableComponentVisualizer"

namespace AGX_CableComponentVisualizer_helpers
{
	void DrawBodyFixedNodesWithLockedRotation(
		const UAGX_CableComponent& Cable, const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		if (Cable.SegmentLength <= 0.0)
			return;

		UWorld* World = Cable.GetWorld();
		if (World != nullptr && World->IsGameWorld())
			return;

		const static FColor Color = FAGX_SlateUtilities::GetAGXColorOrange();
		for (auto& Node : Cable.RouteNodes)
		{
			if (Node.NodeType == EAGX_CableNodeType::BodyFixed && Node.LockRotationToBody)
			{
				const FTransform WorldTransform = Node.Frame.GetWorldTransform(Cable);
				const FVector Start = WorldTransform.GetLocation();
				const FVector Direction = WorldTransform.GetUnitAxis(EAxis::Z);
				const FVector End = Start + Direction * Cable.SegmentLength;
				PDI->DrawLine(Start, End, Color, SDPG_Foreground, 1.5f);
			}
		}
	}

	constexpr float GetNodeHandleSize()
	{
		return 10.f;
	}
}

class FCableVisualizerOperations
{
public:
	static bool NodeProxyClicked(
		FAGX_CableComponentVisualizer& Visualizer, const UAGX_CableComponent& Cable,
		const bool bNewCable, HAGX_CableNodeProxy& Proxy)
	{
		if (Cable.HasNative())
		{
			// Node editing is currently only for route nodes. All node manipulation operations
			// operate on the route nodes, but when the Cable is initialized what we're seeing are
			// the simulation nodes.
			Visualizer.ClearEdit();
			return false;
		}

		if (!bNewCable && Proxy.NodeIndex == Visualizer.EditNodeIndex)
		{
			// Clicking a selected node deselects it.
			Visualizer.ClearEdit();
			return true;
		}

		// A new node became selected.
		SetEditNode(Visualizer, Cable, Proxy.NodeIndex);
		return true;
	}

	static void SetEditNode(
		FAGX_CableComponentVisualizer& Visualizer, const UAGX_CableComponent& Cable,
		int32 NodeIndex)
	{
		// We must handle editor selection before our Cable edit selection because if another Cable
		// was selected then the editor selection change will clear the Cable Component Visualizer
		// and we want the edit selection we're about to make to persist after the return from this
		// function.

		// Make sure the Cable Component that the node is part of is selected. Deselect any other
		// selected Component.
		if (FActorEditorUtils::IsAPreviewOrInactiveActor(Cable.GetOwner()))
		{
			// We are in a Blueprint Editor. Find the Blueprint Editor instance and select the
			// SCS node for the Cable.
			TSharedPtr<IBlueprintEditor> BlueprintEditor =
				FKismetEditorUtilities::GetIBlueprintEditorForObject(&Cable, /*Open*/ false);
			if (BlueprintEditor.IsValid())
			{
				TArray<TSharedPtr<FSubobjectEditorTreeNode>> Selection =
					BlueprintEditor->GetSelectedSubobjectEditorTreeNodes();
				// Determine if the Cable already is the only selected SCS node. If not select it.
				if (Selection.Num() != 1 || Selection[0]->GetComponentTemplate() == &Cable)
				{
					BlueprintEditor->FindAndSelectSubobjectEditorTreeNode(&Cable, false);
				}
			}
		}
		else
		{
			// We are in the Level Editor. Handle selection directly through the GEditor instance.
			TArray<UActorComponent*> SelectedComponents;
			GEditor->GetSelectedComponents()->GetSelectedObjects(SelectedComponents);
			bool bCableAlreadySelected {false};
			// Unselect everything not the Cable Component.
			for (UActorComponent* Component : SelectedComponents)
			{
				if (Component == &Cable)
				{
					bCableAlreadySelected = true;
				}
				else
				{
					GEditor->SelectComponent(Component, /*Selected*/ false, /*Notify*/ true);
				}
			}
			if (!bCableAlreadySelected)
			{
				GEditor->SelectComponent(const_cast<UAGX_CableComponent*>(&Cable), true, true);
			}
		}

		Visualizer.EditNodeIndex = NodeIndex;
		Visualizer.EditCablePropertyPath = FComponentPropertyPath(&Cable);
	}

	static void NodeProxyDragged(
		FAGX_CableComponentVisualizer& Visualizer, UAGX_CableComponent& Cable,
		FEditorViewportClient& ViewportClient, const FVector& DeltaTranslate,
		const FRotator& DeltaRotate)
	{
		const UE::Widget::EWidgetMode Mode = ViewportClient.GetWidgetMode();

		const bool bWantsTranslate = (Mode == UE::Widget::WM_Translate) && !DeltaTranslate.IsZero();
		const bool bWantsRotate = (Mode == UE::Widget::WM_Rotate) && !DeltaRotate.IsZero();

		if (!bWantsTranslate && !bWantsRotate)
			return;

		Cable.Modify();

		if (ViewportClient.IsAltPressed())
		{
			// A drag with Alt held down means that the current node should be duplicated and the
			// copy selected.

			if (!Visualizer.bIsDuplicatingNode)
			{
				// This is the start of a duplication drag. Create the duplicate and select it.
				DuplicateNode(Visualizer, Cable);
			}
			else
			{
				// This is a continuation of a previously started duplication drag. Move the
				// selected node, i.e., the copy.
				if (bWantsTranslate)
					MoveNode(Visualizer, Cable, DeltaTranslate);

				if (bWantsRotate)
					RotateNode(Visualizer, Cable, DeltaRotate);
			}
		}
		else
		{
			// This is a regular drag, move the selected node.
			if (bWantsTranslate)
				MoveNode(Visualizer, Cable, DeltaTranslate);

			if (bWantsRotate)
				RotateNode(Visualizer, Cable, DeltaRotate);
		}
	}

	static void DuplicateNode(FAGX_CableComponentVisualizer& Visualizer, UAGX_CableComponent& Cable)
	{
		Visualizer.bIsDuplicatingNode = true;
		int32 NewNodeIndex = Visualizer.EditNodeIndex + 1;
		FAGX_CableRouteNode Clone = FAGX_CableRouteNode(Cable.RouteNodes[Visualizer.EditNodeIndex]);
		Cable.RouteNodes.Insert(Clone, NewNodeIndex);
		Visualizer.EditNodeIndex = NewNodeIndex;
		Visualizer.NotifyPropertyModified(&Cable, Visualizer.RouteNodesProperty);
		Cable.MarkVisualsDirty();
	}

	static void MoveNode(
		FAGX_CableComponentVisualizer& Visualizer, UAGX_CableComponent& Cable,
		const FVector& DeltaTranslate)
	{
		FAGX_CableRouteNode& SelectedNode = Cable.RouteNodes[Visualizer.EditNodeIndex];
		const FVector CurrentWorldLocation = SelectedNode.Frame.GetWorldLocation(Cable);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		SelectedNode.Frame.SetWorldLocation(NewWorldLocation, Cable);

		Visualizer.NotifyPropertyModified(&Cable, Visualizer.RouteNodesProperty);
		Cable.MarkVisualsDirty();
	}

	static void RotateNode(
		FAGX_CableComponentVisualizer& Visualizer, UAGX_CableComponent& Cable,
		const FRotator& DeltaRotate)
	{
		FAGX_CableRouteNode& SelectedNode = Cable.RouteNodes[Visualizer.EditNodeIndex];

		const FQuat CurrentWorldRot = SelectedNode.Frame.GetWorldRotation(Cable).Quaternion();
		const FQuat DeltaQ = DeltaRotate.Quaternion();

		// Apply delta in world-space. (Order matters!)
		const FQuat NewWorldRot = DeltaQ * CurrentWorldRot;

		SelectedNode.Frame.SetWorldRotation(NewWorldRot.Rotator(), Cable);

		Visualizer.NotifyPropertyModified(&Cable, Visualizer.RouteNodesProperty);
		Cable.MarkVisualsDirty();
	}
};

/**
 * A collection of commands that can be triggered through the Cable Component Visualizer.
 */
class FAGX_CableComponentVisualizerCommands
	: public TCommands<FAGX_CableComponentVisualizerCommands>
{
public:
	FAGX_CableComponentVisualizerCommands()
		: TCommands<FAGX_CableComponentVisualizerCommands>(
			  "AGX_CableComponentVisualizer",
			  LOCTEXT("AGX_CableComponentVisualizer", "AGX Cable Component Visualizer"), NAME_None,
			  FAGX_RuntimeStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(
			DeleteKey, "Delete Cable node.", "Delete the currently selected Cable node",
			EUserInterfaceActionType::Button, FInputChord(EKeys::Delete));
	}

	TSharedPtr<FUICommandInfo> DeleteKey;
};

FAGX_CableComponentVisualizer::FAGX_CableComponentVisualizer()
{
	FAGX_CableComponentVisualizerCommands::Register();
	UClass* Class = UAGX_CableComponent::StaticClass();
	RouteNodesProperty =
		FindFProperty<FProperty>(Class, GET_MEMBER_NAME_CHECKED(UAGX_CableComponent, RouteNodes));

	CommandList = MakeShareable(new FUICommandList());
}

FAGX_CableComponentVisualizer::~FAGX_CableComponentVisualizer()
{
	FAGX_CableComponentVisualizerCommands::Unregister();
}

void FAGX_CableComponentVisualizer::OnRegister()
{
	const auto& Commands = FAGX_CableComponentVisualizerCommands::Get();

	CommandList->MapAction(
		Commands.DeleteKey,
		FExecuteAction::CreateSP(this, &FAGX_CableComponentVisualizer::OnDeleteKey),
		FCanExecuteAction::CreateSP(this, &FAGX_CableComponentVisualizer::CanDeleteKey));
}

namespace AGX_CableComponentVisualizer_helpers
{
	constexpr uint32 NumNodeColors = (uint32) EAGX_CableNodeType::NumNodeTypes;

	TStaticArray<FLinearColor, NumNodeColors> CreateCableNodeColors()
	{
		TStaticArray<FLinearColor, (uint32) EAGX_CableNodeType::NumNodeTypes> CableNodeColors;
		for (uint32 I = 0; I < NumNodeColors; ++I)
		{
			// Fallback color for any node type not assigned below.
			CableNodeColors[I] = FLinearColor::Gray;
		}
		CableNodeColors[(int) EAGX_CableNodeType::Free] = FLinearColor::Red;
		CableNodeColors[(int) EAGX_CableNodeType::BodyFixed] = FLinearColor::Blue;
		return CableNodeColors;
	}

	FLinearColor CableNodeTypeToColor(EAGX_CableNodeType Type)
	{
		static const TStaticArray<FLinearColor, NumNodeColors> CableNodeColors =
			CreateCableNodeColors();
		const uint32 I = static_cast<uint32>(Type);
		return CableNodeColors[I];
	}

	/**
	 * Draw the route nodes in a Cable, with hit proxies if selected.
	 */
	template <typename FNodeColorFunc>
	void DrawRouteNodes(
		const UAGX_CableComponent& Cable, FPrimitiveDrawInterface* PDI,
		const FLinearColor& LineColor, FNodeColorFunc NodeColorFunc)
	{
		const FTransform& LocalToWorld = Cable.GetComponentTransform();
		const TArray<FAGX_CableRouteNode>& Nodes = Cable.RouteNodes;
		const int32 NumNodes = Nodes.Num();

		FVector PrevLocation;

		for (int32 I = 0; I < NumNodes; ++I)
		{
			const FAGX_CableRouteNode& Node = Nodes[I];
			const FLinearColor NodeColor = NodeColorFunc(I, Node.NodeType);
			const FVector Location = Node.Frame.GetWorldLocation(Cable);

			PDI->SetHitProxy(new HAGX_CableNodeProxy(&Cable, I));
			PDI->DrawPoint(Location, NodeColor, GetNodeHandleSize(), SDPG_Foreground);
			PDI->SetHitProxy(nullptr);

			if (I > 0)
			{
				PDI->DrawLine(PrevLocation, Location, LineColor, SDPG_Foreground);
			}

			PrevLocation = Location;
		}
	}

	/**
	 * Draw the route nodes in a Cable that is not selected.
	 */
	void DrawRouteNodes(const UAGX_CableComponent& Cable, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor LineColor = FLinearColor::White;
		auto NodeColorFunc = [](int32 I, EAGX_CableNodeType NodeType)
		{ return CableNodeTypeToColor(NodeType); };
		DrawRouteNodes(Cable, PDI, LineColor, NodeColorFunc);
	}

	/**
	 * Draw the route nodes in a Cable that is selected.
	 */
	void DrawRouteNodes(
		const UAGX_CableComponent& Cable, int32 SelectedNodeIndex, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor LineColor = GEngine->GetSelectionOutlineColor();
		auto NodeColorFunc = [SelectedNodeIndex](int32 I, EAGX_CableNodeType NodeType)
		{
			return I == SelectedNodeIndex ? GEditor->GetSelectionOutlineColor()
										  : CableNodeTypeToColor(NodeType);
		};
		DrawRouteNodes(Cable, PDI, LineColor, NodeColorFunc);
	}

	/**
	 * Draw the simulation nodes of the given Cable, including lines between them. Hit proxies
	 * are not created when drawing simulation nodes.
	 */
	void DrawSimulationNodes(const UAGX_CableComponent& Cable, FPrimitiveDrawInterface* PDI)
	{
		int32 I = 0;
		TOptional<FVector> PrevLocation;

		for (auto& Node : Cable.GetNodeInfo())
		{
			const EAGX_CableNodeType NodeType = Node.NodeType;
			const FLinearColor Color = CableNodeTypeToColor(NodeType);
			const FVector Location = Node.WorldTransform.GetLocation();

			PDI->DrawPoint(Location, Color, GetNodeHandleSize(), SDPG_Foreground);
			if (PrevLocation.IsSet())
			{
				PDI->DrawLine(*PrevLocation, Location, FLinearColor::White, SDPG_Foreground);
			}

			PrevLocation = Location;
		}
	}
}

void FAGX_CableComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_CableComponentVisualizer_helpers;

	FComponentVisualizer::DrawVisualization(Component, View, PDI);
	const UAGX_CableComponent* Cable = Cast<const UAGX_CableComponent>(Component);
	if (Cable == nullptr || !Cable->ShouldRender())
		return;

	DrawBodyFixedNodesWithLockedRotation(*Cable, View, PDI);

	const bool bSelected = FAGX_EditorUtilities::IsSelected(*Cable);
	const bool bEditing = Cable == GetEditCable();

	if (Cable->HasNative())
	{
		DrawSimulationNodes(*Cable, PDI);
	}
	else
	{
		if (bSelected && bEditing)
			DrawRouteNodes(*Cable, EditNodeIndex, PDI);
		else
			DrawRouteNodes(*Cable, PDI);
	}

	if (HasValidEditNode())
	{
		if (!FAGX_EditorUtilities::IsSelected(*GetEditCable()))
		{
			// Do not maintain a node selection if the selected Cable isn't selected
			// anymore. This is so that the transform widget is placed at the newly selected
			// Component instead of at the now no longer selected node.
			ClearEdit();
		}
	}
}

// Called by Unreal Editor when an element with a hit proxy of the visualization is clicked.
bool FAGX_CableComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_CableComponent* Cable = Cast<const UAGX_CableComponent>(VisProxy->Component);
	if (Cable == nullptr)
	{
		// Clicked something not a Cable, deselect whatever we had selected before.
		ClearEdit();
		return false;
	}

	const AActor* OldOwningActor = EditCablePropertyPath.GetParentOwningActor();
	const AActor* NewOwningActor = Cable->GetOwner();
	if (NewOwningActor != OldOwningActor)
	{
		// Don't reuse selection data between Actors. It's completely different Cables.
		ClearEdit();
	}

	const bool bNewCable = Cable != GetEditCable();

	if (HAGX_CableNodeProxy* Proxy = HitProxyCast<HAGX_CableNodeProxy>(VisProxy))
	{
		return FCableVisualizerOperations::NodeProxyClicked(*this, *Cable, bNewCable, *Proxy);
	}

	// The give proxy isn't one of ours, return false to pass on to the next handler in line.
	return false;
}

// Called by Unreal Editor to decide where the transform widget should be rendered. We place it on
// the selected edit node, if there is one.
bool FAGX_CableComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UAGX_CableComponent* Cable = GetEditCable();
	if (Cable == nullptr)
	{
		return false;
	}
	if (!FAGX_EditorUtilities::IsSelected(*Cable))
	{
		// Is this const-case safe?
		// If not, how do we clear the node selection when the Cable Component becomes unselected?
		const_cast<FAGX_CableComponentVisualizer*>(this)->ClearEdit();
		return false;
	}
	if (HasValidEditNode())
	{
		OutLocation = Cable->RouteNodes[EditNodeIndex].Frame.GetWorldLocation(*Cable);
		return true;
	}

	return false;
}

// Called by Unreal Editor when the transform widget is moved, rotated, or scaled.
bool FAGX_CableComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	using namespace AGX_CableComponentVisualizer_helpers;

	UAGX_CableComponent* Cable = GetEditCable();
	if (Cable == nullptr)
		return false;

	if (Cable->HasNative())
		return false;

	if (HasValidEditNode())
	{
		FCableVisualizerOperations::NodeProxyDragged(
			*this, *Cable, *ViewportClient, DeltaTranslate, DeltaRotate);
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
bool FAGX_CableComponentVisualizer::HandleInputKey(
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

bool FAGX_CableComponentVisualizer::IsVisualizingArchetype() const
{
	/*
	 * I have no idea what this is or why it's even a thing but for route node editing to work in
	 * the Blueprint Editor it must be here because otherwise
	 * FSCSEditorViewportClient::GetWidgetLocation never calls our GetWidgetLocation.
	 */
	UAGX_CableComponent* Cable = GetEditCable();
	if (Cable == nullptr)
	{
		return false;
	}
	AActor* Owner = Cable->GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}
	return FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
}

// I assume this is called by Unreal Editor, but not sure when, or what we should do here.
void FAGX_CableComponentVisualizer::EndEditing()
{
	ClearEdit();
}

bool FAGX_CableComponentVisualizer::HasValidEditNode() const
{
	return GetEditCable() != nullptr &&
		   // Node selection is currently only for route nodes, i.e. Cables without a Native.
		   !GetEditCable()->HasNative() && GetEditCable()->RouteNodes.IsValidIndex(EditNodeIndex);
}

UAGX_CableComponent* FAGX_CableComponentVisualizer::GetEditCable() const
{
	return Cast<UAGX_CableComponent>(EditCablePropertyPath.GetComponent());
}

int32 FAGX_CableComponentVisualizer::GetEditNodeIndex() const
{
	return EditNodeIndex;
}

void FAGX_CableComponentVisualizer::SetEditNodeIndex(int32 InIndex)
{
	EditNodeIndex = InIndex;
}

void FAGX_CableComponentVisualizer::ClearEdit()
{
	bIsDuplicatingNode = false;
	EditNodeIndex = INDEX_NONE;
	EditCablePropertyPath.Reset();
}

void FAGX_CableComponentVisualizer::OnDeleteKey()
{
	if (!HasValidEditNode())
	{
		ClearEdit();
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteCableNode", "Delete Cable node"));

	UAGX_CableComponent* Cable = GetEditCable();
	Cable->Modify();
	Cable->RouteNodes.RemoveAt(EditNodeIndex);
	Cable->MarkVisualsDirty();
	EditNodeIndex = INDEX_NONE;
	bIsDuplicatingNode = false;

	NotifyPropertyModified(Cable, RouteNodesProperty);

	GEditor->RedrawLevelEditingViewports(true);
}

bool FAGX_CableComponentVisualizer::CanDeleteKey() const
{
	return HasValidEditNode();
}

#undef LOCTEXT_NAMESPACE
