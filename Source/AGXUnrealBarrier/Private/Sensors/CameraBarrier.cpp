// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "RigidBodyBarrier.h"
#include "Sensors/CameraBackendBarrier.h"
#include "Sensors/CameraBackendParameters.h"
#include "Sensors/CameraBackendPropagatorBase.h"
#include "Sensors/CameraLensBarrier.h"
#include "Sensors/CameraOutputBarrier.h"
#include "Sensors/CameraPhotodetectorBarrier.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/Camera.h>
#include <agxSensor/CameraCMOSSensor.h>
#include <agxSensor/CameraColorOutput.h>
#include <agxSensor/CameraLensSingleElement.h>
#include <agxSensor/CameraModel.h>
#include <agxSensor/CameraOutput.h>
#include <agxSensor/CameraOutputHandler.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>

namespace CameraBarrier_helpers
{
	agxSensor::Camera* GetCameraNative(FCameraBarrier& Camera)
	{
		AGX_CHECK(Camera.HasNative());
		return Camera.GetNative()->Native->asSafe<agxSensor::Camera>();
	}

	agxSensor::Camera* GetCameraNative(const FCameraBarrier& Camera)
	{
		AGX_CHECK(Camera.HasNative());
		return Camera.GetNative()->Native->asSafe<agxSensor::Camera>();
	}

	size_t GenerateUniqueOutputId()
	{
		static size_t Id = 1;
		return Id++;
	}
}

FCameraBarrier::FCameraBarrier(
	std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride)
	: FSensorBarrier(std::move(Native), std::move(StepStride))
{
}

void FCameraBarrier::AllocateNative(
	const FTransform& Transform, FCameraLensBarrier* Lens,
	FCameraPhotodetectorBarrier* Photodetector)
{
	check(!HasNative());
	check(Lens == nullptr || Lens->HasNative());
	check(Photodetector == nullptr || Photodetector->HasNative());

	auto Frame = new agx::Frame(Convert(Transform));
	agxSensor::CameraLens* NativeLens = Lens != nullptr ? Lens->GetNative()->Native.get()
														: new agxSensor::CameraLensSingleElement();
	agxSensor::CameraPhotodetector* NativePhotodetector =
		Photodetector != nullptr ? Photodetector->GetNative()->Native.get()
								 : new agxSensor::CameraCMOSSensor();
	auto Model = new agxSensor::CameraModel(NativeLens, NativePhotodetector);

	NativeRef->Native = new agxSensor::Camera(
		Frame, Model, FCameraBackendBarrier::GetInstance().GetNative()->Native);
	RegisterWithBackend();
}

void FCameraBarrier::ReleaseNative()
{
	if (HasNative())
		UnregisterFromBackend();

	FSensorBarrier::ReleaseNative();
	BackendPropagator = nullptr;
}

void FCameraBarrier::RegisterWithBackend()
{
	if (FCameraBackendBarrier::GetInstance().HasNative())
		FCameraBackendBarrier::GetInstance().RegisterCamera(*this);
}

void FCameraBarrier::UnregisterFromBackend()
{
	if (FCameraBackendBarrier::GetInstance().HasNative())
		FCameraBackendBarrier::GetInstance().UnregisterCamera(*this);
}

void FCameraBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	CameraBarrier_helpers::GetCameraNative(*this)->getFrame()->setMatrix(Convert(Transform));
}

FTransform FCameraBarrier::GetTransform() const
{
	check(HasNative());
	return Convert(CameraBarrier_helpers::GetCameraNative(*this)->getFrame()->getMatrix());
}

FRigidBodyBarrier FCameraBarrier::GetRigidBody() const
{
	using namespace CameraBarrier_helpers;
	check(HasNative());

	agx::RigidBody* Body = nullptr;
	for (agx::Frame* Frame = GetCameraNative(*this)->getFrame(); Frame != nullptr;
		 Frame = Frame->getParent())
	{
		Body = Frame->getRigidBody();
		if (Body != nullptr)
			break;
	}

	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}

