// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_Simulation.h"
#include "AgxAutomationCommon.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"
#include "Vehicle/AGX_VehicleTypes.h"
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

BEGIN_DEFINE_SPEC(
	FAGX_TrackInternalMergePropertiesSpec, "AGXUnreal.Spec.TrackInternalMergeProperties",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_TrackInternalMergePropertiesSpec)

void FAGX_TrackInternalMergePropertiesSpec::Define()
{
	It("matches the native attenuation default and copies attenuation",
	   [this]()
	   {
		   FTrackBarrier Native;
		   Native.AllocateNative(120, 35.0f, 2.5f, FAGX_TrackInitialTension());
		   TestEqual(
			   TEXT("Native attenuation default"),
			   GetDefault<UAGX_TrackInternalMergeProperties>()
				   ->LockToReachMergeConditionAttenuation.Value,
			   Native.InternalMergeProperties_GetLockToReachMergeConditionAttenuation());

		   Native.InternalMergeProperties_SetLockToReachMergeConditionAttenuation(8.0);
		   TestEqual(
			   TEXT("Native attenuation setter"),
			   Native.InternalMergeProperties_GetLockToReachMergeConditionAttenuation(), 8.0);
		   auto* Properties = NewObject<UAGX_TrackInternalMergeProperties>();
		   Properties->CopyFrom(Native);
		   TestEqual(
			   TEXT("Copy from native"), Properties->GetLockToReachMergeConditionAttenuation(),
			   8.0);
		   auto* Copy = NewObject<UAGX_TrackInternalMergeProperties>();
		   Copy->CopyFrom(*Properties);
		   TestEqual(
			   TEXT("Copy from properties"), Copy->GetLockToReachMergeConditionAttenuation(), 8.0);
	   });

	It("keeps legacy double and float functions in seconds",
	   [this]()
	   {
		   UAGX_Simulation* Simulation = GetMutableDefault<UAGX_Simulation>();
		   TGuardValue<double> TimeStepGuard(Simulation->TimeStep, 0.02);
		   auto* Properties = NewObject<UAGX_TrackInternalMergeProperties>();
		   Properties->SetLockToReachMergeConditionSpookDamping(0.08);
		   TestEqual(
			   TEXT("Double setter conversion"),
			   Properties->GetLockToReachMergeConditionAttenuation(), 4.0);
		   TestEqual(
			   TEXT("Double getter conversion"),
			   Properties->GetLockToReachMergeConditionSpookDamping(), 0.08);

		   Properties->SetLockToReachMergeConditionSpookDamping_BP(0.1f);
		   TestEqual(
			   TEXT("Float setter conversion"),
			   Properties->GetLockToReachMergeConditionAttenuation(), 5.0);
		   TestEqual(
			   TEXT("Float getter conversion"),
			   Properties->GetLockToReachMergeConditionSpookDamping_BP(), 0.1f);
		   Properties->SetLockToReachMergeConditionAttenuation(6.0);
		   TestEqual(
			   TEXT("New setter uses steps"),
			   Properties->GetLockToReachMergeConditionSpookDamping(), 0.12);
	   });

	It("retains the original deprecated property type and default",
	   [this]()
	   {
		   // An independent historical expectation, not a value derived from today's defaults.
		   FStructProperty* LegacyProperty = FindFProperty<FStructProperty>(
			   UAGX_TrackInternalMergeProperties::StaticClass(),
			   TEXT("LockToReachMergeConditionSpookDamping"));
		   if (!TestNotNull(TEXT("Legacy property"), LegacyProperty))
			   return;
		   TestTrue(
			   TEXT("Deprecated property flag"),
			   LegacyProperty->HasAnyPropertyFlags(CPF_Deprecated));
		   if (!TestTrue(
				   TEXT("Original property type"),
				   LegacyProperty->Struct == FAGX_Real::StaticStruct()))
			   return;
		   auto* Properties = NewObject<UAGX_TrackInternalMergeProperties>();
		   const FAGX_Real* LegacyValue =
			   LegacyProperty->ContainerPtrToValuePtr<FAGX_Real>(Properties);
		   TestEqual(TEXT("Historical default in seconds"), LegacyValue->Value, 0.05);
	   });
}
