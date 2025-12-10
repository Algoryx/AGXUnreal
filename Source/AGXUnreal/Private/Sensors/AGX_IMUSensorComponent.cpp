// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_IMUSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

#define LOCTEXT_NAMESPACE "AGX_IMUSensor"

UAGX_IMUSensorComponent::UAGX_IMUSensorComponent()
{
	NativeBarrier.Reset(new FIMUBarrier());
}

#if WITH_EDITOR
bool UAGX_IMUSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			AGX_MEMBER_NAME(bUseAccelerometer), AGX_MEMBER_NAME(bUseGyroscope),
			AGX_MEMBER_NAME(bUseMagnetometer), AGX_MEMBER_NAME(RigidBody)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
			return false;
	}

	return SuperCanEditChange;
}

void UAGX_IMUSensorComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_IMUSensorComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_IMUSensorComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerRange);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerCrossAxisSensitivityX);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerCrossAxisSensitivityY);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerCrossAxisSensitivityZ);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerZeroGBias);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerNoiseRMS);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerSpectralNoiseDensity);

	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeRange);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeCrossAxisSensitivityX);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeCrossAxisSensitivityY);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeCrossAxisSensitivityZ);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeZeroRateBias);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeNoiseRMS);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeSpectralNoiseDensity);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeLinearAccelerationEffects);

	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerRange);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerCrossAxisSensitivityX);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerCrossAxisSensitivityY);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerCrossAxisSensitivityZ);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerZeroFluxBias);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerNoiseRMS);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerSpectralNoiseDensity);
}
#endif // WITH_EDITOR

namespace AGX_IMUSensorComponent_helpers
{
	void SetLocalScope(UAGX_IMUSensorComponent& Component)
	{
		AActor* const Owner = FAGX_ObjectUtilities::GetRootParentActor(Component);
		Component.RigidBody.LocalScope = Owner;
	}

	bool EnsureHasNative(const UAGX_IMUSensorComponent& IMU, const TCHAR* FunctionName)
	{
		if (IMU.HasNative())
			return true;

		UE_LOG(
			LogAGX, Warning,
			TEXT("%s called on IMU Sensor Component '%s' in '%s' with no Native object. Ignoring. "
				 "Make sure this IMU Sensor Component has been registered with an AGX Sensor "
				 "Environment Actor in the Level."),
			FunctionName, *IMU.GetName(), *GetLabelSafe(IMU.GetOwner()));
		return false;
	}
}

void UAGX_IMUSensorComponent::OnRegister()
{
	Super::OnRegister();

	// On Register is called after all object initialization has completed, i.e. Unreal Engine
	// will not be messing with this object anymore. It is now safe to set the Local Scope on our
	// Component References.
	AGX_IMUSensorComponent_helpers::SetLocalScope(*this);
}

FIMUBarrier* UAGX_IMUSensorComponent::GetNativeAsIMU()
{
	if (!HasNative())
		return nullptr;

	return static_cast<FIMUBarrier*>(NativeBarrier.Get());
}

const FIMUBarrier* UAGX_IMUSensorComponent::GetNativeAsIMU() const
{
	if (!HasNative())
		return nullptr;

	return static_cast<FIMUBarrier*>(NativeBarrier.Get());
}

void UAGX_IMUSensorComponent::MarkOutputAsRead()
{
	if (HasNative())
		GetNativeAsIMU()->MarkOutputAsRead();
}

