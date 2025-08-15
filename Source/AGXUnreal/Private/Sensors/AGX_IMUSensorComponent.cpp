// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_IMUSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RealInterval.h"
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"

#define LOCTEXT_NAMESPACE "AGX_IMUSensor"

UAGX_IMUSensorComponent::UAGX_IMUSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_IMUSensorComponent::SetEnabled(bool InEnabled)
{
	bEnabled = InEnabled;

	if (HasNative())
		NativeBarrier.SetEnabled(InEnabled);
}

bool UAGX_IMUSensorComponent::GetEnabled() const
{
	if (HasNative())
		return NativeBarrier.GetEnabled();

	return bEnabled;
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
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseAccelerometer),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseGyroscope),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseMagnetometer),
			GET_MEMBER_NAME_CHECKED(ThisClass, RigidBody)};

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

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_IMUSensorComponent, bEnabled),
		[](ThisClass* This) { This->SetEnabled(This->bEnabled); });

	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerRange);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerCrossAxisSensitivity);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerZeroGBias);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerNoiseRMS);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AccelerometerSpectralNoiseDensity);

	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeRange);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeCrossAxisSensitivity);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeZeroRateBias);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeNoiseRMS);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeSpectralNoiseDensity);
	AGX_COMPONENT_DEFAULT_DISPATCHER(GyroscopeLinearAccelerationEffects);

	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerRange);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MagnetometerCrossAxisSensitivity);
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
			TEXT("%s called on IMU Sensor Component '%s' in '%s' with no Native object. Ignoring."),
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

