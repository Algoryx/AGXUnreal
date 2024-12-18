
// Copyright 2024, Algoryx Simulation AB.

#include "AGX_EditorStyle.h"

// Unreal Engine includes.
#include "Framework/Application/SlateApplication.h"
#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Runtime/SlateCore/Public/Styling/SlateStyle.h"
#include "Runtime/SlateCore/Public/Styling/SlateStyleRegistry.h"
#include "Runtime/SlateCore/Public/Styling/SlateTypes.h"
#include "SlateOptMacros.h"

TSharedPtr<FSlateStyleSet> FAGX_EditorStyle::StyleInstance = nullptr;
const FName FAGX_EditorStyle::AgxIcon("AgxIcon");
const FName FAGX_EditorStyle::AgxIconSmall("AgxIcon.Small");
const FName FAGX_EditorStyle::AgxIconTiny("AgxIcon.Tiny");
const FName FAGX_EditorStyle::GrabIcon("GrabIcon");
const FName FAGX_EditorStyle::FileIcon("FileIcon");
const FName FAGX_EditorStyle::FileIconSmall("FileIcon.Small");
const FName FAGX_EditorStyle::JointIcon("JointIcon");
const FName FAGX_EditorStyle::JointIconSmall("JointIcon.Small");
const FName FAGX_EditorStyle::LicenseKeyIcon("LicenseKey");
const FName FAGX_EditorStyle::TerrainIcon("TerrainIcon");
const FName FAGX_EditorStyle::TerrainIconSmall("TerrainIcon.Small");



void FAGX_EditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAGX_EditorStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance.Get());
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

void FAGX_EditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedPtr<class ISlateStyle> FAGX_EditorStyle::Get()
{
	return StyleInstance;
}

