// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelHitProxies.h"
#include "Terrain/AGX_ShovelUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "ActorEditorUtils.h"
#include "EditorViewportClient.h"
#include "Utilities/AGX_EditorUtilities.h"

#define LOCTEXT_NAMESPACE "AGX_ShovelComponentVisualizer"
#define MEMBER(Name) GET_MEMBER_NAME_CHECKED(UAGX_ShovelComponent, Name)

/**
 * A collection of helper functions called by the Shovel Visualizer.
 *
 * Made as a separate struct only to reduce the amount of code and changes in the header file.
 */
struct FShovelVisualizerOperations
{
	static bool ShovelProxyClicked(
		FAGX_ShovelComponentVisualizer& Visualizer, const UAGX_ShovelComponent& Shovel,
		HShovelHitProxy& Proxy, FEditorViewportClient& InViewportClient)
	{
		if (Proxy.Frame == Visualizer.SelectedFrame)
		{
			// Clicking a selected node deselects it.
			Visualizer.ClearSelection();
			return true;
		}

		// All checks passed, we really should select a frame.
		Visualizer.SelectedFrame = Proxy.Frame;
		Visualizer.ShovelPropertyPath = FComponentPropertyPath(&Shovel);
		if (FAGX_Frame* Frame = Visualizer.GetSelectedFrame())
		{
			Visualizer.CachedRotation = Frame->GetWorldRotation(Shovel).Quaternion();
		}
		switch (Visualizer.GetSelectedFrameSource())
		{
			// Some frames (begin and end for an edge) only support translation while some
			// (directions) are mainly controlled through rotation. This code tries to switch to
			// the appropriate transform gizmo mode but fails (on Unreal Engine 5.0) while in the
			// level editor because the ModesTool is currently in tracking mode. Leaving the code
			// anyway, maybe it starts working in the future, and since it does work in the
			// Blueprint editor viewport.
			case EAGX_ShovelFrame::None:
				break;
			case EAGX_ShovelFrame::CuttingDirection:
				InViewportClient.SetWidgetMode(UE::Widget::WM_Rotate);
				break;
			case EAGX_ShovelFrame::CuttingEdgeBegin:
			case EAGX_ShovelFrame::CuttingEdgeEnd:
			case EAGX_ShovelFrame::TopEdgeBegin:
			case EAGX_ShovelFrame::TopEdgeEnd:
				InViewportClient.SetWidgetMode(UE::Widget::WM_Translate);
				break;
		}
		return true;
	}

	static bool CanDrag(FAGX_ShovelComponentVisualizer& Visualizer, const FVector& DeltaTranslate)
	{
		return IsTranslatable(Visualizer.GetSelectedFrameSource()) && !DeltaTranslate.IsZero();
	}

	static bool CanRotate(FAGX_ShovelComponentVisualizer& Visualizer, const FRotator& DeltaRotate)
	{
		return IsRotatable(Visualizer.GetSelectedFrameSource()) && !DeltaRotate.IsZero();
	}

	// TODO: Consider merging with FAGX_WireWinchPose. Is there a larger concept here that we want
	// to realize? Is it FAGX_Frame that is that larger concept and we want Wire Winch to use that?
	struct FAGX_ShovelFramePose
	{
		const FTransform& LocalToWorld; // Will reference into the frame's parent, or the shovel.
		const FVector LocalLocation;
		const FRotator LocalRotation;
	};

	static FAGX_ShovelFramePose GetShovelPose(FAGX_Frame& Frame, UAGX_ShovelComponent& Shovel)
	{
		USceneComponent* Parent = Frame.GetParentComponent();
		if (Parent == nullptr)
		{
			// No parent has been set of the frame, which means that the location and rotation is
			// relative to the shovel.
			Parent = &Shovel;
		}
		return {Parent->GetComponentTransform(), Frame.LocalLocation, Frame.LocalRotation};
	}

