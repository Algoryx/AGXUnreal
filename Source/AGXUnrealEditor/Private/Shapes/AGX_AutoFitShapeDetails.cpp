// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/AGX_AutoFitShapeDetails.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/SCS_Node.h"
#include "Engine/StaticMesh.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_AutoFitShapeDetails"

namespace AGX_AutoFitShapeDetals_helpers
{
	UStaticMeshComponent* GetParentMeshComponent(
		UAGX_ShapeComponent* Component, UBlueprintGeneratedClass* BPGC)
	{
		if (BPGC == nullptr || Component == nullptr)
			return nullptr;

		UBlueprint* Blueprint = BPGC->SimpleConstructionScript->GetBlueprint();
		if (Blueprint == nullptr)
			return nullptr;

		const FString ComponentName =
			FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(Component->GetName());
		FAGX_BlueprintUtilities::FAGX_BlueprintNodeSearchResult NodeResult =
			FAGX_BlueprintUtilities::GetSCSNodeFromName(*Blueprint, ComponentName, true);

		USCS_Node* Node = NodeResult.FoundNode;
		if (Node == nullptr)
			return nullptr;

		USCS_Node* NextParent = FAGX_BlueprintUtilities::GetParentSCSNode(Node, true);
		if (NextParent == nullptr)
			return nullptr;

		// Find the first Static Mesh Component parent.
		while (NextParent != nullptr)
		{
			if (UStaticMeshComponent* S = Cast<UStaticMeshComponent>(NextParent->ComponentTemplate))
			{
				// The ComponentTemplate might reside in a parent Blueprint of the Blueprint
				// holding the Component. Therefore, we need to go down the archetype instance chain
				// and find the matching archetype instance such that it resides in the same
				// Blueprint as Component.
				UStaticMeshComponent* MatchedMesh =
					FAGX_ObjectUtilities::GetMatchedInstance(S, Component->GetOuter());
				if (MatchedMesh != nullptr)
					return MatchedMesh;
			}

			NextParent = FAGX_BlueprintUtilities::GetParentSCSNode(NextParent, true);
		}

		return nullptr;
	}

	void GetChildNodesRecursive(USCS_Node* Node, TArray<USCS_Node*>& OutChildren)
	{
		if (Node == nullptr)
		{
			return;
		}

		for (USCS_Node* N : Node->GetChildNodes())
		{
			OutChildren.Add(N);
			GetChildNodesRecursive(N, OutChildren);
		}
	}

	TArray<USCS_Node*> GetChildNodes(USCS_Node* Node, bool Recursive)
	{
		TArray<USCS_Node*> Children;
		if (Node == nullptr)
		{
			return Children;
		}

		if (Recursive)
		{
			GetChildNodesRecursive(Node, Children);
			return Children;
		}
		else
		{
			return Node->GetChildNodes();
		}
	}

