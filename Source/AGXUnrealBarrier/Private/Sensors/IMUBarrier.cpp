// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/IMUBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "RigidBodyBarrier.h"
#include "Sensors/SensorRef.h"

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

namespace IMUBarrier_helpers
{
	static constexpr size_t AccelerometerID = 1;
	static constexpr size_t GyroscopeID = 2;
	static constexpr size_t MagnetometerID = 3;

	struct IMUOut3Dof
	{
		agx::Real Data[3];
	};

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
			if (T* WantedModifier = Modifier->template asSafe<T>())
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

	void SetMatrixRow(agx::Matrix3x3& M, size_t RowIndex, const FVector V)
	{
		M(RowIndex, 0) = V.X;
		M(RowIndex, 1) = V.Y;
		M(RowIndex, 2) = V.Z;
	}

	FVector GetMatrixRow(const agx::Matrix3x3& M, size_t RowIndex)
	{
		return FVector {M(RowIndex, 0), M(RowIndex, 1), M(RowIndex, 2)};
	}

	agxSensor::IMU* GetIMUNative(FIMUBarrier& IMU)
	{
		AGX_CHECK(IMU.HasNative());
		return IMU.GetNative()->Native->as<agxSensor::IMU>();
	}

	agxSensor::IMU* GetIMUNative(const FIMUBarrier& IMU)
	{
		AGX_CHECK(IMU.HasNative());
		return IMU.GetNative()->Native->as<agxSensor::IMU>();
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

	agx::FrameRef IMUFrame = new agx::Frame(Convert(Params.LocalTransform));
	IMUFrame->setParent(Body.GetNative()->Native->getFrame());

	NativeRef->Native = new agxSensor::IMU(IMUFrame, new agxSensor::IMUModel(SensorAttachments));

	// clang-format off

	// Add Outputs.
	if (Params.bUseAccelerometer)
	{
		GetIMUNative(*this)->getOutputHandler()->add<
			IMUOut3Dof,
			agxSensor::IMUOutput::ACCELEROMETER_X_F64,
			agxSensor::IMUOutput::ACCELEROMETER_Y_F64,
			agxSensor::IMUOutput::ACCELEROMETER_Z_F64>(IMUBarrier_helpers::AccelerometerID);
	}

	if (Params.bUseGyroscope)
	{
		GetIMUNative(*this)->getOutputHandler()->add<
			IMUOut3Dof,
			agxSensor::IMUOutput::GYROSCOPE_X_F64,
			agxSensor::IMUOutput::GYROSCOPE_Y_F64,
			agxSensor::IMUOutput::GYROSCOPE_Z_F64>(IMUBarrier_helpers::GyroscopeID);
	}

	if (Params.bUseMagnetometer)
	{
		GetIMUNative(*this)->getOutputHandler()->add<
			IMUOut3Dof,
			agxSensor::IMUOutput::MAGNETOMETER_X_F64,
			agxSensor::IMUOutput::MAGNETOMETER_Y_F64,
			agxSensor::IMUOutput::MAGNETOMETER_Z_F64>(IMUBarrier_helpers::MagnetometerID);
	}
	// clang-format on
}

void FIMUBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	IMUBarrier_helpers::GetIMUNative(*this)->getFrame()->setMatrix(Convert(Transform));
}

FTransform FIMUBarrier::GetTransform() const
{
	check(HasNative());
	return Convert(IMUBarrier_helpers::GetIMUNative(*this)->getFrame()->getMatrix());
}

void FIMUBarrier::SetPosition(FVector Position)
{
	check(HasNative());
	IMUBarrier_helpers::GetIMUNative(*this)->getFrame()->setTranslate(
		ConvertDisplacement(Position));
}

FVector FIMUBarrier::GetPosition() const
{
	check(HasNative());
	return ConvertDisplacement(IMUBarrier_helpers::GetIMUNative(*this)->getFrame()->getTranslate());
}

