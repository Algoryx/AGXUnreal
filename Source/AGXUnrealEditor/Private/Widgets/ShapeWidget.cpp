// Copyright 2025, Algoryx Simulation AB.

#include "Widgets/ShapeWidget.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
// #include "Utilities/AGX_EditorUtilities.h"
#include "AGX_StaticMeshComponent.h"

// Unreal Engine includes.
#include "PhysicsEngine/AggregateGeom.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SShapeWidget"

// The purpose of this block of code is to build an interactive GUI widget intended to be placed
// in the AGX_StaticMeshComponent's Details Panel and provide per-instance collision shape
// editing and configuration options. I don't know enough about Slate to do anything practical
// yet, and I don't know enough about the intended workflow to know what editing and
// configuration options are required. This code is just placeholder until we know more.
void SShapeWidget::Construct(const FArguments& InArguments)
{
	// clang-format off
	FAGX_Shape* Shape = InArguments._Shape;
	EAggCollisionShape::Type ShapeType = InArguments._ShapeType;
	if (Shape == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Got nullptr shape in SShapeWidget. That's unexpected"));
		ChildSlot
		[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Red)
					.Text(LOCTEXT("SShapeWidget_ShapeNull", "Get nullptr shape. That's unexpected"))
		];
		return;
	}
	if (ShapeType == EAggCollisionShape::Unknown)
	{
		UE_LOG(LogAGX, Error, TEXT("Got unknown shape type in SShapeWidget. That's unexpected"));
		ChildSlot
		[
				SNew(STextBlock)
				.ColorAndOpacity(FLinearColor::Red)
					.Text(LOCTEXT(
						"SShapeWidget_ShapeTypeUnknown",
						"Got unknown shape type. That's unexpected"))
		];
		return;
	}
	ChildSlot
	[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(1.0f, 0.2f, 0.0f))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
			[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					[
							SNew(STextBlock)
							.Text(LOCTEXT("Shape ", "I'm a ShapeWidget"))
					]
			]
	];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