	TArray<UStaticMeshComponent*> GetChildrenMeshComponents(
		UAGX_ShapeComponent* Component, UBlueprintGeneratedClass* Blueprint, bool Recursive)
	{
		TArray<UStaticMeshComponent*> Children;
		if (Blueprint == nullptr || Component == nullptr)
		{
			return Children;
		}

		const FString ComponentName =
			FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(Component->GetName());
		USCS_Node* ComponentNode =
			FAGX_BlueprintUtilities::GetSCSNodeFromName(
				*Blueprint->SimpleConstructionScript->GetBlueprint(), ComponentName, true)
				.FoundNode;
		if (ComponentNode == nullptr)
		{
			return Children;
		}

		TArray<USCS_Node*> ChildNodes = GetChildNodes(ComponentNode, Recursive);
		for (USCS_Node* Child : ChildNodes)
		{
			if (Child == nullptr)
			{
				continue;
			}

			if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Child->ComponentTemplate))
			{
				// The ComponentTemaplte might reside in a parent Blueprint of the Blueprint holding
				// the Component. Therefore, we need to go down the archetype instance chain and
				// find the matching arhctetype instance such that it resides in the same Blueprint
				// as Component.
				auto MatchedMesh =
					FAGX_ObjectUtilities::GetMatchedInstance(Mesh, Component->GetOuter());
				if (MatchedMesh != nullptr)
					Children.Add(MatchedMesh);
			}
		}

		return Children;
	}

	bool AutoFitBox(
		UAGX_BoxShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const FVector OrigHalfExtent = Component->GetHalfExtent();

		Component->Modify();
		if (!Component->AutoFit(Meshes, Component->GetWorld(), Component->GetName()))
		{
			// Logging done in AutoFit.
			return false;
		}

		// The following is a little ugly, but necessary. Component->AutoFit will set a new world
		// transform on the Component, but world transform is not handled correctly by template
		// components inside a Blueprint. In actuality, SetWorldTransform will behave as if the
		// component is in world origin, i.e. the same way as SetRelativeTransform. Therefore, we
		// use the SetTemplateComponentWorldTransform which correctly handles the world transform
		// of template components.
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			Component, Component->GetComponentTransform(), false);

		// Update any archetype instance in need of update.
		for (UAGX_BoxShapeComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
		{
			// Only update instances that are "in sync" with the archetype.
			if (Instance->GetRelativeLocation() == OrigRelLocation &&
				Instance->GetRelativeRotation() == OrigRelRotation &&
				Instance->GetHalfExtent() == OrigHalfExtent)
			{
				Instance->Modify();
				Instance->SetHalfExtent(Component->GetHalfExtent());
				Instance->SetRelativeLocation(Component->GetRelativeLocation());
				Instance->SetRelativeRotation(Component->GetRelativeRotation());

				// The purpose of this for-loop is to make sure the Instances get exactly the same
				// relative transform as the archetype. However, SetRelativeLocation/Rotation does
				// some transformation calculations internally which in some cases result in (small)
				// rounding errors, which is enough to break the state in the Blueprint. We call the
				// Set..._Direct functions here to ensure that the RelativeLocation/Rotation matches
				// exactly with the archetype. The above calls are still needed because those make
				// sure the component is updated in the viewport without the need to recompile the
				// Blueprint.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}

		return true;
	}

	bool AutoFitCylinder(
		UAGX_CylinderShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const float OrigRadius = Component->GetRadius();
		const float OrigHeight = Component->GetHeight();

		Component->Modify();
		if (!Component->AutoFit(Meshes, Component->GetWorld(), Component->GetName()))
		{
			// Logging done in AutoFit.
			return false;
		}

		// See comment in AutoFitBox.
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			Component, Component->GetComponentTransform(), false);

		// Update any archetype instance in need of update.
		for (UAGX_CylinderShapeComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
		{
			// Only update instances that are "in sync" with the archetype.
			if (Instance->GetRelativeLocation() == OrigRelLocation &&
				Instance->GetRelativeRotation() == OrigRelRotation &&
				Instance->GetRadius() == OrigRadius && Instance->GetHeight() == OrigHeight)
			{
				Instance->Modify();
				Instance->SetRadius(Component->GetRadius());
				Instance->SetHeight(Component->GetHeight());
				Instance->SetRelativeLocation(Component->GetRelativeLocation());
				Instance->SetRelativeRotation(Component->GetRelativeRotation());

				// See comment in AutoFitBox.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}

		return true;
	}

	bool AutoFitCapsule(
		UAGX_CapsuleShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		const FVector OrigRelLocation = Component->GetRelativeLocation();
		const FRotator OrigRelRotation = Component->GetRelativeRotation();
		const float OrigRadius = Component->GetRadius();
		const float OrigHeight = Component->GetHeight();

		Component->Modify();
		if (!Component->AutoFit(Meshes, Component->GetWorld(), Component->GetName()))
		{
			// Logging done in AutoFit.
			return false;
		}

		// See comment in AutoFitBox.
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			Component, Component->GetComponentTransform(), false);

		// Update any archetype instance in need of update.
		for (UAGX_CapsuleShapeComponent* Instance :
			 FAGX_ObjectUtilities::GetArchetypeInstances(*Component))
		{
			// Only update instances that are "in sync" with the archetype.
			if (Instance->GetRelativeLocation() == OrigRelLocation &&
				Instance->GetRelativeRotation() == OrigRelRotation &&
				Instance->GetRadius() == OrigRadius && Instance->GetHeight() == OrigHeight)
			{
				Instance->Modify();
				Instance->SetRadius(Component->GetRadius());
				Instance->SetHeight(Component->GetHeight());
				Instance->SetRelativeLocation(Component->GetRelativeLocation());
				Instance->SetRelativeRotation(Component->GetRelativeRotation());

				// See comment in AutoFitBox.
				Instance->SetRelativeLocation_Direct(Component->GetRelativeLocation());
				Instance->SetRelativeRotation_Direct(Component->GetRelativeRotation());
			}
		}

		return true;
	}

	bool AutoFitAny(
		UAGX_ShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		const TArray<FAGX_MeshWithTransform>& Meshes)
	{
		if (Component == nullptr || Blueprint == nullptr)
		{
			return false;
		}

		if (UAGX_BoxShapeComponent* Box = Cast<UAGX_BoxShapeComponent>(Component))
		{
			return AutoFitBox(Box, Blueprint, Meshes);
		}
		else if (
			UAGX_CylinderShapeComponent* Cylinder = Cast<UAGX_CylinderShapeComponent>(Component))
		{
			return AutoFitCylinder(Cylinder, Blueprint, Meshes);
		}
		else if (UAGX_CapsuleShapeComponent* Capsule = Cast<UAGX_CapsuleShapeComponent>(Component))
		{
			return AutoFitCapsule(Capsule, Blueprint, Meshes);
		}
		else
		{
			UE_LOG(
				LogAGX, Error, TEXT("Unknown Auto Fit Shape Component type passed to AutoFitAny."));
			return false;
		}
	}

	FReply AutoFitToAssetInBlueprint(
		UAGX_ShapeComponent* Component, UBlueprintGeneratedClass* Blueprint, UStaticMesh* Asset)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		if (Asset == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		const FTransform WorldTransform =
			FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(Component);
		FAGX_MeshWithTransform Mesh(Asset, WorldTransform);
		AutoFitAny(Component, Blueprint, {Mesh});

		// Logging done in AutoFitAny.
		return FReply::Handled();
	}

	FReply AutoFitToParentInBlueprint(
		UAGX_ShapeComponent* Component, UBlueprintGeneratedClass* Blueprint)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		UStaticMeshComponent* MeshParent = GetParentMeshComponent(Component, Blueprint);
		if (MeshParent == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		const FTransform WorldTransform =
			FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(MeshParent);
		FAGX_MeshWithTransform Mesh(MeshParent->GetStaticMesh(), WorldTransform);
		AutoFitAny(Component, Blueprint, {Mesh});

		// Logging done in AutoFitAny.
		return FReply::Handled();
	}

	FReply AutoFitToChildInBlueprint(
		UAGX_ShapeComponent* Component, UBlueprintGeneratedClass* Blueprint, bool Recursive)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		TArray<UStaticMeshComponent*> MeshChildren =
			GetChildrenMeshComponents(Component, Blueprint, Recursive);
		TMap<UStaticMeshComponent*, FTransform> MeshOrigTransform;
		TArray<FAGX_MeshWithTransform> MeshesWithTransform;
		for (UStaticMeshComponent* Mesh : MeshChildren)
		{
			if (Mesh != nullptr)
			{
				const FTransform WorldTransform =
					FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(Mesh);
				MeshOrigTransform.Add(Mesh, WorldTransform);
				MeshesWithTransform.Add({Mesh->GetStaticMesh(), WorldTransform});
			}
		}

		if (MeshesWithTransform.Num() == 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not find any Static Meshes from the current selection."));
			return FReply::Handled();
		}

		if (!AutoFitAny(Component, Blueprint, MeshesWithTransform))
		{
			// Logging done in AutoFitAny.
			return FReply::Handled();
		}

		// Finally, we need to restore the children meshes to their original world location.
		for (UStaticMeshComponent* MeshChild : MeshChildren)
		{
			if (MeshChild == nullptr)
			{
				continue;
			}

			FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
				MeshChild, MeshOrigTransform[MeshChild]);
		}

		return FReply::Handled();
	}

	FReply AutoFitInBlueprint(
		UAGX_ShapeComponent* Component, UBlueprintGeneratedClass* Blueprint,
		EAGX_MeshLocation MeshLocation, UStaticMesh* MeshAsset)
	{
		if (Blueprint == nullptr || Component == nullptr)
		{
			return FReply::Handled();
		}

		switch (MeshLocation)
		{
			case EAGX_MeshLocation::AllChildren:
				return AutoFitToChildInBlueprint(Component, Blueprint, true);
			case EAGX_MeshLocation::ImmediateChildren:
				return AutoFitToChildInBlueprint(Component, Blueprint, false);
			case EAGX_MeshLocation::Parent:
				return AutoFitToParentInBlueprint(Component, Blueprint);
			case EAGX_MeshLocation::Asset:
				return AutoFitToAssetInBlueprint(Component, Blueprint, MeshAsset);
		}

		UE_LOG(LogAGX, Error, TEXT("Unknown MeshLocation given to AutoFitInBlueprint."));
		return FReply::Handled();
	}

	AGX_AutoFitShape* ToAutoFitShape(UAGX_ShapeComponent* Shape)
	{
		if (UAGX_BoxShapeComponent* Box = Cast<UAGX_BoxShapeComponent>(Shape))
		{
			return (AGX_AutoFitShape*) Box;
		}
		if (UAGX_CapsuleShapeComponent* Capsule = Cast<UAGX_CapsuleShapeComponent>(Shape))
		{
			return (AGX_AutoFitShape*) Capsule;
		}
		if (UAGX_CylinderShapeComponent* Cylinder = Cast<UAGX_CylinderShapeComponent>(Shape))
		{
			return (AGX_AutoFitShape*) Cylinder;
		}
		return nullptr;
	}

	struct FScopedProperyNotify
	{
		FScopedProperyNotify() = delete;
		FScopedProperyNotify(TArray<TSharedRef<IPropertyHandle>>& InProperties)
			: Properties(InProperties)
		{
			for (TSharedRef<IPropertyHandle> Handle : Properties)
			{
				if (!Handle->IsValidHandle())
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("FScopedProperyNotify was given an invalid propery handle."));
					continue;
				}

				Handle->NotifyPreChange();
			}
		}

		~FScopedProperyNotify()
		{
			for (TSharedRef<IPropertyHandle> Handle : Properties)
			{
				if (Handle->IsValidHandle())
				{
					Handle->NotifyPostChange(EPropertyChangeType::ValueSet);
				}
			}
		}

	private:
		const TArray<TSharedRef<IPropertyHandle>>& Properties;
	};
}