void FIMUBarrier::SetRotation(FQuat Rotation)
{
	check(HasNative());
	IMUBarrier_helpers::GetIMUNative(*this)->getFrame()->setRotate(Convert(Rotation));
}

FQuat FIMUBarrier::GetRotation() const
{
	check(HasNative());
	return Convert(IMUBarrier_helpers::GetIMUNative(*this)->getFrame()->getRotate());
}

void FIMUBarrier::SetAccelerometerRange(FAGX_RealInterval Range)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerRange");
		return;
	}

	// [cm/s^2] to [m/s^2].
	Accel->getModel()->setRange(agxSensor::TriaxialRange(ConvertDistance(Range)));
}

FAGX_RealInterval FIMUBarrier::GetAccelerometerRange() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerRange");
		return {0.0, 0.0};
	}

	// We assume same range for all axes. [m/s^2] to [cm/s^2].
	return ConvertDistance(Accel->getModel()->getRange().getRangeX());
}

void FIMUBarrier::SetAccelerometerCrossAxisSensitivityX(FVector X)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerCrossAxisSensitivityX");
		return;
	}

	agx::Matrix3x3 M = Accel->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 0, X);
	Accel->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetAccelerometerCrossAxisSensitivityX() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerCrossAxisSensitivityX");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Accel->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 0);
}

void FIMUBarrier::SetAccelerometerCrossAxisSensitivityY(FVector Y)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerCrossAxisSensitivityY");
		return;
	}

	agx::Matrix3x3 M = Accel->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 1, Y);
	Accel->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetAccelerometerCrossAxisSensitivityY() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerCrossAxisSensitivityY");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Accel->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 1);
}

void FIMUBarrier::SetAccelerometerCrossAxisSensitivityZ(FVector Z)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerCrossAxisSensitivityZ");
		return;
	}

	agx::Matrix3x3 M = Accel->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 2, Z);
	Accel->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetAccelerometerCrossAxisSensitivityZ() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerCrossAxisSensitivityZ");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Accel->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 2);
}

void FIMUBarrier::SetAccelerometerZeroGBias(FVector Bias)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerZeroGBias");
		return;
	}

	Accel->getModel()->setZeroGBias(ConvertDisplacement(Bias));
}

FVector FIMUBarrier::GetAccelerometerZeroGBias() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerZeroGBias");
		return FVector::ZeroVector;
	}

	return ConvertDisplacement(Accel->getModel()->getZeroGBias());
}

void FIMUBarrier::SetAccelerometerNoiseRMS(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerNoiseRMS");
		return;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialGaussianNoise>(*Accel);

	// [cm/s^2] to [m/s^2], no sign flip.
	Modifier->setNoiseRms(ConvertDistance(Noise));
}

FVector FIMUBarrier::GetAccelerometerNoiseRMS() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerNoiseRMS");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialGaussianNoise>(*Accel);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	// [m/s^2] to [cm/s^2], no sign flip.
	return ConvertDistance(Modifier->getNoiseRms());
}

void FIMUBarrier::SetAccelerometerSpectralNoiseDensity(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::SetAccelerometerSpectralNoiseDensity");
		return;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Accel);

	// [cm/s^2/hz] to [m/s^2/hz], no sign flip.
	Modifier->setNoiseDensity(ConvertDistance(Noise));
}

FVector FIMUBarrier::GetAccelerometerSpectralNoiseDensity() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Accelerometer* Accel = GetAccelerometer(*GetIMUNative(*this));
	if (Accel == nullptr)
	{
		LogMissingObject("Accelerometer", "FIMUBarrier::GetAccelerometerSpectralNoiseDensity");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Accel);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	// [m/s^2/hz] to [cm/s^2/hz], no sign flip.
	return ConvertDistance(Modifier->getNoiseDensity());
}

