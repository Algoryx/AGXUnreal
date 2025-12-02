// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_RealInterval.h"
#include "AGX_RigidBodyReference.h"
#include "Wire/WireWinchBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/EngineBaseTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireWinch.generated.h"

/// \todo Should FAGX_WireWinchSettings and FAGX_WireWinch be BlueprintType, or only FAGX_WireApi?

/**
 * Holds all the properties that make up the settings of a Wire Winch. The Barrier object and all
 * the functions are in FAGX_WireWinch, which inherits from this struct. We need this extra layer
 * because Unreal Engine require that all structs be copy assignable but our Barrier objects are
 * not copyable. By moving all the members that should be copied by the copy assignment operator
 * to a base class we make it safer to implement the operator.
 *
 * Access to these properties are provided by FAGX_WireWinch_BP and UAGX_WireWinch_FL because of
 * limitations in how the Blueprint VM handles structs.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_WireWinchSettings
{
	GENERATED_BODY()
public:
	/**
	 * The location of the winch relative to some parent object [cm].
	 *
	 * For a Wire Component owned winch without a body attachment the parent is the Wire Component.
	 * For a Wire Component owned winch with a body attachment the parent is the body.
	 * For a Winch Component owned winch the parent is always the Winch Component.
	 *
	 * Only used during setup, cannot be changed after Begin Play.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FVector Location {FVector::ZeroVector};

	/**
	 * The orientation of the winch relative to some parent object [deg].
	 *
	 * For a Wire Component owned winch without a body attachment the parent is the Wire Component.
	 * For a Wire Component owned winch with a body attachment the parent is the body.
	 * For a Winch Component owned winch the parent is always the Winch Component.
	 *
	 * With a zero rotation the winch will point the wire along the X axis.
	 *
	 * Only used during setup, cannot be changed once Begin Play has been called.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FRotator Rotation {FRotator::ZeroRotator};

	/**
	 * The location passed to AGX Dynamics when the native wire winch is created. It is computed
	 * once by the owning Component's CreateNative function based on the Location UProperty and the
	 * Location's parent object, according to the documentation for Location. It holds either the
	 * world space or the body-relative location of the winch based on the Location UProperty.
	 */
	FVector LocationSim {FVector::ZeroVector};

	/**
	 * The rotation passed to AGX Dynamics when the native wire winch is created. It is computed
	 * once by the owning Component's CreateNative function based on the Rotation UProperty and the
	 * Rotation's parent object, according to the documentation for Rotation. It holds either the
	 * world space or the body-relative rotation of the winch based on the Rotation UProperty.
	 */
	FRotator RotationSim {FRotator::ZeroRotator};

	// I would perhaps like to make this a BlueprintReadWrite property, but FAGX_RigidBodyReference
	// is currently not a BlueprintType. Should it be? We deliberately did not make it a Blueprint
	// type previously because the type is a bit "special" and we didn't know what the implications
	// of making it a BlueprintType would be. Do we know more now? Should it be made a
	// BlueprintType? What operations on it should we support in Blueprint Visual Scripts?
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FAGX_RigidBodyReference BodyAttachment;

	/**
	 * The amount of wire that exists inside the winch [cm].
	 *
	 * If auto feed is enabled then this value is decreased by the length of the wire route on
	 * initialization.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	double PulledInLength = 1000.0;

	/**
	 * This mode can be used to facilitate routing. E.g., put all wire in this winch (start of
	 * route), set mode to auto feed, and route. The wire will automatically decrease in this winch
	 * to fulfill the rout without initial tension.
	 *
	 * Only used during setup, cannot be changed once Begin Play has been called.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	bool bAutoFeed = false;

	/**
	 * Set to true to enable the motor paying out or hauling in wire. The direction is controlled
	 * by the sign of Target Speed.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	bool bMotorEnabled = true;

	/**
	 * The speed at which this winch is trying to haul in or pay out wire [cm/s].
	 *
	 * It may be unable to reach the target speed because of force range limitations and resistance
	 * from the brake.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	double TargetSpeed = 0.0;

	/**
	 * The allowed force range when paying out and hauling in wire [N].
	 *
	 * The lower end of the range is used when hauling in and the upper range when paying out. The
	 * lower end must be less than zero and the upper range must be greater than zero.
	 *
	 * The actual force range on the Native is set to this value while the motor is enabled. When
	 * the motor is disabled the Native's motor force range is instead set to zero.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FAGX_RealInterval MotorForceRange {
		-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};

	/**
	 * Set to true to enable the brake.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	bool bBrakeEnabled = false;

	/**
	 * Set the desired brake force range [N].
	 *
	 * The lower end of the range is the force with which this winch brake can hold the wire from
	 * hauling in and the upper end of the range is the force with which this winch brake can hold
	 * the wire from paying out.
	 *
	 * It's important that the lower value is less than zero and the upper larger than zero.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Winch")
	FAGX_RealInterval BrakeForceRange {
		-std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
};

/**
 * A Wire Winch is a carrier for the data required to create a Wire Winch along with the Barrier
 * object that houses the native AGX Dynamics instance. It is a simple struct, not any kind of
 * UObject, which means that it can't do much of the work itself, it needs help from an owning
 * Component. Most of the logic and customization required is performed by the UObject owning the
 * Wire Winch.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_WireWinch : public FAGX_WireWinchSettings
{
	/*
	 * I would like to have this class implement the IAGX_NativeOwner interface. Unreal Header Tool
	 * doesn't allow UStructs inheriting from non-UStructs, but that can be worked around by
	 * guarding that inheritance behind #if CPP. However, simply adding a virtual destructor to this
	 * class and marking the native-address-related member functions, named identical to the
	 * IAGX_NativeOwner member functions, causes Unreal Editor to crash due to a segmentation fault
	 * during Blueprint Reconstruction while the editor is starting if there is a Wire in the level
	 * being loaded. I have no idea why, or how to fix it. I don't know if we're doing something
	 * illegal, or if it's because of incompatible state restored from disk. So for now this class
	 * is a Native Owner despite not implementing the interface.
	 */

	GENERATED_BODY()

