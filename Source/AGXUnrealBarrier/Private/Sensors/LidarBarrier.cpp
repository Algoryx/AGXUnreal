// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/LidarBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "RigidBodyBarrier.h"
#include "Sensors/AGX_CustomRayPatternParameters.h"
#include "Sensors/AGX_DistanceGaussianNoiseSettings.h"
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"
#include "Sensors/AGX_LivoxAviaParameters.h"
#include "Sensors/AGX_LivoxHAPParameters.h"
#include "Sensors/AGX_LivoxHorizonParameters.h"
#include "Sensors/AGX_LivoxMid360Parameters.h"
#include "Sensors/AGX_LivoxMid40Parameters.h"
#include "Sensors/AGX_LivoxMid70Parameters.h"
#include "Sensors/AGX_LivoxTeleParameters.h"
#include "Sensors/AGX_OusterOS0Parameters.h"
#include "Sensors/AGX_OusterOS1Parameters.h"
#include "Sensors/AGX_OusterOS2Parameters.h"
#include "Sensors/CustomPatternGenerator.h"
#include "Sensors/CustomPatternFetcherBase.h"
#include "Sensors/LidarOutputBarrier.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Frame.h>
#include <agx/ref_ptr.h>
#include <agxSensor/LidarModel.h>
#include <agxSensor/LidarModelLivox.h>
#include <agxSensor/LidarModelOusterOS.h>
#include <agxSensor/LidarRayPatternGenerator.h>
#include <agxSensor/LidarRayPatternHorizontalSweep.h>
#include <agxSensor/RaytraceDistanceGaussianNoise.h>
#include <agxSensor/RaytraceOutput.h>
#include <agxSensor/SensorGroupStepStride.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <limits>

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

	template <typename ModelT>
	agxSensor::Lidar* CreateLivoxLidar(int32 Downsample)
	{
		if (Downsample < 1)
		{
			UE_LOG(LogAGX, Error, TEXT("Livox Downsample must be at least 1."));
			return nullptr;
		}

		agx::ref_ptr<ModelT> Model = new ModelT(static_cast<size_t>(Downsample));
		if (Model->getRayPatternGenerator()->getNumRays() == 0)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Unable to load the Livox ray pattern. Ensure the AGX Dynamics "
					"MaterialLibrary/LidarRayPatterns files are available."));
			return nullptr;
		}

		return new agxSensor::Lidar(new agx::Frame(), Model.get());
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_LivoxAviaParameters& Params)
	{
		return CreateLivoxLidar<agxSensor::LidarModelLivoxAvia>(Params.Downsample);
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_LivoxHAPParameters& Params)
	{
		return CreateLivoxLidar<agxSensor::LidarModelLivoxHap>(Params.Downsample);
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_LivoxHorizonParameters& Params)
	{
		return CreateLivoxLidar<agxSensor::LidarModelLivoxHorizon>(Params.Downsample);
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_LivoxMid40Parameters& Params)
	{
		return CreateLivoxLidar<agxSensor::LidarModelLivoxMid40>(Params.Downsample);
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_LivoxMid70Parameters& Params)
	{
		return CreateLivoxLidar<agxSensor::LidarModelLivoxMid70>(Params.Downsample);
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_LivoxMid360Parameters& Params)
	{
		return CreateLivoxLidar<agxSensor::LidarModelLivoxMid360>(Params.Downsample);
	}

	agxSensor::Lidar* CreateAGXLidar(const UAGX_LivoxTeleParameters& Params)
	{
		return CreateLivoxLidar<agxSensor::LidarModelLivoxTele>(Params.Downsample);
	}

	agxSensor::Lidar* CreateAGXLidar(FCustomPatternFetcherBase* PatternFetcher)
	{
		return new agxSensor::Lidar(
			new agx::Frame(), new UnrealLidarModel(new FCustomPatternGenerator(PatternFetcher)));
	}

	agxSensor::Lidar* GetLidarNative(FLidarBarrier& Lidar)
	{
		AGX_CHECK(Lidar.HasNative());
		return Lidar.GetNative()->Native->asSafe<agxSensor::Lidar>();
	}

	agxSensor::Lidar* GetLidarNative(const FLidarBarrier& Lidar)
	{
		AGX_CHECK(Lidar.HasNative());
		return Lidar.GetNative()->Native->asSafe<agxSensor::Lidar>();
	}

	const agxSensor::LidarModel* GetLidarModel(const FLidarBarrier& Lidar)
	{
		return GetLidarNative(Lidar)->getModel();
	}

	const agxSensor::LidarModelOusterOS* GetOusterOSModel(const FLidarBarrier& Lidar)
	{
		const agxSensor::LidarModel* Model = GetLidarModel(Lidar);
		return Model != nullptr ? Model->asSafe<agxSensor::LidarModelOusterOS>() : nullptr;
	}

	void ReadOusterOSModelParameters(
		const agxSensor::LidarModelOusterOS& Model, EAGX_OusterOSChannelCount& ChannelCount,
		EAGX_OusterOSBeamSpacing& BeamSpacing, EAGX_OusterOSMode& Mode)
	{
		ChannelCount = Convert(Model.getChannelCount());
		BeamSpacing = Convert(Model.getBeamSpacing());
		Mode = Convert(Model.getLidarMode());
	}

	template <typename ModelT, typename ParametersT>
	bool ReadLivoxModelParameters(const FLidarBarrier& Lidar, ParametersT& Parameters)
	{
		const agxSensor::LidarModel* Model = GetLidarModel(Lidar);
		const ModelT* LivoxModel = Model != nullptr ? Model->asSafe<ModelT>() : nullptr;
		if (LivoxModel == nullptr)
			return false;

		const size_t Downsample = LivoxModel->getDownsample();
		if (Downsample == 0 || Downsample > static_cast<size_t>(std::numeric_limits<int32>::max()))
			return false;

		Parameters.Downsample = static_cast<int32>(Downsample);
		return true;
	}

	double CalculateAngularResolution(double Fov, agx::UInt Resolution)
	{
		return Resolution > 1 ? Fov / (static_cast<double>(Resolution) - 1.0) : 0.0;
	}
}

