// Copyright 2024, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Constraints/AGX_HingeConstraintComponent.h"

// Unreal Engine includes.
#include "Editor.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "GameFramework/Actor.h"

// Standard library includes.
#include <limits>

/*
 * This is a unit test that ensures that we can restore Properties that used to be FFloatInterval
 * but are now FAGX_RealInterval instead. For storage we use a level containin a Hinge on which
 * we have set the force ranges for the different dimensions to various values.
 */

// Latent command that does the actual testing. Should be run after the level has been loaded.
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FCheckImportedRealIntervals, FAutomationTestBase&, Test);

bool FCheckImportedRealIntervals::Update()
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	if (World == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("FCheckImportedRealIntervals: Could not get world."));
		return true;
	}

	// Find the Actor owning the Hinge.
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
	AActor* HingeActor = nullptr;
	for (AActor* Actor : Actors)
	{
		if (Actor->GetName() == TEXT("HingeActor"))
		{
			if (HingeActor != nullptr)
			{
				Test.AddError(TEXT("Found two Actors named 'HingeActor'. Something is wrong"));
				return true;
			}
			HingeActor = Actor;
		}
	}
	if (HingeActor == nullptr)
	{
		Test.AddError(TEXT("Did not find Hinge Actor."));
		return true;
	}

	// Find the Hinge in the Actor.
	TArray<UAGX_HingeConstraintComponent*> Hinges;
	HingeActor->GetComponents(Hinges);
	if (Hinges.Num() == 0)
	{
		Test.AddError(
			FString::Printf(TEXT("Did not find any Hinge in Actor '%s'."), *HingeActor->GetName()));
		return true;
	}
	if (Hinges.Num() > 1)
	{
		Test.AddError(FString::Printf(
			TEXT("Found too many Hinges in Actor '%s'. Found %d."), *HingeActor->GetName(),
			Hinges.Num()));
		return true;
	}
	UAGX_HingeConstraintComponent* Hinge = Hinges[0];

	// Helper function that tests if a Range has the expected minimum and maximum values.
	auto TestInterval =
		[this](FAGX_RealInterval Actual, double ExpectedMin, double ExpectedMax, const TCHAR* Name)
	{
		AgxAutomationCommon::TestEqual(Test, TEXT("%s Min"), Actual.Min, ExpectedMin);
		AgxAutomationCommon::TestEqual(Test, TEXT("%s Max"), Actual.Max, ExpectedMax);
	};

	// Test all the force ranges. The numbers should match what has been set in the level.
	FAGX_ConstraintRangePropertyPerDof& ForceRanges = Hinge->ForceRange;
	TestInterval(ForceRanges.Translational_1, 0.0, 1.0, TEXT("Translational 1"));
	TestInterval(ForceRanges.Translational_2, -10, 10, TEXT("Translational 2"));
	TestInterval(ForceRanges.Translational_3, -1000000.0, 1000000000.0, TEXT("Translational 3"));

	// Before FAGX_RealInterval was introduced we used FFloatInterval which does not support
	// infinity. The default value for the force range was therefore TNumericLimits<float>::Max().
	// With the introduction of FAGX_RealInterval the default force range was changed to
	// +/- infinity. The test scene does not set the two rotational force ranges, so they get the
	// new default value.
	//
	// Another thing we could test is to store infinity in the original level, but I don't have an
	// easy way to create a hinge with infinite force range using FFloatInterval since Unreal Editor
	// doesn't support that. Would need to either write C++ code just for that or create and import
	// an AGX Dynamics archive.
	double Infinity = std::numeric_limits<double>::infinity();
	TestInterval(ForceRanges.Rotational_1, -Infinity, Infinity, TEXT("Rotational 1"));
	TestInterval(ForceRanges.Rotational_2, -Infinity, Infinity, TEXT("Rotational 2"));

	return true;
}

/**
 * Unit test that ensures that we can open levels saved before we switched force ranges from
 * using FFloatInterval to FAGX_RealInterval.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FRealIntervalBackwardsCompatibilityTest, "AGXUnreal.Editor.BackwardsCompatibility.RealInterval",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FRealIntervalBackwardsCompatibilityTest::RunTest(const FString& Parameters)
{
	static const FString MapPath {"/Game/Tests/BackwardsCompatibility/PreAGXRealInterval"};

	// Make sure no one accidentally replaced the map file containing FFloatIntervals with
	// FAGX_RealIntervals instead.
	AgxAutomationCommon::CheckMapMD5Checksum(
		MapPath, TEXT("24ce44e3772c70a604f461a4276a5c74"), *this);

	// Queue the latest commands.
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FCheckImportedRealIntervals(*this));

	return true;
}

// WITH_DEV_AUTOMATION_TESTS
#endif
