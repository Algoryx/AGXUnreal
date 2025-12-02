// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "AGX_RigidBodyComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"

// Unreal Engine includes.
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Misc/AutomationTest.h"
#include "Misc/EngineVersionComparison.h"

class UBlueprint;
class USCS_Node;
class UWorld;

struct FLinearColor;

/**
 * A set of helper functions used by our Automation tests.
 */
namespace AgxAutomationCommon
{
	/**
	 * Get a pointer to the best guess for which world is the test world.
	 *
	 * There is some heuristics involved. Returns the world found in the first WorldContext that is
	 * either a play-in-editor context or a game context. For there to be such a world the tests
	 * must either be run from within the editor or from and command line with the parameter '-Game'
	 * passed to UE4Editor.
	 *
	 * I don't know how cooking/packaging/exporting the project affects this.
	 *
	 * @return The UWorld used for the Automation tests, or nullptr if no suitable world is found.
	 */
	UWorld* GetTestWorld();

#if UE_VERSION_OLDER_THAN(5, 5, 0)
	constexpr EAutomationTestFlags::Type DefaultTestFlags =
		static_cast<const EAutomationTestFlags::Type>(
			EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext |
			EAutomationTestFlags::ClientContext);

	constexpr EAutomationTestFlags::Type ETF_ApplicationContextMask =
		static_cast<const EAutomationTestFlags::Type>(EAutomationTestFlags::ApplicationContextMask);
#else
	constexpr EAutomationTestFlags DefaultTestFlags = EAutomationTestFlags::ProductFilter |
													  EAutomationTestFlags::EditorContext |
													  EAutomationTestFlags::ClientContext;

	constexpr EAutomationTestFlags ETF_ApplicationContextMask =
		EAutomationTestFlags_ApplicationContextMask;
#endif