public:
	FAGX_WireWinch() = default;

	/**
	 * Copy constructor that only copies the Properties seen by Unreal, does not copy the Native
	 * Barrier Engine.
	 */
	FAGX_WireWinch(const FAGX_WireWinch& Other);

	/**
	 * Assignment operator that only copies the Properties seen by Unreal, does not copy the Native
	 * Barrier Engine.
	 */
	FAGX_WireWinch& operator=(const FAGX_WireWinch& Other);

	bool SetBodyAttachment(UAGX_RigidBodyComponent* Body);
	UAGX_RigidBodyComponent* GetBodyAttachment() const;

	void SetPulledInLength(double InPulledInLength);
	double GetPulledInLength() const;

	void EnableMotor();
	void DisableMotor();
	void SetMotorEnabled(bool bInEnable);
	bool IsMotorEnabled() const;

	void SetTargetSpeed(double InTargetSpeed);
	double GetTargetSpeed() const;

	void SetMotorForceRange(const FAGX_RealInterval& InForceRange);
	void SetMotorForceRange(double InMin, double InMax);
	void SetMotorForceRangeMin(double InMin);
	void SetMotorForceRangeMax(double InMax);

	FAGX_RealInterval GetMotorForceRange() const;
	double GetMotorForceRangeMin() const;
	double GetMotorForceRangeMax() const;

	void EnableBrake();
	void DisableBrake();
	void SetBrakeEnabled(bool bEnable);
	bool IsBrakeEnabled() const;

	void SetBrakeForceRange(const FAGX_RealInterval& InBrakeForceRange);
	void SetBrakeForceRange(double InMin, double InMax);
	void SetBrakeForceRangeMin(double InMin);
	void SetBrakeForceRangeMax(double InMax);

	FAGX_RealInterval GetBrakeForceRange() const;
	double GetBrakeForceRangeMin() const;
	double GetBrakeForceRangeMax() const;

	/**
	 * @return The speed with which the wire is currently being hauled in, for negative speeds, or
	 * payed out, for positive speeds.
	 */
	double GetCurrentSpeed() const;
	double GetCurrentMotorForce() const;
	double GetCurrentBrakeForce() const;

	bool HasWire() const;

	void CopyFrom(const FWireWinchBarrier& Barrier);

	//~ Begin AGX_NativeOwner interface.
	// We can't do actual inheritance, see comment at the top of the class, but we can at least
	// expose the same member functions.
	bool HasNative() const;
	uint64 GetNativeAddress() const;
	void SetNativeAddress(uint64 NativeAddress);
	//~ End AGX_NativeOwner interface.

	void CreateNative();
	FWireWinchBarrier* GetNative();
	const FWireWinchBarrier* GetNative() const;
	FWireWinchBarrier* GetOrCreateNative();
	void WritePropertiesToNative();
	void ReadPropertiesFromNative();

public:
	FWireWinchBarrier NativeBarrier;
};

