// Copyright 2026, Algoryx Simulation AB.

#pragma once
#include "CoreMinimal.h"
class UAGX_DistributedAsset;
class UStaticMesh;
class UMaterialInterface;
struct FAGX_DistributedPreviewMesh
{
	UStaticMesh* Mesh {nullptr};
	TArray<UMaterialInterface*> Materials;
	FTransform Transform;
};
/** Reads templates only. Never constructs the source Actor or its AGX components. */
TArray<FAGX_DistributedPreviewMesh> AGX_GetDistributedPreview(const UAGX_DistributedAsset* Asset);
