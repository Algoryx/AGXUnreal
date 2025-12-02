// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Types/SlateEnums.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

class UAGX_CollisionGroupDisablerComponent;

/**
 * Defines the design of the Collision Group Disabler Component object in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_CollisionGroupDisablerComponentCustomization
	: public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void OnComboBoxChanged(
		TSharedPtr<FName> NewSelectedItem, ESelectInfo::Type InSeletionInfo,
		UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler, FName* SelectedGroup);

	void UpdateAvailableCollisionGroups(
		const UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler);

	void AddComboBox(
		IDetailCategoryBuilder& CategoryBuilder,
		UAGX_CollisionGroupDisablerComponent* CollisionGroupDisabler, FText Name,
		FName* SelectedGroup);

private:
	TArray<TSharedPtr<FName>> AvailableCollisionGroups;
};
