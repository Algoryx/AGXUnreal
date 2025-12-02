// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireWinchComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Layout/Visibility.h"

class UAGX_WireWinchComponent;

class IDetailLayoutBuilder;

class FAGX_WireWinchDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

public:
#if 0
	EVisibility WithNative() const;
	EVisibility WithoutNative() const;
#endif

public:
	TWeakObjectPtr<UAGX_WireWinchComponent> WireWinch;
};
