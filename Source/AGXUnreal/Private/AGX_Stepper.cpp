// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_Stepper.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "AGX_Environment.h"

// Unreal Engine includes.
#include "Engine/GameInstance.h"

AAGX_Stepper::AAGX_Stepper()
{
	/// \todo When should we tick things? Here we set the TickGroup for the
	///       stepper to be PrePhysics while everything else retain the default
	///       DuringPhysics. This means that the other classes are guaranteed
	///       to see the new state for this tick.

	// Only tick if the AGX Dynamics license is valid.
	PrimaryActorTick.bCanEverTick = FAGX_Environment::GetInstance().EnsureAGXDynamicsLicenseValid();
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}

AAGX_Stepper::~AAGX_Stepper()
{
}

void AAGX_Stepper::BeginPlay()
{
	Super::BeginPlay();

	UGameInstance* Game = GetGameInstance();
	UAGX_Simulation* Simulation = Game->GetSubsystem<UAGX_Simulation>();
	if (Simulation == nullptr || Simulation->HasNative())
		return;

	// If we reach BeginPlay and the Simulation does not have a Native, it might be because
	// of a level transition, in which case we need to tell the Simulation to re-create
	// its native. This Stepper class will also call Simulation OnLevelTransition if we have
	// a level transition (see the EndPlay function for this class).
	Simulation->CreateNative();
}

void AAGX_Stepper::Tick(float DeltaTime)
{
#if 0
	UE_LOG(LogAGX, Log, TEXT("Stepper ticking"));
#endif
	Super::Tick(DeltaTime);
	UGameInstance* Game = GetGameInstance();
	UAGX_Simulation* Simulation = Game->GetSubsystem<UAGX_Simulation>();
	Simulation->Step(DeltaTime);
}

void AAGX_Stepper::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UGameInstance* Game = GetGameInstance();
	UAGX_Simulation* Simulation = Game->GetSubsystem<UAGX_Simulation>();

	if (EndPlayReason == EEndPlayReason::LevelTransition)
		Simulation->OnLevelTransition();
}