	static void FrameProxyDragged(
		FAGX_ShovelComponentVisualizer& Visualizer, UAGX_ShovelComponent& Shovel,
		FEditorViewportClient& ViewportClient, const FVector& DeltaTranslate)
	{
		FAGX_Frame* Frame = Visualizer.GetSelectedFrame();
		EAGX_ShovelFrame FrameSource = Visualizer.GetSelectedFrameSource();
		// Consider transforming DeltaTranslate to the  local coordinate system and doing the
		// add there instead of the other way around. Fewer transformations.
		FAGX_ShovelFramePose Pose = GetShovelPose(*Frame, Shovel);
		const FVector CurrentWorldLocation =
			Pose.LocalToWorld.TransformPosition(Pose.LocalLocation);
		const FVector NewWorldLocation = CurrentWorldLocation + DeltaTranslate;
		FVector NewLocalLocation = Pose.LocalToWorld.InverseTransformPosition(NewWorldLocation);
		FAGX_ObjectUtilities::TruncateForDetailsPanel(NewLocalLocation);
		Shovel.Modify();
		Shovel.GetFrame(FrameSource)->LocalLocation = NewLocalLocation;
/*
  We would like to use FComponentVisualizer::NotifyPropertyModified, but that doesn't work
  because it looks at the entire Edge or Direction property, not just the FAGX_Frame member
  we are modifying. This means that is also compares the FAGX_ComponentReference when
  checking if an archetype instance is to be updated. These may contain different bytes even
  when they are conceptually the same, i.e. when Parent.OwningActor points to the shovel's
  GetOwner(). I don't see a way to have ComponentVisualizer::NotifyPropertyModified look
  into a nested struct property and compare a specific member within that struct, I don't
  think we can pass the FProperty for FAGX_Frame::LocalLocation, at least not without
  finding a way to express the entire path from the Shovel Component to the leaf Property.
*/
#if 0
		Visualizer.NotifyPropertyModified(&Shovel, Visualizer.GetSelectedFrameProperty());
#endif

		// Instead of calling Visualizer.NotifyPropertyModified, call our shadow implementation.
		auto ReadLocalLocation = [](const FAGX_Frame* Frame) { return Frame->LocalLocation; };

		auto WriteLocalLocation = [](FAGX_Frame* Frame, const FVector& NewLocalLocation)
		{ Frame->LocalLocation = NewLocalLocation; };

		NotifyFrameModified<FVector>(Visualizer, Shovel, ReadLocalLocation, WriteLocalLocation);
	}

	static void FrameProxyRotated(
		FAGX_ShovelComponentVisualizer& Visualizer, UAGX_ShovelComponent& Shovel,
		FEditorViewportClient& ViewportClient, const FRotator& DeltaRotate)
	{
		FAGX_Frame* Frame = Visualizer.GetSelectedFrame();
		EAGX_ShovelFrame FrameSource = Visualizer.GetSelectedFrameSource();
		FAGX_ShovelFramePose Pose = GetShovelPose(*Frame, Shovel);

		// Convert the current local rotation to world space, rotate spline rotation according to
		// delta rotation, and then transform back to local space. Do the computation using
		// quaternions instead of rotators to avoid unwanted signed flip at 90 degree angles.
		const FQuat CurrentLocal = Pose.LocalRotation.Quaternion();
		const FQuat CurrentWorld = Pose.LocalToWorld.GetRotation() * CurrentLocal;
		const FQuat NewWorld = DeltaRotate.Quaternion() * CurrentWorld;
		const FQuat NewLocal = Pose.LocalToWorld.GetRotation().Inverse() * NewWorld;
		FRotator NewLocalRotation = NewLocal.Rotator();

		FAGX_ObjectUtilities::TruncateForDetailsPanel(NewLocalRotation);
		Shovel.Modify();
		Frame->LocalRotation = NewLocalRotation;

		auto ReadLocalRotation = [](const FAGX_Frame* Frame) { return Frame->LocalRotation; };

		auto WriteLocalRotation = [](FAGX_Frame* Frame, const FRotator& NewLocalRotation)
		{ Frame->LocalRotation = NewLocalRotation; };

		NotifyFrameModified<FRotator>(Visualizer, Shovel, ReadLocalRotation, WriteLocalRotation);

		Visualizer.CachedRotation = NewWorld;
	}

