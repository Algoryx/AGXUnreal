// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/AGX_ConstraintController.h"
#include "Constraints/AGX_ConstraintEnums.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_ElementaryConstraint.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/TwistRangeControllerBarrier.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGXUtilities.h"
#include "Vehicle/AGX_WheelJointComponent.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace AGX_ConstraintAttenuationSpec_helpers
{
	// Exercise the actual property loader without modifying reflection or archived fixtures.
	struct FLegacyConstraintWriter : FObjectAndNameAsStringProxyArchive
	{
		FLegacyConstraintWriter(FArchive& Inner)
			: FObjectAndNameAsStringProxyArchive(Inner, false)
		{
			SetCustomVersion(
				FAGX_CustomVersion::GUID, FAGX_CustomVersion::DampingToAttenuation - 1,
				TEXT("AGXUnrealVer"));
		}

		virtual FArchive& operator<<(FName& Name) override
		{
			FName LegacyName = Name == TEXT("Attenuation") ? FName(TEXT("SpookDamping")) : Name;
			return FNameAsStringProxyArchive::operator<<(LegacyName);
		}
	};

	TArray<uint8> Save(UObject& Object, bool LegacyNames = false)
	{
		TArray<uint8> Bytes;
		FMemoryWriter Writer(Bytes);
		if (LegacyNames)
		{
			FLegacyConstraintWriter Archive(Writer);
			Object.Serialize(Archive);
		}
		else
		{
			FObjectAndNameAsStringProxyArchive Archive(Writer, false);
			Object.Serialize(Archive);
		}
		return Bytes;
	}

	bool Load(UObject& Object, const TArray<uint8>& Bytes, int32 Version)
	{
		FMemoryReader Reader(Bytes);
		FObjectAndNameAsStringProxyArchive Archive(Reader, false);
		Archive.SetCustomVersion(FAGX_CustomVersion::GUID, Version, TEXT("AGXUnrealVer"));
		Object.Serialize(Archive);
		return !Archive.IsError();
	}

	TArray<FAGX_ConstraintController*> GetControllers(UAGX_ConstraintComponent& Constraint)
	{
		TArray<FAGX_ConstraintController*> Controllers;
		if (auto* OneDof = Cast<UAGX_Constraint1DofComponent>(&Constraint); OneDof != nullptr)
		{
			for (auto* Controller : OneDof->GetAllControllers())
				Controllers.Add(Controller);
		}
		else if (auto* TwoDof = Cast<UAGX_Constraint2DofComponent>(&Constraint); TwoDof != nullptr)
		{
			for (auto* Controller : TwoDof->GetAllControllers())
				Controllers.Add(Controller);
		}
		else if (auto* Wheel = Cast<UAGX_WheelJointComponent>(&Constraint); Wheel != nullptr)
		{
			Controllers = {
				&Wheel->SteeringLockController,			&Wheel->SteeringRangeController,
				&Wheel->SteeringTargetSpeedController,	&Wheel->WheelLockController,
				&Wheel->WheelRangeController,			&Wheel->WheelTargetSpeedController,
				&Wheel->SuspensionLockController,		&Wheel->SuspensionRangeController,
				&Wheel->SuspensionTargetSpeedController};
		}
		return Controllers;
	}
}

BEGIN_DEFINE_SPEC(
	FAGX_ConstraintAttenuationSpec, "AGXUnreal.Spec.ConstraintAttenuation",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_ConstraintAttenuationSpec)