FVector FIMUBarrier::GetAccelerometerData() const
{
	using namespace IMUBarrier_helpers;
	FVector Result = FVector::ZeroVector;

	if (!GetIMUNative(*this)
			 ->getOutputHandler()
			 ->get(IMUBarrier_helpers::AccelerometerID)
			 ->hasUnreadData(/*markAsRead*/ false))
	{
		return Result; // No unread data available.
	}

	auto OutputAGX = GetIMUNative(*this)->getOutputHandler()->view<IMUOut3Dof>(
		IMUBarrier_helpers::AccelerometerID);
	if (OutputAGX.empty())
		return Result;

	// [m/s^2] to [cm/s^2].
	Result = ConvertDisplacement(OutputAGX[0].Data[0], OutputAGX[0].Data[1], OutputAGX[0].Data[2]);
	return Result;
}

void FIMUBarrier::SetGyroscopeRange(FAGX_RealInterval Range)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope ", "FIMUBarrier::SetGyroscopeRange");
		return;
	}

	// [deg/s] to [rad/s].
	Gyro->getModel()->setRange(agxSensor::TriaxialRange(ConvertAngle(Range)));
}

FAGX_RealInterval FIMUBarrier::GetGyroscopeRange() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::GetGyroscopeRange");
		return {0.0, 0.0};
	}

	// We assume same range for all axes. [rad/s] to [deg/s].
	return ConvertAngle(Gyro->getModel()->getRange().getRangeX());
}

void FIMUBarrier::SetGyroscopeCrossAxisSensitivityX(FVector X)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::SetGyroscopeCrossAxisSensitivityX");
		return;
	}

	agx::Matrix3x3 M = Gyro->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 0, X);
	Gyro->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetGyroscopeCrossAxisSensitivityX() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::GetGyroscopeCrossAxisSensitivityX");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Gyro->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 0);
}

void FIMUBarrier::SetGyroscopeCrossAxisSensitivityY(FVector Y)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::SetGyroscopeCrossAxisSensitivityY");
		return;
	}

	agx::Matrix3x3 M = Gyro->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 1, Y);
	Gyro->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetGyroscopeCrossAxisSensitivityY() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::GetGyroscopeCrossAxisSensitivityY");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Gyro->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 1);
}

void FIMUBarrier::SetGyroscopeCrossAxisSensitivityZ(FVector Z)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::SetGyroscopeCrossAxisSensitivityZ");
		return;
	}

	agx::Matrix3x3 M = Gyro->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 2, Z);
	Gyro->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetGyroscopeCrossAxisSensitivityZ() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::GetGyroscopeCrossAxisSensitivityZ");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Gyro->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 2);
}

void FIMUBarrier::SetGyroscopeZeroRateBias(FVector Bias)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::SetGyroscopeZeroRateBias");
		return;
	}

	Gyro->getModel()->setZeroRateBias(ConvertAngularVelocity(Bias));
}

FVector FIMUBarrier::GetGyroscopeZeroRateBias() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::GetGyroscopeZeroRateBias");
		return FVector::ZeroVector;
	}

	return ConvertAngularVelocity(Gyro->getModel()->getZeroRateBias());
}

void FIMUBarrier::SetGyroscopeNoiseRMS(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Gyroscope* Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::SetGyroscopeNoiseRMS");
		return;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialGaussianNoise>(*Gyro);

	// [deg/s] to [rad/s], no sign flip.
	Modifier->setNoiseRms(agx::Vec3(
		FMath::DegreesToRadians(Noise.X), FMath::DegreesToRadians(Noise.Y),
		FMath::DegreesToRadians(Noise.Z)));
}

FVector FIMUBarrier::GetGyroscopeNoiseRMS() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Gyroscope* Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::GetGyroscopeNoiseRMS");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialGaussianNoise>(*Gyro);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	// [rad/s] to [deg/s], no sign flip.
	agx::Vec3 NoiseAGX = Modifier->getNoiseRms();
	return FVector(
		FMath::RadiansToDegrees(NoiseAGX.x()), FMath::RadiansToDegrees(NoiseAGX.y()),
		FMath::RadiansToDegrees(NoiseAGX.z()));
}