	/**
	 * Apply the change made on the Shovel to all instances of that Shovel, and call the Property
	 * Changed callback. If the Shovel is part of a preview, i.e. lives in the world shown in a
	 * Blueprint editor viewport, then the change is promoted to be applied on the Blueprint's CSC
	 * node's template Component and its instances instead, excluding the preview shovel since that
	 * has already been updated.
	 *
	 * This does conceptually the same thing as FComponentVisualizer::NotifyPropertyModified, with
	 * the difference being that this implementation is specialized for FAGX_Frame. We cannot use
	 * FComponentVisualizer::NotifyPropertyModified because I have not been able to figure out how
	 * to use that with nested properties, i.e. when a struct member is modified, and we cannot call
	 * FComponentVisualizer::NotifyPropertyModified for the entire Shovel property because the frame
	 * contains a parent Component reference which may be conceptually equal but bit-wise non-equal,
	 * for example in the common case of a local reference, i.e. the frame has a parent that is in
	 * the same Actor.
	 *
	 * If this part of the code breaks then compare this implementation with that of
	 * FComponentVisualizer::NotifyPropertyModified, perhaps Epic Games changed something and we
	 * need to do the same change here.
	 */
	template <typename FPropertyType, typename FReadFunc, typename FWriteFunc>
	static void NotifyFrameModified(
		FAGX_ShovelComponentVisualizer& Visualizer, UAGX_ShovelComponent& Shovel,
		FReadFunc ReadFunc, FWriteFunc WriteFunc)
	{
		// Get the frame that is currently being modified.
		const EAGX_ShovelFrame FrameSource = Visualizer.GetSelectedFrameSource();
		FProperty* FrameProperty = Visualizer.GetSelectedFrameProperty();

		// If the owning Actor of a Component is a Blueprint or a Blueprint instance then the
		// Construction Script needs to be run after each edit. There is no direct way to trigger
		// a Blueprint Reconstruction, so we rely on the Post Edit Move event to eventually trigger
		// a Blueprint Reconstruction, if necessary for a particular Actor.
		//
		// I'm not 100% sure that we really need this, or if the PostEditChangeChainProperty call
		// will do this for us.
		auto RerunConstructionScript = [](AActor& Actor)
		{
			// todo Should the parameter, bFinished, be true or false?
			//      Depends on if this was the last input delta for the transform gizmo drag or not.
			//      Passing false for now because that is what calling
			//      FComponentVisualizer::NotifyFrameModified does when called with default
			//      arguments, which is what FSplineComponentVisualizer does as of Unreal
			//      Engine 5.1.
			Actor.PostEditMove(false);
		};

		// Construct a Property Changed Chain Event and trigger that for the given object. We need
		// this because we apply side effects in the PostEditChangeChainProperty callback in our
		// Native Owner objects. A common side effect is to apply the new state to the AGX Dynamics
		// instance.
		//
		// It may be that we should collect all modified objects in a single Property Changed Chain
		// Event, but this seems to work for now.
		auto TriggerChainEvent = [FrameProperty](UObject* ModifiedObject)
		{
			FEditPropertyChain PropertyChain;
			PropertyChain.AddTail(FrameProperty);
			TArray<const UObject*> ModifiedObjects {ModifiedObject};

			// We would like to pass EPropertyChangeType::Interactive for all calls to
			// HandleInputDelta except for the last one, but I don't know how to detect if this is
			// the last one or not. or is there some other function that is called when the user
			// releases the transform gizmo?
			//
			// For now use ValueSet, that is the safest but also the most costly.
			FPropertyChangedEvent ChangedEvent(
				FrameProperty, EPropertyChangeType::ValueSet, ModifiedObjects);

			FPropertyChangedChainEvent ChainEvent(PropertyChain, ChangedEvent);
			ModifiedObject->PostEditChangeChainProperty(ChainEvent);
		};

		// The source shovel has already been updated, so trigger its property changed chain event
		// immediately.
		TriggerChainEvent(&Shovel);

		AActor* Owner = Shovel.GetOwner();
		if (Owner == nullptr)
		{
			// Is there any case where a Component does not have an owner but does have instances?
			// That is, an archetype Component that we want to modify that does not have an Owner.
			// The FComponentVisualizer::NotifyFrameModified code does not handle this case, so we
			// early-out as well.
			return;
		}
		if (!FActorEditorUtils::IsAPreviewOrInactiveActor(Owner))
		{
			// If the owner is not a preview Actor then we don't need to do all the change
			// propagation done below. Just rerun the construction script for the Actor that was
			// directly modified.
			//
			// Similar question as with a nullptr Owner, is there no case in which a non-preview
			// Actor can contain a Component that is an archetype, i.e. has instances? The
			// FComponentVisualizer::NotifyFrameModified code does not handle that case, so we
			// early-out as well.
			RerunConstructionScript(*Owner);
			return;
		}

		// We now know that we have a Component that is part of a preview Actor, i.e. one that
		// lives in e.g. the Blueprint editor viewport.

		// The component belongs to the preview actor in the BP editor, so we need to propagate the
		// property change to the archetype. Before this, we exploit the fact that the archetype and
		// the preview actor have the old and new value of the property, respectively. So we can go
		// through all archetype instances, and if they hold the (old) archetype value, update it to
		// the new value.

		// Get archetype, which should be a Blueprint CSC node's template Component.
		UAGX_ShovelComponent* Archetype = Cast<UAGX_ShovelComponent>(Shovel.GetArchetype());
		AGX_CHECK(Archetype != nullptr);
		if (Archetype == nullptr)
		{
			// Don't think this can even happen. Leaving a log message so we can be made aware if it
			// even does.
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX_ShovelComponentVisualizer::NotifyFrameModified found a preview shovel "
					 "named '%s' that does not have an archetype."),
				*Shovel.GetName());
			return;
		}
		if (!IsValid(Archetype))
		{
			// Not sure if the archetype can become invalid without the preview instance doing so
			// first, but bail just in case.
			return;
		}
		AGX_CHECK(Archetype != UAGX_ShovelComponent::StaticClass()->GetDefaultObject());
		if (Archetype == UAGX_ShovelComponent::StaticClass()->GetDefaultObject())
		{
			// Don't think this can even happen. Leaving a log message so we can be made aware if it
			// even does.
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGX_ShovelComponentVisualzier::NotifyFrameModified found a preview shovel "
					 "that has the Class Default Object as its archetype."));
			// The preview instance of a Blueprint template Component always (I hope.) has the
			// template Component as its archetype, and the template Component is never the Class
			// Default Object for the type. If we ever try to modify the Class Default Object
			// here then something has gone wrong and we should bail.
			return;
		}

		// Get all archetype instances, which should include the preview Shovel.
		TArray<UObject*> ArchetypeInstances;
		Archetype->GetArchetypeInstances(ArchetypeInstances);
		AGX_CHECK(ArchetypeInstances.Contains(&Shovel));

		// This is the old value for Local Location. Only instances that has this exact value
		// should be updated.
		const FPropertyType OriginalValue = ReadFunc(Archetype->GetFrame(FrameSource));

		// Among the archetype instances, find the ones that have the old value for Local Location.
		// These are the Shovels that should be updated.
		TArray<UAGX_ShovelComponent*> InstancesToUpdate;
		InstancesToUpdate.Reserve(ArchetypeInstances.Num());
		for (UObject* Instance : ArchetypeInstances)
		{
			UAGX_ShovelComponent* InstanceShovel = Cast<UAGX_ShovelComponent>(Instance);
			AGX_CHECK(IsValid(InstanceShovel));
			if (InstanceShovel == &Shovel || !IsValid(InstanceShovel))
			{
				// This is the preview Shovel that the original write was done on, do not
				// propagate to this one. Or either a nullptr instance or an instance that wasn't
				// a shovel at all, or an instance that has been marked pending kill or garbage
				// collected.
				continue;
			}

			const FPropertyType CurrentValue = ReadFunc(InstanceShovel->GetFrame(FrameSource));
			if (CurrentValue == OriginalValue) // Bit comparison even for floating-point types.
			{
				InstancesToUpdate.Add(InstanceShovel);
			}
		}

		// Value that should be propagated to the archetype and the archetype instances.
		const FPropertyType NewValue = ReadFunc(Shovel.GetFrame(FrameSource));

		// Propagate the new value to the archetype.
		{
			Archetype->SetFlags(RF_Transactional);
			Archetype->Modify();
			AActor* ArchetypeOwner = Archetype->GetOwner();
			if (ArchetypeOwner)
			{
				ArchetypeOwner->Modify();
			}
			WriteFunc(Archetype->GetFrame(FrameSource), NewValue);
			TriggerChainEvent(Archetype);

			// Why not call RerunConstructionScript for ArchetypeOwner?
			// We don't because FComponentVisualizer::NotifyPropertyModified doesn't, but why
			// doesn't it? Should we?
		}

		// Propagate the new value to the archetype instances.
		for (UAGX_ShovelComponent* InstanceToUpdate : InstancesToUpdate)
		{
			InstanceToUpdate->SetFlags(RF_Transactional);
			InstanceToUpdate->Modify();
			AActor* OwnerToUpdate = InstanceToUpdate->GetOwner();
			if (OwnerToUpdate != nullptr)
			{
				OwnerToUpdate->Modify();
			}
			WriteFunc(InstanceToUpdate->GetFrame(FrameSource), NewValue);
			TriggerChainEvent(InstanceToUpdate);
			if (OwnerToUpdate != nullptr)
			{
				RerunConstructionScript(*OwnerToUpdate);
			}
		}

		RerunConstructionScript(*Owner);
	}
};

