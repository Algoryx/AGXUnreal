// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireNodeDetails.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireComponentVisualizer.h"

// Unreal Engine includes.
#if UE_VERSION_OLDER_THAN(5, 0, 0) == false
#include "ActorTreeItem.h"
#endif

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "Misc/Attribute.h"
#include "PropertyCustomizationHelpers.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "ScopedTransaction.h"
#include "UnrealEdGlobals.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Input/SVectorInputBox.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "AGX_WireNodeDetails"

FAGX_WireNodeDetails::FAGX_WireNodeDetails(IDetailLayoutBuilder& InDetailBuilder)
	: DetailBuilder(InDetailBuilder)
{
	// Get the Wire Component Visualizer.
	UClass* WireClass = UAGX_WireComponent::StaticClass();
	FComponentVisualizer* Visualizer = GUnrealEd->FindComponentVisualizer(WireClass).Get();
	WireVisualizer = (FAGX_WireComponentVisualizer*) Visualizer; /// @todo How verify this cast?
	check(WireVisualizer);

	// Create backing storage for the node type combo box.
	UEnum* NodeTypesEnum = StaticEnum<EWireNodeType>();
	check(NodeTypesEnum);
	for (int32 EnumIndex = 0; EnumIndex < (int32) EWireNodeType::NUM_USER_CREATABLE; ++EnumIndex)
	{
		WireNodeTypes.Add(
			MakeShareable(new FString(NodeTypesEnum->GetNameStringByIndex(EnumIndex))));
	}

	// Should always have the empty string entry in the body names list, so that it's possible
	// to select None.
	RigidBodyNames.Add(MakeShareable(new FString("")));

	RouteNodesProperty = FindFProperty<FProperty>(
		UAGX_WireComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes));

	// Build initial state.
	UpdateValues();
}

//~ Begin IDetailCustomNodeBuilder interface

void FAGX_WireNodeDetails::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section for the node
	// details. The Category we're part of will still be collapsable.
}

