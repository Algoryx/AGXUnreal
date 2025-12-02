// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Misc/AutomationTest.h"

class AActor;

namespace AGX_PlayInEditorUtils
{
	const FString EmptyMapPath(TEXT("/Game/Tests/Test_EmptyLevel"));

	TMap<FString, AActor*> GetActorsByName(UWorld* TestWorld, const TArray<FString>& Names);

	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
		FTickOnlyCommand, int, TickCurrent, int, TickMax);

	/*
	 * Command that remains active until the Play In Editor world's AGX Simulation reaches the
	 * given time stamp. The end time stamp must be known ahead of time, before the latent commands
	 * start executing.
	 */
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FTickUntilTimeStamp, double, TimeStamp);

	/**
	 * Command that remains active until the Play In Editor world's AGX Simulation reaches the
	 * given time stamp. The end time stamp can be set by prior latent commands.
	 */
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FTickUntilDynamicTimeStamp, double*, TimeStamp);
}
