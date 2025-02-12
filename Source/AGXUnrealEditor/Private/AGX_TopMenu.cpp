// Copyright 2024, Algoryx Simulation AB.

#include "AGX_TopMenu.h"

// AGX Dynamics for Unreal includes.
#include "AGX_EditorStyle.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AgxEdMode/AGX_AgxEdModeFile.h"
#include "AgxEdMode/AGX_GrabMode.h"
#include "AGXUnrealEditor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_BallConstraintActor.h"
#include "Constraints/AGX_CylindricalConstraintActor.h"
#include "Constraints/AGX_DistanceConstraintActor.h"
#include "Constraints/AGX_HingeConstraintActor.h"
#include "Constraints/AGX_LockConstraintActor.h"
#include "Constraints/AGX_PrismaticConstraintActor.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "AGX_Environment.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Widgets/AGX_GenerateRuntimeActivationDialog.h"
#include "Widgets/AGX_LicenseDialog.h"
#include "Widgets/AGX_OfflineActivationDialog.h"

#if __has_include("AGX_BuildInfo.h")
#include "AGX_BuildInfo.h"
#else
#define AGXUNREAL_HAS_GIT_BRANCH 0
#define AGXUNREAL_HAS_GIT_TAG 0
#define AGXUNREAL_HAS_GIT_HASH 0
#endif

// Unreal Engine includes.
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GenericPlatform/GenericPlatformProcess.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#define LOCTEXT_NAMESPACE "FAGX_TopMenu"

FAGX_TopMenu::FAGX_TopMenu()
	: Extender(nullptr)
	, UnrealMenuBarExtension(nullptr)
{
	// Get Prerequisites.

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtensibilityManager> ExtensibilityManager =
		LevelEditorModule.GetMenuExtensibilityManager();

	// Create our Unreal Main Menu extender, and its callback delegate.

	Extender = MakeShareable(new FExtender());

	FMenuBarExtensionDelegate UnrealMenuBarExtensionDelegate =
		FMenuBarExtensionDelegate::CreateStatic(&FAGX_TopMenu::CreateTopMenu);

	UnrealMenuBarExtension = Extender->AddMenuBarExtension(
		"Help", EExtensionHook::Before,
		nullptr, // Using inline FActions instead of FUICommands, for less hot reloading problems!
		UnrealMenuBarExtensionDelegate); // Delegate is only invoked during Editor startup (i.e.
										 // when Unreal Main Menu Bar is built).

	ExtensibilityManager->AddExtender(Extender);
}

FAGX_TopMenu::~FAGX_TopMenu()
{
	// Get prerequisites.

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtensibilityManager> ExtensibilityManager =
		LevelEditorModule.GetMenuExtensibilityManager();

	// Cleanup.

	if (Extender)
	{
		if (UnrealMenuBarExtension)
			Extender->RemoveExtension(UnrealMenuBarExtension.ToSharedRef());

		if (ExtensibilityManager.IsValid())
			ExtensibilityManager->RemoveExtender(Extender);
	}

	UnrealMenuBarExtension = nullptr;
	Extender = nullptr;
}

