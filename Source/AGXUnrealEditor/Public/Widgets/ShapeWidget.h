// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "PhysicsEngine/ShapeElem.h"
#include "Widgets/SCompoundWidget.h"

struct FAGX_Shape;

class SShapeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShapeWidget)
		: _Shape(nullptr)
		, _ShapeType(EAggCollisionShape::Unknown)
	{
	}

	SLATE_ARGUMENT(FAGX_Shape*, Shape)
	SLATE_ARGUMENT(EAggCollisionShape::Type, ShapeType)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArguments);
};