void UAGX_IMUSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeProperties called on IMU Sensor Component '%s' in '%s' which does "
				 "not have a Native object. Nothing will be done."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Super::UpdateNativeProperties();

	if (bUseAccelerometer)
	{
		GetNativeAsIMU()->SetAccelerometerRange(AccelerometerRange);
		GetNativeAsIMU()->SetAccelerometerCrossAxisSensitivityX(AccelerometerCrossAxisSensitivityX);
		GetNativeAsIMU()->SetAccelerometerCrossAxisSensitivityY(AccelerometerCrossAxisSensitivityY);
		GetNativeAsIMU()->SetAccelerometerCrossAxisSensitivityZ(AccelerometerCrossAxisSensitivityZ);
		GetNativeAsIMU()->SetAccelerometerZeroGBias(AccelerometerZeroGBias);
		GetNativeAsIMU()->SetAccelerometerNoiseRMS(AccelerometerNoiseRMS);
		GetNativeAsIMU()->SetAccelerometerSpectralNoiseDensity(AccelerometerSpectralNoiseDensity);
	}

	if (bUseGyroscope)
	{
		GetNativeAsIMU()->SetGyroscopeRange(GyroscopeRange);
		GetNativeAsIMU()->SetGyroscopeCrossAxisSensitivityX(GyroscopeCrossAxisSensitivityX);
		GetNativeAsIMU()->SetGyroscopeCrossAxisSensitivityY(GyroscopeCrossAxisSensitivityY);
		GetNativeAsIMU()->SetGyroscopeCrossAxisSensitivityZ(GyroscopeCrossAxisSensitivityZ);
		GetNativeAsIMU()->SetGyroscopeZeroRateBias(GyroscopeZeroRateBias);
		GetNativeAsIMU()->SetGyroscopeNoiseRMS(GyroscopeNoiseRMS);
		GetNativeAsIMU()->SetGyroscopeSpectralNoiseDensity(GyroscopeSpectralNoiseDensity);
	}

	if (bUseMagnetometer)
	{
		GetNativeAsIMU()->SetMagnetometerRange(MagnetometerRange);
		GetNativeAsIMU()->SetMagnetometerCrossAxisSensitivityX(MagnetometerCrossAxisSensitivityX);
		GetNativeAsIMU()->SetMagnetometerCrossAxisSensitivityY(MagnetometerCrossAxisSensitivityY);
		GetNativeAsIMU()->SetMagnetometerCrossAxisSensitivityZ(MagnetometerCrossAxisSensitivityZ);
		GetNativeAsIMU()->SetMagnetometerZeroFluxBias(MagnetometerZeroFluxBias);
		GetNativeAsIMU()->SetMagnetometerNoiseRMS(MagnetometerNoiseRMS);
		GetNativeAsIMU()->SetMagnetometerSpectralNoiseDensity(MagnetometerSpectralNoiseDensity);
	}
}

void UAGX_IMUSensorComponent::SetAccelerometerRange(double Min, double Max)
{
	SetAccelerometerRange({Min, Max});
}

void UAGX_IMUSensorComponent::SetAccelerometerRange(FAGX_RealInterval Range)
{
	AccelerometerRange = Range;

	if (HasNative())
		GetNativeAsIMU()->SetAccelerometerRange(Range);
}

void UAGX_IMUSensorComponent::GetAccelerometerRange(double& Min, double& Max) const
{
	FAGX_RealInterval Range = GetAccelerometerRange();
	Min = Range.Min;
	Max = Range.Max;
}

FAGX_RealInterval UAGX_IMUSensorComponent::GetAccelerometerRange() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetAccelerometerRange();

	return AccelerometerRange;
}

