// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "CanvasItem.h"
#include "CanvasTypes.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "SceneManagement.h"
#include "SceneView.h"

#define LOCTEXT_NAMESPACE "FAGX_ConstraintComponentVisualizer"

struct HConstraintHitProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HConstraintHitProxy(const UActorComponent* InComponent)
		: HComponentVisProxy(InComponent, HPP_Wireframe)
	{
	}
};

IMPLEMENT_HIT_PROXY(HConstraintHitProxy, HComponentVisProxy);

namespace
{
	const FColor ColorViolated = FColor::Red;

	void DrawCoordinateSystemAxes(
		FPrimitiveDrawInterface* PDI, FVector const& AxisLoc, FRotator const& AxisRot, float Scale,
		uint8 DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
	{
		FRotationMatrix R(AxisRot);
		FVector const X = R.GetScaledAxis(EAxis::X);
		FVector const Y = R.GetScaledAxis(EAxis::Y);
		FVector const Z = R.GetScaledAxis(EAxis::Z);

		PDI->DrawLine(
			AxisLoc, AxisLoc + X * Scale, FLinearColor::Red, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
		PDI->DrawLine(
			AxisLoc, AxisLoc + Y * Scale, FLinearColor::Green, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
		PDI->DrawLine(
			AxisLoc, AxisLoc + Z * Scale, FLinearColor::Blue, DepthPriority, Thickness, DepthBias,
			bScreenSpace);
	}

	static FVector2D operator*(const FVector& NormalizedCoordinates, const FIntPoint& ViewSize)
	{
		return FVector2D(
			NormalizedCoordinates.X * ViewSize.X, NormalizedCoordinates.Y * ViewSize.Y);
	}

	float GetScreenToWorldFactorCapped(float FOV, float WorldDistance)
	{
		const UAGX_Simulation* Simulation = GetDefault<UAGX_Simulation>();
		const float DistanceMax = Simulation->ConstraintVisualizationScalingDistanceMax;
		static constexpr float Hack = 0.5f; // because result seemed a bit off...
		return Hack * 2.0f * std::min(WorldDistance, DistanceMax) * FMath::Atan(FOV / 2.0f);
	}

	float GetWorldSizeFromScreenFactor(float ScreenFactor, float FOV, float WorldDistance)
	{
		return ScreenFactor * GetScreenToWorldFactorCapped(FOV, WorldDistance);
	}

	float GetScreenFactorFromWorldSize(float WorldSize, float FOV, float WorldDistance)
	{
		return WorldSize / GetScreenToWorldFactorCapped(FOV, WorldDistance);
	}

	FBox GetBoundingBox(UAGX_RigidBodyComponent* Body)
	{
		FBox Box(ForceInit);

		const FTransform& BodyToWorld = Body->GetComponentTransform();
		const FTransform WorldToBody = BodyToWorld.Inverse();

		TArray<USceneComponent*> Children;
		Body->GetChildrenComponents(true, Children);
		for (auto Child : Children)
		{
			const FTransform ComponentToBody = Child->GetComponentTransform() * WorldToBody;
			const FBoxSphereBounds BoundInBodySpace = Child->CalcBounds(ComponentToBody);
			Box += BoundInBodySpace.GetBox();
		}
		return Box;
	}

	FVector GetConstantDistanceLocation(
		const FSceneView* View, const FVector& Location, float Distance)
	{
		if (!View)
		{
			return FVector(0.0f, 0.0f, 0.0f);
		}

		const FVector Direction = (Location - View->ViewLocation).GetSafeNormal();
		return View->ViewLocation + Direction * Distance;
	}

	void DrawTranslationalPrimitive(
		FPrimitiveDrawInterface* PDI, const FColor& Color, const FTransform& WorldTransform,
		EAxis::Type Axis, float Height, float Offset)
	{
		constexpr int32 NUM_SIDES {64};
		constexpr float CONE_ANGLE {20};

		// DrawWireCone renders the cone along the x-axis with the tip at the origin of
		// WorldTransform. Create a new transform with its x-axis in the negative Axis direction and
		// translate it Height + Offset distance.
		FTransform ConeAlignedTransform;
		const FVector X_w = WorldTransform.GetUnitAxis(EAxis::X);
		const FVector Y_w = WorldTransform.GetUnitAxis(EAxis::Y);
		const FVector Z_w = WorldTransform.GetUnitAxis(EAxis::Z);
		const FVector Origin_w = WorldTransform.GetLocation();

		switch (Axis)
		{
			case EAxis::X:
				ConeAlignedTransform =
					FTransform(-X_w, -Y_w, Z_w, Origin_w + (Height + Offset) * X_w);
				break;
			case EAxis::Y:
				ConeAlignedTransform =
					FTransform(-Y_w, X_w, Z_w, Origin_w + (Height + Offset) * Y_w);
				break;
			case EAxis::Z:
				ConeAlignedTransform =
					FTransform(-Z_w, Y_w, X_w, Origin_w + (Height + Offset) * Z_w);
				break;
		}

		TArray<FVector> Unused;
		DrawWireCone(
			PDI, Unused, ConeAlignedTransform, Height, CONE_ANGLE, NUM_SIDES, Color,
			SDPG_Foreground);
	}

	// Draws NumArrows arrows of height Height along a circle with radius Radius around
	// WorldTransform's x-axis.
	void DrawArrowsAlongCircle(
		FPrimitiveDrawInterface* PDI, const FColor& Color, FTransform WorldTransform, float Radius,
		float Height, int32 NumArrows)
	{
		constexpr int32 NUM_SIDES {32};
		constexpr float CONE_ANGLE {30};

		// LocalArrowTransform is the final location and rotation of the arrow being drawn,
		// expressed in the WorldTransform coordinate system.
		FTransform LocalArrowTransform;
		LocalArrowTransform.SetLocation(FVector(Radius, -Height / 2, 0));
		LocalArrowTransform.SetRotation(FQuat::MakeFromEuler(FVector(0.0f, 0.0f, 90.0f)));

		for (int i = 0; i < NumArrows; i++)
		{
			TArray<FVector> Unused;
			DrawWireCone(
				PDI, Unused, (LocalArrowTransform * WorldTransform), Height, CONE_ANGLE, NUM_SIDES,
				Color, SDPG_Foreground);

			// Rotate WorldTransform by (360 / NumArrows) deg so that the next drawn arrow gets the
			// correct position and orientation.
			WorldTransform.SetRotation(
				WorldTransform.GetRotation() *
				FQuat::MakeFromEuler(FVector(0.0f, 0.0f, 360.0f / NumArrows)));
		}
	}

	void DrawRotationalPrimitive(
		FPrimitiveDrawInterface* PDI, const FColor& Color, const FTransform& WorldTransform,
		EAxis::Type Axis, float Radius, float Offset)
	{
		constexpr int32 NUM_SIDES {64};

		const FVector X_w = WorldTransform.GetUnitAxis(EAxis::X);
		const FVector Y_w = WorldTransform.GetUnitAxis(EAxis::Y);
		const FVector Z_w = WorldTransform.GetUnitAxis(EAxis::Z);
		const FVector Origin_w = WorldTransform.GetLocation();

		FTransform CylinderAlignedTransform;
		const float CylinderHalfHeight = 0.15f * Radius;
		switch (Axis)
		{
			case EAxis::X:
				CylinderAlignedTransform = FTransform(Y_w, Z_w, X_w, Origin_w + Offset * X_w);
				break;
			case EAxis::Y:
				CylinderAlignedTransform = FTransform(Z_w, X_w, Y_w, Origin_w + Offset * Y_w);
				break;
			case EAxis::Z:
				CylinderAlignedTransform = FTransform(X_w, Y_w, Z_w, Origin_w + Offset * Z_w);
				break;
		}

		DrawWireCylinder(
			PDI, CylinderAlignedTransform.GetLocation(),
			CylinderAlignedTransform.GetUnitAxis(EAxis::X),
			CylinderAlignedTransform.GetUnitAxis(EAxis::Y),
			CylinderAlignedTransform.GetUnitAxis(EAxis::Z), Color, Radius, CylinderHalfHeight,
			NUM_SIDES, SDPG_Foreground);

		const float ArrowHeight = 0.6f * Radius;
		DrawArrowsAlongCircle(PDI, Color, CylinderAlignedTransform, Radius, ArrowHeight, 3);
	}

	void RenderBodyMarker(
		const FAGX_ConstraintBodyAttachment& Attachment, UAGX_RigidBodyComponent* Body,
		float CircleScreenFactor, const FColor& Color, const float Thickness,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
#if 0 // bHighlightUsingBoundingBox
	FBox LocalAABB = GetBoundingBox(Body);

	DrawOrientedWireBox(
		PDI, Body->GetComponentLocation(), Body->GetForwardVector(), Body->GetRightVector(),
		Body->GetUpVector(), LocalAABB.GetExtent(), Color, SDPG_World, Thickness,
		/*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
#endif

		// Highlight Body with circle.
		{
			static constexpr float CIRCLE_SCALE {0.05f};
			const float Distance = FVector::Dist(Body->GetComponentLocation(), View->ViewLocation);
			const float Radius = GetWorldSizeFromScreenFactor(
				CIRCLE_SCALE, FMath::DegreesToRadians(View->FOV), Distance);

			DrawCircle(
				PDI, Body->GetComponentLocation(), View->GetViewRight(), View->GetViewUp(), Color,
				Radius, 32, SDPG_Foreground, Thickness, /*DepthBias*/ 0.0f,
				/*bScreenSpace*/ true);
		}

		// Draw frame attachment gizmo.
		{
			// It is important to make sure that drawn coordinate system does not interfere
			// with the default transform gizmo. Therefore, make sure thickness of drawn
			// coordinate system is thinner than transform gizmo, so that the transform gizmo is
			// always selectable.
			//
			// If is not thinner, in the scenario where the coordinate system equals the
			// currently active transform gizmo (i.e. visually overlapping), and they are
			// located inside a mesh while the camera is outside of the mesh, there are
			// difficulties	selecting the transform gizmo (even if HitProxy, depth bias, etc are
			// used).

			/// \todo This doesn't work in the Blueprint editor as RigidBodyReference is currently
			/// implemented because we currently have no way of getting access to the
			/// RigidBodyComponents referenced by the RigidBodyReferences in the BodyAttachments in
			/// the ConstraintComponent.
			constexpr float FRAME_GIZMO_SCALE {0.2f};

			const float AttachemtFrameDistance =
				FVector::Dist(Attachment.GetGlobalFrameLocation(), View->ViewLocation);
			const float FrameGizmoSize = GetWorldSizeFromScreenFactor(
				FRAME_GIZMO_SCALE, FMath::DegreesToRadians(View->FOV), AttachemtFrameDistance);

			DrawCoordinateSystemAxes(
				PDI, Attachment.GetGlobalFrameLocation(),
				Attachment.GetGlobalFrameRotation().Rotator(), FrameGizmoSize, SDPG_Foreground,
				0.0f,
				/*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
		}
	}

	void RenderDofPrimitives(
		FPrimitiveDrawInterface* PDI, const FSceneView* View,
		const UAGX_ConstraintComponent* Constraint, const FAGX_ConstraintBodyAttachment& Attachment,
		bool IsViolated)
	{
		constexpr float TRANSLATIONAL_PRIMITIVE_SCALE {0.06f};
		constexpr float ROTATIONAL_PRIMITIVE_SCALE {0.07f};
		const static FColor ROT_COLOR_DEFAULT = FAGX_SlateUtilities::GetAGXColorOrange();
		constexpr FColor TRANS_COLOR_DEFAULT {243, 200, 0};

		const FTransform AttachmentTransform(Attachment.GetGlobalFrameMatrix());
		const float AttachemtFrameDistance =
			FVector::Dist(Attachment.GetGlobalFrameLocation(), View->ViewLocation);

		const float Radius = GetWorldSizeFromScreenFactor(
			ROTATIONAL_PRIMITIVE_SCALE, FMath::DegreesToRadians(View->FOV), AttachemtFrameDistance);
		const float RotOffset = 1.4f * Radius;

		const float Height = GetWorldSizeFromScreenFactor(
			TRANSLATIONAL_PRIMITIVE_SCALE, FMath::DegreesToRadians(View->FOV),
			AttachemtFrameDistance);
		const float TransOffset = 1.4f * Height;

		const FColor RotDofPrimitiveColor = IsViolated ? ColorViolated : ROT_COLOR_DEFAULT;
		const FColor TransDofPrimitiveColor = IsViolated ? ColorViolated : TRANS_COLOR_DEFAULT;
		if (!Constraint->IsDofLocked(EDofFlag::DofFlagRotational1))
		{
			DrawRotationalPrimitive(
				PDI, RotDofPrimitiveColor, AttachmentTransform, EAxis::X, Radius, RotOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DofFlagRotational2))
		{
			DrawRotationalPrimitive(
				PDI, RotDofPrimitiveColor, AttachmentTransform, EAxis::Y, Radius, RotOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DofFlagRotational3))
		{
			DrawRotationalPrimitive(
				PDI, RotDofPrimitiveColor, AttachmentTransform, EAxis::Z, Radius, RotOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DofFlagTranslational1))
		{
			DrawTranslationalPrimitive(
				PDI, TransDofPrimitiveColor, AttachmentTransform, EAxis::X, Height, TransOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DofFlagTranslational2))
		{
			DrawTranslationalPrimitive(
				PDI, TransDofPrimitiveColor, AttachmentTransform, EAxis::Y, Height, TransOffset);
		}
		if (!Constraint->IsDofLocked(EDofFlag::DofFlagTranslational3))
		{
			DrawTranslationalPrimitive(
				PDI, TransDofPrimitiveColor, AttachmentTransform, EAxis::Z, Height, TransOffset);
		}
	}
}

void FAGX_ConstraintComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_ConstraintComponent* Constraint = Cast<const UAGX_ConstraintComponent>(Component);
	if (Constraint == nullptr)
		return;

	DrawConstraint(Constraint, View, PDI);
}

void FAGX_ConstraintComponentVisualizer::DrawVisualizationHUD(
	const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	const UAGX_ConstraintComponent* Constraint = Cast<const UAGX_ConstraintComponent>(Component);

	if (!Constraint)
	{
		return;
	}

	DrawConstraintHUD(Constraint, Viewport, View, Canvas);
}

bool FAGX_ConstraintComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	UActorComponent* Component = const_cast<UActorComponent*>(VisProxy->Component.Get());

	// The Blueprint Editor behaves differently from the Level Editor. The Component we click on in
	// the Viewport is not the same as the Component in the Components list, and selecting it causes
	// a crash if the Component remain selected when the Blueprint Editor is closed.
	//
	// For these reasons we try to avoid doing anything at all when the selected Component is in the
	// Blueprint Editor. One might think that `IsInBlueprint` would return true when in a Blueprint,
	// but it does not.
	//
	// The best heuristic I have found for being in the Blueprint Editor is that the Owner isn't
	// selected and/or the current UWorld is a preview world.
	UWorld* World = Component->GetWorld();
	if (!Component->IsOwnerSelected() || World == nullptr || World->IsPreviewWorld())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("This looks like a Blueprint Editor component and selections in the Blueprint "
				 "Editor sometimes causes crashes. Ignoring the click."));
		return false;
	}

	// Selection behavior based on how selection works when clicking on the regular Component icons:
	// Shift-click means unconditionally add to selection.
	// Ctrl-click means toggle selection of this specific component.
	// Shift-Ctrl-click is the same as Shift-click
	// Unmodified click means select only this component.

	if (Click.IsShiftDown())
	{
		// Unconditionally add to selection.
		GEditor->SelectComponent(Component, true, true);
	}
	else if (Click.IsControlDown())
	{
		// Toggle selection of this Component.
		GEditor->SelectComponent(Component, !Component->IsSelected(), true);
	}
	else
	{
		// No modifier, replace current selection.
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(Component->GetOwner(), true, true);
		GEditor->SelectComponent(Component, true, true);
	}
	return true;
}

void FAGX_ConstraintComponentVisualizer::DrawConstraint(
	const UAGX_ConstraintComponent* Constraint, const FSceneView* View,
	FPrimitiveDrawInterface* PDI)
{
	if (Constraint == nullptr || !Constraint->IsVisible())
	{
		return;
	}

	// Do not show constraint visualizers while playing. (they are not up-to-date anyway)
	if (Constraint->GetWorld()->IsGameWorld())
	{
		return;
	}

	const bool Violated = Constraint->AreFramesInViolatedState(KINDA_SMALL_NUMBER);

	UAGX_RigidBodyComponent* Body1 = Constraint->BodyAttachment1.GetRigidBody();
	UAGX_RigidBodyComponent* Body2 = Constraint->BodyAttachment2.GetRigidBody();

	const static FColor HighlightColor = FAGX_SlateUtilities::GetAGXColorOrange();
	const float HighlightThickness(1.0f);

	// \todo Is there a better way to determine if we are in the BP Actor Editor?
	bool bIsInBlueprintEditor = Constraint->GetOutermost()->GetName().Equals("/Engine/Transient");

	// By default DrawVisualization is called when the component's Actor is selected in the Level
	// Editor or when the Component is selected in the BP Actor Editor. If there's many constraints
	// etc in the same actor the Level Editor's viewport gets very messy. Therefore, with the
	// following boolean we only draw the visualization if the component is selected in the Level
	// Editor, not the actor.
	bool bShowConnections = Constraint->IsSelectedInEditor() || bIsInBlueprintEditor;

	if (bShowConnections)
	{
		if (Body1 != nullptr)
		{
			float CircleScreenFactor = 0.08f;
			RenderBodyMarker(
				Constraint->BodyAttachment1, Body1, CircleScreenFactor, HighlightColor,
				HighlightThickness, View, PDI);
		}
		if (Body2 != nullptr)
		{
			float CircleScreenFactor = 0.05f;
			FColor Color = FColor(
				HighlightColor.R * 0.6f, HighlightColor.G * 0.6f, HighlightColor.B * 0.6f,
				HighlightColor.A);
			RenderBodyMarker(
				Constraint->BodyAttachment2, Body2, CircleScreenFactor, Color, HighlightThickness,
				View, PDI);
		}
	}

	PDI->SetHitProxy(new HConstraintHitProxy(Constraint));
	RenderDofPrimitives(PDI, View, Constraint, Constraint->BodyAttachment1, Violated);
	RenderDofPrimitives(PDI, View, Constraint, Constraint->BodyAttachment2, Violated);
	PDI->SetHitProxy(nullptr);

	const float Distance = 100.0f;

	const FVector LocationAttach1 = GetConstantDistanceLocation(
		View, Constraint->BodyAttachment1.GetGlobalFrameLocation(), Distance);
	const FVector LocationAttach2 = GetConstantDistanceLocation(
		View, Constraint->BodyAttachment2.GetGlobalFrameLocation(), Distance);

	if (bShowConnections)
	{
		if (Body1 != nullptr)
		{
			const FVector LocationBody1 =
				GetConstantDistanceLocation(View, Body1->GetComponentLocation(), Distance);

			// Draw line between body1 and attachment frame 1.
			DrawDashedLine(
				PDI, LocationBody1, LocationAttach1, HighlightColor, HighlightThickness,
				SDPG_Foreground,
				/*DepthBias*/ 0.0f);
		}

		if (Body2 != nullptr)
		{
			const FVector LocationBody2 =
				GetConstantDistanceLocation(View, Body2->GetComponentLocation(), Distance);

			// Draw line between body2 and attachment frame 2.
			DrawDashedLine(
				PDI, LocationBody2, LocationAttach2, HighlightColor, HighlightThickness,
				SDPG_Foreground,
				/*DepthBias*/ 0.0f);
		}

		if (Violated)
		{
			// Draw red line between attachment frame 1 and attachment frame 2 if the constraint is
			// violated.
			DrawDashedLine(
				PDI, LocationAttach1, LocationAttach2, ColorViolated, HighlightThickness,
				SDPG_Foreground,
				/*DepthBias*/ 0.0f);
		}
	}
}

void FAGX_ConstraintComponentVisualizer::DrawConstraintHUD(
	const UAGX_ConstraintComponent* Constraint, const FViewport* Viewport, const FSceneView* View,
	FCanvas* Canvas)
{
	if (Constraint == nullptr || !Constraint->IsVisible())
	{
		return;
	}

// The purpose of this code is to alert the user of the fact that the current constraint is
// violated. While that information is useful, simply printing it to screen at a hard-coded location
// isn't the best way to go about it. Sometimes the constraints are intentionally violated, and
// sometimes multiple constraints are printed at the same time, which makes the text unreadable.
// Find a better way to provide this information, see internal GitLab issue 270.
#if 0
	FString Message;
	if (Constraint->AreFramesInViolatedState(KINDA_SMALL_NUMBER, &Message))
	{
		FVector2D Position(0.45f, 0.35f);
		FText Text = FText::FromString(
			FString::Printf(TEXT("Constraint Frames In Violated State!\n%s"), *Message));
		UFont* Font = GEngine->GetSubtitleFont();
		FCanvasTextItem CanvasText(
			Position * Canvas->GetViewRect().Size(), Text, Font, FColor::Red);

		Canvas->DrawItem(CanvasText);
	}
#endif
}

#undef LOCTEXT_NAMESPACE