void FAGX_WireNodeDetails::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	// clang-format off

	// "No Selection" marker widget.
	ChildrenBuilder.AddCustomRow(LOCTEXT("NoSelection", "No Selection"))
	.Visibility(TAttribute<EVisibility>(this, &FAGX_WireNodeDetails::WithoutSelection))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("NoNodeSelection", "No node is selected"))
	];

	// Button to select the first node, which is sometimes difficult because of overlap with the
	// Wire's on-screen widget and icon. May want a Select Next/Prev Node as well.
	ChildrenBuilder.AddCustomRow(LOCTEXT("SelectFirstNode", "Select first node"))
	.Visibility(TAttribute<EVisibility>(this, &FAGX_WireNodeDetails::WithoutSelection))
	.WholeRowContent()
	[
		SNew(SButton)
		.OnClicked(this, &FAGX_WireNodeDetails::OnClickedSelectFirstNode)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SelectFirstNode", "Select First Node"))
		]
	];

	// "Selected Node" title widget.
	ChildrenBuilder.AddCustomRow(LOCTEXT("Title", "Title"))
	.Visibility(TAttribute<EVisibility>(this, &FAGX_WireNodeDetails::WithSelection))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(this, &FAGX_WireNodeDetails::OnGetSelectedNodeIndexText)
	];

	// Location widget.
	ChildrenBuilder.AddCustomRow(LOCTEXT("Location", "Location"))
	.Visibility(TAttribute<EVisibility>(this, &FAGX_WireNodeDetails::WithSelection))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Location", "Location"))
	]
	.ValueContent()
	[
		SNew(SVectorInputBox)
		.X(this, &FAGX_WireNodeDetails::OnGetLocationX)
		.Y(this, &FAGX_WireNodeDetails::OnGetLocationY)
		.Z(this, &FAGX_WireNodeDetails::OnGetLocationZ)
#if UE_VERSION_OLDER_THAN(5, 0, 0)
		.AllowResponsiveLayout(true)
#endif
		.AllowSpin(false)
		.OnXCommitted(this, &FAGX_WireNodeDetails::OnSetLocation, 0)
		.OnYCommitted(this, &FAGX_WireNodeDetails::OnSetLocation, 1)
		.OnZCommitted(this, &FAGX_WireNodeDetails::OnSetLocation, 2)
	];

	// Node type widget.
	ChildrenBuilder.AddCustomRow(LOCTEXT("Type", "Type"))
	.Visibility(TAttribute<EVisibility>(this, &FAGX_WireNodeDetails::WithSelection))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("Type", "Type"))
	]
		.ValueContent()
	[
		SAssignNew(NodeTypeComboBox, SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&WireNodeTypes)
		.OnGenerateWidget(this, &FAGX_WireNodeDetails::OnGetNodeTypeEntryWidget)
		.OnSelectionChanged(this, &FAGX_WireNodeDetails::OnSetNodeType)
		[
			SNew(STextBlock)
			.Text(this, &FAGX_WireNodeDetails::OnGetNodeTypeLabel)
		]
	];

	// Group to hold the two Rigid Body widgets.
	IDetailGroup& RigidBody = ChildrenBuilder.AddGroup(
		TEXT("RigidBodyTitle"), LOCTEXT("RigidBodyTitle", "RigidBody"));

	// Rigid Body owning Actor widget.
	RigidBody.AddWidgetRow()
	.Visibility(TAttribute<EVisibility>(this, &FAGX_WireNodeDetails::NodeHasRigidBody))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("OwningActor", "Owning Actor"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(this, &FAGX_WireNodeDetails::OnGetRigidBodyOwnerLabel)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(SBox)
			.MaxDesiredWidth(40)
			.MaxDesiredHeight(20)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					// Creates a button that opens a menu with a list of actors in the level.
					PropertyCustomizationHelpers::MakeActorPickerAnchorButton(
						FOnGetActorFilters::CreateSP(this, &FAGX_WireNodeDetails::OnGetActorFilters),
						FOnActorSelected::CreateSP(this, &FAGX_WireNodeDetails::OnSetRigidBodyOwner))
				]
				+ SHorizontalBox::Slot()
				[
					// Creates a button that enables the Actor Picker mode.
					PropertyCustomizationHelpers::MakeInteractiveActorPicker(
						FOnGetAllowedClasses::CreateSP(this, &FAGX_WireNodeDetails::OnGetAllowedClasses),
						FOnShouldFilterActor::CreateSP(this, &FAGX_WireNodeDetails::OnGetHasRigidBody),
						FOnActorSelected::CreateSP(this, &FAGX_WireNodeDetails::OnSetRigidBodyOwner))
				]
			]
		]
	];

	// Rigid Body name widget.
	RigidBody.AddWidgetRow()
	.Visibility(TAttribute<EVisibility>(this, &FAGX_WireNodeDetails::NodeHasRigidBody))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("RigidBodyName", "Rigid Body Name"))
	]
	.ValueContent()
	[
		SAssignNew(BodyNameComboBox, SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&RigidBodyNames)
		.OnGenerateWidget(this, &FAGX_WireNodeDetails::OnGetRigidBodyEntryWidget)
		.OnSelectionChanged(this, &FAGX_WireNodeDetails::OnSetRigidBody)
		[
			SNew(STextBlock)
			.Text(this, &FAGX_WireNodeDetails::OnGetRigidBodyLabel)
			.ColorAndOpacity(this, &FAGX_WireNodeDetails::OnGetRigidBodyNameColor)
		]
	];

	// clang-format on
}

bool FAGX_WireNodeDetails::InitiallyCollapsed() const
{
	return false;
}

void FAGX_WireNodeDetails::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
	OnRegenerateChildren = InOnRegenerateChildren;
}

FName FAGX_WireNodeDetails::GetName() const
{
	return TEXT("Wire Node Details");
}

bool FAGX_WireNodeDetails::RequiresTick() const
{
	return true;
}

void FAGX_WireNodeDetails::Tick(float DeltaTime)
{
	UpdateValues();
}

//~ End IDetailCustomNodeBuilder interface

namespace AGX_WireNodeDetails_helpers
{
	bool NodeTypeHasBody(EWireNodeType NodeType)
	{
		return NodeType == EWireNodeType::BodyFixed || NodeType == EWireNodeType::Eye;
	}

