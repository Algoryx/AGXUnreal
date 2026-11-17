// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/TestUtilities.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
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
	Data.Init(0, 8 + 120);
	Data[7] = 1;
	auto SetValue = [&Data](int32 Row, int32 Column, uint64 Bits)
	{
		const int32 Offset = 8 + (Row * 3 + Column) * 8;
		for (int32 Byte = 0; Byte < 8; ++Byte)
			Data[Offset + Byte] = static_cast<uint8_t>(Bits >> (56 - Byte * 8));
	};
	// AGX row-vector matrix: 90 degrees about Z and translation (1, 2, 3) meters.
	SetValue(0, 1, 0x3ff0000000000000ULL); // 1
	SetValue(1, 0, 0xbff0000000000000ULL); // -1
	SetValue(2, 2, 0x3ff0000000000000ULL);
	SetValue(3, 0, 0x3ff0000000000000ULL);
	SetValue(3, 1, 0x4000000000000000ULL); // 2
	SetValue(3, 2, 0x4008000000000000ULL); // 3

	TArray<FTransform> Transforms;
	TestTrue(
		TEXT("Valid example wire payload"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	TestEqual(TEXT("Body count"), Transforms.Num(), 1);
	if (Transforms.Num() == 1)
	{
		TestTrue(
			TEXT("Meters to centimeters and Y handedness"),
			Transforms[0].GetTranslation().Equals(FVector(100, -200, 300)));
		TestTrue(
			TEXT("Rotation handedness"), Transforms[0]
											 .GetRotation()
											 .RotateVector(FVector::ForwardVector)
											 .Equals(FVector(0, -1, 0), 1.e-6));
	}
	// Current protocol must reject the old 16-double, transform-only body representation.
	TArray<uint8> OldData;
	OldData.Init(0, 8 + 128);
	OldData[7] = 1;
	TestFalse(
		TEXT("Old protocol rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(OldData, Transforms));
	// A float infinity in the linear velocity must also be rejected.
	Data[8 + 96] = 0x7f;
	Data[8 + 97] = 0x80;
	TestFalse(
		TEXT("Non-finite velocity rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Data[8 + 96] = 0;
	Data[8 + 97] = 0;
	Transforms.Reset();
	SetValue(3, 0, 0x7ff0000000000000ULL); // infinity
	TestFalse(
		TEXT("Non-finite matrix rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Transforms.Reset();
	SetValue(3, 0, 0x3ff0000000000000ULL);
	Data[7] = 2;
	TestFalse(
		TEXT("Count mismatch rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Transforms.Reset();
	Data[7] = 1;
	Data.RemoveAt(Data.Num() - 1);
	TestFalse(
		TEXT("Truncated matrix rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Transforms.Reset();
	Data.Init(0xff, 8);
	TestFalse(
		TEXT("Oversized count rejected without allocation"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	Transforms.Reset();
	Data.Reset();
	TestFalse(
		TEXT("Missing header rejected"),
		FTestUtilities::DecodeDistributedBodyTransforms(Data, Transforms));
	return true;
}
#endif
