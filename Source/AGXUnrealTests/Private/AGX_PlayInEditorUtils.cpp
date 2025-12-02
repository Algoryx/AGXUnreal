// Copyright 2025, Algoryx Simulation AB.

#include "AGX_PlayInEditorUtils.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Editor.h"
#include "Kismet/GameplayStatics.h"

TMap<FString, AActor*> AGX_PlayInEditorUtils::GetActorsByName(
	UWorld* TestWorld, const TArray<FString>& Names)
{
	TMap<FString, AActor*> FoundActors;
	if (TestWorld == nullptr)
	{
		return FoundActors;
	}

	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(TestWorld, AActor::StaticClass(), AllActors);
	for (const FString& Name : Names)
	{
		AActor** FoundActor = AllActors.FindByPredicate(
			[&Name](AActor* Actor) { return Actor->GetActorLabel().Equals(Name); });
		if (FoundActor != nullptr)
		{
			FoundActors.Add(Name, *FoundActor);
		}
	}

	return FoundActors;
}

bool AGX_PlayInEditorUtils::FTickOnlyCommand::Update()
{
	if (!GEditor->IsPlayingSessionInEditor())
	{
		return false;
	}

	TickCurrent++;
	if (TickCurrent < TickMax)
		return false;

	return true;
}

bool AGX_PlayInEditorUtils::FTickUntilTimeStamp::Update()
{
	check(GEditor->IsPlayingSessionInEditor());

	UWorld* World = GEditor->GetPIEWorldContext()->World();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	return Simulation->GetTimeStamp() >= TimeStamp;
}

bool AGX_PlayInEditorUtils::FTickUntilDynamicTimeStamp::Update()
{
	check(GEditor->IsPlayingSessionInEditor());

	UWorld* World = GEditor->GetPIEWorldContext()->World();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	return Simulation->GetTimeStamp() >= *TimeStamp;
}