	/// @todo Remove this TestEqual implementation for double once Unreal Engine's get support
	/// for infinity.
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, double Actual, double Expected,
		double Tolerance = UE_KINDA_SMALL_NUMBER);

	/// @todo Remove this TestEqual implementation for FVector4 once it's included in-engine.
	/// @see Misc/AutomationTest.h
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FVector4& Actual,
		const FVector4& Expected, float Tolerance = UE_KINDA_SMALL_NUMBER);

	/// @todo Remove this TestEqual implementation for FQuat once it's included in-engine.
	/// @see Misc/AutomationTest.h
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FQuat& Actual, const FQuat& Expected,
		float Tolerance = UE_KINDA_SMALL_NUMBER);

	/// @todo Remove this TestEqual implementation for FRotator once it's included in-engine.
	/// @see Misc/AutomationTest.h
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FRotator& Actual,
		const FRotator& Expected, float Tolerance = UE_KINDA_SMALL_NUMBER);

	/// @todo Remove this TestEqual implementation for FLinearColor once it's included in-engine.
	/// @see Misc/AutomaitonTest.h
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FLinearColor& Actual,
		const FLinearColor& Expected, float Tolerance = UE_KINDA_SMALL_NUMBER);

	bool TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FAGX_RealInterval& Actual,
		const FAGX_RealInterval& Expected, double Tolerance = UE_KINDA_SMALL_NUMBER);

	void TestLess(
		FAutomationTestBase& Test, const TCHAR* SmallerName, double Smaller,
		const TCHAR* LargerName, double Larger);

	void TestAllLess(
		FAutomationTestBase& Test, const TCHAR* SmallerName, const FVector& Smaller,
		const TCHAR* LargerName, const FVector& Larger);

	/// @todo Figure out how to use UEnum::GetValueAsString instead of this helper function.
	/// I get linker errors.
	FString WorldTypeToString(EWorldType::Type Type);

	// Not 'enum class' because I want implicit conversion to bool, with 'NoReason' being false.
	// We can do 'if (Reason) { <Cannot do world tests.> }'.
	enum NoWorldTestsReason
	{
		NoReason = 0, // It is safe to run world tests.
		TooManyWorlds,
		IllegalWorldType
	};

	FString GetNoWorldTestsReasonText(NoWorldTestsReason Reason);

	/**
	 * Perform the same checks as FLoadGameMapCommand in AutomationCommon, but return a reason on
	 * failure instead of crashing.
	 *
	 * @return The reason why world tests can't be run, or NoReason if world tests can be run.
	 */
	NoWorldTestsReason CanRunWorldTests();

	/**
	 * Get the file system path to an AGX Dynamics archive or URDF file intended for Automation
	 * testing.
	 * @param SceneName The name of the AGX Dynamics archive or URDF file to find.
	 * @return File system path to the test scene.
	 */
	FString GetTestScenePath(const TCHAR* SceneName);

	/**
	 * Get the file system path to an AGX Dynamics archive or URDF file intended for Automation
	 * testing.
	 * @param ArchiveName The name of the AGX Dynamics archive or UDF file to find.
	 * @return File system path to the test scene.
	 */
	FString GetTestScenePath(const FString& SceneName);

	/**
	 * Get the absolute path to a directory within the TestScenes directory.
	 * @param SubDir Optional sub directory. Leave empty to get the Test Scene directory.
	 * @return Absolute path to a directory within the TestScenes directory.
	 */
	FString GetTestSceneDirPath(const FString& SubDir = "");

	/**
	 * Compare the MD5 checksum of the .umap file proving drive storage for the map at the given
	 * path with the given expected MD5 checksum. Reports a test failure on mismatch.
	 *
	 * The expected MD5 checksum is typically found by running system 'md5sum' on the .umap file
	 * at the time of map creation.
	 *
	 * \param MapPath Package path to a Map, such as /Game/Tests/MyMap.
	 * \param Expected Expected MD5 checksum.
	 * \param Test The currently running FAutomationTestBase, used for test failure reporting.
	 * \return True if the MD5 checksums match, false if they differ.
	 */
	bool CheckMapMD5Checksum(
		const FString& MapPath, const TCHAR* Expected, FAutomationTestBase& Test);

	/**
	 * Compare the MD5 checksum of the .uasset file proving drive storage for the asset at the given
	 * path with the given expected MD5 checksum. Reports a test failure on mismatch.
	 *
	 * The expected MD5 checksum is typically found by running system 'md5sum' on the .uasset file
	 * at the time of asset creation.
	 *
	 * \param MapPath Package path to an Asset, such as /Game/Tests/MyAsset.
	 * \param Expected Expected MD5 checksum.
	 * \param Test The currently running FAutomationTestBase, used for test failure reporting.
	 * \return True if the MD5 checksums match, false if they differ.
	 */
	bool CheckAssetMD5Checksum(
		const FString& PackagePath, const TCHAR* Expected, FAutomationTestBase& Test);

	/**
	 * Compare the MD5 checksum of the given file with the given expected MD5 checksum. Report a
	 * test failure on mismatch.
	 *
	 * @param FilePath File system path to the file to check.
	 * @param Expected MD5 hash that the file should have.
	 * @param Test The currently running test, used for failure reporting.
	 * \return True if the MD5 checksums match, false if they differ.
	 */
	bool CheckFileMD5Checksum(
		const FString& FilePath, const TCHAR* Expected, FAutomationTestBase& Test);

	/**
	 * Get the MD5 checksum for the file at the given path. Returns empty string if file load fails.
	 *
	 * @param FilePath Path to file to generate checksum for.
	 * @return The MD5 checksum for the given file.
	 */
	FString GetFileMD5Checksum(const FString& FilePath);

	/**
	 * Delete all assets created when the given archive was imported.
	 *
	 * Will do a file system delete of the entire import directory.
	 *
	 * WARNING: The implementation currently assumes that the import was does without name conflict
	 * with a previous import of an archive with the same name. If there are several imports then
	 * the one that did not get a directory name suffix is deleted.
	 *
	 * This will result in an error being printed to the log, which will cause the current test to
	 * fail. Prevent this on Linux by adding
	 *     Test.AddExpectedError(TEXT("inotify_rm_watch cannot remove descriptor"));
	 * to the test. Additional AddExpectedError may be required for other platforms.
	 *
	 * @param ArchiveName The name of the archive whose imported assets are to be deleted, without
	 * '.agx' suffix.
	 * @param ExpectedFileAndDirectoryNames List of file and directory names that is expected to
	 * be found in the archive. No delete will be performed if any file not in this list is found in
	 * the directory.
	 * @return True if the directory was deleted. False otherwise.
	 */
	bool DeleteImportDirectory(
		const TCHAR* ArchiveName, const TArray<const TCHAR*>& ExpectedFileAndDirectoryNames);

	/**
	 * Get the name (including file extension) of Static Mesh Assets referenced by Trimesh
	 * Components or Static Mesh Components.
	 * @param Components The Components to look through for referenced Static Mesh Assets.
	 * @return Array of (unique) referenced Static Mesh Asset names.
	 */
	TArray<FString> GetReferencedStaticMeshAssets(const TArray<UActorComponent*>& Components);

	USCS_Node* GetNodeChecked(const UBlueprint& Blueprint, const FString& Name);
	USCS_Node* GetOnlyAttachChildChecked(USCS_Node* Node);

	template <typename T>
	T* GetByName(TArray<UActorComponent*>& Components, const TCHAR* Name)
	{
		UActorComponent** Match = Components.FindByPredicate(
			[Name](UActorComponent* Component)
			{ return Cast<T>(Component) && Component->GetName() == Name; });

		return Match != nullptr ? Cast<T>(*Match) : nullptr;
	}

	template <typename T>
	void GetByName(TArray<UActorComponent*>& Components, const TCHAR* Name, T*& Out)
	{
		Out = GetByName<T>(Components, Name);
	}

	template <typename T>
	T* GetComponentByName(const AActor& Owner, const FName& Name)
	{
		for (UActorComponent* Component : Owner.GetComponents())
		{
			if (Component->GetFName() == Name)
			{
				// Component names are unique within an Actor so no point in continuing the search
				// if this Component has the wrong type.
				return Cast<T>(Component);
			}
		}

		return nullptr;
	}

	inline UAGX_RigidBodyComponent* GetRigidBodyByName(const AActor& Owner, FName Name)
	{
		return GetComponentByName<UAGX_RigidBodyComponent>(Owner, Name);
	}

	inline UAGX_BallConstraintComponent* GetBallConstraintByName(const AActor& Owner, FName Name)
	{
		return GetComponentByName<UAGX_BallConstraintComponent>(Owner, Name);
	}

	inline bool IsAnyNullptr()
	{
		// Base case. The elements are or'd so false won't make the entire expression false.
		return false;
	}

	template <typename T, typename... Ts>
	bool IsAnyNullptr(T* Head, Ts*... Tail)
	{
		return Head == nullptr || IsAnyNullptr(Tail...);
	}

	template <typename T>
	T MeterToCentimeter(T Meter)
	{
		return Meter * T {100};
	}

	template <typename T>
	T CentimeterToMeter(T Centimeter)
	{
		return Centimeter / T {100};
	}

	void AddExpectedError(FAutomationTestBase& Test, const FString& Error);

	/**
	 * Latent Command that tests that TestHelper::GetTestWorld and
	 * FAGX_EditorUtilities::GetCurrentWorld return the same world.
	 *
	 * \note This could be implemented directly in the Test itself, instead of as a Latent Command.
	 * Done this way for experimentation/learning purposes. Move the actual test code to the Test's
	 * RunTest once we're confident in our ability to write both Tests and Latent Commands.
	 */
	/// @todo Replace this with a DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER.
	class FCheckWorldsCommand final : public IAutomationLatentCommand
	{
	public:
		FCheckWorldsCommand(FAutomationTestBase& InTest)
			: Test(InTest)
		{
		}

		virtual bool Update() override;

	private:
		FAutomationTestBase& Test;
	};

	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FLogWarningAgxCommand, FString, Message);
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FLogErrorAgxCommand, FString, Message);
	DEFINE_LATENT_AUTOMATION_COMMAND(FWaitUntilPIEUpCommand);
	DEFINE_LATENT_AUTOMATION_COMMAND(FWaitUntilPIEDownCommand);
	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitNTicksCommand, int32, NumTicks);
	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(
		FWaitUntilTimeCommand, UWorld*&, World, float, Time);
	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitUntilSimTime, float, Time, int32, MaxTicks);

	/**
	 * Latent Command that waits until a given number of seconds has passed in the given world.
	 * Timing starts when the Latent Commands' Update member function is first called.
	 */
	class FWaitWorldDuration final : public IAutomationLatentCommand
	{
	public:
		FWaitWorldDuration(UWorld*& InWorld, float InDuration);
		virtual bool Update() override;

	private:
		UWorld*& World;
		const float Duration;
		float EndTime = -1.0f;
	};

	/**
	 * Default implementations for the virtual functions in FAutomationTestBase to simplify the
	 * creation of Automation Tests that contain state.
	 */
	class FAgxAutomationTest : public FAutomationTestBase
	{
	public:
		FAgxAutomationTest(const FString& InName, const FString& InBeautifiedTestName);
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		uint32 GetTestFlags() const override;
#else
		EAutomationTestFlags GetTestFlags() const override;
#endif
		uint32 GetRequiredDeviceNum() const override;

	protected:
		FString GetBeautifiedTestName() const override;
		void GetTests(
			TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override;

	protected:
		FString BeautifiedTestName;
	};

	/**
	 * Test that TestHelper::GetTestWorld and FAGX_EditorUtilities::GetCurrentWorld return the same
	 * world.
	 */
	class FCheckWorldsTest final : public FAutomationTestBase
	{
	public:
		FCheckWorldsTest();

#if UE_VERSION_OLDER_THAN(5, 5, 0)
		uint32 GetTestFlags() const override;
#else
		EAutomationTestFlags GetTestFlags() const override;
#endif

		uint32 GetRequiredDeviceNum() const override;

		FString GetBeautifiedTestName() const override;

	protected:
		void GetTests(
			TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const override;

		bool RunTest(const FString& InParameter) override;
	};

	inline float RelativeTolerance(float Expected, float Tolerance)
	{
		return FMath::Abs(Expected * Tolerance);
	}

	constexpr float AgxToUnreal {100.0f};
	constexpr float UnrealToAgx {0.01f};

	inline float AgxToUnrealDistance(float Agx)
	{
		return Agx * AgxToUnreal;
	}

	inline double AgxToUnrealDistance(double Agx)
	{
		return Agx * AgxToUnreal;
	}

	inline FVector AgxToUnrealDisplacement(const FVector& Agx)
	{
		return FVector(Agx.X * AgxToUnreal, -Agx.Y * AgxToUnreal, Agx.Z * AgxToUnreal);
	}

	inline FVector AgxToUnrealDisplacement(double X, double Y, double Z)
	{
		return AgxToUnrealDisplacement(FVector(X, Y, Z));
	}

	inline FVector AgxToUnrealVector(const FVector& Agx)
	{
		return FVector(Agx.X, -Agx.Y, Agx.Z);
	}

	inline FVector AgxToUnrealVector(double X, double Y, double Z)
	{
		return AgxToUnrealVector(FVector(X, Y, Z));
	}

	inline FRotator AgxToUnrealEulerAngles(const FVector& Agx)
	{
		/// @todo Verify a third time that the order and signs are correct.
		return FRotator(
			/*pitch*/ FMath::RadiansToDegrees(-Agx.Y),
			/*yaw*/ FMath::RadiansToDegrees(-Agx.Z),
			/*roll*/ FMath::RadiansToDegrees(Agx.X));
	}

	inline FRotator AgxToUnrealEulerAngles(double X, double Y, double Z)
	{
		return AgxToUnrealEulerAngles(FVector(X, Y, Z));
	}

	inline FVector AgxToUnrealAngularVelocity(const FVector& Agx)
	{
		return FVector(
			FMath::RadiansToDegrees(Agx.X), FMath::RadiansToDegrees(-Agx.Y),
			FMath::RadiansToDegrees(-Agx.Z));
	}
}

#define BAIL_TEST_IF(Expression, Ret) \
	if (Expression)                   \
	{                                 \
		TestFalse(#Expression, true); \
		return Ret;                   \
	}

#define BAIL_TEST_IF_NO_WORLD(Ret)                                                                \
	if (AgxAutomationCommon::NoWorldTestsReason Reason = AgxAutomationCommon::CanRunWorldTests()) \
	{                                                                                             \
		AddError(AgxAutomationCommon::GetNoWorldTestsReasonText(Reason));                         \
		return Ret;                                                                               \
	}

#define BAIL_TEST_IF_WORLDS_MISMATCH(Ret)                                                        \
	if (AgxAutomationCommon::GetTestWorld() != FAGX_EditorUtilities::GetCurrentWorld())          \
	{                                                                                            \
		AddError(                                                                                \
			"Cannot run test because the test world and the AGX Dynamics world are different."); \
		return Ret;                                                                              \
	}
#define BAIL_TEST_IF_NO_AGX(Ret)                                                        \
	if (UAGX_Simulation::GetFrom(AgxAutomationCommon::GetTestWorld()) == nullptr)       \
	{                                                                                   \
		AddError(                                                                       \
			"Cannot run test because the test world doesn't contain a UAGX_Simulation " \
			"subsystem.");                                                              \
		return Ret;                                                                     \
	}

#define BAIL_TEST_IF_NOT_EDITOR(Ret)                       \
	if (!GIsEditor)                                        \
	{                                                      \
		AddError("This test must be run in Editor mode."); \
		return Ret;                                        \
	}

#define BAIL_TEST_IF_NOT_GAME(Ret)                       \
	if (GIsEditor)                                       \
	{                                                    \
		AddError("This test must be run in Game mode."); \
		return Ret;                                      \
	}

#define BAIL_TEST_IF_CANT_SIMULATE(Ret) \
	BAIL_TEST_IF_NO_WORLD(Ret)          \
	BAIL_TEST_IF_WORLDS_MISMATCH(Ret)   \
	BAIL_TEST_IF_NO_AGX(Ret)
