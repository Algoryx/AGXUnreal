// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AgxAutomationCommon.h"
#include "Utilities/AGX_VersionComparison.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"

BEGIN_DEFINE_SPEC(
	FAGX_VersionMacrosSpec, "AGXUnreal.Spec.VersionMacros", AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_VersionMacrosSpec)

void FAGX_VersionMacrosSpec::Define()
{
	// Test OLDER_THAN macros with low version numbers.
	Describe(
		"When checking for older than low version",
		[this]()
		{
			It("should evaluate OLDER_THAN to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN(1, 1, 1)
					   true;
#else
					false;
#endif
				   TestFalse("Older than low version", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL(1, 1, 1)
					   true;
#else
					false;
#endif
				   TestFalse("Older than or equal low version", bEvaluated);
			   });
		});

	// Test OLDER_THAN macros with high version numbers.
	Describe(
		"When checking for older than high version",
		[this]()
		{
			It("should evaluate OLDER_THAN to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN(999, 1, 1)
					   true;
#else
					false;
#endif
				   TestTrue("Older than high version", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL(999, 1, 1)
					   true;
#else
					false;
#endif
				   TestTrue("Older than or equal high version", bEvaluated);
			   });
		});

	// Test NEWER_THAN macros with low version numbers.
	Describe(
		"When checking for newer than low version",
		[this]()
		{
			It("should evaluate NEWER_THAN to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN(1, 1, 1)
					   true;
#else
					false;
#endif
				   TestTrue("Newer than low version", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL(1, 1, 1)
					   true;
#else
					false;
#endif
				   TestTrue("Newer than small or equal version", bEvaluated);
			   });
		});

	// Test NEWER_THAN macros with high version numbers.
	Describe(
		"When checking for newer than high version",
		[this]()
		{
			It("should evaluate NEWER_THAN to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN(999, 1, 1)
					   true;
#else
					false;
#endif
				   TestFalse("Newer than high version", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL(999, 1, 1)
					   true;
#else
					false;
#endif
				   TestFalse("Newer than ore equal large version", bEvaluated);
			   });
		});

	// Test with same version.
	Describe(
		"When same version",
		[this]()
		{
			It("should evaluate NEWER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN( \
	AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
					false;
#endif
				   TestFalse("Equal version", bEvaluated);
			   });

			It("should evaluate OLDER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN( \
	AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
					false;
#endif
				   TestFalse("Equal version", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL( \
	AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
					false;
#endif
				   TestTrue("Equal version", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL( \
	AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
		false;
#endif
				   TestTrue("Equal version", bEvaluated);
			   });
		});

	// Test with one higher major.
	Describe(
		"When one higher major",
		[this]()
		{
			It("should evaluate NEWER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN( \
	AGXUNREAL_MAJOR_VERSION + 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Major larger", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION + 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Major larger", bEvaluated);
			   });

			It("should evaluate OLDER_THAN macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN( \
AGXUNREAL_MAJOR_VERSION + 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Major larger", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION + 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Major larger", bEvaluated);
			   });
		});

	// Test with one higher minor.
	Describe(
		"When one higher minor",
		[this]()
		{
			It("should evaluate NEWER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION + 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Minor larger", bEvaluated);
			   });

			It("should evaluate OLDER_THAN macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION + 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Minor larger", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION + 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Minor larger", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION + 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Minor larger", bEvaluated);
			   });
		});

	// Test with one higher patch.
	Describe(
		"When one higher patch",
		[this]()
		{
			It("should evaluate NEWER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION + 1)
					   true;
#else
false;
#endif
				   TestFalse("Patch larger", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION + 1)
					   true;
#else
false;
#endif
				   TestFalse("Patch larger", bEvaluated);
			   });

			It("should evaluate OLDER_THAN macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION + 1)
					   true;
#else
false;
#endif
				   TestTrue("Patch larger", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION + 1)
					   true;
#else
false;
#endif
				   TestTrue("Patch larger", bEvaluated);
			   });
		});

	// Test with one lower major.
	Describe(
		"When one lower major",
		[this]()
		{
			It("should evaluate NEWER_THAN macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN( \
AGXUNREAL_MAJOR_VERSION - 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Major lower", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION - 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Major lower", bEvaluated);
			   });

			It("should evaluate OLDER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN( \
AGXUNREAL_MAJOR_VERSION - 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Major larger", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION - 1, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Major larger", bEvaluated);
			   });
		});


	// Test with one lower minor.
	Describe(
		"When one lower minor",
		[this]()
		{
			It("should evaluate NEWER_THAN macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION - 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Minor lower", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION - 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestTrue("Minor lower", bEvaluated);
			   });

			It("should evaluate OLDER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION - 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Minor lower", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION - 1, AGXUNREAL_PATCH_VERSION)
					   true;
#else
false;
#endif
				   TestFalse("Minor lower", bEvaluated);
			   });
		});

	// Test with one lower patch.
	Describe(
		"When one lower patch",
		[this]()
		{
			It("should evaluate NEWER_THAN macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION - 1)
					   true;
#else
false;
#endif
				   TestTrue("Patch lower", bEvaluated);
			   });

			It("should evaluate NEWER_THAN_OR_EQUAL macro to true",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_NEWER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION - 1)
					   true;
#else
false;
#endif
				   TestTrue("Patch lower", bEvaluated);
			   });

			It("should evaluate OLDER_THAN macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION - 1)
					   true;
#else
false;
#endif
				   TestFalse("Patch lower", bEvaluated);
			   });

			It("should evaluate OLDER_THAN_OR_EQUAL macro to false",
			   [this]()
			   {
				   bool bEvaluated =
#if AGXUNREAL_VERSION_OLDER_THAN_OR_EQUAL( \
AGXUNREAL_MAJOR_VERSION, AGXUNREAL_MINOR_VERSION, AGXUNREAL_PATCH_VERSION - 1)
					   true;
#else
false;
#endif
				   TestFalse("Patch lower", bEvaluated);
			   });
		});
}
