// Copyright 2024, Algoryx Simulation AB.

#include "AGXUnrealEditor.h"

// Unreal Engine includes.
#include "AssetToolsModule.h"
#include "AssetTypeCategories.h"
#include "Editor/UnrealEdEngine.h"
#include "Framework/Commands/Commands.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "IPlacementModeModule.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "UnrealEdGlobals.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"
#include "AGX_ComponentReferenceCustomization.h"
#include "AGX_EditorStyle.h"
#include "AGX_Environment.h"
#include "AGX_Frame.h"
#include "AGX_FrameCustomization.h"
#include "AGX_RigidBodyActor.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_RigidBodyComponentCustomization.h"
#include "AGX_RigidBodyReference.h"
#include "AGX_Real.h"
#include "AGX_RealDetails.h"
#include "AGX_RuntimeStyle.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "AGX_ModelSourceComponentCustomization.h"
#include "AGX_Simulation.h"
#include "AGX_SimulationCustomization.h"
#include "AGX_StaticMeshComponent.h"
#include "AGX_StaticMeshComponentCustomization.h"
#include "AGX_TopMenu.h"
#include "AgxEdMode/AGX_AgxEdMode.h"
#include "AgxEdMode/AGX_AgxEdModeConstraints.h"
#include "AgxEdMode/AGX_AgxEdModeConstraintsCustomization.h"
#include "AgxEdMode/AGX_AgxEdModeFile.h"
#include "AgxEdMode/AGX_AgxEdModeFileCustomization.h"
#include "AgxEdMode/AGX_AgxEdModeTerrain.h"
#include "AgxEdMode/AGX_AgxEdModeTerrainCustomization.h"
#include "AgxEdMode/AGX_GrabMode.h"
#include "AMOR/AGX_ConstraintMergeSplitThresholdsTypeActions.h"
#include "AMOR/AGX_ShapeContactMergeSplitThresholdsTypeActions.h"
#include "AMOR/AGX_WireMergeSplitThresholdsTypeActions.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerActor.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponentCustomization.h"
#include "CollisionGroups/AGX_CollisionGroupAdderComponent.h"
#include "CollisionGroups/AGX_CollisionGroupAdderComponentCustomization.h"
#include "Constraints/AGX_BallConstraintActor.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintBodyAttachment.h"
#include "Constraints/AGX_ConstraintBodyAttachmentCustomization.h"
#include "Constraints/AGX_ConstraintCustomization.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintComponentVisualizer.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/AGX_ConstraintFrameComponent.h"
#include "Constraints/AGX_ConstraintFrameComponentVisualizer.h"
#include "Constraints/AGX_CylindricalConstraintActor.h"
#include "Constraints/AGX_DistanceConstraintActor.h"
#include "Constraints/AGX_HingeConstraintActor.h"
#include "Constraints/AGX_LockConstraintActor.h"
#include "Constraints/AGX_PrismaticConstraintActor.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Materials/AGX_ContactMaterialAssetTypeActions.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialCustomization.h"
#include "Materials/AGX_ContactMaterialRegistrarActor.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ContactMaterialRegistrarComponentCustomization.h"
#include "Materials/AGX_ShapeMaterialAssetTypeActions.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Materials/AGX_TerrainMaterialAssetTypeActions.h"
#include "Materials/AGX_TerrainMaterialCustomization.h"
#include "Materials/AGX_MaterialLibrary.h"
#include "PlayRecord/AGX_PlayRecordTypeActions.h"
#include "Plot/AGX_PlotComponent.h"
#include "Plot/AGX_PlotComponentCustomization.h"
#include "Sensors/AGX_CameraSensorBase.h"
#include "Sensors/AGX_CameraSensorComponentCustomization.h"
#include "Sensors/AGX_CameraSensorComponentVisualizer.h"
#include "Sensors/AGX_LidarAmbientMaterialTypeActions.h"
#include "Sensors/AGX_LidarLambertianOpaqueMaterialTypeActions.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_LidarSensorComponentCustomization.h"
#include "Sensors/AGX_LidarSensorComponentVisualizer.h"
#include "Sensors/AGX_LidarSensorLineTraceComponent.h"
#include "Sensors/AGX_LidarSensorLineTraceComponentVisualizer.h"
#include "Sensors/AGX_LidarSensorReference.h"
#include "Sensors/AGX_CustomRayPatternParametersTypeActions.h"
#include "Sensors/AGX_GenericHorizontalSweepParametersTypeActions.h"
#include "Sensors/AGX_OusterOS0ParametersTypeActions.h"
#include "Sensors/AGX_OusterOS1ParametersTypeActions.h"
#include "Sensors/AGX_OusterOS2ParametersTypeActions.h"
#include "Sensors/AGX_SensorEnvironment.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_ShapeComponentCustomization.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Terrain/AGX_HeightFieldBoundsComponentCustomization.h"
#include "Terrain/AGX_HeightFieldBoundsComponentVisualizer.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/AGX_ShovelComponentVisualizer.h"
#include "Terrain/AGX_ShovelPropertiesActions.h"
#include "Terrain/AGX_ShovelReference.h"
#include "Tires/AGX_TireComponentVisualizer.h"
#include "Tires/AGX_TireComponent.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Tires/AGX_TwoBodyTireActor.h"
#include "Tires/AGX_TwoBodyTireComponentCustomization.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackComponentDetails.h"
#include "Vehicle/AGX_TrackComponentVisualizer.h"
#include "Vehicle/AGX_TrackPropertiesAssetTypeActions.h"
#include "Vehicle/AGX_TrackRenderer.h"
#include "Vehicle/AGX_TrackRendererDetails.h"
#include "Vehicle/AGX_TrackInternalMergePropertiesAssetTypeActions.h"
#include "Wire/AGX_WireActor.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/AGX_WireComponentVisualizer.h"
#include "Wire/AGX_WireDetails.h"
#include "Wire/AGX_WireWinchActor.h"
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/AGX_WireWinchDetails.h"
#include "Wire/AGX_WireWinchVisualizer.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealEditorModule"