FAGX_AutoFitShapeDetails::FAGX_AutoFitShapeDetails(IDetailLayoutBuilder& InDetailBuilder)
	: DetailBuilder(InDetailBuilder)
{
	MeshLocations.Add(MakeShareable(new FAutoFitMeshLocation(
		"All Children",
		"Searches recursively for all Static Mesh Components that are children of this component.",
		EAGX_MeshLocation::AllChildren)));

	MeshLocations.Add(MakeShareable(new FAutoFitMeshLocation(
		"Immediate Children",
		"Searches for all Static Mesh Components that are immediate children of this component.",
		EAGX_MeshLocation::ImmediateChildren)));

	MeshLocations.Add(MakeShareable(new FAutoFitMeshLocation(
		"Parent",
		"Searches for the first ancestor of this component that is a Static Mesh Component.",
		EAGX_MeshLocation::Parent)));

	MeshLocations.Add(MakeShareable(new FAutoFitMeshLocation(
		"Asset", "Uses an explicitly chosen Static Mesh Asset.", EAGX_MeshLocation::Asset)));

	CurrentlySelectedMeshLocation = MeshLocations[0];
}

void FAGX_AutoFitShapeDetails::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
	// By having an empty header row Slate won't generate a collapsable section.
	// The Category we're part of will still be collapsable.
}