FAGX_ShovelComponentVisualizer::FAGX_ShovelComponentVisualizer()
{
	UClass* Class = UAGX_ShovelComponent::StaticClass();
	TopEdgeProperty = FindFProperty<FProperty>(Class, MEMBER(TopEdge));
	CuttingEdgeProperty = FindFProperty<FProperty>(Class, MEMBER(CuttingEdge));
	CuttingDirectionProperty = FindFProperty<FProperty>(Class, MEMBER(CuttingDirection));

	// Here is where we would register the commands class and create a command list, if we ever have
	// the need for keyboard shortcuts in the Shovel setup workflow.
}

FAGX_ShovelComponentVisualizer::~FAGX_ShovelComponentVisualizer()
{
	// Here is where we would unregister the commands class, if we ever have the n eed for keyboard
	// shortcuts in the Shovel setup workflow.
}

void FAGX_ShovelComponentVisualizer::OnRegister()
{
	FComponentVisualizer::OnRegister();

	// Here is where we would register command list actions and bind callbacks, if we ever have
	// the need for keyboard shortcuts in the Shovel setup workflow.
}

void FAGX_ShovelComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	FComponentVisualizer::DrawVisualization(Component, View, PDI);

	const UAGX_ShovelComponent* Shovel = Cast<UAGX_ShovelComponent>(Component);
	if (Shovel == nullptr)
	{
		// Visualizing something not a Shovel, assume we lost the shovel selection.
		ClearSelection();
		return;
	}

	const bool bSelected = FAGX_EditorUtilities::IsSelected(*Shovel);
	const bool bEditable = bSelected;

	const float PointSize {FAGX_ShovelUtilities::HitProxySize};

	// Draw the top edge.
	{
		const FVector BeginLocation = Shovel->TopEdge.Start.GetWorldLocation(*Shovel);
		const FVector EndLocation = Shovel->TopEdge.End.GetWorldLocation(*Shovel);
		FLinearColor Color = FLinearColor::White;
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);

		if (bEditable)
		{
			PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::TopEdgeBegin));
			PDI->DrawPoint(BeginLocation, Color, PointSize, SDPG_Foreground);
			PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::TopEdgeEnd));
			PDI->DrawPoint(EndLocation, Color, PointSize, SDPG_Foreground);
			PDI->SetHitProxy(nullptr);
		}
	}

	// Draw the cutting edge.
	{
		const FVector BeginLocation = Shovel->CuttingEdge.Start.GetWorldLocation(*Shovel);
		const FVector EndLocation = Shovel->CuttingEdge.End.GetWorldLocation(*Shovel);
		FLinearColor Color = FLinearColor::Red;
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);

		if (bEditable)
		{
			PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::CuttingEdgeBegin));
			PDI->DrawPoint(BeginLocation, Color, PointSize, SDPG_Foreground);
			PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::CuttingEdgeEnd));
			PDI->DrawPoint(EndLocation, Color, PointSize, SDPG_Foreground);
			PDI->SetHitProxy(nullptr);
		}
	}

	// Draw the cutting direction.
	{
		const FVector BeginLocation = Shovel->CuttingDirection.GetWorldLocation(*Shovel);
		const FRotator Rotation = Shovel->CuttingDirection.GetWorldRotation(*Shovel);
		const FVector Direction = Rotation.RotateVector(FVector::ForwardVector);
		const FVector EndLocation = BeginLocation + 100 * Direction;
		// The color used by the diagram in the Shovel Setup section of the
		// Terrain chapter of the AGX Dynamics user manual.
		const FLinearColor Color = FLinearColor(FColor(143, 188, 143));
		PDI->DrawLine(BeginLocation, EndLocation, Color, SDPG_Foreground, 1.0f);

		if (bEditable)
		{
			PDI->SetHitProxy(new HShovelHitProxy(Shovel, EAGX_ShovelFrame::CuttingDirection));
			PDI->DrawPoint(BeginLocation, Color, PointSize, SDPG_Foreground);
			PDI->SetHitProxy(nullptr);
		}
	}

	// Not sure where to best put this. Don't want to miss a deselection if the we don't get
	// any more calls to DrawVisualization after the deselection.
	if (HasValidFrameSection())
	{
		if (!FAGX_EditorUtilities::IsSelected(*GetSelectedShovel()))
		{
			// Do not maintain a frame selection if the selected shovel isn't selected anymore.
			// This is so that the transform widget is placed at the newly selected Component
			// instead of at the now no longer selected frame.
			ClearSelection();
		}
	}
}

