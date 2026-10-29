// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "OpenPLX/OpenPLX_Enums.h"
#include "Utilities/OpenPLX_Utilities.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "UObject/Class.h"
#include "UObject/EnumProperty.h"
#include "UObject/UnrealType.h"

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

	bool IsKnown(EOpenPLX_OutputType Type)
	{
		return FOpenPLX_Utilities::IsRealType(Type) || FOpenPLX_Utilities::IsRangeType(Type) ||
			   FOpenPLX_Utilities::IsVectorType(Type) || FOpenPLX_Utilities::IsIntegerType(Type) ||
			   FOpenPLX_Utilities::IsBooleanType(Type) || FOpenPLX_Utilities::IsLidarOutputType(Type) ||
			   FOpenPLX_Utilities::IsIMUOutputType(Type);
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

					   const bool bIsKnown = IsKnown(Value) || Value == EOpenPLX_InputType::Unknown;
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
						   IsKnown(Value) || Value == EOpenPLX_OutputType::Unknown;
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

	Describe(
		"When resolving legacy OpenPLX signal type values",
		[this]()
		{
			It("should redirect Unsupported to Unknown and retain concrete values",
			   [this]()
			   {
				   UEnum* Enums[] {
					   StaticEnum<EOpenPLX_InputType>(), StaticEnum<EOpenPLX_OutputType>()};
				   for (UEnum* Enum : Enums)
				   {
					   const FString EnumName = Enum->GetName();
					   TestEqual(
						   TEXT("Unknown remains the zero/default value"),
						   Enum->GetValueByNameString(TEXT("Unknown")), int64 {0});
					   TestEqual(
						   TEXT("Unqualified Unsupported is redirected"),
						   Enum->GetValueByNameString(TEXT("Unsupported")), int64 {0});
					   TestEqual(
						   TEXT("Qualified Unsupported is redirected"),
						   Enum->GetValueByNameString(EnumName + TEXT("::Unsupported")), int64 {0});
					   // Existing concrete values must still resolve with the current enum prefix.
					   for (int32 Index = 1; Index < Enum->NumEnums() - 1; ++Index)
					   {
						   const FString Entry = Enum->GetNameStringByIndex(Index);
						   TestEqual(
							   *FString::Printf(TEXT("Qualified value %s::%s"), *EnumName, *Entry),
							   Enum->GetValueByNameString(EnumName + TEXT("::") + Entry),
							   Enum->GetValueByIndex(Index));
					   }
				   }
			   });
		});
}