void FAGX_AutoFitShapeDetails::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	// clang-format off

	// Add Mesh Location combobox.
	ChildrenBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("MeshLocation", "Mesh location"))
		]
		.ValueContent()
		[
			SNew(SComboBox<TSharedPtr<FAutoFitMeshLocation>>)
				.OptionsSource(&MeshLocations)
				.OnGenerateWidget_Lambda([=](TSharedPtr<FAutoFitMeshLocation> Item)
				{
					return SNew(STextBlock)
						.Text(FText::FromString(*Item->Name))
						.ToolTipText(FText::FromString(*Item->ToolTip));
				})
				.OnSelectionChanged(
					this, &FAGX_AutoFitShapeDetails::OnMeshLocationComboBoxChanged)
				.Content()
				[
					SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return FText::FromString(CurrentlySelectedMeshLocation->Name);
						})
				]
		];

	// Add Static Mesh Asset picker.
	ChildrenBuilder.AddCustomRow(FText::GetEmpty())
	.Visibility(TAttribute<EVisibility>(this, &FAGX_AutoFitShapeDetails::GetAssetPickerVisibility))
	.NameContent()
	[
		SNew(STextBlock)
			.Text(LOCTEXT("StaticMeshAsset", "Static Mesh Asset"))
	]
	.ValueContent()
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
			.HAlign(HAlign_Center)
			.AutoHeight()
		[
			SNew(SObjectPropertyEntryBox)
				.AllowClear(true)
				.AllowedClass(UStaticMesh::StaticClass())
				.ObjectPath(this, &FAGX_AutoFitShapeDetails::GetCurrentAssetPath)
				.OnObjectChanged(this, &FAGX_AutoFitShapeDetails::OnAssetSelected)
				.ThumbnailPool(DetailBuilder.GetPropertyUtilities()->GetThumbnailPool())
		]
	];

	// Add auto-fit button.
	ChildrenBuilder.AddCustomRow(FText::GetEmpty())
	.NameContent()
	[
		SNew(STextBlock)
			.Text(LOCTEXT("AutoFitToMesh", "Auto-fit to Mesh"))
	]
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("AutoFitButtonText", "Auto-fit"))
					.ToolTipText(LOCTEXT(
						"AutoFitButtonTooltip",
						"Auto-fit this Shape to the Static Meshs(es) given by the current Mesh Location."))
					.OnClicked(this, &FAGX_AutoFitShapeDetails::OnAutoFitButtonClicked)
			]
	];

	// clang-format on
}

