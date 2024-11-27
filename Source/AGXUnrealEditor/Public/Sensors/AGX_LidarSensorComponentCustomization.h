// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Types/SlateEnums.h"

class FReply;
class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Lidar Sensor Component in the Editor's details panel.
 */
class AGXUNREALEDITOR_API FAGX_LidarSensorComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void OnModelComboBoxChanged(
		TSharedPtr<EAGX_LidarModel> NewModel, ESelectInfo::Type InSeletionInfo);

	const TArray<TSharedPtr<EAGX_LidarModel>>* GetAvailableModels();

	FString GetLidarModelString() const;
	static FString FromEnum(EAGX_LidarModel Model);

private:
	FReply OnCreateModelParametersAssetButtonClicked();

	IDetailLayoutBuilder* DetailBuilder {nullptr};
	TArray<TSharedPtr<EAGX_LidarModel>> AvailableModels;
};
