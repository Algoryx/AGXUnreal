// Copyright 2025, Algoryx Simulation AB.

#include "AGX_StaticMeshComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "AGX_StaticMeshComponent.h"
#include "Widgets/ShapeWidget.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/StaticMesh.h"
#include "IDetailGroup.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"

#define LOCTEXT_NAMESPACE "FAGX_StaticMeshComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_StaticMeshComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_StaticMeshComponentCustomization);
}

// This whole CustomizeDetails implementation is one big experimentation playground. There is a
// whole bunch of code blocks that exists solely to try out various ways of creating and layouting
// Slate widgets. Each such experimentation block if #if 0-ed and commented with a description of
// what is being tested.
void FAGX_StaticMeshComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_StaticMeshComponent* SelectedMeshComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_StaticMeshComponent>(
			DetailBuilder);
	if (SelectedMeshComponent == nullptr)
	{
		// We're modifying multiple (or zero) meshes. Either handle that in the below
		// code or simply fall back to the default Details Panel view.
		//
		// The latter is probably unexpected for the user, but kind of works for now.

		/// \todo The intent is to fall back to the default Details Panel.
		/// Is this enough to achieve that?
		return;
	}

	IDetailCategoryBuilder& AgxCategory =
		DetailBuilder.EditCategory("AGX Dynamics", FText::GetEmpty(), ECategoryPriority::Important);

	// Put all the property widgets before the experimental widets. The way to reorder default
	// property widgets is to call AddPropertyRow in the order we want them to be. A side effect
	// of this is that they lose their subcategory set in their UPROPERTY, so we recreate it here.

	IDetailGroup& AgxShapesCategory = AgxCategory.AddGroup(
		FName(TEXT("CollisionShapes")), FText::FromString(TEXT("Collision shapes")));
	AgxShapesCategory
		.HeaderRow()[SNew(STextBlock).Text(LOCTEXT("CollisionShapes", "Collision shapes"))];

	TSharedRef<IPropertyHandle> ShapesProperty =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_StaticMeshComponent, DefaultShape));
	AgxShapesCategory.AddPropertyRow(ShapesProperty);

	TSharedRef<IPropertyHandle> SpheresProperty =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_StaticMeshComponent, Spheres));
	AgxShapesCategory.AddPropertyRow(SpheresProperty);

	TSharedRef<IPropertyHandle> BoxesProperty =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_StaticMeshComponent, Boxes));
	AgxShapesCategory.AddPropertyRow(BoxesProperty);

	TSharedRef<IPropertyHandle> CapsulesProperty =
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_StaticMeshComponent, Capsules));
	AgxShapesCategory.AddPropertyRow(CapsulesProperty);

	// All properties in the Shapes category has been moved, so hide the category.
	/// \todo This doesn't work, the category is still visible. The | separator doesn't work in the
	// IDetailLayoutBuilder member function. Figure out what to do instead.
	DetailBuilder.HideCategory("AGX Dynamics|Shapes");

	// clang-format off

// I had a button here to force a refresh of the AGX Dynamics for Unreal collision shapes. I don't
// thinks that's necessary, and I wanted to make the shape update member function private. Restore
// this if we ever find a reason to have a refresh button.
#if 0
	/// Add a button to force a refresh of the StatiMesh asset's collision shapes into the AGX
	/// StaticMesh's shape arrays.
	AgxShapesCategory.AddWidgetRow()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshCollisionShapes", "Refresh collision shapes"))
			.ToolTipText(LOCTEXT(
				"RefreshCollisionShapesTip",
				"Fetch the current collision shapes from the static mesh and populate the physics "
				"shape lists."))
			.OnClicked_Lambda([SelectedMeshComponent]() {
				SelectedMeshComponent->RefreshCollisionShapes();
				return FReply::Handled();
			})
		]
	];
#endif


// This block of code is intended to provide body/mesh-level editing of collision shapes, e.g.,
// adding and configuring collision shapes from the Details Panel as an alternative to using the
// StaticMesh Editor.
#if 0
	// Create button to add a new sphere.
	// Can we modify the StaticMesh from here?
	// Would that edit the source Asset or this SelectedMeshComponent's instance?
	AgxCategory.AddCustomRow(FText::FromString("Add sphere"))
	[
		SNew(SButton)
		.Text(LOCTEXT("AddSphere", "Add sphere"))
		.ToolTipText(LOCTEXT("AddSphereTooltip", "Add a sphere to this simulation object"))
		.OnClicked_Lambda([SelectedMeshComponent]() {
			UE_LOG(LogAGX, Warning, TEXT("Adding a sphere."));
			/// \todo Only adding the the AGX StaticMeshs' spheres list doesn't do much, must also
			/// add the sphere to the the underlying StaticMesh asset.
			SelectedMeshComponent->Spheres.Add({1.0});
			/// \todo We may want to re-run CustomizeDetails after this change.
			/// How do we trigger that?
			/// Look into IDetailLayoutBuilder::ForceRefreshDetails.
			/// DetailBuilder must be captured by this lambda.
			return FReply::Handled();
		})
	];
