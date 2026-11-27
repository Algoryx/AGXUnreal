// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_SceneCaptureComponent2DReference.h"

// Unreal Engine includes.
#include "Components/SceneCaptureComponent2D.h"

FAGX_SceneCaptureComponent2DReference::FAGX_SceneCaptureComponent2DReference()
	: FAGX_ComponentReference(USceneCaptureComponent2D::StaticClass())
{
}

USceneCaptureComponent2D* FAGX_SceneCaptureComponent2DReference::GetSceneCaptureComponent2D() const
{
	return Super::GetComponent<USceneCaptureComponent2D>();
}

// Blueprint API

void UAGX_SceneCaptureComponent2DReference_FL::SetSceneCaptureComponent2D(
	FAGX_SceneCaptureComponent2DReference& Reference, USceneCaptureComponent2D* Component)
{
	Reference.SetComponent(Component);
}

USceneCaptureComponent2D* UAGX_SceneCaptureComponent2DReference_FL::GetSceneCaptureComponent2D(
	FAGX_SceneCaptureComponent2DReference& Reference)
{
	return Cast<USceneCaptureComponent2D>(Reference.GetComponent());
}