void FAGX_ConstraintAttenuationSpec::Define()
{
	using namespace AGX_ConstraintAttenuationSpec_helpers;

	It("matches native constraint and elementary constraint defaults",
	   [this]()
	   {
		   FRigidBodyBarrier Body;
		   Body.AllocateNative();
		   FHingeBarrier Hinge;
		   Hinge.AllocateNative(
			   Body, FVector::ZeroVector, FQuat::Identity, nullptr, FVector::ZeroVector,
			   FQuat::Identity);
		   const auto* Component = GetDefault<UAGX_HingeConstraintComponent>();
		   for (int32 I = 0; I < 5; ++I)
			   TestEqual(
				   TEXT("Constraint default"), Component->Attenuation[I], Hinge.GetAttenuation(I));
		   TestEqual(
			   TEXT("Controller default"), Component->RangeController.Attenuation,
			   Hinge.GetRangeController()->GetAttenuation());

		   FBallJointBarrier Ball;
		   Ball.AllocateNative(
			   Body, FVector::ZeroVector, FQuat::Identity, nullptr, FVector::ZeroVector,
			   FQuat::Identity);
		   TestEqual(
			   TEXT("Elementary constraint default"),
			   GetDefault<UAGX_BallConstraintComponent>()->TwistRangeController.Attenuation.Value,
			   Ball.GetTwistRangeController().GetAttenuation());
	   });

	It("migrates every constraint controller path only once",
	   [this]()
	   {
		   UAGX_Simulation* Simulation = GetMutableDefault<UAGX_Simulation>();
		   TGuardValue<double> TimeStepGuard(Simulation->TimeStep, 0.02);
		   UClass* Classes[] = {
			   UAGX_HingeConstraintComponent::StaticClass(),
			   UAGX_CylindricalConstraintComponent::StaticClass(),
			   UAGX_BallConstraintComponent::StaticClass(),
			   UAGX_WheelJointComponent::StaticClass()};
		   for (UClass* Class : Classes)
		   {
			   auto* Source = NewObject<UAGX_ConstraintComponent>(
				   GetTransientPackage(), Class, NAME_None, RF_ArchetypeObject);
			   for (int32 I = 0; I < NumGenericDofs; ++I)
				   Source->Attenuation[I] = 0.02 * (I + 1);
			   int32 Index = 1;
			   for (auto* Controller : GetControllers(*Source))
				   Controller->Attenuation = 0.02 * Index++;
			   if (auto* Ball = Cast<UAGX_BallConstraintComponent>(Source); Ball != nullptr)
				   Ball->TwistRangeController.Attenuation = 0.14;

			   auto* Migrated = NewObject<UAGX_ConstraintComponent>(
				   GetTransientPackage(), Class, NAME_None, RF_ArchetypeObject);
			   TestTrue(
				   TEXT("Read legacy constraint"),
				   Load(
					   *Migrated, Save(*Source, true),
					   FAGX_CustomVersion::DampingToAttenuation - 1));
			   for (int32 I = 0; I < NumGenericDofs; ++I)
				   TestEqual(TEXT("Per-DOF attenuation"), Migrated->Attenuation[I], double(I + 1));
			   Index = 1;
			   for (auto* Controller : GetControllers(*Migrated))
				   TestEqual(
					   TEXT("Controller attenuation"), Controller->Attenuation, double(Index++));
			   if (auto* Ball = Cast<UAGX_BallConstraintComponent>(Migrated); Ball != nullptr)
				   TestEqual(
					   TEXT("Twist attenuation"), Ball->TwistRangeController.Attenuation.Value,
					   7.0);

			   const TArray<uint8> Bytes = Save(*Migrated);
			   {
				   TGuardValue<double> ReloadTimeStepGuard(Simulation->TimeStep, 0.01);
				   auto* Reloaded = NewObject<UAGX_ConstraintComponent>(
					   GetTransientPackage(), Class, NAME_None, RF_ArchetypeObject);
				   TestTrue(
					   TEXT("Read resaved constraint"),
					   Load(*Reloaded, Bytes, FAGX_CustomVersion::LatestVersion));
				   TestTrue(
					   TEXT("Resaved per-DOF attenuation unchanged"),
					   Reloaded->Attenuation == Migrated->Attenuation);
				   Index = 1;
				   for (auto* Controller : GetControllers(*Reloaded))
					   TestEqual(
						   TEXT("Resaved controller unchanged"), Controller->Attenuation,
						   double(Index++));
				   if (auto* Ball = Cast<UAGX_BallConstraintComponent>(Reloaded); Ball != nullptr)
					   TestEqual(
						   TEXT("Resaved twist unchanged"),
						   Ball->TwistRangeController.Attenuation.Value, 7.0);
			   }

			   // No legacy property names are written: the old defaults must still be converted.
			   auto* Defaults = NewObject<UAGX_ConstraintComponent>(
				   GetTransientPackage(), Class, NAME_None, RF_ArchetypeObject);
			   TestTrue(
				   TEXT("Read omitted defaults"),
				   Load(*Defaults, Save(*Source), FAGX_CustomVersion::DampingToAttenuation - 1));
			   const double Expected = FAGXUtilities::ConvertSpookDampingToAttenuation(
				   ConstraintConstants::DefaultSpookDamping(), Simulation->TimeStep);
			   for (int32 I = 0; I < NumGenericDofs; ++I)
				   TestEqual(TEXT("Omitted per-DOF default"), Defaults->Attenuation[I], Expected);
			   for (auto* Controller : GetControllers(*Defaults))
				   TestEqual(TEXT("Omitted controller default"), Controller->Attenuation, Expected);
			   if (auto* Ball = Cast<UAGX_BallConstraintComponent>(Defaults); Ball != nullptr)
				   TestEqual(
					   TEXT("Omitted twist default"), Ball->TwistRangeController.Attenuation.Value,
					   Expected);
		   }
	   });

	It("keeps legacy function units and stiffness aliases",
	   [this]()
	   {
		   UAGX_Simulation* Simulation = GetMutableDefault<UAGX_Simulation>();
		   TGuardValue<double> TimeStepGuard(Simulation->TimeStep, 0.02);
		   auto* Hinge = NewObject<UAGX_HingeConstraintComponent>();
		   Hinge->SetSpookDamping(EGenericDofIndex::Translational1, 0.08);
		   TestEqual(
			   TEXT("Legacy constraint setter"),
			   Hinge->GetAttenuation(EGenericDofIndex::Translational1), 4.0);
		   TestEqual(
			   TEXT("Legacy constraint getter"),
			   Hinge->GetSpookDamping(EGenericDofIndex::Translational1), 0.08);
		   Hinge->RangeController.SetSpookDamping(0.1);
		   TestEqual(
			   TEXT("Legacy controller setter"), Hinge->RangeController.GetAttenuation(), 5.0);
		   TestEqual(
			   TEXT("Legacy controller getter"), Hinge->RangeController.GetSpookDamping(), 0.1);

		   FAGX_ElementaryConstraint Elementary;
		   Elementary.SetSpookDamping(0.12);
		   TestEqual(TEXT("Legacy elementary setter"), Elementary.GetAttenuation(), 6.0);
		   TestEqual(TEXT("Legacy elementary getter"), Elementary.GetSpookDamping(), 0.12);
		   Elementary.SetElasticity(500.0);
		   TestEqual(TEXT("Stiffness alias"), Elementary.GetStiffness(), 500.0);
		   TestEqual(TEXT("Stiffness to compliance"), Elementary.GetCompliance(), 0.002);
		   Hinge->RangeController.SetElasticity(250.0);
		   TestEqual(
			   TEXT("Controller stiffness alias"), Hinge->RangeController.GetStiffness(), 250.0);
		   Hinge->SetStiffness(EGenericDofIndex::Translational1, 125.0);
		   TestEqual(
			   TEXT("Constraint elasticity alias"),
			   Hinge->GetElasticity(EGenericDofIndex::Translational1), 125.0);
	   });
}
