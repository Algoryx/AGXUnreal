// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "EdMode.h"

struct FViewportCursorLocation;

class FAGX_ClickDragMode : public FEdMode
{
public:

	// ~Begin FEdMode interface.
	virtual bool InputKey(
		FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key,
		EInputEvent Event) override;

	virtual bool CapturedMouseMove(
		FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX,
		int32 InMouseY) override;

	virtual bool GetCursor(EMouseCursor::Type& OutCursor) const override;

	virtual bool IsCompatibleWith(FEditorModeID OtherModeID) const override;

	virtual void Tick(FEditorViewportClient* ViewportClient, float DeltaTime) override;

	virtual bool LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport) override;
	// ~End FEdMode interface.

protected:
	virtual void OnMouseClickComponent(
		UPrimitiveComponent* Component, const FVector& WorldLocation,
		const FViewportCursorLocation& CursorInfo)
		PURE_VIRTUAL(FAGX_ClickDragMode::OnMouseClickComponent, );

	virtual void OnDeactivateMode() PURE_VIRTUAL(FAGX_ClickDragMode::OnDeactivateMode, );

	virtual void OnMouseDrag(const FViewportCursorLocation& InCursorInfo)
		PURE_VIRTUAL(FAGX_ClickDragMode::OnMouseDrag, );

	virtual void OnEndMouseDrag() PURE_VIRTUAL(FAGX_ClickDragMode::OnEndMouseDrag, );

private:
	enum class EAGX_ClickDragState
	{
		Inactive,
		Click,
		Drag
	};

	EAGX_ClickDragState ClickDragState {EAGX_ClickDragState::Inactive};

	void SetClickDragInactive();
};