class FAGX_GlobalKeyboardCommands : public TCommands<FAGX_GlobalKeyboardCommands>
{
public:
	FAGX_GlobalKeyboardCommands()
		: TCommands<FAGX_GlobalKeyboardCommands>(
			  TEXT("FAGXUnrealEditorModule"), LOCTEXT("FAGXUnrealEditorModule", "AGX UE Editor"),
			  NAME_None, FAGX_RuntimeStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override
	{
		auto Sim = Cast<UAGX_Simulation>(UAGX_Simulation::StaticClass()->GetDefaultObject());
		if (Sim == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("RegisterCommands failed in AGXUnrealEditor, could not get the AGX Simulation "
					 "CDO. Keyboard commands may not work as expected."));
			return;
		}

		UI_COMMAND(
			ActivateGrabCommand, "Activate AGX Grab", "Activate AGX Grab",
			EUserInterfaceActionType::Button, Sim->GrabModeKeyboardShortcut);
	}

public:
	TSharedPtr<FUICommandInfo> ActivateGrabCommand;
};

void FAGXUnrealEditorModule::StartupModule()
{
	FAGX_EditorStyle::Initialize();
	FAGX_EditorStyle::ReloadTextures();

	RegisterProjectSettings();
	RegisterCommands();
	RegisterAssetTypeActions();
	RegisterCustomizations();
	RegisterComponentVisualizers();
	RegisterModes();
	RegisterPlacementCategory();
	InitializeAssets();

	AgxTopMenu = MakeShareable(new FAGX_TopMenu());
}

void FAGXUnrealEditorModule::ShutdownModule()
{
	FAGX_EditorStyle::Shutdown();

	UnregisterCommands();
	UnregisterProjectSettings();
	UnregisterAssetTypeActions();
	UnregisterCustomizations();
	UnregisterComponentVisualizers();
	UnregisterModes();
	UnregisterPlacementCategory();

	AgxTopMenu = nullptr;
}

const TSharedPtr<FAGX_TopMenu>& FAGXUnrealEditorModule::GetAgxTopMenu() const
{
	return AgxTopMenu;
}

void FAGXUnrealEditorModule::RegisterProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project", "Plugins", "UAGX_Simulation",
			LOCTEXT("UAGX_Simulation_ProjectSettingsName", "AGX Dynamics"),
			LOCTEXT(
				"UAGX_Simulation_ProjectSettingsDesc",
				"Configure the simulation settings of the AGX Unreal plugin."),
			GetMutableDefault<UAGX_Simulation>());
	}
}

void FAGXUnrealEditorModule::UnregisterProjectSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "UAGX_Simulation");
	}
}

