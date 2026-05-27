// Copyright 2026, Algoryx Simulation AB.

#include "Model/AGX_WindAndWaterAwareShapeMaterial.h"

void UAGX_WindAndWaterAwareShapeMaterial::CopyShapeMaterialProperties(const UAGX_ShapeMaterial* Source)
{
	Super::CopyShapeMaterialProperties(Source);
	if (auto SubSource = Cast<UAGX_WindAndWaterAwareShapeMaterial>(Source))
	{
		bIsWaterGeometry = SubSource->bIsWaterGeometry;
		HydroParameters = SubSource->HydroParameters;
		AeroParameters = SubSource->AeroParameters;
	}
}