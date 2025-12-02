// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/AGX_PropertyUtilities.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"

bool FAGX_PropertyUtilities::PropertyEquals(
	const TSharedPtr<IPropertyHandle>& First, const TSharedPtr<IPropertyHandle>& Second)
{
	return First->IsValidHandle() && Second->IsValidHandle() &&
		   First->GetProperty() == Second->GetProperty();
}

UObject* FAGX_PropertyUtilities::GetParentObjectOfStruct(
	const TSharedPtr<IPropertyHandle>& StructPropertyHandle)
{
	if (!StructPropertyHandle.IsValid())
	{
		return nullptr;
	}
	return GetParentObjectOfStruct(*StructPropertyHandle);
}

UObject* FAGX_PropertyUtilities::GetParentObjectOfStruct(IPropertyHandle& StructPropertyHandle)
{
	TArray<UObject*> OuterObjects;
	StructPropertyHandle.GetOuterObjects(OuterObjects);
	return OuterObjects.Num() > 0 ? OuterObjects[0] : nullptr;
}

UObject* FAGX_PropertyUtilities::GetObjectFromHandle(
	const TSharedPtr<IPropertyHandle>& PropertyHandle)
{
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	UClass* PropertyClass = UObjectProperty::StaticClass();
#else
	FFieldClass* PropertyClass = FObjectProperty::StaticClass();
#endif
	if (PropertyHandle && PropertyHandle->IsValidHandle() && PropertyHandle->GetProperty() &&
		PropertyHandle->GetProperty()->IsA(PropertyClass))
	{
		UObject* Object = nullptr;
		if (PropertyHandle->GetValue(Object) != FPropertyAccess::Result::Fail)
		{
			/// \todo Make sure we get the correct result even when multiple objects are selected. I
			/// worry that we should test for equality with FPRopertyAccess::Success instead of
			/// inequality with FPropertyAccess::Fail because of the possibility of MultipleValues.
			return Object;
		}
	}
	return nullptr;
}

FString FAGX_PropertyUtilities::GetActualDisplayName(const UField* Field, bool bRemoveAgxPrefix)
{
	FString Name;

	if (Field)
	{
		if (Field->HasMetaData(TEXT("DisplayName")))
		{
			Name = Field->GetMetaData(TEXT("DisplayName"));
		}
		else
		{
#if UE_VERSION_OLDER_THAN(2, 25, 0)
			bool bIsBool = Field->IsA(UBoolProperty::StaticClass());
#else
			bool bIsBool = false; /// @todo How test if the property is a bool in 4.25?
#endif
			Name = FName::NameToDisplayString(Field->GetFName().ToString(), bIsBool);
		}

		if (bRemoveAgxPrefix)
		{
			Name.RemoveFromStart("AGX", ESearchCase::CaseSensitive);
			Name.TrimStartAndEndInline();
		}
	}

	return Name;
}

FString FAGX_PropertyUtilities::GetActualDisplayName(const FField* Field, bool bRemoveAgxPrefix)
{
	FString Name;

	if (Field)
	{
		if (Field->HasMetaData(TEXT("DisplayName")))
		{
			Name = Field->GetMetaData(TEXT("DisplayName"));
		}
		else
		{
			Name = FName::NameToDisplayString(
				Field->GetFName().ToString(), Field->IsA(FBoolProperty::StaticClass()));
		}

		if (bRemoveAgxPrefix)
		{
			Name.RemoveFromStart("AGX", ESearchCase::CaseSensitive);
			Name.TrimStartAndEndInline();
		}
	}

	return Name;
}