FLidarBarrier::FLidarBarrier(
	std::shared_ptr<FSensorRef> Native, std::shared_ptr<FSensorGroupStepStrideRef> StepStride)
	: FSensorBarrier(std::move(Native), std::move(StepStride))
{
}

bool FLidarBarrier::IsLidar(const FSensorBarrier& Sensor)
{
	return Sensor.HasNative() && Sensor.GetNative()->Native->is<agxSensor::Lidar>();
}

EAGX_LidarModel FLidarBarrier::GetModel() const
{
	check(HasNative());

	const agxSensor::LidarModel* Model = LidarBarrier_helpers::GetLidarNative(*this)->getModel();
	if (Model == nullptr)
		return EAGX_LidarModel::Invalid;

	if (Model->is<agxSensor::LidarModelHorizontalSweep>())
		return EAGX_LidarModel::GenericHorizontalSweep;

	if (Model->is<agxSensor::LidarModelOusterOS0>())
		return EAGX_LidarModel::OusterOS0;

	if (Model->is<agxSensor::LidarModelOusterOS1>())
		return EAGX_LidarModel::OusterOS1;

	if (Model->is<agxSensor::LidarModelOusterOS2>())
		return EAGX_LidarModel::OusterOS2;

	if (Model->is<agxSensor::LidarModelLivoxAvia>())
		return EAGX_LidarModel::LivoxAvia;

	if (Model->is<agxSensor::LidarModelLivoxHap>())
		return EAGX_LidarModel::LivoxHAP;

	if (Model->is<agxSensor::LidarModelLivoxHorizon>())
		return EAGX_LidarModel::LivoxHorizon;

	if (Model->is<agxSensor::LidarModelLivoxMid40>())
		return EAGX_LidarModel::LivoxMid40;

	if (Model->is<agxSensor::LidarModelLivoxMid70>())
		return EAGX_LidarModel::LivoxMid70;

	if (Model->is<agxSensor::LidarModelLivoxMid360>())
		return EAGX_LidarModel::LivoxMid360;

	if (Model->is<agxSensor::LidarModelLivoxTele>())
		return EAGX_LidarModel::LivoxTele;

	return EAGX_LidarModel::Invalid;
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
		case EAGX_LidarModel::LivoxAvia:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_LivoxAviaParameters*>(&Params));
			return;
		case EAGX_LidarModel::LivoxHAP:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_LivoxHAPParameters*>(&Params));
			return;
		case EAGX_LidarModel::LivoxHorizon:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_LivoxHorizonParameters*>(&Params));
			return;
		case EAGX_LidarModel::LivoxMid40:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_LivoxMid40Parameters*>(&Params));
			return;
		case EAGX_LidarModel::LivoxMid70:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_LivoxMid70Parameters*>(&Params));
			return;
		case EAGX_LidarModel::LivoxMid360:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_LivoxMid360Parameters*>(&Params));
			return;
		case EAGX_LidarModel::LivoxTele:
			NativeRef->Native =
				CreateAGXLidar(*static_cast<const UAGX_LivoxTeleParameters*>(&Params));
			return;
		case EAGX_LidarModel::Invalid:
			break;
	}

	UE_LOG(LogAGX, Error, TEXT("Unknown Lidar Model given to to FLidarBarrier::AllocateNative."));
}

