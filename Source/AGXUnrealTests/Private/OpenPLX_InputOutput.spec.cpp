// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "OpenPLX/OpenPLX_Enums.h"
#include "Utilities/OpenPLX_Utilities.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/EnumProperty.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"

BEGIN_DEFINE_SPEC(
	FOpenPLX_InputOutputSpec, "AGXUnreal.Spec.PLXInputOutput",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FOpenPLX_InputOutputSpec)

namespace OpenPLX_InputOutputSpec_helpers
{
	template <typename InoutOrOutputT>
	bool IsKnown(InoutOrOutputT Type)
	{
		return FOpenPLX_Utilities::IsRealType(Type) || FOpenPLX_Utilities::IsRangeType(Type) ||
			   FOpenPLX_Utilities::IsVectorType(Type) || FOpenPLX_Utilities::IsIntegerType(Type) ||
			   FOpenPLX_Utilities::IsBooleanType(Type);
	}
}

void FOpenPLX_InputOutputSpec::Define()
{
	using namespace OpenPLX_InputOutputSpec_helpers;

	// Test that all input and output enum literals are covered in one of the Is...Type in
	// FOpenPLX_Utilities.
	Describe(
		"When iterating all OpenPLX Input enum literals",
		[this]()
		{
			It("should be handled by one of the Is...Type functions in FOpenPLX_Utilities",
			   [this]()
			   {
				   for (int32 Index = 0; Index < StaticEnum<EOpenPLX_InputType>()->NumEnums() - 1;
						++Index)
				   {
					   EOpenPLX_InputType Value = static_cast<EOpenPLX_InputType>(
						   StaticEnum<EOpenPLX_InputType>()->GetValueByIndex(Index));

					   const bool bIsKnown =
						   IsKnown(Value) || Value == EOpenPLX_InputType::Unsupported;
					   if (!bIsKnown)
					   {
						   FString Name =
							   StaticEnum<EOpenPLX_InputType>()->GetNameStringByIndex(Index);
						   UE_LOG(LogAGX, Warning, TEXT("Input '%s' not handled!"), *Name);
					   }

					   this->TestTrue("Input is known", bIsKnown);
				   }
			   });
		});

	Describe(
		"When iterating all OpenPLX Output enum literals",
		[this]()
		{
			It("should be handled by one of the Is...Type functions in FOpenPLX_Utilities",
			   [this]()
			   {
				   for (int32 Index = 0; Index < StaticEnum<EOpenPLX_OutputType>()->NumEnums() - 1;
						++Index)
				   {
					   EOpenPLX_OutputType Value = static_cast<EOpenPLX_OutputType>(
						   StaticEnum<EOpenPLX_OutputType>()->GetValueByIndex(Index));

					   const bool bIsKnown =
						   IsKnown(Value) || Value == EOpenPLX_OutputType::Unsupported;
					   if (!bIsKnown)
					   {
						   FString Name =
							   StaticEnum<EOpenPLX_OutputType>()->GetNameStringByIndex(Index);
						   UE_LOG(LogAGX, Warning, TEXT("Output '%s' not handled!"), *Name);
					   }

					   this->TestTrue("Output is known", bIsKnown);
				   }
			   });
		});
}