/**
 * A Wire Winch is a device that can haul in and pay out a connected wire.
 *
 * You can get access to a particular Wire Component's begin or end winch by calling Get Begin Winch
 * or Get End Winch on it. Wire Winch Components also contain a Wire Winch, but provide manipulator
 * functions directly so direct access to the Wire Winch object is rarely necessary.
 *
 * This struct is only a handle to the actual Wire Winch and should only be used as a means to get
 * access to the Wire Winch from a Blueprint Visual Script. It must not be stored past the end of
 * the current Blueprint execution sequence! Re-fetch the Wire Winch from the Wire Component or the
 * Wire Winch Component every time you need it.
 */
/*
 * We need this extra level of indirection between the Blueprint VM and FAGX_WireWinch because the
 * VM cannot handle references to structs returned from a function, such as GetBeginWinch, it always
 * copies the struct into the VM's memory before calling whatever function the next Blueprint node
 * along the wire is, typically one of the ones from UAGX_WireWinch_FL below. Also, Unreal Engine
 * does not support UFunctions on structs. Combined is a problem because FAGX_WireWinch contains a
 * non-copyable handle to the AGX Dynamics representation of the winch and thus the copy can't
 * manipulate the AGX Dynamics winch state.
 *
 * Consider the following Visual Script example:
 *
 *   Wire Blueprint variable -> Get Begin Winch -> Set Target Speed
 *
 * Get Begin Winch could return a reference to the actual FAGX_WireWinch, but the VM would create a
 * copy before calling Set Target Speed, which would therefore operate on an empty winch, one with
 * an invalid Barrier.
 *
 * We work around the limitation with this extra layer of indirection. By retuning a struct
 * containing a pointer to the FAGX_WireWinch the Blueprint VM can copy all it wants, and by
 * unpacking the pointer in the Blueprint Function Library functions below we are able to manipulate
 * the AGX Dynamics winch.
 *
 * An alternative approach would be to make the Barriers copyable, as opposed to just movable, but
 * testing has showed that changes made to the Blueprint VM's copy by the Blueprint Function Library
 * won't be copied back to the Wire Component's internal FAGX_WireWinch instance. This may be a bug
 * or something we're not doing right.
 */
USTRUCT(BlueprintType, Meta = (DisplayName = "AGX Wire Winch Ref"))
struct AGXUNREAL_API FAGX_WireWinchRef
{
	GENERATED_BODY()

	FAGX_WireWinchRef() = default;
	FAGX_WireWinchRef(FAGX_WireWinch* InWinch);

	/**
	 * @return True if there is a Wire Winch available.
	 */
	bool IsValid() const;

	FAGX_WireWinch* Winch = nullptr;
};

/**
 * Blueprint function library for the Wire Winch. Mostly conversions between float and double.
 * Required because structs can't have Blueprint Callable functions.
 */
UCLASS()
class AGXUNREAL_API UAGX_WireWinch_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "AGX Wire Winch")
	static FAGX_WireWinchRef MakeRef(UPARAM(Ref) FAGX_WireWinch& Winch);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	static void SetLocation(FAGX_WireWinchRef Winch, FVector InLocation);

	UFUNCTION(BlueprintPure, Category = "AGX Wire Winch")
	static FVector GetLocation(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	static void SetRotation(FAGX_WireWinchRef Winch, FRotator InRotation);

	UFUNCTION(BlueprintPure, Category = "AGX Wire Winch")
	static FRotator GetRotation(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	static bool SetBodyAttachment(FAGX_WireWinchRef Winch, UAGX_RigidBodyComponent* Body);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static UAGX_RigidBodyComponent* GetBodyAttachment(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	static void SetPulledInLength(FAGX_WireWinchRef Winch, float InPulledInLength);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static float GetPulledInLength(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	static void SetMotorEnabled(FAGX_WireWinchRef Winch, bool bMotorEnabled);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static bool IsMotorEnabled(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	static void SetMotorForceRange(FAGX_WireWinchRef Winch, float Min, float Max);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static float GetMotorForceRangeMin(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static float GetMotorForceRangeMax(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	static void SetBrakeForceRange(FAGX_WireWinchRef Winch, float Min, float Max);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static float GetBrakeForceRangeMin(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static float GetBrakeForceRangeMax(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	static void SetBrakeEnabled(FAGX_WireWinchRef Winch, bool bInBrakeEnabled);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static bool IsBrakeEnabled(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintCallable, Category = "Wire Winch")
	static void SetTargetSpeed(FAGX_WireWinchRef Winch, float InTargetSpeed);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static float GetTargetSpeed(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static float GetCurrentSpeed(FAGX_WireWinchRef Winch);

	UFUNCTION(BlueprintPure, Category = "Wire Winch")
	static bool HasWire(FAGX_WireWinchRef Winch);
};
