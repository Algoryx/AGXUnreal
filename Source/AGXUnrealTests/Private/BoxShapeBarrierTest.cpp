// Copyright 2025, Algoryx Simulation AB.

// This test is disabled because including it leads to
//    Warning: dlopen failed: libUE4Editor-AGXUnrealBarrier.so: undefined symbol: "typeinfo for
//    FAutomationTestBase"
#if 0
// AGX Dynamics for Unreal includes.
#include "Shapes/BoxShapeBarrier.h"

// Unreal Engine includes.
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
  FBoxShapeBarrierTest,
  "AGXUnreal.Barrier.Box",
  EAutomationTestFlags::EditorContext | EAutomationTestFlags::CriticalPriority | EAutomationTestFlags::ProductFilter)

bool FBoxShapeBarrierTest::RunTest(const FString& Parameters)
{
  return true;
}
#endif