	/**
	 * @return The index where the given body name is in the given array. INDEX_NONE if not there.
	 */
	int32 FindRigidBodyName(
		const FString& RigidBodyName, const TArray<TSharedPtr<FString>>& RigidBodyNames)
	{
		const TSharedPtr<FString>* Element =
			RigidBodyNames.FindByPredicate([RigidBodyName](const TSharedPtr<FString>& Element)
										   { return *Element == RigidBodyName; });
		if (Element == nullptr)
		{
			return INDEX_NONE;
		}

		return Element - &RigidBodyNames[0];
	}

	bool ContainsRigidBody(const AActor* Actor)
	{
		if (Actor == nullptr)
		{
			return false;
		}

		return Actor->FindComponentByClass<UAGX_RigidBodyComponent>() != nullptr;
	}
}

// Begin selection getters.

FText FAGX_WireNodeDetails::OnGetSelectedNodeIndexText() const
{
	return SelectedNodeIndexText;
}

// Begin selection click callbacks.

FReply FAGX_WireNodeDetails::OnClickedSelectFirstNode()
{
	// While this does set the selected node index, if there is no active node selection then there
	// is no selected wire either so setting a selected index doesn't really do anything. We would
	// like to find the currently selected Wire Component in the selected Actor's Component list,
	// the one above the Details Panel or in the My Components Panel for the Blueprint Editor, but I
	// don't yet know how to do that.
	WireVisualizer->SetEditNodeIndex(0);
	UpdateValues();
	return FReply::Handled();
}

// Begin Location getters.

TOptional<float> FAGX_WireNodeDetails::OnGetLocationX() const
{
	return LocationX;
}

TOptional<float> FAGX_WireNodeDetails::OnGetLocationY() const
{
	return LocationY;
}

TOptional<float> FAGX_WireNodeDetails::OnGetLocationZ() const
{
	return LocationZ;
}

// Begin location setters.

namespace AGX_WireNodeDetails_helpers
{
	struct FSelection
	{
		UAGX_WireComponent* Wire;
		int32 NodeIndex;
		bool IsValid()
		{
			return Wire != nullptr;
		}
	};

	FSelection GetSelection(const FAGX_WireNodeDetails& Details)
	{
		UAGX_WireComponent* Wire = Details.GetWire();
		if (Wire == nullptr)
		{
			return {nullptr, INDEX_NONE};
		}
		int32 Index = Details.GetNodeIndex();
		if (!Wire->RouteNodes.IsValidIndex(Index))
		{
			return {nullptr, INDEX_NONE};
		}
		return {Wire, Index};
	}

	bool GetSelection(
		const FAGX_WireNodeDetails& Details, UAGX_WireComponent*& OutWire, int32& OutNodeIndex)
	{
		FSelection Selection = GetSelection(Details);
		OutWire = Selection.Wire;
		OutNodeIndex = Selection.NodeIndex;
		return Selection.IsValid();
	}

	bool ValidateSelection(const FAGX_WireNodeDetails& Details)
	{
		FSelection Selection = GetSelection(Details);
		return Selection.Wire == Details.GetPreviousWire() &&
			   Selection.NodeIndex == Details.GetPreviousNodeIndex();
	}

	/**
	 * Get the currently selected wire and node index, but only if they both are the same as during
	 * the last call to UpdateValues.
	 */
	bool GetValidatedSelection(
		const FAGX_WireNodeDetails& Details, UAGX_WireComponent*& OutWire, int32& OutNodeIndex,
		const TCHAR*& OutError)
	{
		OutWire = nullptr;
		OutNodeIndex = INDEX_NONE;
		OutError = TEXT("");

		FSelection Selection = GetSelection(Details);
		if (Selection.Wire != Details.GetPreviousWire())
		{
			OutError = TEXT("The selected wire has changed.");
			return false;
		}
		if (Selection.NodeIndex != Details.GetPreviousNodeIndex())
		{
			OutError = TEXT("The selected node has changed.");
			return false;
		}
		if (!Selection.IsValid())
		{
			OutError = TEXT("No selection.");
			return false;
		}
		OutWire = Selection.Wire;
		OutNodeIndex = Selection.NodeIndex;
		return true;
	}