/*static*/ void FAGX_TopMenu::CreateTopMenu(FMenuBarBuilder& Builder)
{
	// The reason why we use the complicated approach below is because of the
	// following problem in Unreal:
	//
	// If this module is recompiled and reloaded without restarting the Editor,
	// the old Unreal Main Menu Bar extension, its callback delegate, the AGX Top Menu
	// (header of the pull down menu only), and its callback delegate (for filling it,
	// see below) seem to still be in use. This is because Unreal only rebuilds its
	// Main Menu Bar during Editor startup (as it seems).
	//
	// Our first loaded Main Menu Bar extension callback delegate is invoked once, during
	// Editor startup, and then never again, even if the module is reloaded. And the
	// AGX Top Menu delegate callback is invoked each time AGX Top Menu is clicked, but with
	// the first loaded delegate, which is incorrect if we have recompiled our module.
	//
	// So, without our hack below, after module recompile the old AGX Top Menu delegate will
	// still point to a member function in the old instance of FAGX_TopMenu, which will quickly
	// lead to crashes. Furthermore, static functions such as FAGX_TopMenuCommands::Get, which
	// are frequently used in the delegate implementations, will be executed on static objects
	// in the old dll module (which still exists, Unreal just adds a new one and "abandons"
	// the old one).
	//
	// To work around this, we need to make sure that callbacks are:
	// 1. Using correct instance of FAGX_TopMenu (i.e. the this pointer), and
	// 2. Are executed in the context of the new dll module so that correct static objects are used.
	//
	// Hacky solution:
	//
	// 1. Is solved by fetching the new correct instance of FAGX_TopMenu from
	// FAGXUnrealEditorModule, and invoke our target function (FillTopMenu) on that instance.
	//
	// 2. Is solved by making our target function (FillTopMenu) virtual so that the v-table picks
	// the implementation of the function that exists in the new dll module, and therefore is
	// exectued in the next dll module context.
	//
	// Limitations:
	//
	// Can only make changes to FillTopMenu and sub-menues without resterating the Editor,
	// (e.g. cannot change name or position of the AGX Top Menu).
	//

	FNewMenuDelegate NewMenuDelegate = FNewMenuDelegate::CreateLambda(
		[](FMenuBuilder& Builder)
		{
			if (FAGXUnrealEditorModule* AGXUnrealEditorModule =
					FModuleManager::GetModulePtr<FAGXUnrealEditorModule>("AGXUnrealEditor"))
			{
				if (TSharedPtr<FAGX_TopMenu> AgxTopMenu = AGXUnrealEditorModule->GetAgxTopMenu())
				{
					AgxTopMenu->FillTopMenu(Builder);
				}
			}
		});

	Builder.AddPullDownMenu(
		LOCTEXT("TopMenuLabel", "AGX"), LOCTEXT("TopMenuToolTip", "Open the AGX top menu"),
		NewMenuDelegate); // Delegate is invoked when AGX menu is clicked (but same delegate
						  // regardless of reloading this module).
}

/*virtual*/ void FAGX_TopMenu::FillTopMenu(FMenuBuilder& Builder)
{
	{
		const FSlateIcon FileIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::FileIconSmall,
			FAGX_EditorStyle::FileIconSmall);
		Builder.AddSubMenu(
			LOCTEXT("FileMenuLabel", "File"),
			LOCTEXT(
				"FileMenuTooltip",
				"Interoperability with external file formats, such AGX Dynamics files (.agx) "
				"or URDF files (.urdf)."),
			FNewMenuDelegate::CreateRaw(this, &FAGX_TopMenu::FillFileMenu), false, FileIcon);
	}

	Builder.AddMenuSeparator();

	{
		const FSlateIcon ConstraintIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::JointIconSmall,
			FAGX_EditorStyle::JointIconSmall);
		Builder.AddSubMenu(
			LOCTEXT("ConstraintMenuLabel", "Constraints"),
			LOCTEXT("ConstraintMenuTooltip", "Create a constraint."),
			FNewMenuDelegate::CreateRaw(this, &FAGX_TopMenu::FillConstraintMenu), false,
			ConstraintIcon);
	}

	Builder.AddMenuSeparator();

	{
		const FSlateIcon LicenseIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::LicenseKeyIcon,
			FAGX_EditorStyle::LicenseKeyIcon);
		Builder.AddSubMenu(
			LOCTEXT("LicenseMenuLabel", "License"),
			LOCTEXT("LicenseMenuTooltip", "Manage your AGX Dynamics for Unreal license."),
			FNewMenuDelegate::CreateRaw(this, &FAGX_TopMenu::FillLicenseMenu), false, LicenseIcon);
	}

	Builder.AddMenuSeparator();

	{
		const FSlateIcon GrabIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::GrabIcon,
			FAGX_EditorStyle::GrabIcon);
		Builder.AddSubMenu(
			LOCTEXT("GrabModeMenuLabel", "Grab Mode"),
			LOCTEXT(
				"GrabModeMenuTooltip",
				"Activate/Deactivate the Grab mode, making it possible to grab a "
				"Rigid Body in the Level by clicking and dragging the mouse in the viewport."),
			FNewMenuDelegate::CreateRaw(this, &FAGX_TopMenu::FillGrabModeMenu), false, GrabIcon);
	}

	Builder.AddMenuSeparator();

	{
		const FSlateIcon AgxIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIconSmall,
			FAGX_EditorStyle::AgxIconSmall);
		Builder.AddMenuEntry(
			LOCTEXT("DemoProjectsLabel", "Demo Projects"),
			LOCTEXT("DemoProjectsToolTip", "Visit the demo projects download page."), AgxIcon,
			FExecuteAction::CreateRaw(this, &FAGX_TopMenu::OnVisitDemoPageClicked), NAME_None,
			EUserInterfaceActionType::Button);
	}

	{
		const FSlateIcon AgxIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIconSmall,
			FAGX_EditorStyle::AgxIconSmall);
		Builder.AddMenuEntry(
			LOCTEXT("UserManualLabel", "User Manual"),
			LOCTEXT("UserManualToolTip", "Visit the user manual page."), AgxIcon,
			FExecuteAction::CreateRaw(this, &FAGX_TopMenu::OnVisitUserManualPageClicked), NAME_None,
			EUserInterfaceActionType::Button);
	}

	{
		const FSlateIcon AgxIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIconSmall,
			FAGX_EditorStyle::AgxIconSmall);
		Builder.AddMenuEntry(
			LOCTEXT("AboutAgxDialogLabel", "About..."),
			LOCTEXT("AboutAgxDialogToolTip", "Open the About AGX Window."), AgxIcon,
			FExecuteAction::CreateRaw(this, &FAGX_TopMenu::OnOpenAboutDialogClicked), NAME_None,
			EUserInterfaceActionType::Button);
	}
}

