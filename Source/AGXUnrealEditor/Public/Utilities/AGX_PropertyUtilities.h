// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Misc/EngineVersionComparison.h"
#include "PropertyHandle.h"
#include "UObject/UnrealType.h"

class USceneComponent;

/**
 * Provides helper functions for working with UProperty and IPropertyHandle.
 */
class AGXUNREALEDITOR_API FAGX_PropertyUtilities
{
public:
	static bool PropertyEquals(
		const TSharedPtr<IPropertyHandle>& First, const TSharedPtr<IPropertyHandle>& Second);

	static UObject* GetParentObjectOfStruct(
		const TSharedPtr<IPropertyHandle>& StructPropertyHandle);

	static UObject* GetParentObjectOfStruct(IPropertyHandle& StructPropertyHandle);

	/**
	 * Given a PropertyHandle that identifies a UObject* Property, return that UObject* if only a
	 * single object is selected. Return nullptr if multiple objects are selected.
	 *
	 * There is no way to distinguish between a nullptr UObject* Property and the multiple selected
	 * objects case.
	 */
	static UObject* GetObjectFromHandle(const TSharedPtr<IPropertyHandle>& PropertyHandle);

	/**
	 * Given a PropertyHandle that identifies a UStruct Property and an UObject that has such a
	 * property, read the property value, i.e., a pointer to an actual C++ struct, from the UObject.
	 */
	template <typename TStruct>
	static TStruct* GetStructFromHandle(
		const TSharedPtr<IPropertyHandle>& PropertyHandle, UObject* Owner);

	/**
	 * Returns the Display Name metadata if such exists, or else the name converted to
	 * display format (e.g. adding spacing between words, etc). Does not do localization yet!
	 *
	 * Note that this functions takes UField as input. The reason for that is because most
	 * metadata classes ([UF]Property, UClass, UStruct) inherit from UField.
	 *
	 * @note It is still unclear how the UProperty -> FProperty change in 4.25 changes this.
	 */
	// #if UE_VERSION_OLDER_THAN(2,25,0)
	static FString GetActualDisplayName(const UField* Field, bool bRemoveAgxPrefix);
	// #else
	static FString GetActualDisplayName(const FField* Field, bool bRemoveAgxPrefix);
	// #endif
};


template <typename TStruct>
TStruct* FAGX_PropertyUtilities::GetStructFromHandle(
	const TSharedPtr<IPropertyHandle>& PropertyHandle, UObject* Owner)
{
	if (!PropertyHandle->IsValidHandle())
	{
		return nullptr;
	}
	if (Owner == nullptr)
	{
		return nullptr;
	}

	return PropertyHandle->GetProperty()->ContainerPtrToValuePtr<TStruct>(Owner);
}
