// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/IMUBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/AccelerometerModel.h>
#include <agxSensor/GyroscopeModel.h>
#include <agxSensor/IMUModelAccelerometerAttachment.h>
#include <agxSensor/IMUModelGyroscopeAttachment.h>
#include <agxSensor/IMUModelMagnetometerAttachment.h>
#include <agxSensor/IMUModelSensorAttachment.h>
#include <agxSensor/MagnetometerModel.h>
#include "EndAGXIncludes.h"

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

	struct IMUOut6Dof
	{
		agx::Real Data[6];
	};

	struct IMUOut9Dof
	{
		agx::Real Data[9];
	};

	size_t GetNumSensors(const agxSensor::IMU& IMU)
	{
		return IMU.getSensorAttachments().size();
	}

	template <typename T>
	IMUOut3Dof GetDataFrom(agxSensor::IMUOutputHandler* Handler, size_t Offset)
	{
		auto View = Handler->template view<T>(0);
		if (View.size() == 0)
		{
			return {}; // default-initialized IMUOutXYZ (all zeros)
		}

		const T& O = View[0];
		return {O.Data[Offset + 0], O.Data[Offset + 1], O.Data[Offset + 2]};
	}

	IMUOut3Dof GetData(agxSensor::IMUOutputHandler* Handler, size_t NumSensors, size_t Offset)
	{
		switch (NumSensors)
		{
			case 1:
				return GetDataFrom<IMUOut3Dof>(Handler, Offset);
			case 2:
				return GetDataFrom<IMUOut6Dof>(Handler, Offset);
			case 3:
				return GetDataFrom<IMUOut9Dof>(Handler, Offset);
		}

		UE_LOG(
			LogAGX, Warning, TEXT("GetIMUData called with unsupported sensor count: %d"),
			NumSensors);
		return {};
	}

	using FieldT = agxSensor::IMUOutput::Field;

	template <typename T, FieldT... Fs>
	static agxSensor::IMUOutput* AddOutput(agxSensor::IMUOutputHandler* H)
	{
		// If add() fails, return nullptr
		auto Opt = H->add<T, Fs...>();
		if (!Opt)
			return nullptr;
		return Opt->second; // IMUOutput*
	}

	agxSensor::IMUOutput* AddOutputs(agxSensor::IMUOutputHandler* H, size_t NumSensors)
	{
		switch (NumSensors)
		{
			case 1:
				return AddOutput<
					IMUOut3Dof, FieldT::SENSOR_0_X_F64, FieldT::SENSOR_0_Y_F64, FieldT::SENSOR_0_Z_F64>(
					H);

			case 2:
				return AddOutput<
					IMUOut6Dof, FieldT::SENSOR_0_X_F64, FieldT::SENSOR_0_Y_F64, FieldT::SENSOR_0_Z_F64,
					FieldT::SENSOR_1_X_F64, FieldT::SENSOR_1_Y_F64, FieldT::SENSOR_1_Z_F64>(H);

			case 3:
				return AddOutput<
					IMUOut9Dof, FieldT::SENSOR_0_X_F64, FieldT::SENSOR_0_Y_F64, FieldT::SENSOR_0_Z_F64,
					FieldT::SENSOR_1_X_F64, FieldT::SENSOR_1_Y_F64, FieldT::SENSOR_1_Z_F64, FieldT::SENSOR_2_X_F64,
					FieldT::SENSOR_2_Y_F64, FieldT::SENSOR_2_Z_F64>(H);
		}

		return nullptr;
	}
}

void FIMUBarrier::AllocateNative(const FIMUAllocationParameters& Params)
{
	check(!HasNative());

	using namespace agxSensor;
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

	NativeRef->Native =
		new agxSensor::IMU(new agx::Frame(), new agxSensor::IMUModel(SensorAttachments));

	const auto NumSensors = IMUBarrier_helpers::GetNumSensors(*NativeRef->Native);
	if (NumSensors > 0)
	{
		auto Outputs =
			IMUBarrier_helpers::AddOutputs(NativeRef->Native->getOutputHandler(), NumSensors);

		if (Outputs == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("FIMUBarrier::AllocateNative unable to add Outputs to IMU Sensor. The sensor "
					 "may not produce a valid output."));
		}
	}
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

FVector FIMUBarrier::GetAccelerometerData() const
{
	using namespace IMUBarrier_helpers;
	const size_t NumSensors = GetNumSensors(*NativeRef->Native);
	FVector Result = FVector::ZeroVector;
	if (NumSensors == 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FIMUBarrier::GetAccelerometerData called on IMU Sensor withot an Accelerometer. "
				 "Returning zero data."));
		return Result;
	}
	
	static constexpr size_t Offset = 0; // Accelerometer is always first.
	IMUOut3Dof OutputAGX =
		IMUBarrier_helpers::GetData(NativeRef->Native->getOutputHandler(), NumSensors, Offset);

	// [m/s^2] to [cm/s^2].
	Result = ConvertDisplacement(OutputAGX.Data[0], OutputAGX.Data[1], OutputAGX.Data[2]);
	return Result;
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

bool FIMUBarrier::HasAccelerometer() const
{
	check(HasNative());
	for (auto SensorAttachment : NativeRef->Native->getModel()->getSensorAttachments())
	{
		if (SensorAttachment->is<agxSensor::IMUModelAccelerometerAttachment>())
			return true;
	}

	return false;
}
