// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/LidarBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Sensors/AGX_CustomRayPatternParameters.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"
#include "Sensors/AGX_OusterOS0Parameters.h"
#include "Sensors/AGX_OusterOS1Parameters.h"
#include "Sensors/AGX_OusterOS2Parameters.h"
#include "Sensors/CustomPatternGenerator.h"
#include "Sensors/CustomPatternFetcherBase.h"
#include "Sensors/LidarOutputBarrier.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/LidarModel.h>
#include <agxSensor/LidarModelOusterOS.h>
#include <agxSensor/LidarRayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceDistanceGaussianNoise.h>
#include <agxSensor/RaytraceOutput.h>
#include <agxSensor/SensorGroupStepStride.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <limits>

FLidarBarrier::FLidarBarrier()
	: NativeRef {new FLidarRef}
	, StepStrideRef {new FSensorGroupStepStrideRef}
{
}

FLidarBarrier::FLidarBarrier(
	std::unique_ptr<FLidarRef> Native, std::unique_ptr<FSensorGroupStepStrideRef> StepStride)
	: NativeRef(std::move(Native))
	, StepStrideRef(std::move(StepStride))
{
}

FLidarBarrier::FLidarBarrier(FLidarBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
	, StepStrideRef {std::move(Other.StepStrideRef)}
{
	Other.NativeRef.reset(new FLidarRef);
	Other.StepStrideRef.reset(new FSensorGroupStepStrideRef);
}

FLidarBarrier::~FLidarBarrier()
{
	ReleaseNative();
}

bool FLidarBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

namespace LidarBarrier_helpers
{
	class UnrealLidarModel : public agxSensor::LidarModel
	{
	public:
		UnrealLidarModel(agxSensor::LidarRayPatternGenerator* PatternGenerator)
			: LidarModel(PatternGenerator, {0.f, std::numeric_limits<agx::Real32>::max()})
		{
		}
	};

	agxSensor::Lidar* CreateAGXLidar(const UAGX_GenericHorizontalSweepParameters& Params)
	{
		const agx::Vec2 FovAGX {ConvertAngleToAGX(Params.FOV.X), ConvertAngleToAGX(Params.FOV.Y)};
		const agx::Vec2 ResolutionAGX {
			ConvertAngleToAGX(Params.Resolution.X), ConvertAngleToAGX(Params.Resolution.Y)};

		return new agxSensor::Lidar(
			new agx::Frame(),
			new agxSensor::LidarModelHorizontalSweep(FovAGX, ResolutionAGX, Params.Frequency));
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_OusterOS0Parameters& Params)
	{
		auto CountAGX = Convert(Params.ChannelCount);
		auto BeamSpacingAGX = Convert(Params.BeamSpacing);
		auto Mode = Convert(Params.Mode);
		return new agxSensor::Lidar(
			new agx::Frame(), new agxSensor::LidarModelOusterOS0(CountAGX, BeamSpacingAGX, Mode));
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_OusterOS1Parameters& Params)
	{
		auto CountAGX = Convert(Params.ChannelCount);
		auto BeamSpacingAGX = Convert(Params.BeamSpacing);
		auto Mode = Convert(Params.Mode);
		return new agxSensor::Lidar(
			new agx::Frame(), new agxSensor::LidarModelOusterOS1(CountAGX, BeamSpacingAGX, Mode));
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_OusterOS2Parameters& Params)
	{
		auto CountAGX = Convert(Params.ChannelCount);
		auto BeamSpacingAGX = Convert(Params.BeamSpacing);
		auto Mode = Convert(Params.Mode);
		return new agxSensor::Lidar(
			new agx::Frame(), new agxSensor::LidarModelOusterOS2(CountAGX, BeamSpacingAGX, Mode));
	}

	agxSensor::Lidar* CreateAGXLidar(FCustomPatternFetcherBase* PatternFetcher)
	{
		return new agxSensor::Lidar(
			new agx::Frame(), new UnrealLidarModel(new FCustomPatternGenerator(PatternFetcher)));
	}
}

void FLidarBarrier::AllocateNative(EAGX_LidarModel Model, const UAGX_LidarModelParameters& Params)
{
	using namespace LidarBarrier_helpers;
	check(!HasNative());

	switch (Model)
	{
		case EAGX_LidarModel::CustomRayPattern:
			UE_LOG(
				LogAGX, Error,
				TEXT("CustomRayPattern model passed to FLidarBarrier::AllocateNative, but this "
					 "case should be handled by FLidarBarrier::AllocateNativeCustomRayPattern."));
			return;
		case EAGX_LidarModel::GenericHorizontalSweep:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_GenericHorizontalSweepParameters*>(&Params));
			return;
		case EAGX_LidarModel::OusterOS0:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_OusterOS0Parameters*>(&Params));
			return;
		case EAGX_LidarModel::OusterOS1:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_OusterOS1Parameters*>(&Params));
			return;
		case EAGX_LidarModel::OusterOS2:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_OusterOS2Parameters*>(&Params));
			return;
	}

	UE_LOG(LogAGX, Error, TEXT("Unknown Lidar Model given to to FLidarBarrier::AllocateNative."));
}

