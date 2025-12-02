// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "RigidBodyBarrier.h"
#include "AgxAutomationCommon.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

BEGIN_DEFINE_SPEC(
	FRigidBodyBarrierSpec, "AGXUnreal.Barrier.RigidBody", AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FRigidBodyBarrierSpec)

void FRigidBodyBarrierSpec::Define()
{
	Describe(
		"Allocating and releasing native",
		[this]()
		{
			It("should be created without native",
			   [this]()
			   {
				   FRigidBodyBarrier RigidBody;
				   TestFalse("Barrier should be created without native.", RigidBody.HasNative());
			   });

			It("should have a native after allocation",
			   [this]()
			   {
				   FRigidBodyBarrier RigidBody;
				   RigidBody.AllocateNative();
				   TestTrue("Allocation should give the barrier a native.", RigidBody.HasNative());
				   BAIL_TEST_IF(!RigidBody.HasNative(), )
				   TestNotNull(
					   "Allocation should give the barrier a non-null native",
					   RigidBody.GetNative());
			   });

			It("should not have a native after release",
			   [this]()
			   {
				   FRigidBodyBarrier RigidBody;
				   RigidBody.AllocateNative();
				   TestTrue("Allocation should give the barrier a native.", RigidBody.HasNative());
				   BAIL_TEST_IF(!RigidBody.HasNative(), )
				   RigidBody.ReleaseNative();
				   TestFalse(
					   "Release should remove the native from the barrier.", RigidBody.HasNative());
			   });
		});

	Describe(
		"Copying",
		[this]()
		{
			It("should copy the native between barriers",
			   [this]()
			   {
				   FRigidBodyBarrier Source;
				   Source.AllocateNative();
				   TestTrue("Allocation should give teh barrier a native.", Source.HasNative());
				   BAIL_TEST_IF(!Source.HasNative(), );
				   FRigidBodyRef* Native = Source.GetNative();
				   FRigidBodyBarrier Destination(std::move(Source));
				   TestFalse("Move should clear source", Source.HasNative());
				   TestTrue("Move should set destination", Destination.HasNative());
				   BAIL_TEST_IF(!Destination.HasNative(), );
				   TestEqual(
					   "Move should preserve native pointer", Destination.GetNative(), Native);
			   });
		});

	Describe(
		"Setting and getting properties",
		[this]()
		{
			It("should return the same value that was set to a property",
			   [this]()
			   {
				   // All hard-coded numbers in this 'It' are arbitrarily selected. We just want the
				   // same numbers back again.
				   //
				   // We test what looks like trivial set/get methods because there is conversion
				   // happening between the Unreal Engine types and units and the AGX Dynamics types
				   // and units.

				   FRigidBodyBarrier RigidBody;
				   RigidBody.AllocateNative();
				   TestTrue("Allocation should give the barrier a native.", RigidBody.HasNative());
				   BAIL_TEST_IF(!RigidBody.HasNative(), )

				   {
					   const FVector ExpectedPosition(1.0f, 2.0f, 3.0f);
					   RigidBody.SetPosition(ExpectedPosition);
					   const FVector ActualPosition = RigidBody.GetPosition();
					   TestEqual("Position", ActualPosition, ExpectedPosition);
				   }
				   {
					   const FQuat ExpectedRotation(
						   FVector(2.0f, 4.0f, 3.0f).GetSafeNormal(), 4.0f);
					   RigidBody.SetRotation(ExpectedRotation);
					   const FQuat ActualRotation = RigidBody.GetRotation();
					   AgxAutomationCommon::TestEqual(
						   *this, TEXT("Rotation"), ActualRotation, ExpectedRotation);
				   }
				   {
					   const FVector ExpectedVelocity(10.0f, 20.0f, 30.0f);
					   RigidBody.SetVelocity(ExpectedVelocity);
					   const FVector ActualVelocity = RigidBody.GetVelocity();
					   TestEqual("Velocity", ActualVelocity, ExpectedVelocity);
				   }
				   {
					   const FVector ExpectedAngularVelocity(11.0f, 21.0f, 31.0f);
					   RigidBody.SetAngularVelocity(ExpectedAngularVelocity);
					   const FVector ActualAngularVelocity = RigidBody.GetAngularVelocity();
					   TestEqual(
						   "Angular velocity", ActualAngularVelocity, ExpectedAngularVelocity);
				   }
				   {
					   const float ExpectedMass(3.0f);
					   RigidBody.GetMassProperties().SetMass(ExpectedMass);
					   const float ActualMass = RigidBody.GetMassProperties().GetMass();
					   TestEqual("Mass", ActualMass, ExpectedMass);
				   }
				   {
					   const FString ExpectedName("MyRigidBody");
					   RigidBody.SetName(ExpectedName);
					   const FString ActualName = RigidBody.GetName();
					   TestEqual("Name", ActualName, ExpectedName);
				   }
				   {
					   const EAGX_MotionControl ExpectedMotionControl =
						   EAGX_MotionControl::MC_KINEMATICS;
					   RigidBody.SetMotionControl(ExpectedMotionControl);
					   const EAGX_MotionControl ActualMotionControl = RigidBody.GetMotionControl();
					   TestEqual("MotionControl", ActualMotionControl, ExpectedMotionControl);
				   }
			   });
		});
}
