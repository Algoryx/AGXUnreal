// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/IMUBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/Accelerometer.h>
#include <agxSensor/Gyroscope.h>
#include <agxSensor/Magnetometer.h>
#include <agxSensor/IMUModelAccelerometerAttachment.h>
#include <agxSensor/IMUModelGyroscopeAttachment.h>
#include <agxSensor/IMUModelMagnetometerAttachment.h>
#include <agxSensor/IMUModelSensorAttachment.h>
#include <agxSensor/MagnetometerModel.h>
#include <agxSensor/TriaxialOutputHandler.h>
#include "EndAGXIncludes.h"

#define AccelOutputID 1
#define GyroOutputID 2
#define MagnOutputID 3

FIMUBarrier::FIMUBarrier()
	: NativeRef {new FIMURef}
{
}

FIMUBarrier::FIMUBarrier(std::unique_ptr<FIMURef> Native)
	: NativeRef(std::move(Native))
{
}

FIMUBarrier::FIMUBarrier(FIMUBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FIMURef);
}

FIMUBarrier::~FIMUBarrier()
{
}

bool FIMUBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

namespace IMUBarrier_helpers
{
	struct IMUOut3Dof
	{
		agx::Real Data[3];
	};

	agxSensor::AccelerometerModel* GetAccelerometerModel(const agxSensor::IMU& IMU)
	{
		for (auto SensorAttachment : IMU.getModel()->getSensorAttachments())
		{
			if (auto AccelerometerAttachment =
					SensorAttachment->as<agxSensor::IMUModelAccelerometerAttachment>())
				return AccelerometerAttachment->getModel();
		}

		return nullptr;
	}

	agxSensor::GyroscopeModel* GetGyroscopeModel(const agxSensor::IMU& IMU)
	{
		for (auto SensorAttachment : IMU.getModel()->getSensorAttachments())
		{
			if (auto GyroscopeAttachment =
					SensorAttachment->as<agxSensor::IMUModelGyroscopeAttachment>())
				return GyroscopeAttachment->getModel();
		}

		return nullptr;
	}

	agxSensor::MagnetometerModel* GetMagnetometerModel(const agxSensor::IMU& IMU)
	{
		for (auto SensorAttachment : IMU.getModel()->getSensorAttachments())
		{
			if (auto MagnetometerAttachment =
					SensorAttachment->as<agxSensor::IMUModelMagnetometerAttachment>())
				return MagnetometerAttachment->getModel();
		}

		return nullptr;
	}

	agxSensor::Accelerometer* GetAccelerometer(const agxSensor::IMU& IMU)
	{
		return IMU.findChild<agxSensor::Accelerometer>(/*recursive*/ true);
	}

	agxSensor::Gyroscope* GetGyroscope(const agxSensor::IMU& IMU)
	{
		return IMU.findChild<agxSensor::Gyroscope>(/*recursive*/ true);
	}

	agxSensor::Magnetometer* GetMagnetometer(const agxSensor::IMU& IMU)
	{
		return IMU.findChild<agxSensor::Magnetometer>(/*recursive*/ true);
	}

	template <typename T>
	T* GetOutputModifier(const auto& Sensor)
	{
		for (auto Modifier : Sensor.getOutputHandler()->getOutputModifiers())
		{
			if (T* WantedModifier = Modifier->as<T>())
				return WantedModifier;
		}

		return nullptr;
	}

	template <typename T>
	T* GetOrCreateOutputModifier(const auto& Sensor)
	{
		auto Modifier = GetOutputModifier<T>(Sensor);

		if (Modifier == nullptr)
		{
			Modifier = new T();
			Sensor.getOutputHandler()->add(Modifier);
		}

		return Modifier;
	}

	void LogMissingObject(const FString& ObjectName, const FString& FunctionName)
	{
		UE_LOG(LogAGX, Warning, TEXT("Unable to get %s in %s."), *ObjectName, *FunctionName);
	}
}