bool FAGX_AutoFitShapeDetails::InitiallyCollapsed() const
{
	return false;
}

void FAGX_AutoFitShapeDetails::SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren)
{
}

FName FAGX_AutoFitShapeDetails::GetName() const
{
	return TEXT("Auto-fit Shape Details");
}

bool FAGX_AutoFitShapeDetails::RequiresTick() const
{
	return false;
}

void FAGX_AutoFitShapeDetails::Tick(float DeltaTime)
{
}

void FAGX_AutoFitShapeDetails::OnAssetSelected(const FAssetData& AssetData)
{
	CurrentlySelectedAsset = AssetData;
}

FString FAGX_AutoFitShapeDetails::GetCurrentAssetPath() const
{
	return CurrentlySelectedAsset.IsValid() ?
#if UE_VERSION_OLDER_THAN(5, 1, 0)
											CurrentlySelectedAsset.ObjectPath.ToString()
#else
											CurrentlySelectedAsset.GetObjectPathString()
#endif
											: FString("");
}

FReply FAGX_AutoFitShapeDetails::OnAutoFitButtonClicked()
{
	using namespace AGX_AutoFitShapeDetals_helpers;
	UAGX_ShapeComponent* Shape =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ShapeComponent>(DetailBuilder);

	if (Shape == nullptr)
	{
		return FReply::Handled();
	}

	TArray<TSharedRef<IPropertyHandle>> Properties;
	Properties.Add(DetailBuilder.GetProperty(
		USceneComponent::GetRelativeLocationPropertyName(), USceneComponent::StaticClass()));
	Properties.Add(DetailBuilder.GetProperty(
		USceneComponent::GetRelativeRotationPropertyName(), USceneComponent::StaticClass()));

	const FScopedProperyNotify PropertyNotify(Properties);
	const FScopedTransaction Transaction(LOCTEXT("AutoFitUndo", "Undo Auto-fit operation"));

	if (Shape->IsInBlueprint())
	{
		// Logging done in AutoFitInBlueprint.
		return AutoFitInBlueprint(
			Shape, Cast<UBlueprintGeneratedClass>(Shape->GetOuter()),
			CurrentlySelectedMeshLocation->MeshLocation, GetSelectedStaticMeshAsset());
	}

	// Call Modify on children meshes if EAGX_MeshLocation::Children is used, to support
	// undo/redo.
	if (CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AllChildren ||
		CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::ImmediateChildren)
	{
		const bool Recursive =
			CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AllChildren;

		for (UStaticMeshComponent* Child :
			 AGX_MeshUtilities::FindChildrenMeshComponents(*Shape, Recursive))
		{
			if (Child == nullptr)
			{
				continue;
			}
			Child->Modify();
		}
	}

	Shape->Modify();
	AGX_AutoFitShape* AutoFitShape = ToAutoFitShape(Shape);
	check(AutoFitShape);
	if (CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AllChildren ||
		CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::ImmediateChildren)
	{
		const bool Recursive =
			CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::AllChildren;
		AutoFitShape->AutoFitToChildren(
			AGX_MeshUtilities::FindChildrenMeshComponents(*Shape, Recursive), Shape->GetWorld(),
			Shape->GetName());
	}
	else
	{
		AutoFitShape->AutoFit(GetSelectedStaticMeshes(Shape), Shape->GetWorld(), Shape->GetName());
	}

	// Logging done in AutoFit.
	return FReply::Handled();
}

