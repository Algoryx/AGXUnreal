// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_Utilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Import/AGX_Importer.h"
#include "Import/AGX_ImportSettings.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGXUtilities.h"
#include "Utilities/PLXUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

void UAGX_AGXUtilities::AddParentVelocity(
	UAGX_RigidBodyComponent* Parent, UAGX_RigidBodyComponent* Body)
{
	if (Parent == nullptr || !Parent->HasNative() || Body == nullptr || !Body->HasNative())
		return;

	FAGXUtilities::AddParentVelocity(*Parent->GetNative(), *Body->GetNative());
}

void UAGX_AGXUtilities::AddParentVelocityMany(
	UAGX_RigidBodyComponent* Parent, const TArray<UAGX_RigidBodyComponent*>& Bodies)
{
	for (UAGX_RigidBodyComponent* Body : Bodies)
	{
		if (Body == Parent)
			continue;

		AddParentVelocity(Parent, Body);
	}
}

FVector UAGX_AGXUtilities::CalculateCenterOfMass(const TArray<UAGX_RigidBodyComponent*>& Bodies)
{
	FVector Com = FVector::ZeroVector;
	float TotalMass = 0.f;
	for (UAGX_RigidBodyComponent* Body : Bodies)
	{
		if (Body == nullptr || Body->GetWorld() == nullptr || !Body->GetWorld()->IsGameWorld())
			continue;

		const auto Mass = Body->GetMass();
		const auto Cm = Body->GetCenterOfMassPosition();
		Com += Cm * Mass;
		TotalMass += Mass;
	}

	if (!FMath::IsNearlyZero(TotalMass))
		Com /= TotalMass;

	return Com;
}

namespace AGX_AGXUtilities_helpers
{
	void PreOpenPLXImport(FAGX_ImportSettings& OutSettings)
	{
		if (OutSettings.ImportType != EAGX_ImportType::Plx)
			return;

		if (OutSettings.FilePath.StartsWith(FPLXUtilities::GetModelsDirectory()))
			return;

		// We need to copy the OpenPLX file (and any dependency) to the OpenPLX ModelsDirectory.
		// We also update the filepath in the ImportSettings to point to the new, copied OpenPLX
		// file.
		const FString DestinationDir =
			FPLXUtilities::CreateUniqueModelDirectory(OutSettings.FilePath);
		const FString NewLocation =
			FPLXUtilities::CopyAllDependenciesToProject(OutSettings.FilePath, DestinationDir);
		OutSettings.FilePath = NewLocation;
	}

	void ResolveImportPath(FAGX_ImportSettings& OutSettings)
	{
		if (FPaths::FileExists(OutSettings.FilePath))
			return;

		// Try relative to project dir.
		const FString ProjectDir = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		const FString FullPath = FPaths::Combine(ProjectDir, OutSettings.FilePath);
		if (FPaths::FileExists(FullPath))
			OutSettings.FilePath = FullPath;
	}
}

AActor* UAGX_AGXUtilities::Import(UObject* WorldContextObject, FAGX_ImportSettings Settings)
{
	if (WorldContextObject == nullptr || WorldContextObject->GetWorld() == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not get World object from call to UAGX_AGXUtilities::Import."));
		return nullptr;
	}

	AGX_AGXUtilities_helpers::ResolveImportPath(Settings);
	UWorld* World = WorldContextObject->GetWorld();
	if (Settings.ImportType == EAGX_ImportType::Plx)
		AGX_AGXUtilities_helpers::PreOpenPLXImport(Settings);

	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings, *World);
	if (IsUnrecoverableError(Result.Result) || Result.Actor == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Got unrecoverable error from Importer result in UAGX_AGXUtilities::Import. The "
				 "Output Log may contain more information."));
		if (Result.Actor != nullptr)
			Result.Actor->Destroy();

		return nullptr;
	}

	return Result.Actor;
}

AActor* UAGX_AGXUtilities::InstantiateActor(
	UObject* WorldContextObject, AActor* Template, const FTransform& Transform)
{
	if (WorldContextObject == nullptr || WorldContextObject->GetWorld() == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not get World object from call to UAGX_AGXUtilities::InstantiateActor."));
		return nullptr;
	}

	if (!IsValid(Template))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_AGXUtilities::InstantiateActor got invalid TemplateActor."));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	FActorSpawnParameters Params;
	Params.Name =
		*FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(World, Template->GetName(), nullptr);
	Params.NameMode = FActorSpawnParameters::ESpawnActorNameMode::Requested;
	Params.Template = Template;
	AActor* SpawnedActor = World->SpawnActor<AActor>(AActor::StaticClass(), Transform, Params);

#if WITH_EDITOR
	SpawnedActor->SetActorLabel(Params.Name.ToString());
#endif
	return SpawnedActor;
}
