// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_Shovel.h"

#include "Terrain/ShovelBarrier.h"

void FAGX_Shovel::UpdateNativeShovelProperties(
	FShovelBarrier& ShovelBarrier, const FAGX_Shovel& Shovel)
{
	ShovelBarrier.SetNumberOfTeeth(Shovel.NumberOfTeeth);
	ShovelBarrier.SetToothLength(Shovel.ToothLength);
	ShovelBarrier.SetToothMinimumRadius(Shovel.MinimumToothRadius);
	ShovelBarrier.SetToothMaximumRadius(Shovel.MaximumToothRadius);
	ShovelBarrier.SetNoMergeExtensionDistance(Shovel.NoMergeExtensionDistance);
	ShovelBarrier.SetMinimumSubmergedContactLengthFraction(
		Shovel.MinimumSubmergedContactLengthFraction);
	ShovelBarrier.SetVerticalBladeSoilMergeDistance(Shovel.VerticalBladeSoilMergeDistance);
	ShovelBarrier.SetSecondarySeparationDeadloadLimit(Shovel.SecondarySeparationDeadloadLimit);
	ShovelBarrier.SetPenetrationDepthThreshold(Shovel.PenetrationDepthThreshold);
	ShovelBarrier.SetPenetrationForceScaling(Shovel.PenetrationForceScaling);
	ShovelBarrier.SetMaximumPenetrationForce(Shovel.MaximumPenetrationForce);
	ShovelBarrier.SetAlwaysRemoveShovelContacts(Shovel.AlwaysRemoveShovelContacts);

	if (Shovel.bOverrideContactRegionThreshold)
		ShovelBarrier.SetContactRegionThreshold(Shovel.ContactRegionThreshold);

	auto SetExcavationSettings =
		[&ShovelBarrier](EAGX_ExcavationMode Mode, const FAGX_ShovelExcavationSettings& Settings)
	{
		ShovelBarrier.SetExcavationSettingsEnabled(Mode, Settings.bEnabled);
		ShovelBarrier.SetExcavationSettingsEnableCreateDynamicMass(
			Mode, Settings.bEnableCreateDynamicMass);
		ShovelBarrier.SetExcavationSettingsEnableForceFeedback(Mode, Settings.bEnableForceFeedback);
	};

	SetExcavationSettings(EAGX_ExcavationMode::Primary, Shovel.PrimaryExcavationSettings);
	SetExcavationSettings(EAGX_ExcavationMode::DeformBack, Shovel.DeformBackExcavationSettings);
	SetExcavationSettings(EAGX_ExcavationMode::DeformRight, Shovel.DeformRightExcavationSettings);
	SetExcavationSettings(EAGX_ExcavationMode::DeformLeft, Shovel.DeformLeftExcavationSettings);
}
