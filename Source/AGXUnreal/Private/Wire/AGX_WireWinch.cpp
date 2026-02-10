// Copyright 2026, Algoryx Simulation AB.

#include "Wire/AGX_WireWinch.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "CoreGlobals.h"
#include "Kismet/KismetMathLibrary.h"

bool FAGX_WireWinch::SetBodyAttachment(UAGX_RigidBodyComponent* Body)
{
	if (HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("SetBodyAttachment called on Wire Winch that have already been initialized. "
				 "Cannot move initialized winches between bodies."));
		return false;
	}

	BodyAttachment.SetComponent(Body);
	return true;
}

UAGX_RigidBodyComponent* FAGX_WireWinch::GetBodyAttachment() const
{
	return BodyAttachment.GetRigidBody();
}

void FAGX_WireWinch::SetPulledInLength(double InPulledInLength)
{
	PulledInLength = InPulledInLength;
	if (HasNative())
	{
		NativeBarrier.SetPulledInWireLength(InPulledInLength);
	}
}

double FAGX_WireWinch::GetPulledInLength() const
{
	if (HasNative())
	{
		return NativeBarrier.GetPulledInWireLength();
	}
	return PulledInLength;
}

void FAGX_WireWinch::EnableMotor()
{
	bMotorEnabled = true;
	if (HasNative())
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
}

void FAGX_WireWinch::DisableMotor()
{
	bMotorEnabled = false;
	if (HasNative())
	{
		NativeBarrier.SetForceRange(0.0, 0.0);
	}
}

void FAGX_WireWinch::SetMotorEnabled(bool bInEnable)
{
	bInEnable ? EnableMotor() : DisableMotor();
}

bool FAGX_WireWinch::IsMotorEnabled() const
{
	return bMotorEnabled;
}

void FAGX_WireWinch::SetTargetSpeed(double InTargetSpeed)
{
	TargetSpeed = InTargetSpeed;
	if (HasNative())
	{
		NativeBarrier.SetSpeed(InTargetSpeed);
	}
}

double FAGX_WireWinch::GetTargetSpeed() const
{
	if (HasNative())
	{
		return NativeBarrier.GetSpeed();
	}
	return TargetSpeed;
}

void FAGX_WireWinch::SetMotorForceRange(const FAGX_RealInterval& InForceRange)
{
	MotorForceRange = InForceRange;
	if (bMotorEnabled && HasNative())
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
}

void FAGX_WireWinch::SetMotorForceRange(double InMin, double InMax)
{
	SetMotorForceRange(FAGX_RealInterval(InMin, InMax));
}

void FAGX_WireWinch::SetMotorForceRangeMin(double InMin)
{
	SetMotorForceRange(InMin, MotorForceRange.Max);
}

void FAGX_WireWinch::SetMotorForceRangeMax(double InMax)
{
	SetMotorForceRange(MotorForceRange.Min, InMax);
}

FAGX_RealInterval FAGX_WireWinch::GetMotorForceRange() const
{
	if (HasNative())
	{
		return NativeBarrier.GetForceRange();
	}
	return bMotorEnabled ? MotorForceRange : FAGX_RealInterval(0.0, 0.0);
}

double FAGX_WireWinch::GetMotorForceRangeMin() const
{
	return GetMotorForceRange().Min;
}

double FAGX_WireWinch::GetMotorForceRangeMax() const
{
	return GetMotorForceRange().Max;
}

void FAGX_WireWinch::EnableBrake()
{
	bBrakeEnabled = true;
	if (HasNative())
	{
		NativeBarrier.SetBrakeForceRange(BrakeForceRange);
	}
}

void FAGX_WireWinch::DisableBrake()
{
	bBrakeEnabled = false;
	if (HasNative())
	{
		NativeBarrier.SetBrakeForceRange(0.0, 0.0);
	}
}

void FAGX_WireWinch::SetBrakeEnabled(bool bEnable)
{
	if (bEnable)
	{
		EnableBrake();
	}
	else
	{
		DisableBrake();
	}
}

bool FAGX_WireWinch::IsBrakeEnabled() const
{
	return bBrakeEnabled;
}

void FAGX_WireWinch::SetBrakeForceRange(const FAGX_RealInterval& InForceRange)
{
	BrakeForceRange = InForceRange;
	if (bBrakeEnabled && HasNative())
	{
		NativeBarrier.SetBrakeForceRange(InForceRange);
	}
}

void FAGX_WireWinch::SetBrakeForceRange(double InMin, double InMax)
{
	SetBrakeForceRange(FAGX_RealInterval(InMin, InMax));
}

void FAGX_WireWinch::SetBrakeForceRangeMin(double InMin)
{
	SetBrakeForceRange(InMin, BrakeForceRange.Max);
}

void FAGX_WireWinch::SetBrakeForceRangeMax(double InMax)
{
	SetBrakeForceRange(BrakeForceRange.Min, InMax);
}

FAGX_RealInterval FAGX_WireWinch::GetBrakeForceRange() const
{
	if (HasNative())
	{
		return NativeBarrier.GetBrakeForceRange();
	}
	return BrakeForceRange;
}