bool FAGX_ShovelComponentVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_ShovelComponent* Shovel = Cast<const UAGX_ShovelComponent>(VisProxy->Component);
	if (Shovel == nullptr)
	{
		// Clicked something not a shovel, deselect whatever we had selected before.
		ClearSelection();
		return false;
	}

	AActor* OldOwningActor = ShovelPropertyPath.GetParentOwningActor();
	AActor* NewOwningActor = Shovel->GetOwner();
	if (NewOwningActor != OldOwningActor)
	{
		// Don't reuse selection data between Actors, it's completely different shovels.
		ClearSelection();
	}

	if (HShovelHitProxy* Proxy = HitProxyCast<HShovelHitProxy>(VisProxy))
	{
		return FShovelVisualizerOperations::ShovelProxyClicked(
			*this, *Shovel, *Proxy, *InViewportClient);
	}

	// Add additional proxy types here when needed.

	// The clicked proxy isn't a Shovel proxy, return false to pass on to the next handler in line.
	return false;
}

// Called by Unreal Editor to decide where the transform widget should be rendered. We place it on
// the selected frame, if there is one.
bool FAGX_ShovelComponentVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}
	if (!FAGX_EditorUtilities::IsSelected(*Shovel))
	{
		// Is this const-cast safe?
		// If not, how do we clear the frame selection when the Shovel becomes unselected?
		const_cast<FAGX_ShovelComponentVisualizer*>(this)->ClearSelection();
		return false;
	}
	if (FAGX_Frame* Frame = GetSelectedFrame())
	{
		OutLocation = Frame->GetWorldLocation(*Shovel);
		return true;
	}

	return false;
}

