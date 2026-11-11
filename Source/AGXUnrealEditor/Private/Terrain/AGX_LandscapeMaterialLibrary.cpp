// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_LandscapeMaterialLibrary.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_TerrainNaniteDisplacementData.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FileHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Landscape.h"
#include "LandscapeNaniteComponent.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpression.h"
#include "Materials/MaterialFunction.h"
#include "MeshDescription.h"
#include "StaticMeshAttributes.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionActorDescInstance.h"
#include "WorldPartition/WorldPartitionHelpers.h"

bool UAGX_LandscapeMaterialLibrary::SetNaniteDisplacement(
	UMaterial* Material, UMaterialExpression* Expression, int32 OutputIndex, float Magnitude,
	float Center, bool bClearLegacyOffsets)
{
	if (Material == nullptr || Expression == nullptr || Expression->GetOuter() != Material ||
		OutputIndex < 0 || OutputIndex >= Expression->GetOutputs().Num() ||
		!FMath::IsFinite(Magnitude) || Magnitude <= 0.f || !FMath::IsFinite(Center))
		return false;
	Material->Modify();
	Material->bEnableTessellation = true;
	Material->DisplacementScaling.Magnitude = Magnitude;
	Material->DisplacementScaling.Center = Center;
	Material->GetEditorOnlyData()->Displacement.Connect(OutputIndex, Expression);
	if (bClearLegacyOffsets)
	{
		Material->GetEditorOnlyData()->WorldPositionOffset.Expression = nullptr;
		Material->GetEditorOnlyData()->PixelDepthOffset.Expression = nullptr;
	}
	return true;
}

TArray<UMaterialExpression*> UAGX_LandscapeMaterialLibrary::GetMaterialExpressions(
	UMaterial* Material)
{
	TArray<UMaterialExpression*> Result;
	if (Material != nullptr)
		for (UMaterialExpression* Expression : Material->GetExpressions())
			Result.Add(Expression);
	return Result;
}

TArray<UMaterialExpression*> UAGX_LandscapeMaterialLibrary::GetMaterialFunctionExpressions(
	UMaterialFunction* Function)
{
	TArray<UMaterialExpression*> Result;
	if (Function != nullptr)
		for (UMaterialExpression* Expression : Function->GetExpressions())
			Result.Add(Expression);
	return Result;
}

bool UAGX_LandscapeMaterialLibrary::ReplaceExpressionConsumers(
	UMaterial* Material, UMaterialExpression* Previous, UMaterialExpression* Replacement,
	int32 OutputIndex)
{
	if (Material == nullptr || Previous == nullptr || Replacement == nullptr ||
		Previous->GetOuter() != Material || Replacement->GetOuter() != Material ||
		OutputIndex < 0 || OutputIndex >= Replacement->GetOutputs().Num())
		return false;
	Material->Modify();
	for (UMaterialExpression* Expression : Material->GetExpressions())
	{
		for (FExpressionInputIterator Input(Expression); Input; ++Input)
			if (Input->Expression == Previous)
				Input->Connect(OutputIndex, Replacement);
	}
	return true;
}