void UAGX_IMUSensorComponent::SetAccelerometerCrossAxisSensitivityX(FVector X)
{
	AccelerometerCrossAxisSensitivityX = X;
	if (HasNative())
		GetNativeAsIMU()->SetAccelerometerCrossAxisSensitivityX(X);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerCrossAxisSensitivityX()
{
	if (HasNative())
		return GetNativeAsIMU()->GetAccelerometerCrossAxisSensitivityX();

	return AccelerometerCrossAxisSensitivityX;
}

void UAGX_IMUSensorComponent::SetAccelerometerCrossAxisSensitivityY(FVector Y)
{
	AccelerometerCrossAxisSensitivityY = Y;
	if (HasNative())
		GetNativeAsIMU()->SetAccelerometerCrossAxisSensitivityY(Y);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerCrossAxisSensitivityY()
{
	if (HasNative())
		return GetNativeAsIMU()->GetAccelerometerCrossAxisSensitivityY();

	return AccelerometerCrossAxisSensitivityY;
}

void UAGX_IMUSensorComponent::SetAccelerometerCrossAxisSensitivityZ(FVector Z)
{
	AccelerometerCrossAxisSensitivityZ = Z;
	if (HasNative())
		GetNativeAsIMU()->SetAccelerometerCrossAxisSensitivityZ(Z);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerCrossAxisSensitivityZ()
{
	if (HasNative())
		return GetNativeAsIMU()->GetAccelerometerCrossAxisSensitivityZ();

	return AccelerometerCrossAxisSensitivityZ;
}

void UAGX_IMUSensorComponent::SetAccelerometerCrossAxisSensitivity(FVector X, FVector Y, FVector Z)
{
	SetAccelerometerCrossAxisSensitivityX(X);
	SetAccelerometerCrossAxisSensitivityY(Y);
	SetAccelerometerCrossAxisSensitivityZ(Z);
}

void UAGX_IMUSensorComponent::SetAccelerometerZeroGBias(FVector Bias)
{
	AccelerometerZeroGBias = Bias;

	if (HasNative())
		GetNativeAsIMU()->SetAccelerometerZeroGBias(Bias);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerZeroGBias() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetAccelerometerZeroGBias();

	return AccelerometerZeroGBias;
}

void UAGX_IMUSensorComponent::SetAccelerometerNoiseRMS(FVector Noise)
{
	AccelerometerNoiseRMS = Noise;

	if (HasNative())
		GetNativeAsIMU()->SetAccelerometerNoiseRMS(Noise);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerNoiseRMS() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetAccelerometerNoiseRMS();

	return AccelerometerNoiseRMS;
}

void UAGX_IMUSensorComponent::SetAccelerometerSpectralNoiseDensity(FVector Noise)
{
	AccelerometerSpectralNoiseDensity = Noise;

	if (HasNative())
		GetNativeAsIMU()->SetAccelerometerSpectralNoiseDensity(Noise);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerSpectralNoiseDensity() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetAccelerometerSpectralNoiseDensity();

	return AccelerometerSpectralNoiseDensity;
}

void UAGX_IMUSensorComponent::SetGyroscopeRange(double Min, double Max)
{
	SetGyroscopeRange({Min, Max});
}

void UAGX_IMUSensorComponent::SetGyroscopeRange(FAGX_RealInterval Range)
{
	GyroscopeRange = Range;

	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeRange(Range);
}

void UAGX_IMUSensorComponent::GetGyroscopeRange(double& Min, double& Max) const
{
	FAGX_RealInterval Range = GetGyroscopeRange();
	Min = Range.Min;
	Max = Range.Max;
}

FAGX_RealInterval UAGX_IMUSensorComponent::GetGyroscopeRange() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetGyroscopeRange();

	return GyroscopeRange;
}

void UAGX_IMUSensorComponent::SetGyroscopeCrossAxisSensitivityX(FVector X)
{
	GyroscopeCrossAxisSensitivityX = X;
	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeCrossAxisSensitivityX(X);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeCrossAxisSensitivityX()
{
	if (HasNative())
		return GetNativeAsIMU()->GetGyroscopeCrossAxisSensitivityX();

	return GyroscopeCrossAxisSensitivityX;
}

void UAGX_IMUSensorComponent::SetGyroscopeCrossAxisSensitivityY(FVector Y)
{
	GyroscopeCrossAxisSensitivityY = Y;
	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeCrossAxisSensitivityY(Y);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeCrossAxisSensitivityY()
{
	if (HasNative())
		return GetNativeAsIMU()->GetGyroscopeCrossAxisSensitivityY();

	return GyroscopeCrossAxisSensitivityY;
}

void UAGX_IMUSensorComponent::SetGyroscopeCrossAxisSensitivityZ(FVector Z)
{
	GyroscopeCrossAxisSensitivityZ = Z;
	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeCrossAxisSensitivityZ(Z);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeCrossAxisSensitivityZ()
{
	if (HasNative())
		return GetNativeAsIMU()->GetGyroscopeCrossAxisSensitivityZ();

	return GyroscopeCrossAxisSensitivityZ;
}

void UAGX_IMUSensorComponent::SetGyroscopeCrossAxisSensitivity(FVector X, FVector Y, FVector Z)
{
	SetGyroscopeCrossAxisSensitivityX(X);
	SetGyroscopeCrossAxisSensitivityY(Y);
	SetGyroscopeCrossAxisSensitivityZ(Z);
}

void UAGX_IMUSensorComponent::SetGyroscopeZeroRateBias(FVector Bias)
{
	GyroscopeZeroRateBias = Bias;

	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeZeroRateBias(Bias);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeZeroRateBias() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetGyroscopeZeroRateBias();

	return GyroscopeZeroRateBias;
}

void UAGX_IMUSensorComponent::SetGyroscopeNoiseRMS(FVector Noise)
{
	GyroscopeNoiseRMS = Noise;

	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeNoiseRMS(Noise);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeNoiseRMS() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetGyroscopeNoiseRMS();

	return GyroscopeNoiseRMS;
}

void UAGX_IMUSensorComponent::SetGyroscopeSpectralNoiseDensity(FVector Noise)
{
	GyroscopeSpectralNoiseDensity = Noise;

	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeSpectralNoiseDensity(Noise);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeSpectralNoiseDensity() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetGyroscopeSpectralNoiseDensity();

	return GyroscopeSpectralNoiseDensity;
}

void UAGX_IMUSensorComponent::SetGyroscopeLinearAccelerationEffects(FVector Effects)
{
	GyroscopeLinearAccelerationEffects = Effects;

	if (HasNative())
		GetNativeAsIMU()->SetGyroscopeLinearAccelerationEffects(Effects);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeLinearAccelerationEffects() const
{
	// No clean AGX getter exists, but this value should always be in sync since we are the only one
	// that sets it.
	return GyroscopeLinearAccelerationEffects;
}

void UAGX_IMUSensorComponent::SetMagnetometerRange(double Min, double Max)
{
	SetMagnetometerRange({Min, Max});
}

void UAGX_IMUSensorComponent::SetMagnetometerRange(FAGX_RealInterval Range)
{
	MagnetometerRange = Range;

	if (HasNative())
		GetNativeAsIMU()->SetMagnetometerRange(Range);
}

void UAGX_IMUSensorComponent::GetMagnetometerRange(double& Min, double& Max) const
{
	FAGX_RealInterval Range = GetMagnetometerRange();
	Min = Range.Min;
	Max = Range.Max;
}

FAGX_RealInterval UAGX_IMUSensorComponent::GetMagnetometerRange() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetMagnetometerRange();

	return MagnetometerRange;
}

void UAGX_IMUSensorComponent::SetMagnetometerCrossAxisSensitivityX(FVector X)
{
	MagnetometerCrossAxisSensitivityX = X;
	if (HasNative())
		GetNativeAsIMU()->SetMagnetometerCrossAxisSensitivityX(X);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerCrossAxisSensitivityX()
{
	if (HasNative())
		return GetNativeAsIMU()->GetMagnetometerCrossAxisSensitivityX();

	return MagnetometerCrossAxisSensitivityX;
}

void UAGX_IMUSensorComponent::SetMagnetometerCrossAxisSensitivityY(FVector Y)
{
	MagnetometerCrossAxisSensitivityY = Y;
	if (HasNative())
		GetNativeAsIMU()->SetMagnetometerCrossAxisSensitivityY(Y);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerCrossAxisSensitivityY()
{
	if (HasNative())
		return GetNativeAsIMU()->GetMagnetometerCrossAxisSensitivityY();

	return MagnetometerCrossAxisSensitivityY;
}

void UAGX_IMUSensorComponent::SetMagnetometerCrossAxisSensitivityZ(FVector Z)
{
	MagnetometerCrossAxisSensitivityZ = Z;
	if (HasNative())
		GetNativeAsIMU()->SetMagnetometerCrossAxisSensitivityZ(Z);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerCrossAxisSensitivityZ()
{
	if (HasNative())
		return GetNativeAsIMU()->GetMagnetometerCrossAxisSensitivityZ();

	return MagnetometerCrossAxisSensitivityZ;
}

void UAGX_IMUSensorComponent::SetMagnetometerCrossAxisSensitivity(FVector X, FVector Y, FVector Z)
{
	SetMagnetometerCrossAxisSensitivityX(X);
	SetMagnetometerCrossAxisSensitivityY(Y);
	SetMagnetometerCrossAxisSensitivityZ(Z);
}

void UAGX_IMUSensorComponent::SetMagnetometerZeroFluxBias(FVector Bias)
{
	MagnetometerZeroFluxBias = Bias;

	if (HasNative())
		GetNativeAsIMU()->SetMagnetometerZeroFluxBias(Bias);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerZeroFluxBias() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetMagnetometerZeroFluxBias();

	return MagnetometerZeroFluxBias;
}

void UAGX_IMUSensorComponent::SetMagnetometerNoiseRMS(FVector Noise)
{
	MagnetometerNoiseRMS = Noise;

	if (HasNative())
		GetNativeAsIMU()->SetMagnetometerNoiseRMS(Noise);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerNoiseRMS() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetMagnetometerNoiseRMS();

	return MagnetometerNoiseRMS;
}

void UAGX_IMUSensorComponent::SetMagnetometerSpectralNoiseDensity(FVector Noise)
{
	MagnetometerSpectralNoiseDensity = Noise;

	if (HasNative())
		GetNativeAsIMU()->SetMagnetometerSpectralNoiseDensity(Noise);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerSpectralNoiseDensity() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetMagnetometerSpectralNoiseDensity();

	return MagnetometerSpectralNoiseDensity;
}

FVector UAGX_IMUSensorComponent::GetAccelerometerDataLocal() const
{
	using namespace AGX_IMUSensorComponent_helpers;
	if (!EnsureHasNative(*this, TEXT("UAGX_IMUSensorComponent::GetAccelerometerData()")))
	{
		return FVector::ZeroVector;
	}

	if (!bUseAccelerometer)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_IMUSensorComponent::GetAccelerometerData() called in IMU Sensor Component "
				 "'%s' in '%s' that does not have an Accelerometer. Returning zero vector."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return FVector::ZeroVector;
	}

	return GetNativeAsIMU()->GetAccelerometerData();
}

FVector UAGX_IMUSensorComponent::GetAccelerometerDataWorld() const
{
	const FVector LocalAccel = GetAccelerometerDataLocal();
	const FTransform IMUTransform = GetNativeAsIMU()->GetTransform();
	const FVector WorldAccel = IMUTransform.TransformVectorNoScale(LocalAccel);
	return WorldAccel;
}

FVector UAGX_IMUSensorComponent::GetGyroscopeDataLocal() const
{
	using namespace AGX_IMUSensorComponent_helpers;
	if (!EnsureHasNative(*this, TEXT("UAGX_IMUSensorComponent::GetGyroscopeDataLocal()")))
		return FVector::ZeroVector;

	if (!bUseGyroscope)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_IMUSensorComponent::GetGyroscopeDataLocal() called in IMU Sensor Component "
				 "'%s' in '%s' that does not have a Gyroscope. Returning zero vector."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return FVector::ZeroVector;
	}

	return GetNativeAsIMU()->GetGyroscopeData();
}

FVector UAGX_IMUSensorComponent::GetGyroscopeDataWorld() const
{
	const FVector LocalGyro = GetGyroscopeDataLocal();
	const FTransform IMUTransform = GetNativeAsIMU()->GetTransform();
	const FVector WorldGyro = IMUTransform.TransformVectorNoScale(LocalGyro);
	return WorldGyro;
}

FVector UAGX_IMUSensorComponent::GetMagnetometerDataLocal() const
{
	using namespace AGX_IMUSensorComponent_helpers;
	if (!EnsureHasNative(*this, TEXT("UAGX_IMUSensorComponent::GetMagnetometerDataLocal()")))
		return FVector::ZeroVector;

	if (!bUseMagnetometer)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"UAGX_IMUSensorComponent::GetMagnetometerDataLocal() called in IMU Sensor "
				"Component '%s' in '%s' that does not have a Magnetometer. Returning zero vector."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return FVector::ZeroVector;
	}

	return GetNativeAsIMU()->GetMagnetometerData();
}

FVector UAGX_IMUSensorComponent::GetMagnetometerDataWorld() const
{
	const FVector LocalMag = GetMagnetometerDataLocal();
	const FTransform IMUTransform = GetNativeAsIMU()->GetTransform();
	const FVector WorldMag = IMUTransform.TransformVectorNoScale(LocalMag);
	return WorldMag;
}

void UAGX_IMUSensorComponent::SetPosition(FVector Position)
{
	SetWorldLocation(Position);

	if (HasNative())
		GetNativeAsIMU()->SetPosition(Position);
}

FVector UAGX_IMUSensorComponent::GetPosition() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetPosition();

	return GetComponentLocation();
}

void UAGX_IMUSensorComponent::SetRotation(FQuat Rotation)
{
	SetWorldRotation(Rotation);

	if (HasNative())
		GetNativeAsIMU()->SetRotation(Rotation);
}

FQuat UAGX_IMUSensorComponent::GetRotation() const
{
	if (HasNative())
		return GetNativeAsIMU()->GetRotation();

	return GetComponentQuat();
}

void UAGX_IMUSensorComponent::UpdateTransformFromNative()
{
	if (!HasNative())
		return;

	SetWorldTransform(GetNativeAsIMU()->GetTransform());
}

FSensorBarrier* UAGX_IMUSensorComponent::CreateNativeImpl()
{
	Super::CreateNativeImpl();

	AGX_CHECK(!HasNative());
	if (HasNative())
		return NativeBarrier.Get();

	auto Body = RigidBody.GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_IMUSensorComponent::CreateNativeImpl called on IMU Sensor Component '%s' in "
				 "'%s' which does not have a valid Rigid Body selected. Native object will not be "
				 "created."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return nullptr;
	}

	auto BodyBarrier = Body->GetOrCreateNative();

	if (BodyBarrier == nullptr || !BodyBarrier->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The selected Rigid Body for IMU Sensor Component '%s' in "
				 "'%s' does not have a valid Native Object."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return nullptr;
	}

	FTransform BodyTransform(Body->GetRotation(), Body->GetPosition());

	FIMUAllocationParameters Params;
	Params.bUseAccelerometer = bUseAccelerometer;
	Params.bUseGyroscope = bUseGyroscope;
	Params.bUseMagnetometer = bUseMagnetometer;
	Params.LocalTransform = GetComponentTransform().GetRelativeTransform(BodyTransform);

	auto IMUBarrier = static_cast<FIMUBarrier*>(NativeBarrier.Get());
	IMUBarrier->AllocateNative(Params, *BodyBarrier);
	if (HasNative())
		UpdateNativeProperties();

	return IMUBarrier;
}

#undef LOCTEXT_NAMESPACE
