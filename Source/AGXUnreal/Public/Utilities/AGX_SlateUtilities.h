// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


/**
 * Provides helper functions for working with Slate.
 */
class AGXUNREAL_API FAGX_SlateUtilities
{
public:

	/**
	 * Removes the first found child of the specified type, if possible.
	 * The search order is undefined.
	 *
	 * This function should probably only be used if you already know that
	 * there is only one component of the specified type in the subtree.
	 *
	 * Note that this function can only remove the widget if its parent is a
	 * SHorizontalBox, SVerticalBox, or SBoxPanel.
	 */
	static bool RemoveChildWidgetByType(const TSharedPtr<class SWidget>& Parent, const class FString &TypeNameToRemove, bool Recursive = true);

	static void LogChildWidgets(const TSharedPtr<class SWidget>& Parent, bool Recursive = true, const FString &Prefix = "");
};