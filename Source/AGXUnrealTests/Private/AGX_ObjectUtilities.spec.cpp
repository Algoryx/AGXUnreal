// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"

BEGIN_DEFINE_SPEC(
	FAGX_ObjectUtilitiesSpec, "AGXUnreal.Spec.ObjectUtilities",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_ObjectUtilitiesSpec)

void FAGX_ObjectUtilitiesSpec::Define()
{
	auto CreateChain = [](FEditPropertyChain& Chain)
	{
		// Create a chain of bogus properties.
		UClass* Class = UAGX_WireComponent::StaticClass();
		FProperty* RouteNodes = Class->FindPropertyByName(TEXT("RouteNodes"));
		Chain.AddTail(RouteNodes);
		FProperty* Radius = Class->FindPropertyByName(TEXT("Radius"));
		Chain.AddTail(Radius);
	};

	Describe(
		"When checking for prefix path",
		[this, &CreateChain]()
		{
			It("should accept equal length.",
			   [this, &CreateChain]()
			   {
				   // Create a chain of bogus properties.
				   FEditPropertyChain Chain;
			   	   CreateChain(Chain);
				   const TArray<const TCHAR*> Path {TEXT("RouteNodes"), TEXT("Radius")};
				   const bool HasPrefix =
					   FAGX_ObjectUtilities::HasChainPrefixPath(Chain.GetHead(), Path);
				   TestTrue(TEXT("Equal chain"), HasPrefix);
			   });

			It("should accept shorter path",
			   [this, &CreateChain]()
			   {
				   FEditPropertyChain Chain;
				   CreateChain(Chain);
				   const TArray<const TCHAR*> Path {TEXT("RouteNodes")};
				   const bool HasPrefix =
					   FAGX_ObjectUtilities::HasChainPrefixPath(Chain.GetHead(), Path);
				   TestTrue(TEXT("Short path"), HasPrefix);
			   });

			It("should not accept too long path",
				[this, &CreateChain]()
				{
					FEditPropertyChain Chain;
					CreateChain(Chain);
					const TArray<const TCHAR*> Path {TEXT("RouteNodes"), TEXT("Radius"), TEXT("ExtraNode")};
					const bool HasPrefix = FAGX_ObjectUtilities::HasChainPrefixPath(Chain.GetHead(), Path);
					TestFalse(TEXT("Tool long path"), HasPrefix);
				});
		});
}
