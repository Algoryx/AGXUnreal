// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Interface.h"

// Standard library includes.
#include <cstdint>
#include <limits>

#include "AGX_NativeOwner.generated.h"

class UActorComponent;

UINTERFACE(MinimalAPI, Blueprintable)
class UAGX_NativeOwner : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface implemented by all classes that owns an AGX Dynamics native object.
 *
 * Provides access to the address of the AGX Dynamics object as an unsigned integer. This is used
 * during Blueprint reconstruction (RerunConstructionScripts) when all Blueprint-created Components
 * are serialized, destroyed, and then recreated. We store the address of the AGX Dynamics native
 * object in a Component Instance Data and then give it to the newly created replacement Component
 * instance when the Component Instance Data is applied to it.
 *
 * We do this because we're not able to (easily) recreate the AGX Dynamics state for a subset of
 * the scene so instead of destroying and recreating the AGX Dynamics objects we let the new
 * AGX Dynamics for Unreal objects inherit the existing AGX Dynamics objects, via the Component
 * Instance Data.
 *
 * We use uint64 here since that is big enough for the platforms we support so far. It may be better
 * to use UPTRINT, which is the Unreal Engine equivalent of the standard library uintptr_t. We
 * currently don't because first we want to make really sure that these bits won't travel between
 * machines somehow. Also not sure if UPTRINT is supported by Unreal Engine's serialization, which
 * I believe is a requirement for use with Component Instance Data.
 *
 * The class implementing this interface may chose to do explicit reference counting of the AGX
 * Dynamics object for the lifetime of the native address integer. This may be needed because not
 * all AGX Dynamics objects are guaranteed to be owned by some other AGX Dynamics object unaffected
 * by the Blueprint reconstruction. The semantics is that the integer itself participates in the
 * reference counting. So GetNativeAddress may increment the reference count, as the Component
 * Instance Data is in effect keeping the AGX Dynamics object alive, and SetNativeAddress may
 * decrement the reference count, signaling that the Component Instance Data has transferred
 * ownership of the AGX Dynamics object from itself to the new Barrier.
 *
 * An example implementation:
 *
 *     bool UAGX_Example::HasNative() const
 *     {
 *         return NativeBarrier.HasNative();
 *     }
 *
 *     uint64 UAGX_Example::GetNativeAddress() const
 *     {
 *         NativeBarrier.IncrementRefCount(); // This line is optional.
 *         return static_cast<uint64>(NativeBarrier.GetNativeAddress());
 *     }
 *
 *     void UAGX_Example::SetNativeAddress(uint64 NativeAddress)
 *     {
 *     	   NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
 *     	   NativeBarrier.DecrementRefCunt(); // This line is optional.
 *     }
 *
 * We may want to rename Get/SetNativeAddress to something that talks about ownership.
 * Lend/ReturnOwnership perhaps, or Take/ReturnOwnership. Member function names are often written
 * from the perspective of the object itself, so it should probably not be take.
 */
class IAGX_NativeOwner
{
	GENERATED_BODY()
public:
	/** @return True if this Native Owner currently owns a native AGX Dynamics object. */
	virtual bool HasNative() const = 0;

	/** \return The address of the currently owned native AGX Dynamics object, or 0. */
	virtual uint64 GetNativeAddress() const = 0;

	/**
	 * Make this Native Owner the owner of the native AGX Dynamics object at the given address.
	 *
	 * The given address must designate a valid AGX Dynamics object of the correct type for the
	 * actual type implementing this interface. The intention is that Unreal Engine's transaction
	 * system is responsible for maintaining the association between the instance GetNativeAddress
	 * is called on and the instance that SetNativeAddress is called on to guarantee type equality.
	 *
	 * @param NativeAddress The address of the native AGX Dynamics object that this Native Owner
	 * should own.
	 */
	virtual void SetNativeAddress(uint64 NativeAddress) = 0;
};

// Make sure we can hold an address in an uint64.
static_assert(
	std::numeric_limits<uint64>::max() >= std::numeric_limits<uintptr_t>::max(),
	"The Unreal Engine type uint64 isn't large enough to hold a pointer address. We need an Unreal "
	"Engine type for serialization to work. Consider using UPTRINT");
