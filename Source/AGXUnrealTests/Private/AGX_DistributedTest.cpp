// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/TestUtilities.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedProximityTest, "AGXUnreal.Editor.Distributed.Proximity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedProximityTest::RunTest(const FString& Parameters)
{
	FString Error;
	if (!FTestUtilities::TestDistributedProximity(Error))
	{
		AddError(Error);
		return false;
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedProximityDelayedSeparationTest,
	"AGXUnreal.Editor.Distributed.ProximityDelayedSeparation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedProximityDelayedSeparationTest::RunTest(const FString& Parameters)
{
	FString Error;
	if (!FTestUtilities::TestDistributedProximityDelayedSeparation(Error))
	{
		AddError(Error);
		return false;
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedTypedSignalsTest, "AGXUnreal.Editor.Distributed.TypedSignals",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedTypedSignalsTest::RunTest(const FString& Parameters)
{
	FString Error;
	if (!FTestUtilities::TestDistributedSignals(Error))
	{
		AddError(Error);
		return false;
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedEntityStateTest, "AGXUnreal.Editor.Distributed.EntityStateAndAuthority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedEntityStateTest::RunTest(const FString& Parameters)
{
	FString Error;
	if (!FTestUtilities::TestDistributedEntityState(Error))
	{
		AddError(Error);
		return false;
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedControlMessagesTest,
	"AGXUnreal.Editor.Distributed.ControlMessagesWithoutPhysicsStep",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedControlMessagesTest::RunTest(const FString& Parameters)
{
	FString Error;
	if (!FTestUtilities::TestDistributedControlMessages(Error))
	{
		AddError(Error);
		return false;
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedBodyTransformDecodeTest, "AGXUnreal.Editor.Distributed.BodyTransformDecode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedBodyTransformDecodeTest::RunTest(const FString& Parameters)
{
	// Explicit wire values rather than a round-trip through a matching serializer.
	TArray<uint8> Data;
	Data.Init(0, 32 + 2 * 38);
	Data[7] = 2;
	auto SetDouble = [&Data](int32 Offset, uint64 Bits)
	{
		for (int32 Byte = 0; Byte < 8; ++Byte)
			Data[Offset + Byte] = static_cast<uint8>(Bits >> (56 - Byte * 8));
	};
	auto SetInt16 = [&Data](int32 Offset, uint16 Bits)
	{
		Data[Offset] = static_cast<uint8>(Bits >> 8);
		Data[Offset + 1] = static_cast<uint8>(Bits);
	};
	// Reference position (1, 2, 3) metres; first body has zero deltas and a
	// quantized 90-degree Z quaternion (0, 0, 23170, 23170) / 32767.
	SetDouble(8, 0x3ff0000000000000ULL);
	SetDouble(16, 0x4000000000000000ULL);
	SetDouble(24, 0x4008000000000000ULL);
	SetInt16(42, 0x5a82);
	SetInt16(44, 0x5a82);
	// Second body: deltas (-1, 0.5, 32.767) metres and identity rotation.
	SetInt16(70, 0xfc18);
	SetInt16(72, 0x01f4);
	SetInt16(74, 0x7fff);
	SetInt16(82, 0x7fff);

	TArray<FTransform> Transforms;
	TestTrue(
		TEXT("Valid example wire payload"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	TestEqual(TEXT("Body count"), Transforms.Num(), 2);
	if (Transforms.Num() == 2)
	{
		TestTrue(
			TEXT("Meters to centimeters and Y handedness"),
			Transforms[0].GetTranslation().Equals(FVector(100, -200, 300)));
		TestTrue(
			TEXT("Rotation handedness"), Transforms[0]
											 .GetRotation()
											 .RotateVector(FVector::ForwardVector)
											 .Equals(FVector(0, -1, 0), 1.e-6));
		TestTrue(
			TEXT("Signed millimetre deltas"),
			Transforms[1].GetTranslation().Equals(FVector(0, -250, 3576.7), 1.e-6));
	}
	const TArray<uint8> ValidData = Data;
	// Full-precision fallback on the second body: absolute position (64, 2, 3) metres.
	SetInt16(70, 0x8000);
	SetInt16(72, 0x8000);
	SetInt16(74, 0x8000);
	Data.InsertZeroed(76, 24);
	SetDouble(76, 0x4050000000000000ULL);
	SetDouble(84, 0x4000000000000000ULL);
	SetDouble(92, 0x4008000000000000ULL);
	TestTrue(
		TEXT("Escaped absolute position accepted"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	if (Transforms.Num() == 2)
		TestTrue(
			TEXT("Escaped position is absolute"),
			Transforms[1].GetTranslation().Equals(FVector(6400, -200, 300)));
	const TArray<uint8> EscapedData = Data;
	SetDouble(76, 0x7ff0000000000000ULL);
	TestFalse(
		TEXT("Non-finite escaped position rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	for (int32 Size = 0; Size < EscapedData.Num(); ++Size)
	{
		Data = EscapedData;
		Data.SetNum(Size);
		TestFalse(
			FString::Printf(TEXT("Truncated escaped payload rejected at byte %d"), Size),
			FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	}
	// Reject both previous matrix layouts.
	TArray<uint8> OldData;
	OldData.Init(0, 8 + 120);
	OldData[7] = 1;
	TestFalse(
		TEXT("Old 12-double protocol rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(OldData, Transforms));
	OldData.Init(0, 8 + 128);
	OldData[7] = 1;
	TestFalse(
		TEXT("Old 16-double protocol rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(OldData, Transforms));
	// A float infinity in the linear velocity must also be rejected.
	Data = ValidData;
	Data[46] = 0x7f;
	Data[47] = 0x80;
	TestFalse(
		TEXT("Non-finite velocity rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data = ValidData;
	SetDouble(8, 0x7ff0000000000000ULL);
	TestFalse(
		TEXT("Non-finite reference position rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data = ValidData;
	SetInt16(32, 0x8000);
	TestFalse(
		TEXT("Partial position escape rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data = ValidData;
	SetInt16(42, 0);
	SetInt16(44, 0);
	TestFalse(
		TEXT("Zero quaternion rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data = ValidData;
	Data[7] = 3;
	TestFalse(
		TEXT("Count mismatch rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data = ValidData;
	Data.RemoveAt(Data.Num() - 1);
	TestFalse(
		TEXT("Truncated body rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data = ValidData;
	Data.Add(0);
	TestFalse(
		TEXT("Trailing bytes rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data.Init(0xff, 32);
	TestFalse(
		TEXT("Oversized count rejected without allocation"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data.Init(0, 32);
	TestTrue(
		TEXT("Empty assembly accepted"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	TestEqual(TEXT("Empty assembly has no transforms"), Transforms.Num(), 0);
	Transforms.Reset();
	Data.Reset();
	TestFalse(
		TEXT("Missing header rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	return true;
}
#endif
