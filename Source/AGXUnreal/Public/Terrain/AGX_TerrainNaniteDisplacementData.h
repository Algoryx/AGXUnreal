// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"

#include "AGX_TerrainNaniteDisplacementData.generated.h"

class UStaticMesh;

/** Marks generated Landscape Nanite data prepared for vertical AGX height displacement.
 * Original local normals are retained in reserved UV channel 3 for undeformed shading.
 */
UCLASS()
class AGXUNREAL_API UAGX_TerrainNaniteDisplacementData : public UAssetUserData
{
	GENERATED_BODY()
public:
	// The generated layout must fit stock Nanite's four-UV payload.
	static constexpr int32 CurrentVersion = 2;
	static constexpr int32 OriginalNormalUVChannel = 3;
	bool IsCurrent(const UStaticMesh& Mesh) const;

	UPROPERTY()
	int32 Version = 0;
	UPROPERTY()
	FGuid MeshLightingGuid;
};