	bool HasSelection(const FAGX_WireNodeDetails& Details)
	{
		return GetSelection(Details).IsValid();
	}
}

void FAGX_WireNodeDetails::OnSetLocation(float NewValue, ETextCommit::Type CommitInfo, int32 Axis)
{
	if (bIsRunningCallback)
	{
		return;
	}
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	using namespace AGX_WireNodeDetails_helpers;
	UAGX_WireComponent* Wire;
	int32 NodeIndex;
	const TCHAR* Error;
	bool bValidSelection = GetValidatedSelection(*this, Wire, NodeIndex, Error);
	if (!bValidSelection)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Wire node location edited without a valid selection: '%s'"),
			Error);
		UE_LOG(LogAGX, Warning, TEXT("  The new value is discarded."));
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("SetWireNodeLocation", "Set wire node location"));
	Wire->Modify();
	Wire->RouteNodes[NodeIndex].Frame.LocalLocation.Component(Axis) = NewValue;
	FComponentVisualizer::NotifyPropertyModified(Wire, RouteNodesProperty);
	UpdateValues();
}

// Begin node type getters.

/// @todo Hacky import from AGX_WireComponentVisualizer.cpp. Decide where to put this function.
namespace AGX_WireComponentVisualizer_helpers
{
	FLinearColor WireNodeTypeToColor(EWireNodeType Type);
}

namespace AGX_WireNodeDetails_helpers
{
	FLinearColor WireNodeTypeIndexToColor(int32 Type)
	{
		using namespace AGX_WireComponentVisualizer_helpers;
		return WireNodeTypeToColor(static_cast<EWireNodeType>(Type));
	}
}

TSharedRef<SWidget> FAGX_WireNodeDetails::OnGetNodeTypeEntryWidget(
	TSharedPtr<FString> InComboString)
{
	using namespace AGX_WireNodeDetails_helpers;
	const int32 EnumIndex = WireNodeTypes.Find(InComboString);
	const FLinearColor Color = WireNodeTypeIndexToColor(EnumIndex);
	// clang-format off
	return SNew(SHorizontalBox)
		   + SHorizontalBox::Slot()
		   [
			   SNew(STextBlock)
			   .Text(FText::FromString(*InComboString))
		   ]
		   + SHorizontalBox::Slot()
		   [
			   /// @todo Put a SBox here to reduce the width of the Color Block. Try to make it square.
			   SNew(SColorBlock)
			   .Color(Color)
		   ];
	// clang-format on
}

FText FAGX_WireNodeDetails::OnGetNodeTypeLabel() const
{
	return NodeTypeText;
}

// Begin node type setters.

void FAGX_WireNodeDetails::OnSetNodeType(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (bIsRunningCallback)
	{
		return;
	}
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	using namespace AGX_WireNodeDetails_helpers;
	UAGX_WireComponent* Wire;
	int32 NodeIndex;
	const TCHAR* Error;
	bool bValidSelection = GetValidatedSelection(*this, Wire, NodeIndex, Error);
	if (!bValidSelection)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Wire node type edited without a valid selection: '%s'"), Error);
		UE_LOG(LogAGX, Warning, TEXT("  The new value is discarded."));
		return;
	}

	FWireRoutingNode& Node = Wire->RouteNodes[NodeIndex];

	const int32 EnumIndex = WireNodeTypes.Find(NewValue);
	const EWireNodeType NewNodeType = static_cast<EWireNodeType>(EnumIndex);

	const bool OldHadBody = NodeTypeHasBody(Node.NodeType);
	const bool NewHasBody = NodeTypeHasBody(NewNodeType);
	FString NewBodyName;
	if (!OldHadBody && NewHasBody)
	{
		// The RigidBody selector is about to be shown. Prepare it's backing storage.
		NewBodyName = RebuildRigidBodyComboBox_Edit(
			Node.RigidBody.Name.ToString(), Node.RigidBody.GetScope());
	}

	const FScopedTransaction Transaction(LOCTEXT("SetWireNodeType", "Set wire node type"));
	Wire->Modify();
	Node.NodeType = NewNodeType;
	Node.RigidBody.Name = FName(*NewBodyName);
	FComponentVisualizer::NotifyPropertyModified(Wire, RouteNodesProperty);
	UpdateValues();
}

