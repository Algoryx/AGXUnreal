
#pragma once

// Unreal Engine includes.
#include "Engine/EngineTypes.h"
#include "Misc/AutomationTest.h"

class UWorld;

struct FLinearColor;
struct FQuat;
struct FRotator;

/**
 * A set of helper functions used by several Automation tests.
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

	constexpr EAutomationTestFlags::Type DefaultTestFlags =
		static_cast<const EAutomationTestFlags::Type>(
			EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext |
			EAutomationTestFlags::ClientContext);

	/// @todo Remove this TestEqual implementation for FQuat once it's included in-engine.
	/// @see Misc/AutomationTest.h
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FQuat& Actual, const FQuat& Expected,
		float Tolerance = KINDA_SMALL_NUMBER);

	/// @todo Remove this TestEqual implementation for FRotator once it's included in-engine.
	/// @see Misc/AutomationTest.h
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FRotator& Actual,
		const FRotator& Expected, float Tolerance = KINDA_SMALL_NUMBER);

	/// @todo Remove this TestEqual implementation for FLinearColor once it's included in-engine.
	/// @see Misc/AutomaitonTest.h
	void TestEqual(
		FAutomationTestBase& Test, const TCHAR* What, const FLinearColor& Actual,
		const FLinearColor& Expected, float Tolerance = KINDA_SMALL_NUMBER);

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
	 * Get the file system path to an AGX Dynamcis archive intended for Automation testing.
	 * @param ArchiveName The name of the AGX Dynamics archive to find.
	 * @return File system path to the AGX Dynamics archive.
	 */
	FString GetArchivePath(const TCHAR* ArchiveName);

	/**
	 * Get the file system path to an AGX Dynamcis archive intended for Automation testing.
	 * @param ArchiveName The name of the AGX Dynamics archive to find.
	 * @return File system path to the AGX Dynamics archive.
	 */
	FString GetArchivePath(const FString& ArchiveName);

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

	template <typename T>
	T* GetByName(TArray<UActorComponent*>& Components, const TCHAR* Name)
	{
		UActorComponent** Match = Components.FindByPredicate([Name](UActorComponent* Component) {
			return Cast<T>(Component) && Component->GetName() == Name;
		});

		return Match != nullptr ? Cast<T>(*Match) : nullptr;
	}

	template <typename T>
	void GetByName(TArray<UActorComponent*>& Components, const TCHAR* Name, T*& Out)
	{
		Out = GetByName<T>(Components, Name);
	}

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
	DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FTickUntilCommand, UWorld*&, World, float, Time);

	DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitNTicks, int32, NumTicks);

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
		uint32 GetTestFlags() const override;
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

		uint32 GetTestFlags() const override;

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

	inline FVector AgxToUnrealVector(const FVector& Agx)
	{
		return FVector(Agx.X * AgxToUnreal, -Agx.Y * AgxToUnreal, Agx.Z * AgxToUnreal);
	}

	inline FRotator AgxToUnrealEulerAngles(const FVector& Agx)
	{
		/// @todo Verify a third time that the order and signs are correct.
		return FRotator(
			/*pitch*/ FMath::RadiansToDegrees(-Agx.Y),
			/*yaw*/ FMath::RadiansToDegrees(-Agx.Z),
			/*roll*/ FMath::RadiansToDegrees(Agx.X));
	}

	inline FVector AgxToUnrealAngularVelocity(const FVector& Agx)
	{
		return FVector(Agx.X, -Agx.Y, -Agx.Z);
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