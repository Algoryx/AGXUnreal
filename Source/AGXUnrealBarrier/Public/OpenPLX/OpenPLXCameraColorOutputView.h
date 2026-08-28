// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "OpenPLXCameraColorOutputView.generated.h"

struct FOpenPLXCameraColorOutputViewRef;

/**
 * View into Camera Color output data received through OpenPLX.
 *
 * By default this struct references memory owned by the OpenPLX Control Interface. No Camera Color
 * output data is copied until data is requested. A newly received view is only valid until another
 * read operation reuses the underlying Control Interface buffer. Call MakePersistant before storing
 * the view for later use.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXCameraColorOutputView
{
	GENERATED_BODY()

	FOpenPLXCameraColorOutputView();
	FOpenPLXCameraColorOutputView(std::shared_ptr<FOpenPLXCameraColorOutputViewRef> Native);

	bool HasNative() const;

	/**
	 * Copy the underlying Camera Color output data into memory owned by this view.
	 * A newly received Camera Color output view references memory owned by the OpenPLX Control
	 * Interface and is only valid until another read reuses that buffer. Call this before storing
	 * the view for later use. This copies the complete Camera Color output buffer.
	 */
	bool MakePersistant();

	FOpenPLXCameraColorOutputViewRef* GetNative();
	const FOpenPLXCameraColorOutputViewRef* GetNative() const;

private:
	std::shared_ptr<FOpenPLXCameraColorOutputViewRef> NativeRef;
};