void FLidarBarrier::AllocateNativeCustomRayPattern(FCustomPatternFetcherBase& PatternFetcher)
{
	using namespace LidarBarrier_helpers;
	check(!HasNative());

	NativeRef->Native = CreateAGXLidar(&PatternFetcher);
}

FLidarRef* FLidarBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FLidarRef* FLidarBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uint64 FLidarBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FLidarBarrier::SetNativeAddress(uint64 Address)
{
	NativeRef->Native = reinterpret_cast<agxSensor::Lidar*>(Address);

	// At this point, we should be able to find any StepStride object, since it will have been
	// kept alive by the agxSensor::Environment.
	StepStrideRef->Native = NativeRef->Native->findParent<agxSensor::SensorGroupStepStride>();
}

void FLidarBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;

	if (StepStrideRef->Native != nullptr)
		StepStrideRef->Native = nullptr;
}

void FLidarBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FLidarBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FLidarBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	NativeRef->Native->getFrame()->setMatrix(Convert(Transform));
}

FTransform FLidarBarrier::GetTransform() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getFrame()->getMatrix());
}

void FLidarBarrier::SetRange(FAGX_RealInterval Range)
{
	check(HasNative());
	NativeRef->Native->getModel()->getRayRange()->setRange(
		{static_cast<float>(ConvertDistanceToAGX(Range.Min)),
		 static_cast<float>(ConvertDistanceToAGX(Range.Max))});
}

FAGX_RealInterval FLidarBarrier::GetRange() const
{
	check(HasNative());
	const agx::RangeReal32 RangeAGX = NativeRef->Native->getModel()->getRayRange()->getRange();
	return ConvertDistance(RangeAGX);
}

void FLidarBarrier::SetBeamDivergence(double BeamDivergence)
{
	check(HasNative());
	const agx::Real DivergenceAGX = ConvertAngleToAGX(BeamDivergence);
	NativeRef->Native->getModel()->getProperties()->setBeamDivergence(DivergenceAGX);
}

double FLidarBarrier::GetBeamDivergence() const
{
	check(HasNative());
	const agx::Real DivergenceAGX =
		NativeRef->Native->getModel()->getProperties()->getBeamDivergence();
	return ConvertAngleToUnreal<double>(DivergenceAGX);
}

void FLidarBarrier::SetBeamExitRadius(double BeamExitRadius)
{
	check(HasNative());
	const agx::Real ExitRadiusAGX = ConvertDistanceToAGX(BeamExitRadius);
	NativeRef->Native->getModel()->getProperties()->setBeamExitRadius(ExitRadiusAGX);
}

