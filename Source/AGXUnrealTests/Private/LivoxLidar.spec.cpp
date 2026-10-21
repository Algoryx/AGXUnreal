// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "Import/AGX_ImportContext.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_LivoxAviaParameters.h"
#include "Sensors/AGX_LivoxHAPParameters.h"
#include "Sensors/AGX_LivoxHorizonParameters.h"
#include "Sensors/AGX_LivoxMid360Parameters.h"
#include "Sensors/AGX_LivoxMid40Parameters.h"
#include "Sensors/AGX_LivoxMid70Parameters.h"
#include "Sensors/AGX_LivoxTeleParameters.h"
#include "Sensors/AGX_OusterOS0Parameters.h"
#include "Sensors/LidarBarrier.h"
#include "Utilities/AGX_SensorUtilities.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "UObject/Package.h"

BEGIN_DEFINE_SPEC(
	FLivoxLidarSpec, "AGXUnreal.Spec.LivoxLidar", AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FLivoxLidarSpec)

namespace LivoxLidarSpec_helpers
{
	template <typename ParametersT>
	void CheckCreationAndImport(FAutomationTestBase& Test, EAGX_LidarModel Model)
	{
		Test.TestEqual(
			"Model maps to the matching parameter Asset type",
			FAGX_SensorUtilities::GetParameterTypeFrom(Model), ParametersT::StaticClass());

		auto Parameters = NewObject<ParametersT>();
		Test.TestEqual("Default Downsample uses the full output rate", Parameters->Downsample, 1);
		Parameters->Downsample = 7;

		FLidarBarrier Barrier;
		Barrier.AllocateNative(Model, *Parameters);
		Test.TestTrue("Native Livox Lidar was created", Barrier.HasNative());
		if (!Barrier.HasNative())
			return;

		Test.TestEqual("Native model is recognized", Barrier.GetModel(), Model);
		Test.TestFalse("Livox uses a file pattern", Barrier.UsesHorizontalSweepRayPattern());

		auto ReadParameters = NewObject<ParametersT>();
		Test.TestTrue(
			"Native parameters can be read", Barrier.ReadModelParameters(*ReadParameters));
		Test.TestEqual("Native Downsample is preserved", ReadParameters->Downsample, 7);

		auto OusterParameters = NewObject<UAGX_OusterOS0Parameters>();
		Test.TestFalse(
			"Livox parameters cannot be read as Ouster",
			Barrier.ReadModelParameters(*OusterParameters));

		Barrier.SetName("LivoxImportTest");
		FAGX_ImportContext Context;
		Context.Outer = GetTransientPackage();
		Context.SessionGuid = FGuid::NewGuid();
		auto Lidar = NewObject<UAGX_LidarSensorComponent>();
		Lidar->CopyFrom(Barrier, &Context);

		Test.TestEqual("Imported model is preserved", Lidar->GetModel(), Model);
		auto ImportedParameters = Cast<ParametersT>(Lidar->ModelParameters);
		Test.TestNotNull("Import creates the matching parameter Asset", ImportedParameters);
		if (ImportedParameters != nullptr)
		{
			Test.TestEqual("Imported Downsample is preserved", ImportedParameters->Downsample, 7);
			Test.TestEqual(
				"Imported parameters are recorded in the import context",
				Context.LidarModelParameters.FindRef(Barrier.GetGuid()).Get(),
				Lidar->ModelParameters);
			ImportedParameters->MarkAsGarbage();
		}
		Lidar->MarkAsGarbage();
	}
}

void FLivoxLidarSpec::Define()
{
	using namespace LivoxLidarSpec_helpers;

	It("should create and import LivoxAvia", [this]()
	   { CheckCreationAndImport<UAGX_LivoxAviaParameters>(*this, EAGX_LidarModel::LivoxAvia); });

	It("should create and import LivoxHAP", [this]()
	   { CheckCreationAndImport<UAGX_LivoxHAPParameters>(*this, EAGX_LidarModel::LivoxHAP); });

	It("should create and import LivoxHorizon",
	   [this]()
	   {
		   CheckCreationAndImport<UAGX_LivoxHorizonParameters>(
			   *this, EAGX_LidarModel::LivoxHorizon);
	   });

	It("should create and import LivoxMid40", [this]()
	   { CheckCreationAndImport<UAGX_LivoxMid40Parameters>(*this, EAGX_LidarModel::LivoxMid40); });

	It("should create and import LivoxMid70", [this]()
	   { CheckCreationAndImport<UAGX_LivoxMid70Parameters>(*this, EAGX_LidarModel::LivoxMid70); });

	It("should create and import LivoxMid360",
	   [this]()
	   {
		   CheckCreationAndImport<UAGX_LivoxMid360Parameters>(*this, EAGX_LidarModel::LivoxMid360);
	   });

	It("should create and import LivoxTele", [this]()
	   { CheckCreationAndImport<UAGX_LivoxTeleParameters>(*this, EAGX_LidarModel::LivoxTele); });

	It("should reject Downsample values below 1",
	   [this]()
	   {
		   AddExpectedError(
			   "Livox Downsample must be at least 1.", EAutomationExpectedErrorFlags::Contains, 2);
		   auto Parameters = NewObject<UAGX_LivoxAviaParameters>();
		   for (int32 Downsample : {0, -1})
		   {
			   Parameters->Downsample = Downsample;
			   FLidarBarrier Barrier;
			   Barrier.AllocateNative(EAGX_LidarModel::LivoxAvia, *Parameters);
			   TestFalse("Invalid Downsample does not create a native Lidar", Barrier.HasNative());
		   }
	   });
}