double FAGX_WireWinch::GetBrakeForceRangeMin() const
{
	return GetBrakeForceRange().Min;
}

double FAGX_WireWinch::GetBrakeForceRangeMax() const
{
	return GetBrakeForceRange().Max;
}

double FAGX_WireWinch::GetCurrentSpeed() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetCurrentSpeed();
}

double FAGX_WireWinch::GetCurrentMotorForce() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetCurrentForce();
}

double FAGX_WireWinch::GetCurrentBrakeForce() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetCurrentBrakeForce();
}

bool FAGX_WireWinch::HasWire() const
{
	if (!HasNative())
	{
		/// @todo This is not true since a Wire may have a reference a Wire Winch that owns this
		/// winch, but there is currently no way of knowing that.
		return false;
	}
	return NativeBarrier.HasWire();
}

void FAGX_WireWinch::CopyFrom(const FWireWinchBarrier& Barrier)
{
	Location = Barrier.GetLocation();
	Rotation = Barrier.GetNormal().Rotation();
	// Not entirely certain that the rotation computation is correct.
	// An alternative is to use UKismetMathLibrary::MakeRotFromX.
	// They give very similar, but not identical, results.
	// For the winch normal agx.Vec3(0.696526, 0.398015, 0.597022).normal()
	// .Rotation() produces    P=36.656910 Y=-29.744896 R=0.000000 and
	// MakeRotFromX() produces P=36.656910 Y=-29.744896 R=-0.000003.
	//                                                            ^

	// Body Attachment not set here since this is a pure data copy.
	// For AGX Dynamics archive import the Body Attachment is set by AGX_ArchiveImporterHelper.

	PulledInLength = Barrier.GetPulledInWireLength();
	bAutoFeed = Barrier.GetAutoFeed();
	bMotorEnabled = !Barrier.GetForceRange().IsZero();
	TargetSpeed = Barrier.GetSpeed();
	MotorForceRange = Barrier.GetForceRange();
	bBrakeEnabled = !Barrier.GetBrakeForceRange().IsZero();
	BrakeForceRange = Barrier.GetBrakeForceRange();
}

bool FAGX_WireWinch::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 FAGX_WireWinch::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	NativeBarrier.IncrementRefCount();
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void FAGX_WireWinch::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
	NativeBarrier.DecrementRefCount();
}

void FAGX_WireWinch::CreateNative()
{
	check(!GIsReconstructingBlueprintInstances);
	check(!HasNative());
	FRigidBodyBarrier* Body = [this]() -> FRigidBodyBarrier*
	{
		UAGX_RigidBodyComponent* Body = BodyAttachment.GetRigidBody();
		if (Body == nullptr)
		{
			return nullptr;
		}
		return Body->GetOrCreateNative();
	}();
	NativeBarrier.AllocateNative(
		Body, LocationSim, RotationSim.RotateVector(FVector::ForwardVector), PulledInLength);
	WritePropertiesToNative();
}

FWireWinchBarrier* FAGX_WireWinch::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FWireWinchBarrier* FAGX_WireWinch::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

FWireWinchBarrier* FAGX_WireWinch::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

void FAGX_WireWinch::WritePropertiesToNative()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_WireWinch: Cannot read properties from native: Dont' have a native."));
		return;
	}
	NativeBarrier.SetPulledInWireLength(PulledInLength);
	NativeBarrier.SetAutoFeed(bAutoFeed);
	NativeBarrier.SetSpeed(TargetSpeed);
	if (bMotorEnabled)
	{
		NativeBarrier.SetForceRange(MotorForceRange);
	}
	else
	{
		NativeBarrier.SetForceRange(0.0, 0.0);
	}
	if (bBrakeEnabled)
	{
		NativeBarrier.SetBrakeForceRange(BrakeForceRange);
	}
	else
	{
		NativeBarrier.SetBrakeForceRange(0.0, 0.0);
	}
}

void FAGX_WireWinch::ReadPropertiesFromNative()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FAGX_WireWinch: Cannot read properties from native: Dont' have a native."));
		return;
	}

	PulledInLength = NativeBarrier.GetPulledInWireLength();
	bAutoFeed = NativeBarrier.GetAutoFeed();
	TargetSpeed = NativeBarrier.GetSpeed();
	if (bMotorEnabled)
	{
		MotorForceRange = NativeBarrier.GetForceRange();
	}
	if (bBrakeEnabled)
	{
		BrakeForceRange = NativeBarrier.GetBrakeForceRange();
	}
}

FAGX_WireWinch::FAGX_WireWinch(const FAGX_WireWinch& Other)
	: FAGX_WireWinchSettings(Other)
{
}

FAGX_WireWinch& FAGX_WireWinch::operator=(const FAGX_WireWinch& Other)
{
	FAGX_WireWinchSettings::operator=(Other);
	return *this;
}

/* Start of FAGX_WireWinch_BP. */