// Begin rigid body getters.

FText FAGX_WireNodeDetails::OnGetRigidBodyLabel() const
{
	return RigidBodyNameText;
}

FSlateColor FAGX_WireNodeDetails::OnGetRigidBodyNameColor() const
{
	if (BodyNameComboBox->GetSelectedItem().IsValid() &&
		BodyNameComboBox->GetSelectedItem() == RigidBodyNames[0])
	{
		// The first element of RigidBodyNames is the "unknown" marker. It is selected when the
		// underlying node doesn't have a Rigid Body name, and when the name it does have doesn't
		// correspond to any Rigid Body in the Rigid Body owning Actor.
		return FSlateColor(FLinearColor::Red);
	}

	return FSlateColor::UseForeground();
}

FText FAGX_WireNodeDetails::OnGetRigidBodyOwnerLabel() const
{
	return RigidBodyOwnerLabelText;
}

bool FAGX_WireNodeDetails::OnGetHasRigidBody(const AActor* Actor)
{
	const UActorComponent* const* It = Actor->GetInstanceComponents().FindByPredicate(
		[](const UActorComponent* const C) { return C->IsA<UAGX_RigidBodyComponent>(); });

	return It != nullptr;
}

void FAGX_WireNodeDetails::OnGetAllowedClasses(TArray<const UClass*>& AllowedClasses)
{
	AllowedClasses.Add(AActor::StaticClass());
}

#if UE_VERSION_OLDER_THAN(5, 0, 0)
void FAGX_WireNodeDetails::OnGetActorFilters(TSharedPtr<SceneOutliner::FOutlinerFilters>& Filters)
{
	Filters->AddFilterPredicate(SceneOutliner::FActorFilterPredicate::CreateStatic(
		AGX_WireNodeDetails_helpers::ContainsRigidBody));
}
#else
void FAGX_WireNodeDetails::OnGetActorFilters(TSharedPtr<FSceneOutlinerFilters>& OutFilters)
{
	using FActorFilter = TSceneOutlinerPredicateFilter<FActorTreeItem>;
	TSharedPtr<FActorFilter> ActorFilter = MakeShared<FActorFilter>(
		FActorTreeItem::FFilterPredicate::CreateStatic(
			AGX_WireNodeDetails_helpers::ContainsRigidBody),
		FSceneOutlinerFilter::EDefaultBehaviour::Fail);

	OutFilters->Add(ActorFilter);
}
#endif

TSharedRef<SWidget> FAGX_WireNodeDetails::OnGetRigidBodyEntryWidget(
	TSharedPtr<FString> InComboString)
{
	return SNew(STextBlock).Text(FText::FromString(*InComboString));
}

// Begin rigid body setters.

void FAGX_WireNodeDetails::OnSetRigidBody(
	TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (bIsRunningCallback)
	{
		return;
	}
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	using namespace AGX_WireNodeDetails_helpers;
	UAGX_WireComponent* Wire;
	int32 NodeIndex;
	const TCHAR* Error;
	bool bValidSelection = GetValidatedSelection(*this, Wire, NodeIndex, Error);
	if (!bValidSelection)
	{
		UE_LOG(LogAGX, Warning, TEXT("Wire node body edited without a valid selection: %s"), Error);
		UE_LOG(LogAGX, Warning, TEXT("  The new value is discarded."));
		return;
	}

	FWireRoutingNode& Node = Wire->RouteNodes[NodeIndex];

	FName NewName = NewValue.IsValid() ? FName(*NewValue) : NAME_None;
	if (NewName == Node.RigidBody.Name)
	{
		return;
	}

	const FScopedTransaction Transaction(
		LOCTEXT("SetWireNodeRigidBodyName", "Set Wire Node Rigid Body Name"));
	Wire->Modify();
	Node.RigidBody.Name = NewName;
	FComponentVisualizer::NotifyPropertyModified(Wire, RouteNodesProperty);
	UpdateValues();
}