void FIMUBarrier::SetGyroscopeSpectralNoiseDensity(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Gyroscope* Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::SetGyroscopeSpectralNoiseDensity");
		return;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Gyro);

	// [deg/s/hz] to [rad/s/hz], no sign flip.
	Modifier->setNoiseDensity(agx::Vec3(
		FMath::DegreesToRadians(Noise.X), FMath::DegreesToRadians(Noise.Y),
		FMath::DegreesToRadians(Noise.Z)));
}

FVector FIMUBarrier::GetGyroscopeSpectralNoiseDensity() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Gyroscope* Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::GetGyroscopeSpectralNoiseDensity");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Gyro);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	// [rad/s/hz] to [deg/s/hz], no sign flip.
	agx::Vec3 NoiseAGX = Modifier->getNoiseDensity();
	return FVector(
		FMath::RadiansToDegrees(NoiseAGX.x()), FMath::RadiansToDegrees(NoiseAGX.y()),
		FMath::RadiansToDegrees(NoiseAGX.z()));
}

void FIMUBarrier::SetGyroscopeLinearAccelerationEffects(const FVector& Effects)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Gyroscope* Gyro = GetGyroscope(*GetIMUNative(*this));
	if (Gyro == nullptr)
	{
		LogMissingObject("Gyroscope", "FIMUBarrier::SetGyroscopeLinearAccelerationEffects");
		return;
	}

	agxSensor::GyroscopeLinearAccelerationEffectsRef Modifier =
		GetOrCreateOutputModifier<agxSensor::GyroscopeLinearAccelerationEffects>(*Gyro);

	Modifier->setAccelerationEffects(ConvertVector(Effects));
}

FVector FIMUBarrier::GetGyroscopeData() const
{
	using namespace IMUBarrier_helpers;
	FVector Result = FVector::ZeroVector;

	if (!GetIMUNative(*this)
			 ->getOutputHandler()
			 ->get(IMUBarrier_helpers::GyroscopeID)
			 ->hasUnreadData(/*markAsRead*/ false))
	{
		return Result; // No unread data available.
	}

	auto OutputAGX =
		GetIMUNative(*this)->getOutputHandler()->view<IMUOut3Dof>(IMUBarrier_helpers::GyroscopeID);
	if (OutputAGX.empty())
		return Result;

	return ConvertAngularVelocity(
		agx::Vec3(OutputAGX[0].Data[0], OutputAGX[0].Data[1], OutputAGX[0].Data[2]));
}

void FIMUBarrier::SetMagnetometerRange(FAGX_RealInterval Range)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::SetMagnetometerRange");
		return;
	}

	Magn->getModel()->setRange(agxSensor::TriaxialRange(Convert(Range)));
}

FAGX_RealInterval FIMUBarrier::GetMagnetometerRange() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::GetMagnetometerRange");
		return {0.0, 0.0};
	}

	// We assume same range for all axes.
	return Convert(Magn->getModel()->getRange().getRangeX());
}

void FIMUBarrier::SetMagnetometerCrossAxisSensitivityX(FVector X)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::SetMagnetometerCrossAxisSensitivityX");
		return;
	}

	agx::Matrix3x3 M = Magn->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 0, X);
	Magn->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetMagnetometerCrossAxisSensitivityX() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::GetMagnetometerCrossAxisSensitivityX");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Magn->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 0);
}

void FIMUBarrier::SetMagnetometerCrossAxisSensitivityY(FVector Y)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::SetMagnetometerCrossAxisSensitivityY");
		return;
	}

	agx::Matrix3x3 M = Magn->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 1, Y);
	Magn->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetMagnetometerCrossAxisSensitivityY() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::GetMagnetometerCrossAxisSensitivityY");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Magn->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 1);
}

void FIMUBarrier::SetMagnetometerCrossAxisSensitivityZ(FVector Z)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::SetMagnetometerCrossAxisSensitivityZ");
		return;
	}

	agx::Matrix3x3 M = Magn->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	SetMatrixRow(M, /*RowIndex*/ 2, Z);
	Magn->getModel()->setCrossAxisSensitivity(agxSensor::TriaxialCrossSensitivity(M));
}