/*
 * The custom input coordinate system is used when the user is currently interacting with the
 * transformation gizmo in the rotate mode. It is used to tell the engine how our object has been
 * rotated. Without this the transform gizmo visualization rotates backwards instead of showing
 * a delta rotation forwards.
 */
bool FAGX_ShovelComponentVisualizer::GetCustomInputCoordinateSystem(
	const FEditorViewportClient* ViewportClient, FMatrix& OutMatrix) const
{
	if (ViewportClient->GetWidgetCoordSystemSpace() == COORD_Local ||
		ViewportClient->GetWidgetMode() == UE::Widget::WM_Rotate)
	{
		if (HasValidFrameSection())
		{
			OutMatrix = FRotationMatrix::Make(CachedRotation);
			return true;
		}
	}

	return false;
}

bool FAGX_ShovelComponentVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}

	if (HasValidFrameSection())
	{
		UAGX_ShovelComponent* ToModify = Shovel;
		if (FShovelVisualizerOperations::CanDrag(*this, DeltaTranslate))
		{
			FShovelVisualizerOperations::FrameProxyDragged(
				*this, *ToModify, *ViewportClient, DeltaTranslate);
		}
		if (FShovelVisualizerOperations::CanRotate(*this, DeltaRotate))
		{
			FShovelVisualizerOperations::FrameProxyRotated(
				*this, *ToModify, *ViewportClient, DeltaRotate);
		}
	}
	// Add additional selection types, i.e. not frame, here, if we ever get new types.
	else
	{
		// We got a move request but we have no valid selection so don't know what to move.
		// Something's wrong, so reset the selection state.
		ClearSelection();
		return false;
	}

	GEditor->RedrawLevelEditingViewports();
	return true;
}

