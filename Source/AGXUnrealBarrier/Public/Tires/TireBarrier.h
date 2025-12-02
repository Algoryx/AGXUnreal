// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FString;
struct FTireRef;

/**
 * Acts as an interface to a native AGX Tire, and encapsulates it so that it is completely hidden
 * from code that includes this file.
 *
 * To support specialized native Tire types, a barrier class deriving from this class has to be
 * created for each Tire type. The derived class has to provide a function for  allocating the AGX
 * Dynamics native object. It does not need to hold a reference to the native object. Because the
 * derived class creates the native Tire object, it can also safely cast NativeRef->Native to that
 * same type whenever necessary.
 */
class AGXUNREALBARRIER_API FTireBarrier
{
public:
	FTireBarrier();
	FTireBarrier(std::unique_ptr<FTireRef>&& Native);
	FTireBarrier(FTireBarrier&& Other);
	virtual ~FTireBarrier();

	bool HasNative() const;
	FTireRef* GetNative();
	const FTireRef* GetNative() const;

	void SetName(const FString& NewName);
	FString GetName() const;

	void ReleaseNative();

	FGuid GetGuid() const;

private:
	FTireBarrier(const FTireBarrier&) = delete;
	void operator=(const FTireBarrier&) = delete;

protected:
	// NativeRef has the same lifetime as this object.
	// NativeRef->Native can be null.
	// NativeRef->Native is created by the lowermost subclass and released when ReleaseNative is
	// invoked. NativeRef->Native should be type-casted whenever a subclass needs the derived
	// interface (e.g. to agx::TwoBodyTire).
	std::unique_ptr<FTireRef> NativeRef;
};