bool UAGX_LandscapeMaterialLibrary::BuildLandscapeNanite(
	ALandscapeProxy* Landscape, bool bIncludeUnloadedProxies, bool bSavePackages)
{
	if (Landscape == nullptr || !Landscape->IsNaniteEnabled())
		return false;
	if (bIncludeUnloadedProxies && Landscape->IsA<ALandscape>())
		if (UWorldPartition* Partition = Landscape->GetWorld()->GetWorldPartition())
		{
			bool bSuccess = true;
			const FGuid Guid = Landscape->GetLandscapeGuid();
			FWorldPartitionHelpers::FForEachActorWithLoadingParams Params;
			Params.ActorClasses = {ALandscapeStreamingProxy::StaticClass()};
			Params.bGCPerActor = true;
			FWorldPartitionHelpers::ForEachActorWithLoading(
				Partition,
				[&Guid, &bSuccess, bSavePackages](const FWorldPartitionActorDescInstance* Desc)
				{
					auto* Proxy = Cast<ALandscapeStreamingProxy>(Desc->GetActor());
					if (Proxy != nullptr && Proxy->GetLandscapeGuid() == Guid)
						bSuccess &= BuildLandscapeNanite(Proxy, false, bSavePackages);
					return bSuccess;
				},
				Params);
			if (!bSuccess)
				return false;
		}
	// An older or externally rebuilt prepared mesh may already have vertical normals.
	// Regenerate its stock source first, so re-preparation cannot overwrite the saved
	// original shading normals with those vertical displacement normals.
	TInlineComponentArray<ULandscapeNaniteComponent*> PreviousComponents(Landscape);
	for (ULandscapeNaniteComponent* Component : PreviousComponents)
		if (UStaticMesh* Mesh = Component->GetStaticMesh())
			if (auto* Metadata =
					Cast<UAGX_TerrainNaniteDisplacementData>(Mesh->GetAssetUserDataOfClass(
						UAGX_TerrainNaniteDisplacementData::StaticClass())))
				if (!Metadata->IsCurrent(*Mesh))
				{
					Landscape->InvalidateNaniteRepresentation(false);
					break;
				}
	IConsoleVariable* MultithreadBuild =
		IConsoleManager::Get().FindConsoleVariable(TEXT("landscape.Nanite.MultithreadBuild"));
	if (MultithreadBuild == nullptr)
		return false;
	const int32 Previous = MultithreadBuild->GetInt();
	MultithreadBuild->Set(0, ECVF_SetByCode);
	Landscape->UpdateNaniteRepresentation(nullptr);
	MultithreadBuild->Set(Previous, ECVF_SetByCode);
	if (!Landscape->IsNaniteMeshUpToDate())
		return false;
	TInlineComponentArray<ULandscapeNaniteComponent*> Components(Landscape);
	for (ULandscapeNaniteComponent* Component : Components)
	{
		UStaticMesh* Mesh = Component->GetStaticMesh();
		if (Mesh == nullptr)
			return false;
		auto* Existing = Cast<UAGX_TerrainNaniteDisplacementData>(
			Mesh->GetAssetUserDataOfClass(UAGX_TerrainNaniteDisplacementData::StaticClass()));
		if (Existing != nullptr && Existing->IsCurrent(*Mesh))
			continue;
		FMeshDescription* Description = Mesh->GetMeshDescription(0);
		if (Description == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Landscape Nanite mesh '%s' has no editable source mesh. Rebuild stock Nanite "
					"data before preparing AGX deformation."),
				*Mesh->GetName());
			return false;
		}
		Mesh->Modify();
		FStaticMeshAttributes Attributes(*Description);
		auto Normals = Attributes.GetVertexInstanceNormals();
		auto Tangents = Attributes.GetVertexInstanceTangents();
		auto Signs = Attributes.GetVertexInstanceBinormalSigns();
		auto UVs = Attributes.GetVertexInstanceUVs();
		UVs.SetNumChannels(
			FMath::Max(
				UVs.GetNumChannels(),
				UAGX_TerrainNaniteDisplacementData::OriginalNormalUVChannel + 1));
		for (const FVertexInstanceID Id : Description->VertexInstances().GetElementIDs())
		{
			const FVector3f Normal = Normals[Id].GetSafeNormal();
			UVs.Set(
				Id, UAGX_TerrainNaniteDisplacementData::OriginalNormalUVChannel,
				FVector2f(Normal.X, Normal.Y));
			Normals[Id] = FVector3f(0, 0, 1);
			Tangents[Id] = FVector3f(1, 0, 0);
			Signs[Id] = 1.f;
		}
		auto& Settings = Mesh->GetSourceModel(0).BuildSettings;
		Settings.bRecomputeNormals = false;
		Settings.bRecomputeTangents = false;
		Mesh->CommitMeshDescription(0);
		TArray<FText> Errors;
		Mesh->Build(true, &Errors); // OutErrors makes this build synchronous.
		if (!Errors.IsEmpty())
		{
			for (const FText& Error : Errors)
				UE_LOG(LogAGX, Error, TEXT("Landscape Nanite preparation: %s"), *Error.ToString());
			return false;
		}
		Mesh->RemoveUserDataOfClass(UAGX_TerrainNaniteDisplacementData::StaticClass());
		auto* Metadata = NewObject<UAGX_TerrainNaniteDisplacementData>(Mesh);
		Metadata->Version = UAGX_TerrainNaniteDisplacementData::CurrentVersion;
		Metadata->MeshLightingGuid = Mesh->GetLightingGuid();
		Mesh->AddAssetUserData(Metadata);
		Mesh->MarkPackageDirty();
		Component->MarkRenderStateDirty();
	}
	Landscape->MarkPackageDirty();
	if (bSavePackages)
	{
		TArray<UPackage*> Packages;
		Packages.AddUnique(Landscape->GetPackage());
		for (ULandscapeNaniteComponent* Component : Components)
			if (UStaticMesh* Mesh = Component->GetStaticMesh())
				Packages.AddUnique(Mesh->GetPackage());
		return FEditorFileUtils::PromptForCheckoutAndSave(Packages, true, false) ==
			   FEditorFileUtils::PR_Success;
	}
	return true;
}

int32 UAGX_LandscapeMaterialLibrary::RemoveUnusedExpressions(UMaterial* Material)
{
	if (Material == nullptr)
		return 0;
	TArray<UMaterialExpression*> Pending;
	TSet<UMaterialExpression*> Used;
	for (int32 Property = 0; Property < MP_MAX; ++Property)
		if (FExpressionInput* Input =
				Material->GetExpressionInputForProperty(static_cast<EMaterialProperty>(Property)))
			if (Input->Expression != nullptr)
				Pending.Add(Input->Expression);
	while (!Pending.IsEmpty())
	{
		UMaterialExpression* Expression = Pending.Pop();
		if (Used.Contains(Expression))
			continue;
		Used.Add(Expression);
		for (FExpressionInputIterator Input(Expression); Input; ++Input)
			if (Input->Expression != nullptr)
				Pending.Add(Input->Expression);
	}
	int32 Removed = 0;
	for (UMaterialExpression* Expression : GetMaterialExpressions(Material))
	{
		if (!Used.Contains(Expression))
		{
			Material->RemoveExpressionParameter(Expression);
			Material->GetExpressionCollection().RemoveExpression(Expression);
			++Removed;
		}
	}
	return Removed;
}
