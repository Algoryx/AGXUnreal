// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"
#include "Misc/EngineVersionComparison.h"
#include "Tests/AutomationCommon.h"
#include "UObject/Package.h"

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitTicks, int, NumTicks);

bool FWaitTicks::Update()
{
	--NumTicks;
	// UE_LOG(LogAGX, Warning, TEXT("FWaitTicks ticked to %d."), NumTicks);
	return NumTicks < 0;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FStringUtilities_GetFNameSafe_Test, "AGXUnreal.StringUtilities.GetFNameSafe",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::CriticalPriority |
		EAutomationTestFlags::ProductFilter)

bool FStringUtilities_GetFNameSafe_Test::RunTest(const FString& Parameters)
{
	UE_LOG(LogAGX, Warning, TEXT("Running unit test AGXUnreal.StringUtilities"));

	FName Name = GetFNameSafe(static_cast<const UObjectBase*>(nullptr));
	if (Name != NAME_None)
	{
		AddError(FString::Printf(
			TEXT("Expected to get 'NAME_None' for 'nullptr' UObjectBase, got '%s' instead."),
			*Name.ToString()));
	}
#if !UE_VERSION_OLDER_THAN(4, 25, 0)
	Name = GetFNameSafe(static_cast<const FField*>(nullptr));
	if (Name != NAME_None)
	{
		AddError(FString::Printf(
			TEXT("Expected to get 'NAME_None' for 'nullptr' FField, got '%s' instead."),
			*Name.ToString()));
	}
#endif

	TUniquePtr<UObject> TestObject {
		NewObject<AActor>(GetTransientPackage(), TEXT("TestObjectName"), RF_Transient)};
	Name = GetFNameSafe(TestObject.Get());
	if (Name != TEXT("TestObjectName"))
	{
		AddError(FString::Printf(
			TEXT("Expected to get 'TestObjectName' for TestObject, got '%s' instead."),
			*Name.ToString()));
	}

	UE_LOG(LogAGX, Warning, TEXT("Adding latent command waiting 20 ticks"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitTicks(20));

	UE_LOG(LogAGX, Warning, TEXT("Adding latent command waiting 5 ticks"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitTicks(5));

	UE_LOG(LogAGX, Warning, TEXT("RunTest is returning true"));

	return true;
}