void FAGX_TopMenu::FillConstraintMenu(FMenuBuilder& Builder)
{
	AddFileMenuEntry(
		Builder, LOCTEXT("CreateBallConstraintLabel", "Create Ball Constraint"),
		LOCTEXT(
			"CreateBallConstraintTooltip",
			"Create Ball Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]()
		{ FAGX_TopMenu::OnCreateConstraintClicked(AAGX_BallConstraintActor::StaticClass()); });

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateCylindricalConstraintLabel", "Create Cylindrical Constraint"),
		LOCTEXT(
			"CreateCylindricalConstraintTooltip",
			"Create Cylindrical Constraint. \n\nInitially setup using currently selected Rigid "
			"Body Actors, or empty."),
		[&]() {
			FAGX_TopMenu::OnCreateConstraintClicked(AAGX_CylindricalConstraintActor::StaticClass());
		});

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateDistanceConstraintLabel", "Create Distance Constraint"),
		LOCTEXT(
			"CreateDistanceConstraintTooltip",
			"Create Distance Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]()
		{ FAGX_TopMenu::OnCreateConstraintClicked(AAGX_DistanceConstraintActor::StaticClass()); });

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateHingeConstraintLabel", "Create Hinge Constraint"),
		LOCTEXT(
			"CreateHingeConstraintTooltip",
			"Create Hinge Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]()
		{ FAGX_TopMenu::OnCreateConstraintClicked(AAGX_HingeConstraintActor::StaticClass()); });

	AddFileMenuEntry(
		Builder, LOCTEXT("CreateLockConstraintLabel", "Create Lock Constraint"),
		LOCTEXT(
			"CreateLockConstraintTooltip",
			"Create Lock Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]()
		{ FAGX_TopMenu::OnCreateConstraintClicked(AAGX_LockConstraintActor::StaticClass()); });

	AddFileMenuEntry(
		Builder, LOCTEXT("CreatePrismaticConstraintLabel", "Create Prismatic Constraint"),
		LOCTEXT(
			"CreatePrismaticConstraintTooltip",
			"Create Prismatic Constraint. \n\nInitially setup using currently selected Rigid Body "
			"Actors, or empty."),
		[&]()
		{ FAGX_TopMenu::OnCreateConstraintClicked(AAGX_PrismaticConstraintActor::StaticClass()); });
}

void FAGX_TopMenu::FillFileMenu(FMenuBuilder& Builder)
{
	AddFileMenuEntry(
		Builder, LOCTEXT("FileMEnuEntryLabelImportBluePrint", "Import model to Blueprint..."),
		LOCTEXT(
			"FileMenuEntryhTooltopImportBluePrint",
			"Import an AGX Dynamics archive or URDF to a Blueprint."),
		[]() { UAGX_AgxEdModeFile::ImportToBlueprint(); });

	// Export AGX Archive menu item
	AddFileMenuEntry(
		Builder,
		LOCTEXT(
			"FileMenuEntryLabelEx", "Export Play-In-Editor Session to an AGX Dynamics Archive..."),
		LOCTEXT(
			"FileMenuEntryToolTipEx",
			"Export an AGX Archive from the Editor. A Play-In-Editor session must be active."),
		[]() { UAGX_AgxEdModeFile::ExportAgxArchive(); });
}

void FAGX_TopMenu::FillLicenseMenu(FMenuBuilder& Builder)
{
	AddFileMenuEntry(
		Builder, LOCTEXT("ActivateLicenseMenuLabel", "Activate service license..."),
		LOCTEXT(
			"ActivateLicenseMenuLabelToolTip",
			"Activate your AGX Dynamics for Unreal service license."),
		[&]() { FAGX_TopMenu::OnOpenLicenseActivationDialogClicked(); });

	AddFileMenuEntry(
		Builder, LOCTEXT("OfflineActivationMenuLabel", "Offline service license activation..."),
		LOCTEXT(
			"OfflineActivationMenuLabelToolTip",
			"Perform offline activation of a service license."),
		[&]() { FAGX_TopMenu::OnOpenOfflineActivationDialogClicked(); });

	AddFileMenuEntry(
		Builder, LOCTEXT("RuntimeActivationMenuLabel", "Generate runtime activation..."),
		LOCTEXT(
			"RuntimeActivationMenuLabelToolTip",
			"Generate an AGX Dynamics for Unreal runtime activation file bound to an application."),
		[&]() { FAGX_TopMenu::OnOpenGenerateRuntimeActivationDialogClicked(); });
}

void FAGX_TopMenu::FillGrabModeMenu(FMenuBuilder& Builder)
{
	AddFileMenuEntry(
		Builder, LOCTEXT("StartGrabModeMenuLabel", "Start"),
		LOCTEXT(
			"StartGrabModeMenuLabelToolTip",
			"Activate the Grab mode, making it possible to grab a "
			"Rigid Body in the Level by clicking and dragging the mouse in the viewport."),
		[&]() { FAGX_TopMenu::OnStartGrabModeDialogClicked(); });

	AddFileMenuEntry(
		Builder, LOCTEXT("StopGrabModeMenuLabel", "Stop"),
		LOCTEXT("StopGrabModeMenuLabelToolTip", "Deactivate the Grab mode."),
		[&]() { FAGX_TopMenu::OnStopGrabModeDialogClicked(); });
}

void FAGX_TopMenu::OnCreateConstraintClicked(UClass* ConstraintClass)
{
	AActor* Actor1 = nullptr;
	AActor* Actor2 = nullptr;
	FAGX_EditorUtilities::GetRigidBodyActorsFromSelection(
		&Actor1, &Actor2,
		/*bSearchSubtrees*/ true, /*bSearchAncestors*/ true);

	if (Actor1 == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Must select at least one actor with a Rigid Body component before creating a "
			"constraint.");
		return;
	}

	/// \todo Figure out how to setup constraint creation so that we can pick a
	/// single UAGX_RigidBodyComponent from the selected Actors. There is very
	/// similar code in AGX_AgxEdModeConstraints.cpp.

	TArray<UAGX_RigidBodyComponent*> Bodies1 = UAGX_RigidBodyComponent::GetFromActor(Actor1);
	TArray<UAGX_RigidBodyComponent*> Bodies2;
	if (Actor2)
	{
		Bodies2 = UAGX_RigidBodyComponent::GetFromActor(Actor2);
	}

	if (Bodies1.Num() != 1)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Cannot create constraint with actor '%s' because it doesn't contain exactly one "
			"body.");
		return;
	}

	if (Actor2 && Bodies2.Num() != 1)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Cannot create constraint with actor '%s' because it doesn't contain exactly one "
			"body.");
		return;
	}

	UAGX_RigidBodyComponent* Body1 = Bodies1[0];
	UAGX_RigidBodyComponent* Body2 = Actor2 != nullptr ? Bodies2[0] : nullptr;
	AActor* Constraint = FAGX_EditorUtilities::CreateConstraintActor(
		ConstraintClass, Body1, Body2,
		/*Select*/ true,
		/*ShowNotification*/ true,
		/*InPlayingWorldIfAvailable*/ true);

	// Place the Constraint actor right between the two Rigid Bodies, or in the case of a single
	// Rigid Body: at Body1's location.
	if (Constraint)
	{
		FVector NewLocation = Body1->GetComponentLocation();
		if (Body2)
		{
			NewLocation = (NewLocation + Body2->GetComponentLocation()) / 2;
		}

		Constraint->SetActorLocation(NewLocation);
	}
}