void FLidarBarrier::AllocateNativeCustomRayPattern(FCustomPatternFetcherBase& PatternFetcher)
{
	using namespace LidarBarrier_helpers;
	check(!HasNative());

	NativeRef->Native = CreateAGXLidar(&PatternFetcher);
}

void FLidarBarrier::SetLocalTransform(const FTransform& Transform)
{
	check(HasNative());
	LidarBarrier_helpers::GetLidarNative(*this)->getFrame()->setLocalMatrix(Convert(Transform));
}

FTransform FLidarBarrier::GetLocalTransform() const
{
	check(HasNative());
	return Convert(LidarBarrier_helpers::GetLidarNative(*this)->getFrame()->getMatrix());
}

void FLidarBarrier::SetRange(FAGX_RealInterval Range)
{
	check(HasNative());
	LidarBarrier_helpers::GetLidarNative(*this)->getModel()->getRayRange()->setRange(
		{static_cast<float>(ConvertDistanceToAGX(Range.Min)),
		 static_cast<float>(ConvertDistanceToAGX(Range.Max))});
}

FAGX_RealInterval FLidarBarrier::GetRange() const
{
	check(HasNative());
	const agx::RangeReal32 RangeAGX =
		LidarBarrier_helpers::GetLidarNative(*this)->getModel()->getRayRange()->getRange();
	return ConvertDistance(RangeAGX);
}

void FLidarBarrier::SetBeamDivergence(double BeamDivergence)
{
	check(HasNative());
	using namespace LidarBarrier_helpers;
	const agx::Real DivergenceAGX = ConvertAngleToAGX(BeamDivergence);
	GetLidarNative(*this)->getModel()->getProperties()->setBeamDivergence(DivergenceAGX);
}

double FLidarBarrier::GetBeamDivergence() const
{
	check(HasNative());
	using namespace LidarBarrier_helpers;
	const agx::Real DivergenceAGX =
		GetLidarNative(*this)->getModel()->getProperties()->getBeamDivergence();
	return ConvertAngleToUnreal<double>(DivergenceAGX);
}

void FLidarBarrier::SetBeamExitRadius(double BeamExitRadius)
{
	check(HasNative());
	using namespace LidarBarrier_helpers;
	const agx::Real ExitRadiusAGX = ConvertDistanceToAGX(BeamExitRadius);
	GetLidarNative(*this)->getModel()->getProperties()->setBeamExitRadius(ExitRadiusAGX);
}

