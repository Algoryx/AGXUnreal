// Copyright Aker Solutions. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AGX_NativeOwner.h"
#include "Model/WindAndWaterControllerBarrier.h"
#include "AGX_WindAndWaterControllerSubsystemBase.generated.h"

class UAGX_WireComponent;
class UAGX_ShapeComponent;
class UAGX_DynamicWaterComponent;

/**
 * A game instance subsystem that holds the agxModel::WindAndWaterController as a singleton.
 * The native agxModel::WindAndWaterController is instanced and added to the agxSDK::Simulation during initialization.
 */
UCLASS(DisplayName="AGX Wind And Water Subsystem (Extension)", Abstract)
class AGXUNREAL_API UAGX_WindAndWaterControllerSubsystemBase : public UGameInstanceSubsystem, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	FWindAndWaterControllerBarrier* GetNative();
	
	bool AddWater(UAGX_ShapeComponent* Shape);

	bool SetEnableAerodynamics(UAGX_ShapeComponent* Shape, bool bEnabled);
	bool SetEnableAerodynamics(UAGX_WireComponent* Wire, bool bEnabled);

	void SetHydrodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterParametersCoefficient Coefficient, double Value);
	void SetHydrodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterShapeTessellation ShapeTessellation);
	void SetAerodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterParametersCoefficient Coefficient, double Value);
	void SetAerodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterShapeTessellation ShapeTessellation);

	void SetWaterWrapper(UAGX_ShapeComponent* Shape, UAGX_DynamicWaterComponent* WaterWrapper);
	void SetWaterFlowGenerator(UAGX_ShapeComponent* ParentShape, UAGX_DynamicWaterComponent* DynamicWater);
	virtual bool UpdateNativeWindAndWaterParameters(UAGX_ShapeComponent* Shape);
	
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;

	static UAGX_WindAndWaterControllerSubsystemBase* GetFrom(const UActorComponent* Component);

	static UAGX_WindAndWaterControllerSubsystemBase* GetFrom(const AActor* Actor);

private:
	FWindAndWaterControllerBarrier NativeWindAndWaterControllerBarrier;
};