void FCameraBarrier::AddOutput(FCameraOutputBarrier& Output)
{
	check(HasNative());
	check(Output.HasNative());
	using namespace CameraBarrier_helpers;

	const size_t Id = GenerateUniqueOutputId();
	GetCameraNative(*this)->getOutputHandler()->add(Id, Output.GetNative()->Native);
	Output.RegisterWithBackend(*this);
}

TArray<FCameraOutputBarrier> FCameraBarrier::GetOutputs() const
{
	check(HasNative());

	const TArray<FAGX_CameraCaptureState>* CaptureStates =
		FCameraBackendBarrier::GetInstance().FindCaptureStates(const_cast<FCameraBarrier*>(this));
	if (CaptureStates == nullptr)
		return {};

	TArray<FCameraOutputBarrier> Outputs;
	Outputs.Reserve(CaptureStates->Num());
	for (const FAGX_CameraCaptureState& CaptureState : *CaptureStates)
	{
		if (CaptureState.OutputAddr == 0)
			continue;

		agxSensor::ICameraOutput* Output =
			reinterpret_cast<agxSensor::ICameraOutput*>(CaptureState.OutputAddr);
		Outputs.Emplace(std::make_shared<FCameraOutputRef>(Output));
	}

	return Outputs;
}

void FCameraBarrier::MarkOutputAsRead()
{
	check(HasNative());
	using namespace CameraBarrier_helpers;

	GetCameraNative(*this)->getOutputHandler()->visitChildrenOfType<agxSensor::ICameraOutput>(
		[](agxSensor::ICameraOutput& Output) { Output.hasUnreadData(/*markAsRead*/ true); });
}

bool FCameraBarrier::IsCamera(const FSensorBarrier& Sensor)
{
	return Sensor.HasNative() && Sensor.GetNative()->Native->is<agxSensor::Camera>();
}

void FCameraBarrier::SetBackendPropagator(FCameraBackendPropagatorBase* InPropagator)
{
	BackendPropagator = InPropagator;
}

FCameraBackendPropagatorBase* FCameraBarrier::GetBackendPropagator() const
{
	return BackendPropagator;
}

namespace FCameraBackend_Helpers
{
	double GetNextCaptureDelta(agxSensor::ICameraOutput* Output)
	{
		if (auto OptionalFramerate = Output->getFramerate())
		{
			if (*OptionalFramerate > 0)
			{
				return 1.0 / *OptionalFramerate;
			}
		}

		return INFINITY;
	}
}

/// Camera Backend Callbacks.

void FCameraBarrier::OnBackendSynchronize(
	TArray<FAGX_CameraCaptureState>& CaptureStates, double DeltaTime)
{
	check(HasNative());

	for (FAGX_CameraCaptureState& CaptureState : CaptureStates)
	{
		CaptureState.AccumulatedTime += DeltaTime;
		if (CaptureState.OutputAddr == 0)
		{
			UE_LOG(
				LogTemp, Warning,
				TEXT("FCameraBarrier::OnBackendSynchronize found CaptureState with no output "
					 "address."));
			continue;
		}

		agxSensor::ICameraOutput* Output =
			reinterpret_cast<agxSensor::ICameraOutput*>(CaptureState.OutputAddr);

		double NextCaptureDelta = FCameraBackend_Helpers::GetNextCaptureDelta(Output);
		double TimeSinceLastCapture = CaptureState.AccumulatedTime - CaptureState.LastCaptureTime;

		if (TimeSinceLastCapture + SMALL_NUMBER >= NextCaptureDelta)
		{
			// Time for a new capture.
			CaptureState.LastCaptureTime = CaptureState.AccumulatedTime;
			Output->capture();
		}
	}
}

void FCameraBarrier::OnBackendSetCameraLensSingleElement(
	const FCameraLensSingleElementParameters& Parameters)
{
	if (BackendPropagator != nullptr)
		BackendPropagator->OnBackendSetCameraLensSingleElement(Parameters);
}

void FCameraBarrier::OnBackendRequestCapture(uint64 NativeOutputAddress)
{
	if (BackendPropagator != nullptr)
		BackendPropagator->OnBackendRequestCapture(NativeOutputAddress);
}