double FLidarBarrier::GetBeamExitRadius() const
{
	check(HasNative());
	using namespace LidarBarrier_helpers;
	const agx::Real ExitRadiusAGX =
		GetLidarNative(*this)->getModel()->getProperties()->getBeamExitRadius();
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
	LidarBarrier_helpers::GetLidarNative(*this)->getOutputHandler()->setEnableRemoveRayMisses(
		bEnable);
}

bool FLidarBarrier::GetEnableRemoveRayMisses() const
{
	check(HasNative());
	return LidarBarrier_helpers::GetLidarNative(*this)
		->getOutputHandler()
		->getEnableRemoveRayMisses();
}

void FLidarBarrier::SetRaytraceDepth(size_t Depth)
{
	check(HasNative());
	LidarBarrier_helpers::GetLidarNative(*this)->getOutputHandler()->setRaytraceDepth(Depth);
}

size_t FLidarBarrier::GetRaytraceDepth() const
{
	check(HasNative());
	return LidarBarrier_helpers::GetLidarNative(*this)->getOutputHandler()->getRaytraceDepth();
}

void FLidarBarrier::EnableOrUpdateDistanceGaussianNoise(
	const FAGX_DistanceGaussianNoiseSettings& Settings)
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	agxSensor::RtDistanceGaussianNoiseRef DistanceNoise = GetDistanceNoise(*GetLidarNative(*this));
	if (DistanceNoise == nullptr)
	{
		DistanceNoise = new agxSensor::RtDistanceGaussianNoise();
		GetLidarNative(*this)->getOutputHandler()->add(DistanceNoise);
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

	agxSensor::RtDistanceGaussianNoise* DistanceNoise = GetDistanceNoise(*GetLidarNative(*this));
	if (DistanceNoise != nullptr)
		GetLidarNative(*this)->getOutputHandler()->remove(DistanceNoise);
}

bool FLidarBarrier::GetEnableDistanceGaussianNoise() const
{
	using namespace LidarBarrier_helpers;
	check(HasNative());
	return GetDistanceNoise(*GetLidarNative(*this)) != nullptr;
}

FAGX_DistanceGaussianNoiseSettings FLidarBarrier::GetDistanceGaussianNoiseSettings() const
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	FAGX_DistanceGaussianNoiseSettings Settings;
	agxSensor::RtDistanceGaussianNoise* DistanceNoise = GetDistanceNoise(*GetLidarNative(*this));
	if (DistanceNoise == nullptr)
		return Settings;

	Settings.Mean = ConvertDistanceToUnreal<double>(DistanceNoise->getMean());
	Settings.StandardDeviation = ConvertDistanceToUnreal<double>(DistanceNoise->getStdDevBase());
	Settings.StandardDeviationSlope = DistanceNoise->getStdDevSlope();
	return Settings;
}

void FLidarBarrier::EnableOrUpdateRayAngleGaussianNoise(
	const FAGX_RayAngleGaussianNoiseSettings& Settings)
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	agxSensor::LidarRayAngleGaussianNoise* Noise = GetRayAngleNoise(*GetLidarNative(*this));
	if (Noise == nullptr)
	{
		Noise = new agxSensor::LidarRayAngleGaussianNoise();
		GetLidarNative(*this)->getRayDistortionHandler()->add(Noise);
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

	agxSensor::LidarRayAngleGaussianNoise* Noise = GetRayAngleNoise(*GetLidarNative(*this));
	if (Noise != nullptr)
		GetLidarNative(*this)->getRayDistortionHandler()->remove(Noise);
}

bool FLidarBarrier::GetEnableRayAngleGaussianNoise() const
{
	using namespace LidarBarrier_helpers;
	check(HasNative());
	return GetRayAngleNoise(*GetLidarNative(*this)) != nullptr;
}

