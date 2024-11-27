// Copyright 2024, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "Utilities/TestUtilities.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTypeConversionGuidTest, "AGXUnreal.Editor.AGX_TypeConversionsTest.TypeConversionGuid",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FTypeConversionGuidTest::RunTest(const FString& Parameters)
{
	// Starting from a randomly generated FGuid, convert to a agx Uuid and back to an Unreal FGuid
	// again. Their string representations should always match.
	//
	// Unfortunately, we cannot include the TypeConversions.h directly from the AGXUnrealTest module
	// since it includes AGX Dynamics headers. Instead, we need to use the FTestUtilities which
	// itself calls into the TypeConversions functions needed.

	const FGuid GuidUnreal0 = FGuid::NewGuid();
	const FString GuidAGXStr1 = FTestUtilities::ConvertToAGXUuidStr(GuidUnreal0);
	const FGuid GuidUnreal2 = FTestUtilities::ConvertAGXUuidToGuid(GuidAGXStr1);

	if (GuidUnreal0 != GuidUnreal2)
	{
		AddError(FString::Printf(
			TEXT("Guids did not match. Original Guid %s final Guid %s"), *GuidUnreal0.ToString(),
			*GuidUnreal2.ToString()));
	}

	// Ensure textual representations match.
	FString UuidStr = GuidAGXStr1;
	UuidStr = UuidStr.Replace(TEXT("-"), TEXT(""));
	UuidStr = UuidStr.ToUpper();

	TestEqual("Uuid Guid textual representation", UuidStr, GuidUnreal0.ToString());

	return true;
}