void FAGX_TopMenu::OnVisitDemoPageClicked()
{
	static constexpr auto DemoPageUrl =
		TEXT("https://us.download.algoryx.se/AGXUnreal/demo_projects/");
	FPlatformProcess::LaunchURL(DemoPageUrl, NULL, NULL);
}

void FAGX_TopMenu::OnVisitUserManualPageClicked()
{
	static constexpr auto UserManualPageUrl =
		TEXT("https://us.download.algoryx.se/AGXUnreal/documentation/current/index.html");
	FPlatformProcess::LaunchURL(UserManualPageUrl, NULL, NULL);
}

void FAGX_TopMenu::OnOpenAboutDialogClicked()
{
	const FString Title = "About AGX Dynamics for Unreal";
	const FString Version = FAGX_Environment::GetPluginVersion();

	FString LicenseText;
	FString LicenseStatus;
	if (FAGX_Environment::GetInstance().EnsureAGXDynamicsLicenseValid(&LicenseStatus) == false)
	{
		LicenseText =
			"AGX Dynamics license: Invalid\n"
			"Status: " +
			LicenseStatus + "\n\n";
	}
	else
	{
		LicenseText = "AGX Dynamics license: Valid\n";
	}

	// clang-format off
	const FString Message(
		"\n"
		"AGX Dynamics for Unreal\n"
		"Version: " + Version + "\n"
		"Revision: " + FAGX_Environment::GetPluginRevision() + "\n"
		"\n"
		"AGX Dynamics version: " + FAGX_Environment::GetAGXDynamicsVersion() + "\n"
		+ LicenseText + "\n"
		"Copyright Algoryx Simulation AB\n"
		"www.algoryx.com");
	// clang-format on

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(Message, Title);
}

