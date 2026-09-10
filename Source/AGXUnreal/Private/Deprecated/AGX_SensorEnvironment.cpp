// Copyright 2026, Algoryx Simulation AB.

#include "Deprecated/AGX_SensorEnvironment.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_SensorEnvironmentSpriteComponent.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

namespace AGX_SensorEnvironment_helpers
{
	FString GetDeprecationMessage()
	{
		return TEXT(
			"AAGX_SensorEnvironment is deprecated. Sensors are now added automatically to the "
			"SensorEnvironmentSubsystem that is created automatically on Play. To access settings, "
			"go to \"Edit > Project Settings... > Plugins > AGX Sensor Environment\" or access the "
			"AGX_SensorEnvironmentSubsystem directly from a Blueprint Graph. This Sensor "
			"Environment Actor should be removed from the Level.");
	}
}

AAGX_SensorEnvironment::AAGX_SensorEnvironment()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<UAGX_SensorEnvironmentSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());

	DeprecationMessage = AGX_SensorEnvironment_helpers::GetDeprecationMessage();
}

void AAGX_SensorEnvironment::BeginPlay()
{
	Super::BeginPlay();

	FAGX_NotificationUtilities::ShowNotification(
		AGX_SensorEnvironment_helpers::GetDeprecationMessage(), SNotificationItem::CS_None, 8.f);
}