double FLidarBarrier::GetBeamExitRadius() const
{
	check(HasNative());
	const agx::Real ExitRadiusAGX =
		NativeRef->Native->getModel()->getProperties()->getBeamExitRadius();
	return ConvertDistanceToUnreal<double>(ExitRadiusAGX);
}

namespace LidarBarrier_helpers
{
	size_t GenerateUniqueOutputId()
	{
		static size_t Id = 1;
		return Id++;
	}

	agxSensor::RtDistanceGaussianNoise* GetDistanceNoise(agxSensor::Lidar& Lidar)
	{
		agxSensor::RtOutputNoiseRefVector Noises = Lidar.getOutputHandler()->getOutputNoises();
		for (auto Noise : Noises)
		{
			if (auto DistanceNoise = Noise->asSafe<agxSensor::RtDistanceGaussianNoise>())
				return DistanceNoise;
		}

		return nullptr;
	}

	agxSensor::LidarRayAngleGaussianNoise* GetRayAngleNoise(agxSensor::Lidar& Lidar)
	{
		agxSensor::LidarRayDistortionRefVector Distortions =
			Lidar.getRayDistortionHandler()->getDistortions();
		for (auto Distortion : Distortions)
		{
			if (auto DistortionNoise = Distortion->asSafe<agxSensor::LidarRayAngleGaussianNoise>())
				return DistortionNoise;
		}

		return nullptr;
	}
}

void FLidarBarrier::SetEnableRemoveRayMisses(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->getOutputHandler()->setEnableRemoveRayMisses(bEnable);
}

bool FLidarBarrier::GetEnableRemoveRayMisses() const
{
	check(HasNative());
	return NativeRef->Native->getOutputHandler()->getEnableRemoveRayMisses();
}

void FLidarBarrier::SetRaytraceDepth(size_t Depth)
{
	check(HasNative());
	NativeRef->Native->getOutputHandler()->setRaytraceDepth(Depth);
}

size_t FLidarBarrier::GetRaytraceDepth() const
{
	check(HasNative());
	return NativeRef->Native->getOutputHandler()->getRaytraceDepth();
}

void FLidarBarrier::EnableOrUpdateDistanceGaussianNoise(
	const FAGX_DistanceGaussianNoiseSettings& Settings)
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	agxSensor::RtDistanceGaussianNoiseRef DistanceNoise = GetDistanceNoise(*NativeRef->Native);
	if (DistanceNoise == nullptr)
	{
		DistanceNoise = new agxSensor::RtDistanceGaussianNoise();
		NativeRef->Native->getOutputHandler()->add(DistanceNoise);
	}

	const agx::Real MeanAGX = ConvertDistanceToAGX(Settings.Mean);
	const agx::Real StdDevAGX = ConvertDistanceToAGX(Settings.StandardDeviation);
	const agx::Real StdDevSlopeAGX = Settings.StandardDeviationSlope; // Unitless.

	DistanceNoise->setMean(MeanAGX);
	DistanceNoise->setStdDevBase(StdDevAGX);
	DistanceNoise->setStdDevSlope(StdDevSlopeAGX);
	DistanceNoise->setDirty(true);
}

void FLidarBarrier::DisableDistanceGaussianNoise()
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	agxSensor::RtDistanceGaussianNoise* DistanceNoise = GetDistanceNoise(*NativeRef->Native);
	if (DistanceNoise != nullptr)
		NativeRef->Native->getOutputHandler()->remove(DistanceNoise);
}

bool FLidarBarrier::GetEnableDistanceGaussianNoise() const
{
	using namespace LidarBarrier_helpers;
	check(HasNative());
	return GetDistanceNoise(*NativeRef->Native) != nullptr;
}