FVector FIMUBarrier::GetMagnetometerCrossAxisSensitivityZ() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::GetMagnetometerCrossAxisSensitivityZ");
		return FVector::ZeroVector;
	}

	agx::Matrix3x3 M = Magn->getModel()->getCrossAxisSensitivity().getCrossAxisMatrix();
	return GetMatrixRow(M, /*RowIndex*/ 2);
}

void FIMUBarrier::SetMagnetometerZeroFluxBias(FVector Bias)
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::SetMagnetometerZeroFluxBias");
		return;
	}

	Magn->getModel()->setZeroFluxBias(ConvertVector(Bias));
}

FVector FIMUBarrier::GetMagnetometerZeroFluxBias() const
{
	using namespace IMUBarrier_helpers;

	check(HasNative());
	auto Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::GetMagnetometerZeroFluxBias");
		return FVector::ZeroVector;
	}

	return ConvertVector(Magn->getModel()->getZeroFluxBias());
}

void FIMUBarrier::SetMagnetometerNoiseRMS(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Magnetometer* Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::SetMagnetometerNoiseRMS");
		return;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialGaussianNoise>(*Magn);

	Modifier->setNoiseRms(Convert(Noise));
}

FVector FIMUBarrier::GetMagnetometerNoiseRMS() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Magnetometer* Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::GetMagnetometerNoiseRMS");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialGaussianNoise>(*Magn);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	return Convert(Modifier->getNoiseRms());
}

void FIMUBarrier::SetMagnetometerSpectralNoiseDensity(const FVector& Noise)
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Magnetometer* Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::SetMagnetometerSpectralNoiseDensity");
		return;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOrCreateOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Magn);

	Modifier->setNoiseDensity(Convert(Noise));
}

FVector FIMUBarrier::GetMagnetometerSpectralNoiseDensity() const
{
	using namespace IMUBarrier_helpers;
	check(HasNative());
	agxSensor::Magnetometer* Magn = GetMagnetometer(*GetIMUNative(*this));
	if (Magn == nullptr)
	{
		LogMissingObject("Magnetometer", "FIMUBarrier::GetMagnetometerSpectralNoiseDensity");
		return FVector::ZeroVector;
	}

	agxSensor::TriaxialSpectralGaussianNoiseRef Modifier =
		GetOutputModifier<agxSensor::TriaxialSpectralGaussianNoise>(*Magn);
	if (Modifier == nullptr)
		return FVector::ZeroVector;

	return Convert(Modifier->getNoiseDensity());
}

FVector FIMUBarrier::GetMagnetometerData() const
{
	using namespace IMUBarrier_helpers;
	FVector Result = FVector::ZeroVector;

	if (!GetIMUNative(*this)
			 ->getOutputHandler()
			 ->get(IMUBarrier_helpers::MagnetometerID)
			 ->hasUnreadData(/*markAsRead*/ false))
	{
		return Result; // No unread data available.
	}

	auto OutputAGX = GetIMUNative(*this)->getOutputHandler()->view<IMUOut3Dof>(
		IMUBarrier_helpers::MagnetometerID);
	if (OutputAGX.empty())
		return Result;

	// In Tesla [T], no unit conversion needed, just axis flip.
	return ConvertVector(
		agx::Vec3(OutputAGX[0].Data[0], OutputAGX[0].Data[1], OutputAGX[0].Data[2]));
}

void FIMUBarrier::MarkOutputAsRead()
{
	check(HasNative());
	using namespace IMUBarrier_helpers;

	const auto IDs = {
		IMUBarrier_helpers::AccelerometerID, IMUBarrier_helpers::GyroscopeID,
		IMUBarrier_helpers::MagnetometerID};

	for (auto ID : IDs)
	{
		if (auto Output = GetIMUNative(*this)->getOutputHandler()->get(ID))
			Output->hasUnreadData(/*markAsRead*/ true);
	}
}
