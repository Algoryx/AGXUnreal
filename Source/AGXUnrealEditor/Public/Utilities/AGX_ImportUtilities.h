// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "Math/Color.h"

class FTrimeshShapeBarrier;
class FRenderDataBarrier;
class FShapeBarrier;
class FMergeSplitThresholdsBarrier;
class FShapeMaterialBarrier;
class FTrackBarrier;
class FTrackPropertiesBarrier;
class FContactMaterialBarrier;
class UAGX_ShapeContactMergeSplitThresholds;
class UAGX_ConstraintMergeSplitThresholds;
class UAGX_ContactMaterial;
class UAGX_MergeSplitThresholdsBase;
class UAGX_ShapeMaterial;
class UAGX_TrackInternalMergeProperties;
class UAGX_TrackProperties;
class UAGX_WireMergeSplitThresholds;
struct FAGX_RenderMaterial;

class AActor;
class FString;
class UActorComponent;
class UMaterialInterface;
class UMaterialInstanceConstant;
class UStaticMesh;

class AGXUNREALEDITOR_API FAGX_ImportUtilities
{
public:
	/**
	 * Create a package path for an asset of the given type. The returned path is the sanitized
	 * version of "{DirectoryPath}/{AssetType}s/".
	 * @param DirectoryPath The absolute path of the model root directory. Must be valid.
	 * @param AssetType The type of the asset.
	 * @param AppendSeparator Whether or not to ensure the returned path ends with a separator /.
	 * @return A package path for the asset, or the empty string if the names are invalid.
	 */
	static FString CreatePackagePath(
		const FString& DirectoryPath, FString AssetType, bool AppendSeparator = true);

	/**
	 * Pick a name for an imported asset. NativeName and FileName will be sanitized and the first
	 * non-empty of the two is returned. If both sanitize to the empty string then AssetType is
	 * returned unchanged. Even though the name returned will be valid, it may not be unique and may
	 * therefore not be the final asset name.
	 * @param NativeName The name of the restored object.
	 * @param FallbackName Name to use if NativeName is unusable for some reason, for example is
	 * empty.
	 * @param AssetType The type of the asset.
	 * @return A safe name for the asset.
	 */
	static FString CreateAssetName(
		const FString& NativeName, const FString& FallbackName, const FString& AssetType);

	/**
	 * Find package- and asset names that are unique. The package name can be a directory. A unique
	 * name, based on AssetName, will be generated and the full package- and asset names will be
	 * stored in the parameters.
	 *
	 * An example, passing "/Game/Textures/", "MyTexture" when there already is an asset named
	 * "MyTexture" in "/Game/Textures/" will result in "/Game/Textures/MyTexture_1" to be stored in
	 * PackageName and "MyTexture_1" to be stored in AssetName.
	 *
	 * @param PackageName Package path to the folder that should hold the new asset.
	 * @param AssetName Candidate name for the new asset.
	 */
	static void MakePackageAndAssetNameUnique(FString& PackageName, FString& AssetName);

	/// \todo Determine if it's enough to return the asset created in the following few
	/// /Save.+Asset/ functions, or if we must pack it in a struct together with the package path
	/// and/or asset name.

	/**
	 * Sets up the imported Trimesh as an UStaticMesh asset, but does not write it to disk.
	 *
	 * @param Trimesh The imported trimesh to be saved.
	 * @param DirectoryPath The path of the directory where the assets are collected.
	 * @param FallbackName Name to give the asset in case the trimesh doesn't have a source
	 * name.
	 * @param Material Optional default material to apply to the Mesh Asset.
	 * @return The created asset.
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FTrimeshShapeBarrier& Trimesh, const FString& DirectoryPath,
		const FString& FallbackName, UMaterialInstanceConstant* Material);

	/**
	 * Sets up the imported Render Data Mesh as an UStaticMesh asset, but does not write it to disk.
	 *
	 * @param RenderData The Render Data holding the render mesh to store.
	 * @param DirectoryPath The path of the directory where the assets are collected.
	 * @param Material Optional default material to apply to the Mesh Asset.
	 * @return The created asset.
	 */
	static UStaticMesh* SaveImportedStaticMeshAsset(
		const FRenderDataBarrier& RenderData, const FString& DirectoryPath,
		UMaterialInstanceConstant* Material);

	/**
	 * Store an imported AGX Dynamics Track Internal Merge Property as an
	 * UAGX_TrackInternalMergeProperties asset on drive..
	 * @param Barrier The imported Track owning the Internal Merge Property.
	 * @param DirectoryPath The path of the directory where the assets are collected.
	 * @param Name The name to give to the new asset. A sequence number will be added in case of a
	 * conflict.
	 * @return The created UAGX_TrackInternalMergeProperties asset.
	 */
	static UAGX_TrackInternalMergeProperties* SaveImportedTrackInternalMergePropertiesAsset(
		const FTrackBarrier& Barrier, const FString& DirectoryPath, const FString& Name);

