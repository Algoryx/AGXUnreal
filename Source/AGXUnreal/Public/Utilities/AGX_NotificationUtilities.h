// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

/**
 * Provides helper functions for working with notifications, both when in Editor and built
 * executable.
 */
class AGXUNREAL_API FAGX_NotificationUtilities
{
public:
	/**
	 * Displays a dialog box with an OK button and adds a log message with the text. If a title is
	 * not specified, 'AGX Dynamics for Unreal' is used.
	 */
	static void ShowDialogBoxWithSuccess(const FString& Text, const FString& Title = "");

	/**
	 * Same as ShowDialogBoxWithSuccess but does not show a message box during Play.
	 */
	static void ShowDialogBoxWithSuccessInEditor(
		const FString& Text, UWorld* World, const FString& Title = "");

	/**
	 * Displays a dialog box with an OK button and adds a log message with the text. If a title is
	 * not specified, 'AGX Dynamics for Unreal' is used.
	 */
	static void ShowDialogBoxWithInfo(const FString& Text, const FString& Title = "");

	/**
	 * Same as ShowDialogBoxWithInfo but does not show a message box during Play.
	 */
	static void ShowDialogBoxWithInfoInEditor(
		const FString& Text, UWorld* World, const FString& Title = "");

	/**
	 * Displays a dialog box with an OK button and adds a warning log message with the text. If a
	 * title is not specified, 'AGX Dynamics for Unreal' is used.
	 */
	static void ShowDialogBoxWithWarning(const FString& Text, const FString& Title = "");

	/**
	 * Same as ShowDialogBoxWithWarning but does not show a message box during Play.
	 */
	static void ShowDialogBoxWithWarningInEditor(
		const FString& Text, UWorld* World, const FString& Title = "");

	/**
	 * Displays a dialog box with an OK button and add an error log message with the text. If a
	 * title is not specified, 'AGX Dynamics for Unreal' is used.
	 */
	static void ShowDialogBoxWithError(const FString& Text, const FString& Title = "");

	/**
	 * Same as ShowDialogBoxWithError but does not show a message box during Play.
	 */
	static void ShowDialogBoxWithErrorInEditor(
		const FString& Text, UWorld* World, const FString& Title = "");

	/**
	 * Logs a warning if AMOR is disabled globally (reads from AGX_Simulation CDO).
	 */
	static void LogWarningIfAmorDisabled(const FString& OwningType);

	/**
	 * Ask if an action should be performed.
	 */
	static bool YesNoQuestion(const FText& Question);

	/**
	 * Display a temporary notification "slider" in the lower right corner of the screen.
	 * It will automatically disappear after a few seconds.
	 * Multiple Notifications are stacked so that all is visible, and then disappears from the
	 * bottom when they time out.
	 * The State parameter can be used to indicate success or failure etc.
	 * The Duration parameter determines for how many seconds the Notificaion will be visible.
	 * A Output Log is always printed with the same Text, as 'Error' if the State is set to
	 * Failure, as 'Log' otherwise.
	 */
	static void ShowNotification(
		const FString& Text, SNotificationItem::ECompletionState State, float Duration = 5.f);
};
