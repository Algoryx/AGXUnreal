// Copyright 2024, Algoryx Simulation AB.

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
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/LidarModel.h>
#include <agxSensor/LidarModelOusterOS.h>
#include <agxSensor/LidarRayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceDistanceGaussianNoise.h>
#include <agxSensor/RaytraceOutput.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <limits>

FLidarBarrier::FLidarBarrier()
	: NativeRef {new FLidarRef}
{
}

FLidarBarrier::FLidarBarrier(std::unique_ptr<FLidarRef> Native)
	: NativeRef(std::move(Native))
{
}

FLidarBarrier::FLidarBarrier(FLidarBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FLidarRef);
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
			nullptr,
			new agxSensor::LidarModelHorizontalSweep(FovAGX, ResolutionAGX, Params.Frequency));
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_OusterOS0Parameters& Params)
	{
		auto CountAGX = Convert(Params.ChannelCount);
		auto BeamSpacingAGX = Convert(Params.BeamSpacing);
		auto Mode = Convert(Params.Mode);
		return new agxSensor::Lidar(
			nullptr, new agxSensor::LidarModelOusterOS0(CountAGX, BeamSpacingAGX, Mode));
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_OusterOS1Parameters& Params)
	{
		auto CountAGX = Convert(Params.ChannelCount);
		auto BeamSpacingAGX = Convert(Params.BeamSpacing);
		auto Mode = Convert(Params.Mode);
		return new agxSensor::Lidar(
			nullptr, new agxSensor::LidarModelOusterOS1(CountAGX, BeamSpacingAGX, Mode));
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_OusterOS2Parameters& Params)
	{
		auto CountAGX = Convert(Params.ChannelCount);
		auto BeamSpacingAGX = Convert(Params.BeamSpacing);
		auto Mode = Convert(Params.Mode);
		return new agxSensor::Lidar(
			nullptr, new agxSensor::LidarModelOusterOS2(CountAGX, BeamSpacingAGX, Mode));
	}

	agxSensor::Lidar* CreateAGXLidar(FCustomPatternFetcherBase* PatternFetcher)
	{
		return new agxSensor::Lidar(
			nullptr, new UnrealLidarModel(new FCustomPatternGenerator(PatternFetcher)));
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
}

void FLidarBarrier::ReleaseNative()
{
	if (HasNative())
		NativeRef->Native = nullptr;
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
	*NativeRef->Native->getFrame() =
		*ConvertFrame(Transform.GetLocation(), Transform.GetRotation());
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
			if (auto DistanceNoise = Noise->as<agxSensor::RtDistanceGaussianNoise>())
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
			if (auto DistortionNoise = Distortion->as<agxSensor::LidarRayAngleGaussianNoise>())
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

void FLidarBarrier::EnableOrUpdateRayAngleGaussianNoise(const FAGX_RayAngleGaussianNoiseSettings& Settings)
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