	/**
	 * Store an imported AGX Dynamics Track Property as an UAGX_TrackProperties.
	 * @param Barrier The imported Track referencing the Track Property.
	 * @param DirectoryPath The path of the directory where the assets are collected.
	 * @param Name The name to give to the new asset. A sequence number will be added in case of a
	 * conflict.
	 * @return The created UAGX_TrackProperties.
	 */
	static UAGX_TrackProperties* SaveImportedTrackPropertiesAsset(
		const FTrackPropertiesBarrier& Barrier, const FString& DirectoryPath, const FString& Name);

	/**
	 * Generate valid name for the object. Generates a fallback name if the given name can't be
	 * used.
	 */
	static FString CreateName(UObject& Object, const FString& Name);

	/**
	 * Handles the case of renaming Actor Components, where an extra name validation occurs compared
	 * to the more general Rename(UObject&, ...) version of this function.
	 */
	static void Rename(UActorComponent& Component, const FString& Name);

	/**
	 * Convert an sRGB space float channels color, as used in AGX Dynamics' render materials, to a
	 * linear space float channels color, as used by Unreal Engine's render materials.
	 *
	 * @param SRGB An sRGB color with float channels in the 0..1 range.
	 * @return A linear color with float channels in the 0..1 range.
	 */
	static FLinearColor SRGBToLinear(const FVector4& SRGB);

	/**
	 * Convert a linear space float channels color, as used by Unreal Engine's render materials, to
	 * a sRGB space float channels color, as used by AGX Dynamics' render materials.
	 * @param Linear
	 * @return
	 */
	static FVector4 LinearToSRGB(const FLinearColor& Linear);

	static FString GetImportRootDirectoryName();
	static FString GetImportShapeMaterialDirectoryName();
	static FString GetImportContactMaterialDirectoryName();
	static FString GetImportRenderMaterialDirectoryName();
	static FString GetImportMergeSplitThresholdsDirectoryName();
	static FString GetImportStaticMeshDirectoryName();
	static FString GetImportRenderMeshDirectoryName();
	static FString GetImportShovelPropertiesDirectoryName();

	/**
	 * Template version of the asset directory name getter.
	 * Only specialized for the asset types listed above, except for StaticMesh/RenderMesh because
	 * these are both UStaticMesh and it is not possible to know if the mesh comes from Trimesh data
	 * or render data.
	 */
	template <typename UAsset>
	static FString GetImportAssetDirectoryName();

	static FString GetContactMaterialRegistrarDefaultName();
	static FString GetCollisionGroupDisablerDefaultName();

	static FString GetUnsetUniqueImportName();

	/**
	 * Get the file system path to the default import directory for a model with the given name.
	 *
	 * Note that due to name collisions that actual path that a particular import resulted in may
	 * differ from the returned path.
	 */
	static FString GetDefaultModelImportDirectory(const FString& ModelName);

	/**
	 * Create a new asset destined for the given directory path. This functions will only create the
	 * asset and setup it's Package, it will not actually save it to disk. That is the
	 * responsibility of the caller.
	 */
	template <typename UAsset>
	static UAsset* CreateAsset(
		const FString& DirectoryPath, FString AssetName, const FString& AssetType);

	/**
	 * Create a new Component and add it to an Actor and attach it to the given attach parent.
	 * The Component will be given a temporary unique name.
	 */
	template <typename TComponent>
	static TComponent* CreateComponent(AActor& Owner, USceneComponent& AttachParent);

	static EAGX_ImportType GetFrom(const FString& FilePath);
};

template <typename UAsset>
UAsset* FAGX_ImportUtilities::CreateAsset(
	const FString& DirectoryPath, FString AssetName, const FString& AssetType)
{
	AssetName = FAGX_ImportUtilities::CreateAssetName(AssetName, "", AssetType);
	FString PackagePath = FAGX_ImportUtilities::CreatePackagePath(DirectoryPath, AssetType);
	FAGX_ImportUtilities::MakePackageAndAssetNameUnique(PackagePath, AssetName);
	UPackage* Package = CreatePackage(*PackagePath);

	UAsset* Asset = NewObject<UAsset>(Package, FName(*AssetName), RF_Public | RF_Standalone);
	if (Asset == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("Could not create asset '%s' from '%s'."), *AssetName,
			*DirectoryPath);
	}

	return Asset;
}

template <typename TComponent>
TComponent* FAGX_ImportUtilities::CreateComponent(AActor& Owner, USceneComponent& AttachParent)
{
	TComponent* Component = NewObject<TComponent>(
		&Owner, FName(FAGX_ImportUtilities::GetUnsetUniqueImportName()));

	Owner.AddInstanceComponent(Component);
	Component->RegisterComponent();
	Component->AttachToComponent(
		&AttachParent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	return Component;
}