void FAGXUnrealEditorModule::RegisterCommands()
{
	FAGX_GlobalKeyboardCommands::Register();

	FLevelEditorModule& LevelEditorModule =
		FModuleManager::GetModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	const auto GrabModeCommands = FAGX_GlobalKeyboardCommands::Get();
	if (!LevelEditorModule.GetGlobalLevelEditorActions()->IsActionMapped(
			GrabModeCommands.ActivateGrabCommand))
	{
		LevelEditorModule.GetGlobalLevelEditorActions()->MapAction(
			GrabModeCommands.ActivateGrabCommand,
			FExecuteAction::CreateRaw(this, &FAGXUnrealEditorModule::OnGrabModeCommand),
			FCanExecuteAction::CreateRaw(
				this, &FAGXUnrealEditorModule::OnCanExecuteGrabModeCommand));
	}
}

void FAGXUnrealEditorModule::UnregisterCommands()
{
	FAGX_GlobalKeyboardCommands::Unregister();
}

void FAGXUnrealEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	EAssetTypeCategories::Type AgxAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("AgxUnreal")), LOCTEXT("AgxAssetCategory", "AGX Dynamics"));

	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAGX_ConstraintMergeSplitThresholdsTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_ContactMaterialAssetTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAGX_CustomRayPatternParametersTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAGX_GenericHorizontalSweepParametersTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_LidarAmbientMaterialTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAGX_LidarLambertianOpaqueMaterialTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_OusterOS0ParametersTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_OusterOS1ParametersTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_OusterOS2ParametersTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_PlayRecordTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_ShapeMaterialTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAGX_ShapeContactMergeSplitThresholdsTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_ShovelPropertiesActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_TerrainMaterialAssetTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAGX_TrackInternalMergePropertiesAssetTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools, MakeShareable(new FAGX_TrackPropertiesAssetTypeActions(AgxAssetCategoryBit)));
	RegisterAssetTypeAction(
		AssetTools,
		MakeShareable(new FAGX_WireMergeSplitThresholdsTypeActions(AgxAssetCategoryBit)));
}

void FAGXUnrealEditorModule::UnregisterAssetTypeActions()
{
	if (!FModuleManager::Get().IsModuleLoaded("AssetTools"))
		return;

	IAssetTools& AssetTools =
		FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();

	for (const TSharedPtr<IAssetTypeActions>& AssetTypeAction : RegisteredAssetTypeActions)
	{
		AssetTools.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
	}
}

void FAGXUnrealEditorModule::RegisterAssetTypeAction(
	IAssetTools& AssetTools, const TSharedPtr<IAssetTypeActions>& Action)
{
	AssetTools.RegisterAssetTypeActions(Action.ToSharedRef());
	RegisteredAssetTypeActions.Add(Action);
}