void FAGX_WireNodeDetails::OnSetRigidBodyOwner(AActor* Actor)
{
	if (bIsRunningCallback)
	{
		return;
	}
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	using namespace AGX_WireNodeDetails_helpers;
	UAGX_WireComponent* Wire;
	int32 NodeIndex;
	const TCHAR* Error;
	const bool bValidSelection = GetValidatedSelection(*this, Wire, NodeIndex, Error);
	if (!bValidSelection)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Wire node body owner editeod without a valid selection: %s"),
			Error);
		UE_LOG(LogAGX, Warning, TEXT("  The new value is discarded."));
		return;
	}

	FWireRoutingNode& Node = Wire->RouteNodes[NodeIndex];

	FName NewBodyName =
		FName(*RebuildRigidBodyComboBox_Edit(Node.RigidBody.Name.ToString(), Actor));

	if (Actor == Node.RigidBody.OwningActor && NewBodyName == Node.RigidBody.Name)
	{
		return;
	}

	const FScopedTransaction Transaction(
		LOCTEXT("SetWireNodeBodyActor", "Set wire node body actor"));
	Wire->Modify();
	Node.RigidBody.OwningActor = Actor;
	Node.RigidBody.Name = NewBodyName;
	FComponentVisualizer::NotifyPropertyModified(Wire, RouteNodesProperty);
	UpdateValues();
}

void FAGX_WireNodeDetails::ClearStorage()
{
	using namespace AGX_WireNodeDetails_helpers;
	static const FText NoSelection = LOCTEXT("NoNodeSelected", "No node selected.");
	SelectedNodeIndexText = NoSelection;
	LocationX.Reset();
	LocationY.Reset();
	LocationZ.Reset();
	NodeType.Reset();
	NodeTypeText = NoSelection;
	RebuildRigidBodyComboBox_View(TEXT(""), nullptr);
	RigidBodyNameText = NoSelection;
	RigidBodyOwnerLabelText = NoSelection;
}

UAGX_WireComponent* FAGX_WireNodeDetails::GetWire() const
{
	UAGX_WireComponent* BuilderWire = [this]() -> UAGX_WireComponent*
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);
		if (Objects.Num() != 1)
		{
			return nullptr;
		}

		TWeakObjectPtr<UObject> Object = Objects[0];
		if (!Object.IsValid())
		{
			return nullptr;
		}

		return Cast<UAGX_WireComponent>(Object.Get());
	}();

	UAGX_WireComponent* VisualizerWire = [this]() -> UAGX_WireComponent*
	{
		if (WireVisualizer == nullptr)
		{
			return nullptr;
		}
		return WireVisualizer->GetEditWire();
	}();

	/// @todo I though I could avoid showing invalid data by requiring that the Details Panel we're
	/// part of and the Wire Visualizer agree on what Wire is being shown, but as usual the
	/// Blueprint Editor is weird and gives us two different Wires. One named
	/// 'MyVariable_GEN_VARIABLE` and one named 'MyVariable'. Don't know how to determine if the two
	/// are related / part of the same Blueprint Editor so for now I pretend that they are the same
	/// by setting BuilderWire to the VisualizerWire. This mirrors what the engine code does for
	/// Spline. The drawback is that in a Details Panel some data will be from one Wire and other
	/// data will be from another Wire. Risk for confusion.
	BuilderWire = VisualizerWire;

	if (BuilderWire != VisualizerWire)
	{
		UE_LOG(LogAGX, Warning, TEXT("Mismatch between Detail Builder and Wire Visualizer:"));
		if (BuilderWire != nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("  DetailBuilder: '%s' in '%s'."), *BuilderWire->GetName(),
				*GetNameSafe(BuilderWire->GetOwner()));
		}
		else
		{
			UE_LOG(LogAGX, Warning, TEXT("  DetailBuilder: (none)"));
		}
		if (VisualizerWire != nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("  Wire Visualizer: '%s' in '%s'."),
				*VisualizerWire->GetName(), *GetNameSafe(VisualizerWire->GetOwner()));
		}
		else
		{
			UE_LOG(LogAGX, Warning, TEXT("  Wire Visualizer: (none)"));
		}
		return nullptr;
	}

	return BuilderWire; // Equal to VisualizerWire.
}

