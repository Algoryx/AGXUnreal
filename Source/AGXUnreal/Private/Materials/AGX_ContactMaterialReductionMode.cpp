// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialReductionMode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"

namespace AGX_ContactMaterialReductionMode_helpers
{
	EAGX_ContactReductionLevel ConvertFromOldValue(uint8 OldLevel)
	{
		switch (OldLevel)
		{
			case 0:
				return EAGX_ContactReductionLevel::Default;
			case 1:
				return EAGX_ContactReductionLevel::Aggressive;
			case 2:
				return EAGX_ContactReductionLevel::Moderate;
			case 3:
				return EAGX_ContactReductionLevel::Minimal;
			default:
				UE_LOG(
					LogAGX, Warning,
					TEXT("Tried to convert an uint8: %d to an EAGX_ContactReductionLevel, but "
						 "the  value is larger than the corresponding largest enum literal. "
						 "Returning "
						 "EAGX_ContactReductionLevel::Minimal."),
					OldLevel);
				AGX_CHECK(OldLevel > static_cast<uint8>(EAGX_ContactReductionLevel::Minimal));
				return EAGX_ContactReductionLevel::Minimal;
		}
	}
}

void FAGX_ContactMaterialReductionMode::Serialize(FArchive& Archive)
{
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::ContactReductionLevelAsEnum))
	{
		ContactReductionLevel =
			AGX_ContactMaterialReductionMode_helpers::ConvertFromOldValue(BinResolution_DEPRECATED);
	}
}