void FAGXUnrealEditorModule::RegisterCustomizations()
{
	FPropertyEditorModule& PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	/*
	 * Property customizations.
	 */

	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_ComponentReference::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ComponentReferenceCustomization::MakeInstance));

	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ConstraintBodyAttachmentCustomization::MakeInstance));

	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_Frame::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_FrameCustomization::MakeInstance));

	// Lidar Sensor Reference uses the base class customization.
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_LidarSensorReference::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ComponentReferenceCustomization::MakeInstance));

	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_Real::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FAGX_RealDetails::MakeInstance));

	// Body Reference uses the base class customization.
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_RigidBodyReference::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ComponentReferenceCustomization::MakeInstance));

	// Scene Component Reference uses the base class customization.
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_SceneComponentReference::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ComponentReferenceCustomization::MakeInstance));

	// Shovel Reference uses the base class customization.
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FAGX_ShovelReference::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(
			&FAGX_ComponentReferenceCustomization::MakeInstance));

	/*
	 * Class customizations.
	 */

	/// \todo I don't know if this should be AAGX_ConstraintActor or
	/// UAGX_ConstraintComponent. Should we have one for each? Which should be
	/// the new one and what should it contain/do?
	PropertyModule.RegisterCustomClassLayout(
		AAGX_ConstraintActor::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_ConstraintCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_AgxEdModeConstraints::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_AgxEdModeConstraintsCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_AgxEdModeFile::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_AgxEdModeFileCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_AgxEdModeTerrain::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_AgxEdModeTerrainCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_CameraSensorBase::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_CameraSensorComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_CollisionGroupAdderComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_CollisionGroupAdderComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_CollisionGroupDisablerComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_CollisionGroupDisablerComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_ConstraintComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_ConstraintCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_ContactMaterial::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_ContactMaterialCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_ContactMaterialRegistrarComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_ContactMaterialRegistrarComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_HeightFieldBoundsComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_HeightFieldBoundsComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_LidarSensorComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_LidarSensorComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_ModelSourceComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_ModelSourceComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_PlotComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_PlotComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_RigidBodyComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_RigidBodyComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_ShapeComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_ShapeComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_StaticMeshComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_StaticMeshComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_Simulation::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_SimulationCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_TerrainMaterial::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_TerrainMaterialCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_TrackComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_TrackComponentDetails::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_TrackRenderer::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_TrackRendererDetails::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_TwoBodyTireComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(
			&FAGX_TwoBodyTireComponentCustomization::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_WireComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_WireDetails::MakeInstance));

	PropertyModule.RegisterCustomClassLayout(
		UAGX_WireWinchComponent::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FAGX_WireWinchDetails::MakeInstance));

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealEditorModule::UnregisterCustomizations()
{
	FPropertyEditorModule& PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	/*
	 * Property customizations.
	 */

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FAGX_ComponentReference::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FAGX_ConstraintBodyAttachment::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(FAGX_Frame::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FAGX_LidarSensorReference::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(FAGX_Real::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FAGX_RigidBodyReference::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FAGX_SceneComponentReference::StaticStruct()->GetFName());

	PropertyModule.UnregisterCustomPropertyTypeLayout(
		FAGX_ShovelReference::StaticStruct()->GetFName());

	/*
	 * Class Customizations.
	 */

	/// \todo Not sure if this should be AAGX_ConstraintActor,
	/// UAGX_ConstraintComponent, or both.
	PropertyModule.UnregisterCustomClassLayout(AAGX_ConstraintActor::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_AgxEdModeConstraints::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_AgxEdModeFile::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_AgxEdModeTerrain::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_CameraSensorBase::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_CollisionGroupAdderComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_CollisionGroupDisablerComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_ContactMaterial::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_ContactMaterialRegistrarComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_HeightFieldBoundsComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_LidarSensorComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_ModelSourceComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_PlotComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_RigidBodyComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_ShapeComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_Simulation::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_TerrainMaterial::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_TrackComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_TrackRenderer::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(
		UAGX_TwoBodyTireComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_WireComponent::StaticClass()->GetFName());

	PropertyModule.UnregisterCustomClassLayout(UAGX_WireWinchComponent::StaticClass()->GetFName());

	PropertyModule.NotifyCustomizationModuleChanged();
}

void FAGXUnrealEditorModule::RegisterComponentVisualizers()
{
	RegisterComponentVisualizer(
		UAGX_CameraSensorBase::StaticClass()->GetFName(),
		MakeShareable(new FAGX_CameraSensorComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_ConstraintComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_ConstraintComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_ConstraintFrameComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_ConstraintFrameComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_HeightFieldBoundsComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_HeightFieldBoundsComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_LidarSensorComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_LidarSensorComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_LidarSensorLineTraceComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_LidarSensorLineTraceComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_ShovelComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_ShovelComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_TireComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_TireComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_TrackComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_TrackComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_WireComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_WireComponentVisualizer));

	RegisterComponentVisualizer(
		UAGX_WireWinchComponent::StaticClass()->GetFName(),
		MakeShareable(new FAGX_WireWinchVisualizer));
}

void FAGXUnrealEditorModule::UnregisterComponentVisualizers()
{
	UnregisterComponentVisualizer(UAGX_CameraSensorBase::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_ConstraintComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_ConstraintFrameComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_HeightFieldBoundsComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_LidarSensorComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_LidarSensorLineTraceComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_ShovelComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_TireComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_TrackComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_WireComponent::StaticClass()->GetFName());
	UnregisterComponentVisualizer(UAGX_WireWinchComponent::StaticClass()->GetFName());
}

void FAGXUnrealEditorModule::RegisterComponentVisualizer(
	const FName& ComponentClassName, TSharedPtr<FComponentVisualizer> Visualizer)
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->RegisterComponentVisualizer(ComponentClassName, Visualizer);
	}

	if (Visualizer.IsValid())
	{
		Visualizer->OnRegister();
	}
}