bool FAGX_ShovelComponentVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	return FComponentVisualizer::HandleInputKey(ViewportClient, Viewport, Key, Event);
}

bool FAGX_ShovelComponentVisualizer::IsVisualizingArchetype() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return false;
	}
	AActor* Owner = Shovel->GetOwner();
	if (Owner == nullptr)
	{
		return false;
	}
	return FActorEditorUtils::IsAPreviewOrInactiveActor(Owner);
}

void FAGX_ShovelComponentVisualizer::EndEditing()
{
	FComponentVisualizer::EndEditing();
	ClearSelection();
}

bool FAGX_ShovelComponentVisualizer::HasValidFrameSection() const
{
	return GetSelectedFrame() != nullptr;
}

FAGX_Frame* FAGX_ShovelComponentVisualizer::GetSelectedFrame() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return nullptr;
	}

	switch (SelectedFrame)
	{
		case EAGX_ShovelFrame::None:
			return nullptr;
		case EAGX_ShovelFrame::CuttingDirection:
			return &Shovel->CuttingDirection;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
			return &Shovel->CuttingEdge.Start;
		case EAGX_ShovelFrame::CuttingEdgeEnd:
			return &Shovel->CuttingEdge.End;
		case EAGX_ShovelFrame::TopEdgeBegin:
			return &Shovel->TopEdge.Start;
		case EAGX_ShovelFrame::TopEdgeEnd:
			return &Shovel->TopEdge.End;
	}

	// Unknown shovel frame selected, should never happen. Crash in unit tests, return nullptr
	// in user builds.
	AGX_CHECK(false);
	return nullptr;
}

EAGX_ShovelFrame FAGX_ShovelComponentVisualizer::GetSelectedFrameSource() const
{
	return SelectedFrame;
}

FProperty* FAGX_ShovelComponentVisualizer::GetSelectedFrameProperty() const
{
	UAGX_ShovelComponent* Shovel = GetSelectedShovel();
	if (Shovel == nullptr)
	{
		return nullptr;
	}

	switch (SelectedFrame)
	{
		case EAGX_ShovelFrame::None:
			return nullptr;
		case EAGX_ShovelFrame::CuttingDirection:
			return this->CuttingDirectionProperty;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
		case EAGX_ShovelFrame::CuttingEdgeEnd:
			return CuttingEdgeProperty;
		case EAGX_ShovelFrame::TopEdgeBegin:
		case EAGX_ShovelFrame::TopEdgeEnd:
			return TopEdgeProperty;
	}

	// Unknown shovel frame selected, should never happen. Crash in unit tests, return nullptr
	// in user builds.
	AGX_CHECK(false);
	return nullptr;
}

void FAGX_ShovelComponentVisualizer::ClearSelection()
{
	SelectedFrame = EAGX_ShovelFrame::None;
	ShovelPropertyPath.Reset();
	CachedRotation = FQuat(ForceInit);
}

UAGX_ShovelComponent* FAGX_ShovelComponentVisualizer::GetSelectedShovel() const
{
	return Cast<UAGX_ShovelComponent>(ShovelPropertyPath.GetComponent());
}

#undef LOCTEXT_NAMESPACE
#undef MEMBER