void FIMUBarrier::AllocateNative(const FIMUAllocationParameters& Params, FRigidBodyBarrier& Body)
{
	check(!HasNative());
	check(Body.HasNative());

	using namespace agxSensor;
	using namespace IMUBarrier_helpers;
	IMUModelSensorAttachmentRefVector SensorAttachments;

	if (Params.bUseAccelerometer)
	{
		SensorAttachments.push_back(new IMUModelAccelerometerAttachment(
			agx::AffineMatrix4x4(), AccelerometerModel::makeIdealModel()));
	}

	if (Params.bUseGyroscope)
	{
		SensorAttachments.push_back(new IMUModelGyroscopeAttachment(
			agx::AffineMatrix4x4(), GyroscopeModel::makeIdealModel()));
	}

	if (Params.bUseMagnetometer)
	{
		SensorAttachments.push_back(new IMUModelMagnetometerAttachment(
			agx::AffineMatrix4x4(), MagnetometerModel::makeIdealModel()));
	}

	agx::FrameRef IMUFrame =
		ConvertFrame(Params.LocalTransform.GetLocation(), Params.LocalTransform.GetRotation());
	IMUFrame->setParent(Body.GetNative()->Native->getFrame());

	NativeRef->Native = new agxSensor::IMU(IMUFrame, new agxSensor::IMUModel(SensorAttachments));

	// clang-format off

	// Add Outputs.
	if (Params.bUseAccelerometer)
	{
		NativeRef->Native->getOutputHandler()->add<
			IMUOut3Dof,
			agxSensor::IMUOutput::ACCELEROMETER_X_F64,
      agxSensor::IMUOutput::ACCELEROMETER_Y_F64,
      agxSensor::IMUOutput::ACCELEROMETER_Z_F64>(AccelOutputID);
	}

	if (Params.bUseGyroscope)
	{
		NativeRef->Native->getOutputHandler()->add<
			IMUOut3Dof,
			agxSensor::IMUOutput::GYROSCOPE_X_F64,
      agxSensor::IMUOutput::GYROSCOPE_Y_F64,
      agxSensor::IMUOutput::GYROSCOPE_Z_F64>(GyroOutputID);
	}

	if (Params.bUseMagnetometer)
	{
		NativeRef->Native->getOutputHandler()->add<
			IMUOut3Dof,
			agxSensor::IMUOutput::MAGNETOMETER_X_F64,
      agxSensor::IMUOutput::MAGNETOMETER_Y_F64,
      agxSensor::IMUOutput::MAGNETOMETER_Z_F64>(MagnOutputID);
	}
	// clang-format on
}

FIMURef* FIMUBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FIMURef* FIMUBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uint64 FIMUBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FIMUBarrier::SetNativeAddress(uint64 Address)
{
	NativeRef->Native = reinterpret_cast<agxSensor::IMU*>(Address);
}

void FIMUBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;
}

void FIMUBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FIMUBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FIMUBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	*NativeRef->Native->getFrame() =
		*ConvertFrame(Transform.GetLocation(), Transform.GetRotation());
}

FTransform FIMUBarrier::GetTransform() const
{
	check(HasNative());
	return Convert(*NativeRef->Native->getFrame());
}

void FIMUBarrier::SetAccelerometerRange(FAGX_RealInterval Range)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Model = GetAccelerometerModel(*NativeRef->Native);
	if (Model == nullptr)
	{
		LogMissingObject("Accelerometer Model", "FIMUBarrier::SetAccelerometerRange");
		return;
	}

	// [cm/s^2] to [m/s^2].
	Model->setRange(agxSensor::TriaxialRange(ConvertDistance(Range)));
}

FAGX_RealInterval FIMUBarrier::GetAccelerometerRange() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Model = GetAccelerometerModel(*NativeRef->Native);
	if (Model == nullptr)
	{
		LogMissingObject("Accelerometer Model", "FIMUBarrier::GetAccelerometerRange");
		return {0.0, 0.0};
	}

	// We assume same range for all axes. [m/s^2] to [cm/s^2].
	return ConvertDistance(Model->getRange().getRangeX());
}