void FAGXUnrealEditorModule::UnregisterComponentVisualizer(const FName& ComponentClassName)
{
	if (GUnrealEd != nullptr)
	{
		GUnrealEd->UnregisterComponentVisualizer(ComponentClassName);
	}
}

void FAGXUnrealEditorModule::RegisterModes()
{
	FEditorModeRegistry::Get().RegisterMode<FAGX_AgxEdMode>(
		FAGX_AgxEdMode::EM_AGX_AgxEdModeId,
		LOCTEXT("AGX_AgxEdModeDisplayName", "AGX Dynamics Tools"),
		FSlateIcon(
			FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIcon,
			FAGX_EditorStyle::AgxIconSmall),
		/*bVisisble*/ true);

	FEditorModeRegistry::Get().RegisterMode<FAGX_GrabMode>(
		FAGX_GrabMode::EM_AGX_GrabModeId, LOCTEXT("AGXGrabMode", "Grab Mode"), FSlateIcon(),
		/*bVisisble*/ false);
}

void FAGXUnrealEditorModule::UnregisterModes()
{
	FEditorModeRegistry::Get().UnregisterMode(FAGX_AgxEdMode::EM_AGX_AgxEdModeId);
	FEditorModeRegistry::Get().UnregisterMode(FAGX_GrabMode::EM_AGX_GrabModeId);
}

void FAGXUnrealEditorModule::RegisterPlacementCategory()
{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FPlacementCategoryInfo PlacementCategory(LOCTEXT("DisplayName", "AGX"), "AGX", TEXT("PMAGX"));
#else
	const FSlateIcon Icon(
		FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::AgxIconTiny,
		FAGX_EditorStyle::AgxIconTiny);

	FPlacementCategoryInfo PlacementCategory(
		LOCTEXT("DisplayName", "AGX"), Icon, "AGX", TEXT("PMAGX"));
#endif
	IPlacementModeModule::Get().RegisterPlacementCategory(PlacementCategory);

	auto RegisterPlaceableItem = [&](UClass* Class)
	{
		IPlacementModeModule::Get().RegisterPlaceableItem(
			PlacementCategory.UniqueHandle,
			MakeShareable(new FPlaceableItem(nullptr, FAssetData(Class))));
	};

	RegisterPlaceableItem(AAGX_ContactMaterialRegistrarActor::StaticClass());
	RegisterPlaceableItem(AAGX_ConstraintFrameActor::StaticClass());
	RegisterPlaceableItem(AAGX_BallConstraintActor::StaticClass());
	RegisterPlaceableItem(AAGX_CylindricalConstraintActor::StaticClass());
	RegisterPlaceableItem(AAGX_DistanceConstraintActor::StaticClass());
	RegisterPlaceableItem(AAGX_HingeConstraintActor::StaticClass());
	RegisterPlaceableItem(AAGX_LockConstraintActor::StaticClass());
	RegisterPlaceableItem(AAGX_PrismaticConstraintActor::StaticClass());
	RegisterPlaceableItem(AAGX_SensorEnvironment::StaticClass());
	RegisterPlaceableItem(AAGX_Terrain::StaticClass());
	RegisterPlaceableItem(AAGX_CollisionGroupDisablerActor::StaticClass());
	RegisterPlaceableItem(AAGX_RigidBodyActor::StaticClass());
	RegisterPlaceableItem(AAGX_TwoBodyTireActor::StaticClass());
	RegisterPlaceableItem(AAGX_WireActor::StaticClass());
	RegisterPlaceableItem(AAGX_WireWinchActor::StaticClass());
}

void FAGXUnrealEditorModule::UnregisterPlacementCategory()
{
	if (IPlacementModeModule::IsAvailable())
	{
		IPlacementModeModule::Get().UnregisterPlacementCategory("AGX");
	}
}

void FAGXUnrealEditorModule::InitializeAssets()
{
	AGX_MaterialLibrary::InitializeShapeMaterialAssetLibrary();
	AGX_MaterialLibrary::InitializeContactMaterialAssetLibrary();
	AGX_MaterialLibrary::InitializeTerrainMaterialAssetLibrary();
	AGX_MaterialLibrary::InitializeLidarAmbientMaterialAssetLibrary();
}

void FAGXUnrealEditorModule::OnGrabModeCommand() const
{
	FAGX_GrabMode::Activate();
}

bool FAGXUnrealEditorModule::OnCanExecuteGrabModeCommand() const
{
	const UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	return World != nullptr && World->IsGameWorld();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealEditorModule, AGXUnrealEditor);