int32 FAGX_WireNodeDetails::GetNodeIndex() const
{
	if (WireVisualizer == nullptr)
	{
		return INDEX_NONE;
	}
	return WireVisualizer->GetEditNodeIndex();
}

UAGX_WireComponent* FAGX_WireNodeDetails::GetPreviousWire() const
{
	return PreviousWire;
}

int32 FAGX_WireNodeDetails::GetPreviousNodeIndex() const
{
	return PreviousNodeIndex;
}

void FAGX_WireNodeDetails::UpdateValues()
{
	using namespace AGX_WireNodeDetails_helpers;

	// Sometimes Tick is called before GenerateChildContent, meaning that Unreal Editor is trying
	// to update an empty FWireNodeDetails. We detect this by checking for the presence of the
	// node type Combo Box.
	if (!NodeTypeComboBox.IsValid())
	{
		return;
	}

	UAGX_WireComponent* Wire;
	int32 NodeIndex;
	bool bValidSelection = GetSelection(*this, Wire, NodeIndex);

	// Detect if the selection has changed, which will necessitate some state rebuilding.
	const bool bSelectionChanged = Wire != PreviousWire || NodeIndex != PreviousNodeIndex;
	PreviousWire = Wire;
	PreviousNodeIndex = NodeIndex;

	/// @todo I moved this from after the bValidSelection if-statement because the rigid body combo
	/// box rebuild was triggered every frame pretty much always.
	/// The original comment follows:
	///
	/// @todo Should this be moved above the !bValidSelection if-statement? Its block calls
	/// RebuildRigidBodyComboBox_View which in turn calls SetSelectedItem on the body name combo
	/// box.
	TGuardValue<bool> GuardIsRunningCallback(bIsRunningCallback, true);

	// Reset and bail if we don't have a valid selection.
	if (!bValidSelection)
	{
		ClearStorage();
		return;
	}

	SelectedNodeIndexText = FText::Format(
		LOCTEXT("NodeIndexText", "Properties of node {0}."), FText::AsNumber(NodeIndex));

	const FWireRoutingNode& Node = Wire->RouteNodes[NodeIndex];

	// Read node location.
	LocationX = Node.Frame.LocalLocation.X;
	LocationY = Node.Frame.LocalLocation.Y;
	LocationZ = Node.Frame.LocalLocation.Z;

	// Read node type.
	NodeType = Node.NodeType;
	const int32 EnumIndex = static_cast<int32>(NodeType.GetValue());
	NodeTypeText = FText::FromString(*WireNodeTypes[EnumIndex]);
	if (NodeTypeComboBox->GetSelectedItem() != WireNodeTypes[EnumIndex])
	{
		// Setting the selected item while the combo box is open causes it to close again. Setting
		// it on every update makes it impossible to select anything since it closes immediately.
		// So only change if we really have to.
		NodeTypeComboBox->SetSelectedItem(WireNodeTypes[EnumIndex]);
	}

	if (bSelectionChanged)
	{
		RebuildRigidBodyComboBox_View(Node.RigidBody.Name.ToString(), Node.RigidBody.GetScope());
	}

	RigidBodyNameText = FText::FromName(Node.RigidBody.Name);
	RigidBodyOwnerLabelText = FText::FromString(GetLabelSafe(Node.RigidBody.GetScope()));
}

EVisibility FAGX_WireNodeDetails::WithSelection() const
{
	return FAGX_EditorUtilities::VisibleIf(PreviousWire != nullptr);
}

EVisibility FAGX_WireNodeDetails::WithoutSelection() const
{
	return FAGX_EditorUtilities::VisibleIf(PreviousWire == nullptr);
}

EVisibility FAGX_WireNodeDetails::NodeHasRigidBody() const
{
	using namespace AGX_WireNodeDetails_helpers;
	return FAGX_EditorUtilities::VisibleIf(
		NodeType.IsSet() && NodeTypeHasBody(NodeType.GetValue()));
}