void UAGX_IMUSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
	{
		UE_LOG(
			LogTemp, Warning,
			TEXT("UpdateNativeProperties called on IMU Sensor Component '%s' in '%s' which does "
				 "not have a Native object. Nothing will be done."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	NativeBarrier.SetEnabled(bEnabled);
	NativeBarrier.SetAccelerometerRange(AccelerometerRange);
	NativeBarrier.SetAccelerometerCrossAxisSensitivity(AccelerometerCrossAxisSensitivity);
	NativeBarrier.SetAccelerometerZeroGBias(AccelerometerZeroGBias);
	NativeBarrier.SetAccelerometerNoiseRMS(AccelerometerNoiseRMS);
	NativeBarrier.SetAccelerometerSpectralNoiseDensity(AccelerometerSpectralNoiseDensity);

	NativeBarrier.SetGyroscopeRange(GyroscopeRange);
	NativeBarrier.SetGyroscopeCrossAxisSensitivity(GyroscopeCrossAxisSensitivity);
	NativeBarrier.SetGyroscopeZeroRateBias(GyroscopeZeroRateBias);
	NativeBarrier.SetGyroscopeNoiseRMS(GyroscopeNoiseRMS);
	NativeBarrier.SetGyroscopeSpectralNoiseDensity(GyroscopeSpectralNoiseDensity);

	NativeBarrier.SetMagnetometerRange(MagnetometerRange);
	NativeBarrier.SetMagnetometerCrossAxisSensitivity(MagnetometerCrossAxisSensitivity);
	NativeBarrier.SetMagnetometerZeroFluxBias(MagnetometerZeroFluxBias);
	NativeBarrier.SetMagnetometerNoiseRMS(MagnetometerNoiseRMS);
	NativeBarrier.SetMagnetometerSpectralNoiseDensity(MagnetometerSpectralNoiseDensity);
}

void UAGX_IMUSensorComponent::CreateNative()
{
	AGX_CHECK(!HasNative());
	if (HasNative())
		return;

	auto Body = RigidBody.GetRigidBody();
	if (Body == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_IMUSensorComponent::CreateNative called on IMU Sensor Component '%s' in "
				 "'%s' which does not have a valid Rigid Body selected. Native object will not be "
				 "created."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	auto BodyBarrier = Body->GetOrCreateNative();

	if (BodyBarrier == nullptr || !BodyBarrier->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The selected Rigid Body for IMU Sensor Component '%s' in "
				 "'%s' does not have a valid Native Object."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FTransform BodyTransform(Body->GetRotation(), Body->GetPosition());

	FIMUAllocationParameters Params;
	Params.bUseAccelerometer = bUseAccelerometer;
	Params.bUseGyroscope = bUseGyroscope;
	Params.bUseMagnetometer = bUseMagnetometer;
	Params.LocalTransform = GetComponentTransform().GetRelativeTransform(BodyTransform);

	NativeBarrier.AllocateNative(Params, *BodyBarrier);
	if (HasNative())
		UpdateNativeProperties();
}

void UAGX_IMUSensorComponent::SetAccelerometerRange(FAGX_RealInterval Range)
{
	AccelerometerRange = Range;

	if (HasNative())
		NativeBarrier.SetAccelerometerRange(Range);
}

FAGX_RealInterval UAGX_IMUSensorComponent::GetAccelerometerRange() const
{
	if (HasNative())
		return NativeBarrier.GetAccelerometerRange();

	return AccelerometerRange;
}

void UAGX_IMUSensorComponent::SetAccelerometerCrossAxisSensitivity(double Sensitivity)
{
	AccelerometerCrossAxisSensitivity = Sensitivity;

	if (HasNative())
		NativeBarrier.SetAccelerometerCrossAxisSensitivity(Sensitivity);
}

double UAGX_IMUSensorComponent::GetAccelerometerCrossAxisSensitivity() const
{
	// No clean AGX getter exists, but this value should always be in sync since we are the only one
	// that sets it.
	return AccelerometerCrossAxisSensitivity;
}

void UAGX_IMUSensorComponent::SetAccelerometerZeroGBias(FVector Bias)
{
	AccelerometerZeroGBias = Bias;

	if (HasNative())
		NativeBarrier.SetAccelerometerZeroGBias(Bias);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerZeroGBias() const
{
	if (HasNative())
		return NativeBarrier.GetAccelerometerZeroGBias();

	return AccelerometerZeroGBias;
}

void UAGX_IMUSensorComponent::SetAccelerometerNoiseRMS(FVector Noise)
{
	AccelerometerNoiseRMS = Noise;

	if (HasNative())
		NativeBarrier.SetAccelerometerNoiseRMS(Noise);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerNoiseRMS() const
{
	if (HasNative())
		return NativeBarrier.GetAccelerometerNoiseRMS();

	return AccelerometerNoiseRMS;
}

void UAGX_IMUSensorComponent::SetAccelerometerSpectralNoiseDensity(FVector Noise)
{
	AccelerometerSpectralNoiseDensity = Noise;

	if (HasNative())
		NativeBarrier.SetAccelerometerSpectralNoiseDensity(Noise);
}

FVector UAGX_IMUSensorComponent::GetAccelerometerSpectralNoiseDensity() const
{
	if (HasNative())
		return NativeBarrier.GetAccelerometerSpectralNoiseDensity();

	return AccelerometerSpectralNoiseDensity;
}

void UAGX_IMUSensorComponent::SetGyroscopeRange(FAGX_RealInterval Range)
{
	GyroscopeRange = Range;

	if (HasNative())
		NativeBarrier.SetGyroscopeRange(Range);
}

FAGX_RealInterval UAGX_IMUSensorComponent::GetGyroscopeRange() const
{
	if (HasNative())
		return NativeBarrier.GetGyroscopeRange();

	return GyroscopeRange;
}

void UAGX_IMUSensorComponent::SetGyroscopeCrossAxisSensitivity(double Sensitivity)
{
	GyroscopeCrossAxisSensitivity = Sensitivity;

	if (HasNative())
		NativeBarrier.SetGyroscopeCrossAxisSensitivity(Sensitivity);
}

double UAGX_IMUSensorComponent::GetGyroscopeCrossAxisSensitivity() const
{
	// No clean AGX getter exists, but this value should always be in sync since we are the only one
	// that sets it.
	return GyroscopeCrossAxisSensitivity;
}

void UAGX_IMUSensorComponent::SetGyroscopeZeroRateBias(FVector Bias)
{
	GyroscopeZeroRateBias = Bias;

	if (HasNative())
		NativeBarrier.SetGyroscopeZeroRateBias(Bias);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeZeroRateBias() const
{
	if (HasNative())
		return NativeBarrier.GetGyroscopeZeroRateBias();

	return GyroscopeZeroRateBias;
}

void UAGX_IMUSensorComponent::SetGyroscopeNoiseRMS(FVector Noise)
{
	GyroscopeNoiseRMS = Noise;

	if (HasNative())
		NativeBarrier.SetGyroscopeNoiseRMS(Noise);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeNoiseRMS() const
{
	if (HasNative())
		return NativeBarrier.GetGyroscopeNoiseRMS();

	return GyroscopeNoiseRMS;
}

void UAGX_IMUSensorComponent::SetGyroscopeSpectralNoiseDensity(FVector Noise)
{
	GyroscopeSpectralNoiseDensity = Noise;

	if (HasNative())
		NativeBarrier.SetGyroscopeSpectralNoiseDensity(Noise);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeSpectralNoiseDensity() const
{
	if (HasNative())
		return NativeBarrier.GetGyroscopeSpectralNoiseDensity();

	return GyroscopeSpectralNoiseDensity;
}

void UAGX_IMUSensorComponent::SetGyroscopeLinearAccelerationEffects(FVector Effects)
{
	GyroscopeLinearAccelerationEffects = Effects;

	if (HasNative())
		NativeBarrier.SetGyroscopeLinearAccelerationEffects(Effects);
}

FVector UAGX_IMUSensorComponent::GetGyroscopeLinearAccelerationEffects() const
{
	// No clean AGX getter exists, but this value should always be in sync since we are the only one
	// that sets it.
	return GyroscopeLinearAccelerationEffects;
}

void UAGX_IMUSensorComponent::SetMagnetometerRange(FAGX_RealInterval Range)
{
	MagnetometerRange = Range;

	if (HasNative())
		NativeBarrier.SetMagnetometerRange(Range);
}

FAGX_RealInterval UAGX_IMUSensorComponent::GetMagnetometerRange() const
{
	if (HasNative())
		return NativeBarrier.GetMagnetometerRange();

	return MagnetometerRange;
}

void UAGX_IMUSensorComponent::SetMagnetometerCrossAxisSensitivity(double Sensitivity)
{
	MagnetometerCrossAxisSensitivity = Sensitivity;

	if (HasNative())
		NativeBarrier.SetMagnetometerCrossAxisSensitivity(Sensitivity);
}

double UAGX_IMUSensorComponent::GetMagnetometerCrossAxisSensitivity() const
{
	// No clean AGX getter exists, but this value should always be in sync since we are the only one
	// that sets it.
	return MagnetometerCrossAxisSensitivity;
}

void UAGX_IMUSensorComponent::SetMagnetometerZeroFluxBias(FVector Bias)
{
	MagnetometerZeroFluxBias = Bias;

	if (HasNative())
		NativeBarrier.SetMagnetometerZeroFluxBias(Bias);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerZeroFluxBias() const
{
	if (HasNative())
		return NativeBarrier.GetMagnetometerZeroFluxBias();

	return MagnetometerZeroFluxBias;
}

void UAGX_IMUSensorComponent::SetMagnetometerNoiseRMS(FVector Noise)
{
	MagnetometerNoiseRMS = Noise;

	if (HasNative())
		NativeBarrier.SetMagnetometerNoiseRMS(Noise);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerNoiseRMS() const
{
	if (HasNative())
		return NativeBarrier.GetMagnetometerNoiseRMS();

	return MagnetometerNoiseRMS;
}

void UAGX_IMUSensorComponent::SetMagnetometerSpectralNoiseDensity(FVector Noise)
{
	MagnetometerSpectralNoiseDensity = Noise;

	if (HasNative())
		NativeBarrier.SetMagnetometerSpectralNoiseDensity(Noise);
}

FVector UAGX_IMUSensorComponent::GetMagnetometerSpectralNoiseDensity() const
{
	if (HasNative())
		return NativeBarrier.GetMagnetometerSpectralNoiseDensity();

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

	return NativeBarrier.GetAccelerometerData();
}

FVector UAGX_IMUSensorComponent::GetAccelerometerDataWorld() const
{
	const FVector LocalAccel = GetAccelerometerDataLocal();
	const FTransform IMUTransform = NativeBarrier.GetTransform();
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

	return NativeBarrier.GetGyroscopeData();
}

FVector UAGX_IMUSensorComponent::GetGyroscopeDataWorld() const
{
	const FVector LocalGyro = GetGyroscopeDataLocal();
	const FTransform IMUTransform = NativeBarrier.GetTransform();
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

	return NativeBarrier.GetMagnetometerData();
}

FVector UAGX_IMUSensorComponent::GetMagnetometerDataWorld() const
{
	const FVector LocalMag = GetMagnetometerDataLocal();
	const FTransform IMUTransform = NativeBarrier.GetTransform();
	const FVector WorldMag = IMUTransform.TransformVectorNoScale(LocalMag);
	return WorldMag;
}

void UAGX_IMUSensorComponent::SetPosition(FVector Position)
{
	SetWorldLocation(Position);

	if (HasNative())
		NativeBarrier.SetPosition(Position);
}

FVector UAGX_IMUSensorComponent::GetPosition() const
{
	if (HasNative())
		return NativeBarrier.GetPosition();

	return GetComponentLocation();
}

void UAGX_IMUSensorComponent::SetRotation(FQuat Rotation)
{
	SetWorldRotation(Rotation);

	if (HasNative())
		NativeBarrier.SetRotation(Rotation);
}

FQuat UAGX_IMUSensorComponent::GetRotation() const
{
	if (HasNative())
		return NativeBarrier.GetRotation();

	return GetComponentQuat();
}

void UAGX_IMUSensorComponent::UpdateTransformFromNative()
{
	if (!HasNative())
		return;

	SetWorldTransform(NativeBarrier.GetTransform());
}

FIMUBarrier* UAGX_IMUSensorComponent::GetOrCreateNative()
{
	if (HasNative())
		return GetNative();

	CreateNative();
	return GetNative();
}

FIMUBarrier* UAGX_IMUSensorComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FIMUBarrier* UAGX_IMUSensorComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

bool UAGX_IMUSensorComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_IMUSensorComponent::GetNativeAddress() const
{
	if (!HasNative())
		return 0;

	NativeBarrier.IncrementRefCount();
	return NativeBarrier.GetNativeAddress();
}

void UAGX_IMUSensorComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(NativeAddress);
	NativeBarrier.DecrementRefCount();
}

void UAGX_IMUSensorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAGX_IMUSensorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (HasNative())
		NativeBarrier.ReleaseNative();
}

TStructOnScope<FActorComponentInstanceData> UAGX_IMUSensorComponent::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

#undef LOCTEXT_NAMESPACE