FAGX_RayAngleGaussianNoiseSettings FLidarBarrier::GetRayAngleGaussianNoiseSettings() const
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	FAGX_RayAngleGaussianNoiseSettings Settings;
	agxSensor::LidarRayAngleGaussianNoise* Noise = GetRayAngleNoise(*GetLidarNative(*this));
	if (Noise == nullptr)
		return Settings;

	Settings.Axis = Convert(Noise->getAxis());
	Settings.Mean = ConvertAngleToUnreal<double>(Noise->getMean());
	Settings.StandardDeviation = ConvertAngleToUnreal<double>(Noise->getStandardDeviation());
	return Settings;
}

bool FLidarBarrier::ReadModelParameters(UAGX_CustomRayPatternParameters&) const
{
	check(HasNative());
	return true;
}

bool FLidarBarrier::ReadModelParameters(UAGX_GenericHorizontalSweepParameters& Parameters) const
{
	check(HasNative());

	const agxSensor::LidarModel* Model = LidarBarrier_helpers::GetLidarModel(*this);
	if (Model == nullptr)
		return false;

	const auto PatternGenerator = Model->getRayPatternGenerator();
	if (PatternGenerator == nullptr)
		return false;

	const auto Pattern = PatternGenerator->asSafe<agxSensor::LidarRayPatternHorizontalSweep>();
	if (Pattern == nullptr)
		return false;

	const auto HFovAGX = Pattern->getHorizontalFov();
	const auto VFovAGX = Pattern->getVerticalFov();
	const double HorizontalFov =
		ConvertAngleToUnreal<double>(FMath::Abs(HFovAGX.upper() - HFovAGX.lower()));
	const double VerticalFov =
		ConvertAngleToUnreal<double>(FMath::Abs(VFovAGX.upper() - VFovAGX.lower()));
	const agx::Vec2u ResolutionAGX = Pattern->getResolution();

	Parameters.FOV = {HorizontalFov, VerticalFov};
	Parameters.Resolution = {
		LidarBarrier_helpers::CalculateAngularResolution(HorizontalFov, ResolutionAGX.x()),
		LidarBarrier_helpers::CalculateAngularResolution(VerticalFov, ResolutionAGX.y())};
	Parameters.Frequency = Pattern->getFrequency();
	return true;
}

bool FLidarBarrier::ReadModelParameters(UAGX_OusterOS0Parameters& Parameters) const
{
	check(HasNative());
	const agxSensor::LidarModel* Model = LidarBarrier_helpers::GetLidarModel(*this);
	if (Model == nullptr || !Model->is<agxSensor::LidarModelOusterOS0>())
		return false;

	const agxSensor::LidarModelOusterOS* OusterModel =
		LidarBarrier_helpers::GetOusterOSModel(*this);
	if (OusterModel == nullptr)
		return false;

	LidarBarrier_helpers::ReadOusterOSModelParameters(
		*OusterModel, Parameters.ChannelCount, Parameters.BeamSpacing, Parameters.Mode);
	return true;
}

bool FLidarBarrier::ReadModelParameters(UAGX_OusterOS1Parameters& Parameters) const
{
	check(HasNative());
	const agxSensor::LidarModel* Model = LidarBarrier_helpers::GetLidarModel(*this);
	if (Model == nullptr || !Model->is<agxSensor::LidarModelOusterOS1>())
		return false;

	const agxSensor::LidarModelOusterOS* OusterModel =
		LidarBarrier_helpers::GetOusterOSModel(*this);
	if (OusterModel == nullptr)
		return false;

	LidarBarrier_helpers::ReadOusterOSModelParameters(
		*OusterModel, Parameters.ChannelCount, Parameters.BeamSpacing, Parameters.Mode);
	return true;
}

bool FLidarBarrier::ReadModelParameters(UAGX_OusterOS2Parameters& Parameters) const
{
	check(HasNative());
	const agxSensor::LidarModel* Model = LidarBarrier_helpers::GetLidarModel(*this);
	if (Model == nullptr || !Model->is<agxSensor::LidarModelOusterOS2>())
		return false;

	const agxSensor::LidarModelOusterOS* OusterModel =
		LidarBarrier_helpers::GetOusterOSModel(*this);
	if (OusterModel == nullptr)
		return false;

	LidarBarrier_helpers::ReadOusterOSModelParameters(
		*OusterModel, Parameters.ChannelCount, Parameters.BeamSpacing, Parameters.Mode);
	return true;
}