void FAGX_TopMenu::OnOpenLicenseActivationDialogClicked()
{
	TSharedRef<SWindow> Window =
		SNew(SWindow)
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.SizingRule(ESizingRule::Autosized)
			.Title(
				NSLOCTEXT("AGX", "AGXUnrealLicense", "Activate AGX Dynamics for Unreal license"));

	TSharedRef<SAGX_LicenseDialog> LicenseDialog = SNew(SAGX_LicenseDialog);
	Window->SetContent(LicenseDialog);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);
}

void FAGX_TopMenu::OnOpenOfflineActivationDialogClicked()
{
	TSharedRef<SWindow> Window =
		SNew(SWindow)
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.SizingRule(ESizingRule::Autosized)
			.Title(NSLOCTEXT("AGX", "AGXUnrealLicense", "Offline activation of service license"));

	TSharedRef<SAGX_OfflineActivationDialog> Dialog = SNew(SAGX_OfflineActivationDialog);
	Window->SetContent(Dialog);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);
}

void FAGX_TopMenu::OnOpenGenerateRuntimeActivationDialogClicked()
{
	TSharedRef<SWindow> Window =
		SNew(SWindow)
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.SizingRule(ESizingRule::Autosized)
			.Title(NSLOCTEXT("AGX", "AGXUnrealLicense", "Generate runtime activation"));

	TSharedRef<SAGX_GenerateRuntimeActivationDialog> Dialog =
		SNew(SAGX_GenerateRuntimeActivationDialog);
	Window->SetContent(Dialog);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);
}

void FAGX_TopMenu::OnStartGrabModeDialogClicked()
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	if (World == nullptr)
		return;

	if (!World->IsGameWorld())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Grab mode if only supported during play.");
		return;
	}

	FAGX_GrabMode::Activate();
}

void FAGX_TopMenu::OnStopGrabModeDialogClicked()
{
	FAGX_GrabMode::Deactivate();
}

#undef LOCTEXT_NAMESPACE