void FLidarBarrier::EnableOrUpdateRayAngleGaussianNoise(
	const FAGX_RayAngleGaussianNoiseSettings& Settings)
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	agxSensor::LidarRayAngleGaussianNoise* Noise = GetRayAngleNoise(*NativeRef->Native);
	if (Noise == nullptr)
	{
		Noise = new agxSensor::LidarRayAngleGaussianNoise();
		NativeRef->Native->getRayDistortionHandler()->add(Noise);
	}

	const auto AxisAGX = Convert(Settings.Axis);
	const agx::Real MeanAGX = ConvertAngleToAGX(Settings.Mean);
	const agx::Real StdDevAGX = ConvertAngleToAGX(Settings.StandardDeviation);

	Noise->setAxis(AxisAGX);
	Noise->setMean(MeanAGX);
	Noise->setStandardDeviation(StdDevAGX);
}

void FLidarBarrier::DisableRayAngleGaussianNoise()
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	agxSensor::LidarRayAngleGaussianNoise* Noise = GetRayAngleNoise(*NativeRef->Native);
	if (Noise != nullptr)
		NativeRef->Native->getRayDistortionHandler()->remove(Noise);
}

bool FLidarBarrier::GetEnableRayAngleGaussianNoise() const
{
	using namespace LidarBarrier_helpers;
	check(HasNative());
	return GetRayAngleNoise(*NativeRef->Native) != nullptr;
}

void FLidarBarrier::SetStepStride(uint32 Stride)
{
	check(HasNative());

	if (StepStrideRef->Native == nullptr)
	{
		// This is the first time StepStride is used for this Lidar.
		// A quirk of AGX is that if using StepStride for a Sensor, the Sensor itself
		// should not be part of the agxSensor::Environment, but the StepStride should.
		// Instead, the Lidar should be added to the StepStride object (agxSensor::SystemNode).
		StepStrideRef->Native = new agxSensor::SensorGroupStepStride();
		auto LidarAGX = NativeRef->Native;
		if (auto Env = LidarAGX->getEnvironment())
		{
			Env->remove(LidarAGX);
			Env->add(StepStrideRef->Native);
		}
		
		StepStrideRef->Native->add(LidarAGX);
	}

	StepStrideRef->Native->setStride(Stride);
}

uint32 FLidarBarrier::GetStepStride() const
{
	check(HasNative());

	if (StepStrideRef->Native == nullptr)
		return 1; // This is the effective "default" when not using StepStride.

	return StepStrideRef->Native->getStride();
}

bool FLidarBarrier::AddToEnvironment(FSensorEnvironmentBarrier& Environment)
{
	check(HasNative());
	check(Environment.HasNative());

	if (StepStrideRef->Native != nullptr)
	{
		// We add the StepStride instead of the Lidar Native in order to ensure correct stepping.
		// This is a quirk of AGX. See comment in SetStepStride also.
		AGX_CHECK(NativeRef->Native->getEnvironment() == nullptr);
		return Environment.GetNative()->Native->add(StepStrideRef->Native);
	}
	
	return Environment.GetNative()->Native->add(NativeRef->Native);
}

bool FLidarBarrier::RemoveFromEnvironment(FSensorEnvironmentBarrier& Environment)
{
	check(HasNative());
	check(Environment.HasNative());

	if (StepStrideRef->Native != nullptr)
	{
		// See also AddToEnvironment.
		AGX_CHECK(NativeRef->Native->getEnvironment() == nullptr);
		return Environment.GetNative()->Native->remove(StepStrideRef->Native);
	}
	
	return Environment.GetNative()->Native->remove(NativeRef->Native);
}

void FLidarBarrier::AddOutput(FLidarOutputBarrier& Output)
{
	check(HasNative());
	check(Output.HasNative());

	NativeRef->Native->getOutputHandler()->add(
		LidarBarrier_helpers::GenerateUniqueOutputId(), Output.GetNative()->Native);
}

void FLidarBarrier::IncrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->reference();
}

void FLidarBarrier::DecrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->unreference();
}