FName FAGX_EditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AGX_EditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH(RelativePath, ...) \
	FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) \
	FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) \
	FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelateivePath, ...) \
	FSlateFontInfo(Style->RootToContentDi(RelateivePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelateivePath, ...) \
	FSlateFontInfo(Style->RootToContentDir(RelateivePath, TEXT(".otf")), __VA_ARGS__)

namespace
{
	const FVector2D IconSize16(16.0f, 16.0f);
	const FVector2D IconSize32(32.0f, 32.0f);
	const FVector2D IconSize40(40.0f, 40.0f);
	const FVector2D IconSize64(64.0f, 64.0f);
	const FVector2D IconSize128(128.0f, 128.0f);
}

TSharedRef<class FSlateStyleSet> FAGX_EditorStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(
		IPluginManager::Get().FindPlugin("AGXUnreal")->GetContentDir() / TEXT("Editor"));

	// Define icons and stuff here.

	Style->Set(AgxIcon, new IMAGE_BRUSH("Icons/symbol_white_64x64", IconSize64));
	Style->Set(AgxIconSmall, new IMAGE_BRUSH("Icons/symbol_white_32x32", IconSize32));
	Style->Set(AgxIconTiny, new IMAGE_BRUSH("Icons/symbol_white_32x32", IconSize16));
	Style->Set(JointIcon, new IMAGE_BRUSH("Icons/constraint_64x64", IconSize64));
	Style->Set(JointIconSmall, new IMAGE_BRUSH("Icons/constraint_32x32", IconSize32));
	Style->Set(FileIcon, new IMAGE_BRUSH("Icons/file_64x64", IconSize64));
	Style->Set(FileIconSmall, new IMAGE_BRUSH("Icons/file_32x32", IconSize32));
	Style->Set(TerrainIcon, new IMAGE_BRUSH("Icons/Terrain_64x64", IconSize64));
	Style->Set(TerrainIconSmall, new IMAGE_BRUSH("Icons/Terrain_32x32", IconSize32));
	Style->Set(LicenseKeyIcon, new IMAGE_BRUSH("Icons/license_key_16x16", IconSize16));
	Style->Set(GrabIcon, new IMAGE_BRUSH("Icons/grab_32x32", IconSize16));

	// Component icons, visible in the Components list/hierarchy of an Actor and in the Add
	// Component dialog / list. Bodies.
	Style->Set(
		"ClassIcon.AGX_RigidBodyComponent", new IMAGE_BRUSH("Icons/rigid_body_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_StaticMeshComponent",
		new IMAGE_BRUSH("Icons/static_mesh_32x32", IconSize16));
	// Collisions
	Style->Set(
		"ClassIcon.AGX_CollisionGroupAdderComponent",
		new IMAGE_BRUSH("Icons/collision_group_disable_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CollisionGroupDisablerComponent",
		new IMAGE_BRUSH("Icons/collision_group_disable_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ContactMaterialRegistrarComponent",
		new IMAGE_BRUSH("Icons/contact_material_register_32x32", IconSize16));
	// Constraints.
	Style->Set(
		"ClassIcon.AGX_Constraint1DofComponent",
		new IMAGE_BRUSH("Icons/constraint_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_Constraint2DofComponent",
		new IMAGE_BRUSH("Icons/constraint_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ConstraintComponent", new IMAGE_BRUSH("Icons/constraint_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ConstraintFrameComponent",
		new IMAGE_BRUSH("Icons/constraint_frame_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CylindricalConstraintComponent",
		new IMAGE_BRUSH("Icons/constraint_cylindrical_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_DistanceConstraintComponent",
		new IMAGE_BRUSH("Icons/constraint_distance_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_HingeConstraintComponent", new IMAGE_BRUSH("Icons/hinge_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_LockConstraintComponent",
		new IMAGE_BRUSH("Icons/constraint_lock_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_PrismaticConstraintComponent",
		new IMAGE_BRUSH("Icons/constraint_prismatic_32x32", IconSize16));
	// Observer Frame.
	Style->Set(
		"ClassIcon.AGX_ObserverFrameComponent",
		new IMAGE_BRUSH("Icons/observer_frame_32x32", IconSize16));
	// Plots.
	Style->Set("ClassIcon.AGX_PlotComponent", new IMAGE_BRUSH("Icons/plot_32x32", IconSize16));
	// ROS2.
	Style->Set(
		"ClassIcon.AGX_ROS2AnyMessageBuilderComponent",
		new IMAGE_BRUSH("Icons/ros2_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ROS2AnyMessageParserComponent",
		new IMAGE_BRUSH("Icons/ros2_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ROS2PublisherComponent", new IMAGE_BRUSH("Icons/ros2_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ROS2SubscriberComponent", new IMAGE_BRUSH("Icons/ros2_32x32", IconSize16));
	// Shapes.
	Style->Set(
		"ClassIcon.AGX_BoxShapeComponent", new IMAGE_BRUSH("Icons/box_shape_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CapsuleShapeComponent",
		new IMAGE_BRUSH("Icons/capsule_shape_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CylinderShapeComponent",
		new IMAGE_BRUSH("Icons/cylinder_shape_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_HeightFieldShapeComponent",
		new IMAGE_BRUSH("Icons/height_field_shape_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_SphereShapeComponent",
		new IMAGE_BRUSH("Icons/sphere_shape_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_TrimeshShapeComponent",
		new IMAGE_BRUSH("Icons/trimesh_shape_32x32", IconSize16));
	// Sensors.
	Style->Set(
		"ClassIcon.AGX_CameraSensor8BitComponent",
		new IMAGE_BRUSH("Icons/camera_sensor_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CameraSensor16BitComponent",
		new IMAGE_BRUSH("Icons/camera_sensor_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_LidarSensorComponent", new IMAGE_BRUSH("Icons/lidar_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_LidarSensorLineTraceComponent",
		new IMAGE_BRUSH("Icons/lidar_32x32", IconSize16));
	// Tire.
	Style->Set(
		"ClassIcon.AGX_TwoBodyTireComponent",
		new IMAGE_BRUSH("Icons/two_wheel_tire_32x32", IconSize16));
	// Terrain.
	Style->Set(
		"ClassIcon.AGX_CuttingDirectionComponent",
		new IMAGE_BRUSH("Icons/deprecated_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ShovelComponent",
		new IMAGE_BRUSH("Icons/deformable_terrain_shovel_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CuttingEdgeComponent",
		new IMAGE_BRUSH("Icons/deprecated_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_TopEdgeComponent", new IMAGE_BRUSH("Icons/deprecated_32x32", IconSize16));
	// Track.
	Style->Set("ClassIcon.AGX_TrackComponent", new IMAGE_BRUSH("Icons/track_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_TrackRenderer", new IMAGE_BRUSH("Icons/deprecated_32x32", IconSize16));
	// Wire.
	Style->Set("ClassIcon.AGX_WireComponent", new IMAGE_BRUSH("Icons/wire_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_WireWinchComponent", new IMAGE_BRUSH("Icons/wire_winch_32x32", IconSize16));
	// Others.
	Style->Set(
		"ClassIcon.AGX_ModelSourceComponent", new IMAGE_BRUSH("Icons/file_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_PlayRecordComponent",
		new IMAGE_BRUSH("Icons/play_record_32x32", IconSize16));

	// Actor icons, visible in the Place Actors panel. Currently only works with UE >= 5.0.
	Style->Set(
		"ClassIcon.AGX_RigidBodyActor", new IMAGE_BRUSH("Icons/rigid_body_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ContactMaterialRegistrarActor",
		new IMAGE_BRUSH("Icons/contact_material_register_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_ConstraintFrameActor",
		new IMAGE_BRUSH("Icons/constraint_frame_32x32", IconSize16));

	Style->Set(
		"ClassIcon.AGX_ConstraintActor", new IMAGE_BRUSH("Icons/constraint_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_HingeConstraintActor", new IMAGE_BRUSH("Icons/hinge_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_LockConstraintActor",
		new IMAGE_BRUSH("Icons/constraint_lock_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_PrismaticConstraintActor",
		new IMAGE_BRUSH("Icons/constraint_prismatic_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_DistanceConstraintActor",
		new IMAGE_BRUSH("Icons/constraint_distance_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CylindricalConstraintActor",
		new IMAGE_BRUSH("Icons/constraint_cylindrical_32x32", IconSize16));

	Style->Set("ClassIcon.AGX_SensorEnvironment", new IMAGE_BRUSH("Icons/lidar_32x32", IconSize16));
	Style->Set("ClassIcon.AGX_Terrain", new IMAGE_BRUSH("Icons/terrain_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_CollisionGroupDisablerActor",
		new IMAGE_BRUSH("Icons/collision_group_disable_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_TwoBodyTireActor",
		new IMAGE_BRUSH("Icons/two_wheel_tire_32x32", IconSize16));
	Style->Set("ClassIcon.AGX_WireActor", new IMAGE_BRUSH("Icons/wire_32x32", IconSize16));
	Style->Set(
		"ClassIcon.AGX_WireWinchActor", new IMAGE_BRUSH("Icons/wire_winch_32x32", IconSize16));

	////////////////////////

	// Thumbnails (assets etc).
	Style->Set(
		"ClassThumbnail.AGX_ConstraintMergeSplitThresholds",
		new IMAGE_BRUSH("Thumbnails/constraint_mergesplit_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_ContactMaterial",
		new IMAGE_BRUSH("Thumbnails/contact_material_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_LidarAmbientMaterial",
		new IMAGE_BRUSH("Thumbnails/lidar_material_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_LidarModelParameters",
		new IMAGE_BRUSH("Thumbnails/lidar_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_LidarSurfaceMaterial",
		new IMAGE_BRUSH("Thumbnails/lidar_material_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_PlayRecord",
		new IMAGE_BRUSH("Thumbnails/play_record_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_ShapeContactMergeSplitThresholds",
		new IMAGE_BRUSH("Thumbnails/shape_contact_mergesplit_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_ShapeMaterial",
		new IMAGE_BRUSH("Thumbnails/shape_material_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_ShovelProperties",
		new IMAGE_BRUSH("Thumbnails/shovel_properties_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_TerrainMaterial",
		new IMAGE_BRUSH("Thumbnails/terrain_material_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_TrackInternalMergeProperties",
		new IMAGE_BRUSH("Thumbnails/track_internal_merge_properties_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_TrackProperties",
		new IMAGE_BRUSH("Thumbnails/track_properties_128x128", IconSize128));
	Style->Set(
		"ClassThumbnail.AGX_WireMergeSplitThresholds",
		new IMAGE_BRUSH("Thumbnails/wire_mergesplit_128x128", IconSize128));

	return Style;
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