#endif


// This block of code experiments with various ways of adding widgets to a details panel.
#if 1
	/// \todo Create an orange border around AgxCategory, to match the visual style of AGX Dynamics
	/// for Unity.


	// Create a group for this experiment, so we can easily see what the results of our experiments
	// are. The intended output is two subgroups, one for spheres and one for boxes. In the future
	// there will be additional groups for the shape types we don't yet support.
	IDetailGroup& AgxShapeWidgetsCategory =
		AgxCategory.AddGroup("ShapeWidget", FText::FromString(TEXT("Shape widgets")));

	// Create the subgroup for the spheres.
	IDetailGroup& AgxSpheresCategory =
		AgxShapeWidgetsCategory.AddGroup("Spheres", FText::FromString(TEXT("Spheres")));

	// First we add a full-row title, describing the purpose of this subgroup.
	AgxSpheresCategory.AddWidgetRow()
	[
			SNew(STextBlock)
			.Text(LOCTEXT(
				"SpheresTitle",
				"This group will contain a listing of all sphere collision shapes in the static mesh, "
				"visualized as ShapeWidgets"))
	];


	// Next we want to create a collection of widgets that have an orange border around them.
	// We do this by first creating a VerticalBox that holds the individual widgets and then put
	// the VerticalBox into a Border.

	// Create the vertical box and all the widgets that should be enclosed by the border.
	TSharedRef<SVerticalBox> SpheresBox = SNew(SVerticalBox);
	for (auto& Sphere : SelectedMeshComponent->Spheres)
	{
		SpheresBox->AddSlot()
		[
			// Here we can do arbitrarily complicated stuff. For now we just add a single
			// ShapeWidget per sphere.
			SNew(SShapeWidget)
			.Shape(&Sphere)
			.ShapeType(EAggCollisionShape::Sphere)
		];
	}

	// Create the border.
	FDetailWidgetRow& SpheresRow = AgxSpheresCategory.AddWidgetRow(/*FText::FromString("Spheres")*/)
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 0.2f, 0.0f))
		.Padding(FMargin(5.0f, 5.0f))
		[
			SpheresBox
		]
	];
#endif

	// Create placeholder for the ShapeWidgets for the collision boxes.
	IDetailGroup& AgxBoxesCategory =
		AgxShapeWidgetsCategory.AddGroup("Boxes", FText::FromString(TEXT("Boxes")));
	AgxBoxesCategory.AddWidgetRow()
	[
			SNew(STextBlock)
			.Text(LOCTEXT(
				"PlaceForBoxes",
				"This is where the AGX Dynamics settings for box shapes will show up."))
	];

// This block of code demonstrates how to get at the primitive collision shapes stored within a
// StaticMesh asset. This can be used to make our ShapeWidget display and edit data in the
// collision shape, and not only in our FAGX_Shape struct.
#if 0
	UStaticMesh* Mesh = SelectedMeshComponent->GetStaticMesh();
	FKAggregateGeom& Shapes = Mesh->BodySetup->AggGeom;

	for (FKSphereElem& Sphere : Shapes.SphereElems)
	{
	}

	for (FKBxElem& Box : Shapes.BoxElems)
	{
	}
#endif

	// clang-format on
}

#undef LOCTEXT_NAMESPACE

#if 0
A question:

In CustomizeDetails, is it possible to add a property widget nested somewhere inside a row created
with AddCustomRow, inside a SVerticalBox for example?

Category.AddCustomRow(LOCTEXT("FilterKey", "FilterValue"))
.WholeRowContent()
[
    SNew(SVerticalBox)
    + SHorizontalBox::Slot()
    .AutoWidth()
    [
        // AddProperty doesn't work because it doesn't return a SWidget, but I
		// can't find any Property Widget creator function that does.
        Category.CreateProperty(DetailBuilder.GetProperty("MyProperty"))
    ]
    + // More slots here.
];
#endif
