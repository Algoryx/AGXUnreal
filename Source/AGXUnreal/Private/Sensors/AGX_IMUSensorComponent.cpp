// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_IMUSensorComponent.h"


#define LOCTEXT_NAMESPACE "AGX_IMUSensor"

UAGX_IMUSensorComponent::UAGX_IMUSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

#if WITH_EDITOR
bool UAGX_IMUSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseAccelerometer),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseGyroscope),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseMagnetometer)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
			return false;
	}

	return SuperCanEditChange;
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