FString FAGX_WireNodeDetails::RebuildRigidBodyComboBox_Edit(const FString& ToSelect, AActor* Actor)
{
	BodyNameComboBox->ClearSelection();
	RebuildRigidBodyNames(Actor);
	BodyNameComboBox->RefreshOptions();

	int32 Index = AGX_WireNodeDetails_helpers::FindRigidBodyName(ToSelect, RigidBodyNames);
	if (Index == INDEX_NONE)
	{
		// The rigid body name we had selected doesn't exist anymore. Select either the empty
		// string, at index 0, or the first rigid body name, at index 1.
		// RebuildRigidBodyNames guarantees that Num() - 1 will never be negative.
		Index = FMath::Min(RigidBodyNames.Num() - 1, 1);
	}
	BodyNameComboBox->SetSelectedItem(RigidBodyNames[Index]);
	return *RigidBodyNames[Index];
}

void FAGX_WireNodeDetails::RebuildRigidBodyComboBox_View(const FString& ToSelect, AActor* Actor)
{
	BodyNameComboBox->ClearSelection();
	RebuildRigidBodyNames(Actor);
	BodyNameComboBox->RefreshOptions();

	int32 Index = AGX_WireNodeDetails_helpers::FindRigidBodyName(ToSelect, RigidBodyNames);
	if (Index == INDEX_NONE)
	{
		// The rigid body name we had selected doesn't exist anymore. We're in a context where
		// we can't edit the underlying node so select the empty string entry to ensure that
		// the actual body name entries are selectable.
		Index = 0;
	}
	BodyNameComboBox->SetSelectedItem(RigidBodyNames[Index]);
}

void FAGX_WireNodeDetails::RebuildRigidBodyNames(AActor* Actor)
{
	if (Actor == nullptr)
	{
		RebuildRigidBodyNamesFromBlueprint();
		return;
	}

	TArray<UAGX_RigidBodyComponent*> RigidBodyComponents;
	Actor->GetComponents(RigidBodyComponents);
	RigidBodyNames.Reset(RigidBodyComponents.Num() + 1);
	RigidBodyNames.Add(MakeShareable(new FString("")));
	for (const auto& Body : RigidBodyComponents)
	{
		RigidBodyNames.Add(MakeShareable(new FString(Body->GetName())));
	}
}

void FAGX_WireNodeDetails::RebuildRigidBodyNamesFromBlueprint()
{
	RigidBodyNames.Reset(1);
	RigidBodyNames.Add(MakeShareable(new FString("")));

	UAGX_WireComponent* Wire = [this]() -> UAGX_WireComponent*
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);
		if (Objects.Num() != 1)
		{
			return nullptr;
		}

		TWeakObjectPtr<UObject> Object = Objects[0];
		if (!Object.IsValid())
		{
			return nullptr;
		}

		return Cast<UAGX_WireComponent>(Object.Get());
	}();

	if (Wire == nullptr)
	{
		return;
	}

	UBlueprintGeneratedClass* Blueprint = Cast<UBlueprintGeneratedClass>(Wire->GetOuter());
	if (Blueprint == nullptr)
	{
		return;
	}

	// We need to iterate over all parent Blueprints as well since the SCS Nodes of interest may
	// reside in a parent Blueprint, and will in that case not show up in a child Blueprint.
	TArray<UBlueprint*> BlueprintChain;
	UBlueprint::GetBlueprintHierarchyFromClass(Blueprint, BlueprintChain);

	for (UBlueprint* BP : BlueprintChain)
	{
		for (USCS_Node* Node : BP->SimpleConstructionScript->GetAllNodes())
		{
			if (UAGX_RigidBodyComponent* RigidBody =
					Cast<UAGX_RigidBodyComponent>(Node->ComponentTemplate))
			{
				const FString Name = [RigidBody, BP]()
				{
					FString N = RigidBody->GetName();
					N.RemoveFromEnd(BP->SimpleConstructionScript->ComponentTemplateNameSuffix);
					return N;
				}();

				RigidBodyNames.Add(MakeShareable(new FString(Name)));
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
