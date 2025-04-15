// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "Misc/EngineVersionComparison.h"
#include "UObject/Field.h"

// Standard library includes.
#include <algorithm>

/// @todo Consider making these not FORCEINLINE, wrapped in a namespace, and move to .cpp.
/// @note There may be native Unreal Engine versions of these. Use those instead, where available.

FORCEINLINE FName GetFNameSafe(const UObjectBase* Object)
{
	if (Object == nullptr)
	{
		return NAME_None;
	}
	else
	{
		return Object->GetFName();
	}
}

#if UE_VERSION_OLDER_THAN(5, 4, 0)
FORCEINLINE FName GetFNameSafe(const FField* Field)
{
	if (Field == nullptr)
	{
		return NAME_None;
	}
	else
	{
		return Field->GetFName();
	}
}
#else
// GetFNameSafe for FField provided by UObject/Field.h since Unreal Engine 5.4.
#endif

FORCEINLINE FString GetLabel(const AActor& Actor)
{
#if WITH_EDITOR
	return Actor.GetActorLabel();
#else
	return Actor.GetName();
#endif

}

FORCEINLINE FString GetLabelSafe(const AActor* Actor)
{
	if (Actor == nullptr)
	{
		return TEXT("(none)");
	}
	return GetLabel(*Actor);
}

FORCEINLINE bool ContainsOnlyIntegers(const FString& str)
{
	return std::all_of(str.begin(), str.end(), TChar<TCHAR>::IsDigit);
}

FORCEINLINE FString ToReadablePath(const FString& Path)
{
	return IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Path);
}

FORCEINLINE FString RemoveFromString(const FString& Str, const FString& CharsToRemove)
{
	FString Result;
	Result.Reserve(Str.Len());
	for (auto Ch : Str)
	{
		if (!CharsToRemove.Contains(FString(1, &Ch)))
			Result.AppendChar(Ch);
	}

	return Result;
}

