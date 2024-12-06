// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AgxEdMode/AGX_ClickDragMode.h"
#include "Constraints/LockJointBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"


struct FViewportCursorLocation;

class FAGX_GrabMode : public FAGX_ClickDragMode
{
public:
	const static FEditorModeID EM_AGX_GrabModeId;

	static void Activate();

	static void Deactivate();

protected:

	// ~Begin FAGX_ClickDragMode interface.
	virtual void OnMouseClickComponent(
		UPrimitiveComponent* Component, const FVector& WorldLocation,
		const FViewportCursorLocation& CursorInfo) override;

	virtual void OnDeactivateMode() override;

	virtual void OnMouseDrag(const FViewportCursorLocation& CursorInfo) override;

	virtual void OnEndMouseDrag() override;
	// ~End FAGX_ClickDragMode interface.

	// ~Begin FEdMode interface.
	virtual bool GetCursor(EMouseCursor::Type& OutCursor) const override;
	// ~End FEdMode interface.

private:
	TWeakObjectPtr<UAGX_RigidBodyComponent> Body;
	FVector ForceOriginLocalPos {FVector::ZeroVector};
	double ForceOriginInitialDistance {0.0};
	double Force {0.0};

	/** The window that owns the decorator widget */
	TSharedPtr<SWindow> CursorDecoratorWindow;

	bool IsHoldingBody() const;
	FText GetCursorDecoratorText() const;
	void UpdateCursorDecorator();
	void DestroyCursorDecorator();

	void DestroyLockConstraint();

	FLockJointBarrier LockConstraint;
};
