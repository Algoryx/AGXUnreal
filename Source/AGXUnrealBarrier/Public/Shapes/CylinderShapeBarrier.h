// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/ShapeBarrier.h"

#include "CylinderShapeBarrier.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FCylinderShapeBarrier : public FShapeBarrier
{
	GENERATED_BODY()

	FCylinderShapeBarrier();
	FCylinderShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native);

	void SetHeight(double Height);
	double GetHeight() const;

	void SetRadius(double Height);
	double GetRadius() const;

	void SetPulleyProperty(bool bInPulley);
	bool GetPulleyProperty() const;
	void RemovePulleyProperty();

	void SetGypsyProperty(bool bInGypsy);
	bool GetGypsyProperty() const;
	void RemoveGypsyProperty();

private:
	virtual void AllocateNativeShape() override;
	virtual void ReleaseNativeShape() override;
};