void FIMUBarrier::SetAccelerometerAxisCrossSensitivity(double Sensitivity)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Model = GetAccelerometerModel(*NativeRef->Native);
	if (Model == nullptr)
	{
		LogMissingObject("Accelerometer Model", "FIMUBarrier::SetAccelerometerAxisCrossSensitivity");
		return;
	}

	Model->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(Sensitivity));
}

void FIMUBarrier::SetAccelerometerZeroGBias(FVector Bias)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Model = GetAccelerometerModel(*NativeRef->Native);
	if (Model == nullptr)
	{
		LogMissingObject("Accelerometer Model", "FIMUBarrier::SetAccelerometerZeroGBias");
		return;
	}

	Model->setZeroGBias(ConvertDisplacement(Bias));
}

FVector FIMUBarrier::GetAccelerometerZeroGBias() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Model = GetAccelerometerModel(*NativeRef->Native);
	if (Model == nullptr)
	{
		LogMissingObject("Accelerometer Model", "FIMUBarrier::GetAccelerometerZeroGBias");
		return FVector::ZeroVector;
	}

	return ConvertDisplacement(Model->getZeroGBias());
}

void FIMUBarrier::SetAccelerometerNoiseRMS(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*NativeRef->Native);
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerNoiseRMS");
		return;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialGaussianNoise>(*Accel);

	// [cm/s^2] to [m/s^2].
	Modifier->setNoiseRms(ConvertDisplacement(Noise));
}

FVector FIMUBarrier::GetAccelerometerNoiseRMS() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*NativeRef->Native);
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerNoiseRMS");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialGaussianNoise>(*Accel);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	// [cm/s^2] to [m/s^2].
	return ConvertDisplacement(Modifier->getNoiseRms());
}

void FIMUBarrier::SetAccelerometerSpectralNoiseDensity(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*NativeRef->Native);
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerSpectralNoiseDensity");
		return;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Accel);

	// [cm/s^2/hz] to [m/s^2/hz].
	Modifier->setNoiseDensity(ConvertVector(Noise));
}

FVector FIMUBarrier::GetAccelerometerSpectralNoiseDensity() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*NativeRef->Native);
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerSpectralNoiseDensity");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Accel);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	// [cm/s^2/hz] to [m/s^2/hz].
	return ConvertDisplacement(Modifier->getNoiseDensity());
}

FVector FIMUBarrier::GetAccelerometerData() const
{
	using namespace IMUBarrier_helpers;
	FVector Result = FVector::ZeroVector;

	auto OutputAGX = NativeRef->Native->getOutputHandler()->view<IMUOut3Dof>(AccelOutputID);
	if (OutputAGX.empty())
		return Result;

	// [m/s^2] to [cm/s^2].
	Result = ConvertDisplacement(OutputAGX[0].Data[0], OutputAGX[0].Data[1], OutputAGX[0].Data[2]);
	return Result;
}

FVector FIMUBarrier::GetGyroscopeData() const
{
	using namespace IMUBarrier_helpers;
	FVector Result = FVector::ZeroVector;

	auto OutputAGX = NativeRef->Native->getOutputHandler()->view<IMUOut3Dof>(GyroOutputID);
	if (OutputAGX.empty())
		return Result;

	return ConvertAngularVelocity(
		agx::Vec3(OutputAGX[0].Data[0], OutputAGX[0].Data[1], OutputAGX[0].Data[2]));
}

FVector FIMUBarrier::GetMagnetometerData() const
{
	using namespace IMUBarrier_helpers;

	using namespace IMUBarrier_helpers;
	FVector Result = FVector::ZeroVector;

	auto OutputAGX = NativeRef->Native->getOutputHandler()->view<IMUOut3Dof>(MagnOutputID);
	if (OutputAGX.empty())
		return Result;

	// In Tesla [T], no unit conversion needed, just axis flip.
	return ConvertVector(
		agx::Vec3(OutputAGX[0].Data[0], OutputAGX[0].Data[1], OutputAGX[0].Data[2]));
}

void FIMUBarrier::IncrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->reference();
}

void FIMUBarrier::DecrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->unreference();
}