bool FLidarBarrier::ReadModelParameters(UAGX_LivoxAviaParameters& Parameters) const
{
	check(HasNative());
	return LidarBarrier_helpers::ReadLivoxModelParameters<agxSensor::LidarModelLivoxAvia>(
		*this, Parameters);
}

bool FLidarBarrier::ReadModelParameters(UAGX_LivoxHAPParameters& Parameters) const
{
	check(HasNative());
	return LidarBarrier_helpers::ReadLivoxModelParameters<agxSensor::LidarModelLivoxHap>(
		*this, Parameters);
}

bool FLidarBarrier::ReadModelParameters(UAGX_LivoxHorizonParameters& Parameters) const
{
	check(HasNative());
	return LidarBarrier_helpers::ReadLivoxModelParameters<agxSensor::LidarModelLivoxHorizon>(
		*this, Parameters);
}

bool FLidarBarrier::ReadModelParameters(UAGX_LivoxMid40Parameters& Parameters) const
{
	check(HasNative());
	return LidarBarrier_helpers::ReadLivoxModelParameters<agxSensor::LidarModelLivoxMid40>(
		*this, Parameters);
}

bool FLidarBarrier::ReadModelParameters(UAGX_LivoxMid70Parameters& Parameters) const
{
	check(HasNative());
	return LidarBarrier_helpers::ReadLivoxModelParameters<agxSensor::LidarModelLivoxMid70>(
		*this, Parameters);
}

bool FLidarBarrier::ReadModelParameters(UAGX_LivoxMid360Parameters& Parameters) const
{
	check(HasNative());
	return LidarBarrier_helpers::ReadLivoxModelParameters<agxSensor::LidarModelLivoxMid360>(
		*this, Parameters);
}

bool FLidarBarrier::ReadModelParameters(UAGX_LivoxTeleParameters& Parameters) const
{
	check(HasNative());
	return LidarBarrier_helpers::ReadLivoxModelParameters<agxSensor::LidarModelLivoxTele>(
		*this, Parameters);
}

bool FLidarBarrier::UsesHorizontalSweepRayPattern() const
{
	check(HasNative());
	const agxSensor::LidarModel* Model = LidarBarrier_helpers::GetLidarNative(*this)->getModel();
	if (Model == nullptr)
		return false;

	const auto PatternGenerator = Model->getRayPatternGenerator();
	if (PatternGenerator == nullptr)
		return false;

	return PatternGenerator->is<agxSensor::LidarRayPatternHorizontalSweep>();
}

FRigidBodyBarrier FLidarBarrier::GetRigidBody() const
{
	using namespace LidarBarrier_helpers;
	check(HasNative());

	agx::RigidBody* Body = nullptr;
	for (agx::Frame* Frame = GetLidarNative(*this)->getFrame(); Frame != nullptr;
		 Frame = Frame->getParent())
	{
		Body = Frame->getRigidBody();
		if (Body != nullptr)
			break;
	}

	return AGXBarrierFactories::CreateRigidBodyBarrier(Body);
}

void FLidarBarrier::AddOutput(FLidarOutputBarrier& Output)
{
	check(HasNative());
	check(Output.HasNative());
	using namespace LidarBarrier_helpers;

	const size_t Id = GenerateUniqueOutputId();
	GetLidarNative(*this)->getOutputHandler()->add(Id, Output.GetNative()->Native);
}

void FLidarBarrier::MarkOutputAsRead()
{
	check(HasNative());
	using namespace LidarBarrier_helpers;

	GetLidarNative(*this)->getOutputHandler()->visitChildrenOfType<agxSensor::RtOutput>(
		[](agxSensor::RtOutput& Output) { Output.hasUnreadData(/*markAsRead*/ true); });
}
