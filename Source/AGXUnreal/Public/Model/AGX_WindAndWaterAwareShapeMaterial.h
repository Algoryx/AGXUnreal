// Copyright Aker Solutions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Model/WindAndWaterParametersEnums.h"
#include "AGX_WindAndWaterAwareShapeMaterial.generated.h"

class UAGX_WindAndWaterShapeRegistryComponent;
/**
 * Defines hydrodynamic and aerodynamic properties of AGX Shapes.
 */
USTRUCT(BlueprintType)
struct FAGX_Ext_WindAndWaterParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	double PressureDrag_ = 0.0;

	UPROPERTY(EditAnywhere)
	double ViscousDrag_ = 0.0;

	UPROPERTY(EditAnywhere)
	double Lift_ = 0.0;

	UPROPERTY(EditAnywhere)
	double Buoyancy_ = 1.0;
	
	UPROPERTY(EditAnywhere)
	EWindAndWaterShapeTessellation ShapeTessellation_ = EWindAndWaterShapeTessellation::DEFAULT_TESSELLATION;
};

UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_WindAndWaterAwareShapeMaterial : public UAGX_ShapeMaterial
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category="Material Properties | Wind And Water")
	bool bIsWaterGeometry = false;
	
	UPROPERTY(EditAnywhere, Category="Material Properties | Wind And Water", meta=(EditCondition = "!bIsWaterGeometry", EditConditionHides))
	FAGX_Ext_WindAndWaterParameters HydroParameters;

	UPROPERTY(EditAnywhere, Category="Material Properties | Wind And Water", meta=(EditCondition = "!bIsWaterGeometry", EditConditionHides))
	FAGX_Ext_WindAndWaterParameters AeroParameters;

protected:
	virtual void CopyShapeMaterialProperties(const UAGX_ShapeMaterial* Source) override;
};

