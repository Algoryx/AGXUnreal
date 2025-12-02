// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Quat.h"
#include "Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(
	FAGX_TransformSpec, "AGXUnreal.Spec.Transform", AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_TransformSpec)

void FAGX_TransformSpec::Define()
{
	Describe(
		"When creating relative transform",
		[this]()
		{
			It("should produce the same world position.",
			   [this]()
			   {
				   // Test that ensures that GetRelativeTransform does what we expect it to,
				   // for use with Wire Routing Node Frame updates.
				   const FVector TargetInOwner {5.0, 10.0, 15.0};

				   const FTransform Owner = []() -> FTransform
				   {
					   const FVector Offset {1.0, 2.0, 3.0};
					   const FVector Axis = FVector(0.4, 0.3, 0.2).GetSafeNormal();
					   const double Angle {0.54};
					   const FQuat Rotation {Axis, Angle};
					   return {Rotation, Offset};
				   }();

				   const FTransform Other = []() -> FTransform
				   {
					   const FVector Offset {3.0, 2.0, 1.0};
					   const FVector Axis = FVector(0.2, 0.3, 0.4).GetSafeNormal();
					   const double Angle {1.54};
					   const FQuat Rotation {Axis, Angle};
					   return {Rotation, Offset};
				   }();

				   const FTransform OwnerToOther = Owner.GetRelativeTransform(Other);
				   const FVector TargetInOther = OwnerToOther.TransformPosition(TargetInOwner);

				   const FVector TargetAccordingToOwner = Owner.TransformPosition(TargetInOwner);
				   const FVector TargetAccordingToOther = Other.TransformPosition(TargetInOther);

				   TestEqual(
					   TEXT("World location"), TargetAccordingToOwner, TargetAccordingToOther);
			   });
		});
}
