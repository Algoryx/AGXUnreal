// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_Utilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Import/AGX_Importer.h"
#include "Import/AGX_ImporterSettings.h"
#include "Utilities/AGXUtilities.h"

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

AActor* UAGX_AGXUtilities::ImportAGXArchive(const FString& FilePath, bool IgnoreDisabledTrimeshes)
{
	FAGX_AGXImporterSettings Settings;
	Settings.FilePath = FilePath;
	Settings.bIgnoreDisabledTrimeshes = IgnoreDisabledTrimeshes;
	Settings.bOpenBlueprintEditorAfterImport = false; // Todo: remove this member.
	FAGX_Importer Importer;
	FAGX_ImportResult Result = Importer.Import(Settings);
	return Result.Actor;
}

AActor* UAGX_AGXUtilities::InstantiateActor(
	UObject* WorldContextObject, AActor* TemplateActor, FTransform Transform)
{
	if (WorldContextObject == nullptr || WorldContextObject->GetWorld() == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("Could not get World object from call to InstantiateActor."));
		return nullptr;
	}

	if (!IsValid(TemplateActor))
	{
		UE_LOG(LogAGX, Warning, TEXT("InstantiateActor got invalid TemplateActor."));
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();
	FActorSpawnParameters Params;
	Params.Template = TemplateActor;
	return World->SpawnActor<AActor>(AActor::StaticClass(), Transform, Params);
}