FAGX_WireWinchRef::FAGX_WireWinchRef(FAGX_WireWinch* InWinch)
	: Winch(InWinch)
{
}

bool FAGX_WireWinchRef::IsValid() const
{
	return Winch != nullptr;
}

/* Start of Blueprint Function Library. */

FAGX_WireWinchRef UAGX_WireWinch_FL::MakeRef(UPARAM(Ref) FAGX_WireWinch& Winch)
{
	return {&Winch};
}

void UAGX_WireWinch_FL::SetLocation(FAGX_WireWinchRef Winch, FVector InLocation)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetLocation."));
		return;
	}
	Winch.Winch->Location = InLocation;
}

FVector UAGX_WireWinch_FL::GetLocation(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetLocation."));
		return FVector(ForceInit);
	}
	return Winch.Winch->Location;
}

void UAGX_WireWinch_FL::SetRotation(FAGX_WireWinchRef Winch, FRotator InRotation)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetRotation."));
		return;
	}
	Winch.Winch->Rotation = InRotation;
}

FRotator UAGX_WireWinch_FL::GetRotation(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetRotation."));
		return FRotator(ForceInit);
	}
	return Winch.Winch->Rotation;
}

bool UAGX_WireWinch_FL::SetBodyAttachment(FAGX_WireWinchRef Winch, UAGX_RigidBodyComponent* Body)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetBodyAttachment."));
		return false;
	}
	return Winch.Winch->SetBodyAttachment(Body);
}
UAGX_RigidBodyComponent* UAGX_WireWinch_FL::GetBodyAttachment(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetBodyAttachment."));
		return nullptr;
	}
	return Winch.Winch->GetBodyAttachment();
}

void UAGX_WireWinch_FL::SetPulledInLength(FAGX_WireWinchRef Winch, float InPulledInLength)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetPulledInLength."));
		return;
	}
	return Winch.Winch->SetPulledInLength(static_cast<double>(InPulledInLength));
}

float UAGX_WireWinch_FL::GetPulledInLength(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetPulledInLength."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetPulledInLength());
}

void UAGX_WireWinch_FL::SetMotorEnabled(FAGX_WireWinchRef Winch, bool bMotorEnabled)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetMotorEnabled."));
		return;
	}
	return Winch.Winch->SetMotorEnabled(bMotorEnabled);
}

bool UAGX_WireWinch_FL::IsMotorEnabled(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to IsMotorEnabled."));
		return false;
	}
	return Winch.Winch->IsMotorEnabled();
}

void UAGX_WireWinch_FL::SetMotorForceRange(FAGX_WireWinchRef Winch, float Min, float Max)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetMotorForceRange."));
		return;
	}
	return Winch.Winch->SetMotorForceRange(static_cast<double>(Min), static_cast<double>(Max));
}
float UAGX_WireWinch_FL::GetMotorForceRangeMin(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetMotorForceRangeMin."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetMotorForceRangeMin());
}

float UAGX_WireWinch_FL::GetMotorForceRangeMax(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetMotorForceRangeMax."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetMotorForceRangeMax());
}

void UAGX_WireWinch_FL::SetBrakeForceRange(FAGX_WireWinchRef Winch, float Min, float Max)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetBrakeForceRange."));
		return;
	}
	return Winch.Winch->SetBrakeForceRange(static_cast<double>(Min), static_cast<double>(Max));
}

float UAGX_WireWinch_FL::GetBrakeForceRangeMin(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetBrakeForceRangeMin."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetBrakeForceRangeMin());
}

float UAGX_WireWinch_FL::GetBrakeForceRangeMax(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetBrakeForceRangeMax."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetBrakeForceRangeMax());
}

void UAGX_WireWinch_FL::SetBrakeEnabled(FAGX_WireWinchRef Winch, bool bInBrakeEnabled)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetBrakeEnabled."));
		return;
	}
	return Winch.Winch->SetBrakeEnabled(bInBrakeEnabled);
}

bool UAGX_WireWinch_FL::IsBrakeEnabled(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to IsBrakeEnabled."));
		return false;
	}
	return Winch.Winch->IsBrakeEnabled();
}

void UAGX_WireWinch_FL::SetTargetSpeed(FAGX_WireWinchRef Winch, float InTargetSpeed)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to SetTargetSpeed."));
		return;
	}
	return Winch.Winch->SetTargetSpeed(static_cast<double>(InTargetSpeed));
}

float UAGX_WireWinch_FL::GetTargetSpeed(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetTargetSpeed."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetTargetSpeed());
}

float UAGX_WireWinch_FL::GetCurrentSpeed(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to GetCurrentSpeed."));
		return 0.0f;
	}
	return static_cast<float>(Winch.Winch->GetCurrentSpeed());
}

bool UAGX_WireWinch_FL::HasWire(FAGX_WireWinchRef Winch)
{
	if (!Winch.IsValid())
	{
		UE_LOG(LogAGX, Error, TEXT("Invalid Wire Winch Ref passed to Has Wire"));
		return false;
	}

	return Winch.Winch->HasWire();
}