void FAGX_AutoFitShapeDetails::OnMeshLocationComboBoxChanged(
	TSharedPtr<FAutoFitMeshLocation> NewMeshLocation, ESelectInfo::Type InSeletionInfo)
{
	CurrentlySelectedMeshLocation = NewMeshLocation;
}

EVisibility FAGX_AutoFitShapeDetails::GetAssetPickerVisibility() const
{
	return FAGX_EditorUtilities::VisibleIf(
		CurrentlySelectedMeshLocation->MeshLocation == EAGX_MeshLocation::Asset);
}

UStaticMesh* FAGX_AutoFitShapeDetails::GetSelectedStaticMeshAsset() const
{
	const FString AssetPath = GetCurrentAssetPath();
	if (AssetPath.IsEmpty())
	{
		return nullptr;
	}

	return LoadObject<UStaticMesh>(GetTransientPackage(), *GetCurrentAssetPath());
}

TArray<FAGX_MeshWithTransform> FAGX_AutoFitShapeDetails::GetSelectedStaticMeshes(
	USceneComponent* Shape) const
{
	TArray<FAGX_MeshWithTransform> Meshes;
	if (Shape == nullptr)
	{
		return Meshes;
	}

	switch (CurrentlySelectedMeshLocation->MeshLocation)
	{
		case EAGX_MeshLocation::AllChildren:
			Meshes = AGX_MeshUtilities::FindChildrenMeshes(*Shape, true);
			break;
		case EAGX_MeshLocation::ImmediateChildren:
			Meshes = AGX_MeshUtilities::FindChildrenMeshes(*Shape, false);
			break;
		case EAGX_MeshLocation::Parent:
			Meshes.Add(AGX_MeshUtilities::FindFirstParentMesh(*Shape));
			break;
		case EAGX_MeshLocation::Asset:
			if (UStaticMesh* MeshAsset = GetSelectedStaticMeshAsset())
			{
				Meshes.Add(FAGX_MeshWithTransform(MeshAsset, Shape->GetComponentTransform()));
			}
			break;
	}

	return Meshes;
}

#undef LOCTEXT_NAMESPACE
