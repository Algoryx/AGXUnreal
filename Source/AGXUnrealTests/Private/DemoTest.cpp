// Copyright 2024, Algoryx Simulation AB.

#if WITH_DEV_AUTOMATION_TESTS

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AgxAutomationCommon.h"

// Unreal Engine includes.
#include "Editor.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

/*
 * This file demonstrates how to use the Automation framework to write unit tests.
 */

/*
 * The IMPLEMENT_SIMPLE_AUTOMATION_TEST creates a new test class with the given class name, pretty
 * name, and flags.
 *
 * The class name is just the C++ name of the class. Should begin with F, end with Test, and
 * describe what is being tested in between.
 *
 * The pretty name is the name that is shown to the user. It is hierarchical and '.'-separated. When
 * running tests we can choose to either specify a single test by giving the complete name, or run a
 * group of tests by giving a subset of the name hierarchy. For example, giving
 * "AGXUnreal.Demo.SimpleTest" would run only this test, while "AGXUnreal.Demo" would run all tests
 * in that group, including "SimpleTest".
 *
 * The flags specify during which circumstances the test should be run. It's split in three main
 * parts: Filter, Priority and Context. The filter part can be SmokeFilter, EngineFilter,
 * ProductFilter, PerfFilter, StressFilter, and NegativeFilter. The Priority part can be
 * CriticalPriority, HighPriority, MediumPriority, or LowPriority. The Context part can be
 * EditorContext, ClientContext, ServerContext, or CommandletContext. Multiple Filter and Context
 * flags can be or'ed together with one Priority flag. There are a few other flags as well I don't
 * know much about yet.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDemoSimpleTest, "AGXUnreal.Demo.SimpleTest",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

/*
 * The Simple Automation Test provides the RunTest function which is where we implement the test
 * logic. It takes a single FString parameter which I believe is always the empty string.
 */
bool FDemoSimpleTest::RunTest(const FString&)
{
	/*
	 * The test system provide a bunch of Test.+-functions for use in RunTest, such as TestEqual,
	 * TestValid, TestNotNull, TestSame, and so on. All Test.+-functions have parameters following
	 * the <Message>, <Thing to test>, [<Expected>] pattern, where not all Test.+-functions have
	 * the last parameter.
	 *
	 * A failed Test.+ will print an error message and mark the test as failed.
	 */
	int32 Actual = 1 + 1;
	int32 Expected = 2;
	TestEqual("1 + 1 == 2", Actual, Expected);

	/*
	 * We return true or false to indicate pass or failure. If a Test.+-function failed then that
	 * failure overrides a pass/true return value.
	 */
	return true;
}

/*
 * A complex test is in many ways similar to a simple test. The difference is that the complex test
 * can be run many times with varying input. Here "input" refers to the parameter to RunTest. For
 * complex tests we implement one additional member function, GetTests, that lists the input that we
 * want to run the test over.
 */
IMPLEMENT_COMPLEX_AUTOMATION_TEST(
	FDemoComplexTest, "AGXUnreal.Demo.ComplexTest",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

void FDemoComplexTest::GetTests(
	TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	/*
	 * Here we hard-code two test inputs, but we could just as well create them dynamically based on
	 * files found on disk, through a loop, or whatever else we fancy. For each test we give it a
	 * name and a command. The command becomes the parameter to RunTest. The name is appended to
	 * the name hierarchy of the complex test. So in this example the "AGXUnreal.Demo.ComplexTest"
	 * will be expanded to the two runnable tests "AGXUnreal.Demo.ComplexTest.First" and
	 * "AGXUnreal.Demo.ComplexTest.Second". All tests in a complex test can be run by specifying the
	 * name of the complex test, "AGXUnreal.Demo.ComplexTest" in this case.
	 */

	OutBeautifiedNames.Add("First");
	OutTestCommands.Add("1");

	OutBeautifiedNames.Add("Second");
	OutTestCommands.Add("2");
}

/*
 * This RunTest will be called two times, once for each pair created in GetTests.
 */
bool FDemoComplexTest::RunTest(const FString& Parameters)
{
	UE_LOG(LogAGX, Warning, TEXT("Parameters is '%s'."), *Parameters);
	return true;
}

/*
 * The tests we run will be executed one after another in quick succession. Sometimes a test need
 * for some time to pass, or some event to finish, before it can continue. That is accomplished with
 * Latent Commands. A Latent Command is queued in RunTest but not executed until later. Also, a
 * Latent Command is not executed until all preceding Latent Commands in the same test has
 * finished. The Automation framework provides Latent Commands for loading maps, waiting for maps to
 * finish loading, to wait a fixed amount of time, and other useful actions. We can also create our
 * own Latent Commands.
 *
 * The DEFINE_LATENT_AUTOMATION_COMMAND macro defines a Latent Command with the given name, which
 * should begin with F and end with Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND(FWaitTenTicksCommand);

/*
 * Latent Commands has a member function named Update that is called once per tick after that Latent
 * Command has reached the front of the Latent Command queue for the test to which it was added. In
 * Update we should perform some action and/or check the condition that the Latent Command is
 * waiting for and then return true if the Latent Command is done and false if this Latent Command
 * should be called again next tick.
 */
bool FWaitTenTicksCommand::Update()
{
	/// @note Using 'static' to hold state like this is not a good idea and a better way to do
	/// this is demonstrated in the next example.
	static int32 Counter = 0;
	++Counter;
	return Counter > 10;
}

/*
 * Latent commands are added to a test using ADD_LATENT_AUTOMATION_COMMAND, to which we pass an
 * instance of our Latent Command class. The command will be added to the test's Latent Command
 * queue but not executed yet. RunTest will return immediately and the first latent command will
 * start executing soon after.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaitTenTicksTest, "AGXUnreal.Demo.WaitTenTicks",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FWaitTenTicksTest::RunTest(const FString&)
{
	ADD_LATENT_AUTOMATION_COMMAND(FWaitTenTicksCommand())
	return true;
}

/*
 * While the FWaitTenTicksCommand shown above kind of works, using static local variables for state
 * won't work when there multiple instances of the Latent Command created. We can create member
 * variables using the _PARAMETER form of the macro.
 *
 * Every other parameter after the class name is a type of a parameter, and every other after that
 * is the variable name for that parameter. A member variable is created for each parameter with
 * the same name, making the data accessible in the Update member function.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitTicksCommand, int32, NumTicks);

/*
 * The in implementation of Update we have read/write access to the variable. Here we decrement
 * the counter and return true, causing the same Latent Command to be called again the next tick,
 * until the counter reaches zero. At that point we return true, signaling that this Latent Command
 * has completed.
 */
bool FWaitTicksCommand::Update()
{
	--NumTicks;
	return NumTicks <= 0;
}

/*
 * The parameter is passed to the Latent Command when it is queued.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FWaitTwentyTicksTest, "AGXUnreal.Demo.WaitTwentyTicks",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FWaitTwentyTicksTest::RunTest(const FString&)
{
	ADD_LATENT_AUTOMATION_COMMAND(FWaitTicksCommand(20))
	return true;
}

/*
 * A test implemented with Latent Commands cannot use the return value from RunTest to signal
 * success or failure since the actual testing hasn't yet been performed at that point. Instead,
 * the Test class instance can be passed to those Latent Commands that are able to perform
 * the verification. To get access to the Test.+-function, take a reference to FAutomationTestBase.
 *
 * When writing Update member functions that do these kinds of tests it's important to remember that
 * a Latent Command's return value is whether the Latent Command is finished, not whether the
 * test passed or failed.
 *
 * A test will not stop at errors, the Latent Command will continue to have its Update member
 * function called until it returns true and all subsequent Latent commands will still run.
 * Therefore, each Latent Command should be written to handle failure at any prior Latent Command.
 */
DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FTestingLatentCommand, FAutomationTestBase&, Test);

bool FTestingLatentCommand::Update()
{
	int32 Actual = 1 + 1;
	int32 Expected = 2;
	Test.TestEqual("1 + 1 = 2", Actual, Expected);
	return true;
}

/*
 * Multiple Latent Commands a chained one after the other by queuing them, in order, in RunTest.
 * It is not, as far as I know, possible to run Latent Commands in parallel, or do other kinds of
 * dependency graphs.
 *
 * The following test will first wait 25 ticks and then test if 1 + 1 == 2.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTestInLatentCommandTest, "AGXUnreal.Demo.TestInLatentCommand",
	EAutomationTestFlags::ProductFilter | AgxAutomationCommon::ETF_ApplicationContextMask)

bool FTestInLatentCommandTest::RunTest(const FString&)
{
	ADD_LATENT_AUTOMATION_COMMAND(FWaitTicksCommand(25))
	ADD_LATENT_AUTOMATION_COMMAND(FTestingLatentCommand(*this))
	return true;
}

/*
 * The Automation test framework listens to logging messages and any message with Error severity
 * trigger a test failure. This means that a test can fail without any failure being detected and
 * reported by the test itself if any function called by the test uses logging for error reporting.
 * We can tell a particular test that we expect a particular error log message, which means the test
 * won't fail because of it, by calling AddExpectedError.
 *
 * In AgxAutomationCommon we provide Warning and Error logging Latent Commands.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FTestAbort, "AGXUnreal.Demo.ExpectErrorLog",
	AgxAutomationCommon::ETF_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

bool FTestAbort::RunTest(const FString& Parameters)
{
	using namespace AgxAutomationCommon;
	AddExpectedError("At the error.", EAutomationExpectedErrorFlags::Exact, 1);
	ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("Before the error")));
	ADD_LATENT_AUTOMATION_COMMAND(FLogErrorAgxCommand(TEXT("At the error.")));
	ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("After the error.")));
	return true;
}

/*
 * While many tests are self-contained in the sense that they create and verify their own state
 * completely, some require interaction with the global Unreal Engine state. In particular, loading
 * levels and stepping a simulation is somewhat complicated. Unreal Engine provides a few Latent
 * Commands for such purposes and AgxAutomationCommon provides a few more.
 *
 * This test demonstrates how to simulate a falling Rigid Body for a short duration.
 *
 * It also demonstrates one way of passing state between Latent Commands by storing it in the Test.
 * We need to declare the Test class manually, instead of using IMPLEMENT_SIMPLE_AUTOMATION_TEST,
 * in order to include the state member variables. AgxAutomationCommon provide a helper class we
 * can inherit from to cut down a bit on the boilerplate code required.
 */

struct FSimulateShortDurationState
{
	UAGX_RigidBodyComponent* RigidBody = nullptr;
};

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FSpawnRigidBody, FSimulateShortDurationState&, State);

bool FSpawnRigidBody::Update()
{
	if (!GEditor->IsPlayingSessionInEditor())
	{
		// We only want to create simulation objects during Play In Editor sessions, so that they
		// are removed when we tear down the Play In Editor session so that no state leak between
		// the tests.
		//
		// It is the responsibility of the test to ensure that a FStartPIECommand and a
		// FWaitUntilPIEUpCommand has been scheduled before this Command. If no Play In Editor
		// session is running then we don't create any Rigid Body and move immediately to the
		// next Latent Command in the queue.
		return true;
	}

	UE_LOG(LogAGX, Warning, TEXT("Creating Rigid Body"));
	UWorld* World = GEditor->GetPIEWorldContext()->World();
	AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass());
	if (NewActor == nullptr)
	{
		/// @todo Should we replace UE_LOG(, Error, ) with Test.TestNotNull or whatever it might be
		/// called?
		UE_LOG(LogAGX, Error, TEXT("Failed to spawn an empty actor."));
		return true;
	}

	UAGX_RigidBodyComponent* Body = NewObject<UAGX_RigidBodyComponent>(NewActor, TEXT("Body"));
	if (Body == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("Failed to create a Rigid Body."));
		return true;
	}

	NewActor->SetRootComponent(Body);
	NewActor->AddInstanceComponent(Body);
	Body->RegisterComponent();
	State.RigidBody = Body;

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(
	FPrintBodyLocation, FSimulateShortDurationState&, State);

bool FPrintBodyLocation::Update()
{
	UE_LOG(
		LogAGX, Warning, TEXT("Body location: %s"),
		*State.RigidBody->GetComponentLocation().ToString());
	return true;
}

class FSimulateShortDurationTest final : public AgxAutomationCommon::FAgxAutomationTest
{
public:
	FSimulateShortDurationTest()
		: AgxAutomationCommon::FAgxAutomationTest(
			  TEXT("FSimulateShortDurationTest"), TEXT("AGXUnreal.Demo.SimulateShortDuration"))
	{
	}

protected:
	bool RunTest(const FString& Parameters) override
	{
		using namespace AgxAutomationCommon;

		UE_LOG(LogAGX, Warning, TEXT("\n\n\n\nRunning simulation step test."));

		static const FString MapName {"/Game/Tests/Test_ArchiveImport.umap"};
		ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("Test starting, loading map.")));
		ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(MapName))
		ADD_LATENT_AUTOMATION_COMMAND(
			FLogWarningAgxCommand(TEXT("Map loaded, starting Play In Editor session.")));
		ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
		ADD_LATENT_AUTOMATION_COMMAND(
			FLogWarningAgxCommand(TEXT("Waiting for Play In Editor session startup to complete.")));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilPIEUpCommand);
		ADD_LATENT_AUTOMATION_COMMAND(
			FLogWarningAgxCommand(TEXT("Play In Editor session is up, creating Rigid Body.")));
		ADD_LATENT_AUTOMATION_COMMAND(FSpawnRigidBody(State));
		ADD_LATENT_AUTOMATION_COMMAND(FPrintBodyLocation(State));
		ADD_LATENT_AUTOMATION_COMMAND(
			FLogWarningAgxCommand(TEXT("Rigid Body created, simulating until t=5s.")));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilSimTime(5.0f, 100));
		ADD_LATENT_AUTOMATION_COMMAND(FPrintBodyLocation(State));
		ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand);
		ADD_LATENT_AUTOMATION_COMMAND(FLogWarningAgxCommand(TEXT("Play In Editor shutting down.")));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitUntilPIEDownCommand);
		ADD_LATENT_AUTOMATION_COMMAND(
			FLogWarningAgxCommand(TEXT("Play In Editor down, test terminating.")));

		return true;
	}

private:
	FSimulateShortDurationState State;
};

namespace
{
	// Create an instance of the test so that it can register itself.
	// That least that's what I assume why we do this. Not entirely sure that it is necessary.
	FSimulateShortDurationTest SimulateShortDurationTest;
}

/// @todo Show state management with fully defined test and command classes, i.e., not using macros.

/// @todo Show how to create Spec tests.
#if 0
BEGIN_DEFINE_SPEC(
	FArchiveImporterToSingleActor_EmptySceneSpec,
	"AGXUnreal.ArchiveImporterToSingleActor.EmptyScene.Spec", AgxAutomationCommon::DefaultTestFlags)
UWorld* World = nullptr;
AActor* Contents = nullptr;
END_DEFINE_SPEC(FArchiveImporterToSingleActor_EmptySceneSpec)

void FArchiveImporterToSingleActor_EmptySceneSpec::Define()
{
	Describe("Import empty scene", [this]() {
		BeforeEach([this]() {
			World = AgxAutomationCommon::GetTestWorld();
			TestNotNull(TEXT("World"), World);
			UE_LOG(LogAGX, Warning, TEXT("Got world %p"), (void*) World);
		});

		BeforeEach([this]() {
			UE_LOG(LogAGX, Warning, TEXT("Opening map 'Test_ArchiveImport'."));
			GEngine->Exec(World, TEXT("open Test_ArchiveImport"));
		});

		BeforeEach([this]() {
			FString ArchiveName = TEXT("empty_scene.agx");
			FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(ArchiveName);
			Contents = AGX_ImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
			UE_LOG(LogAGX, Warning, TEXT("Imported archive '%s'."), *ArchiveFilePath);
		});

		It("should contain the archive actor", [this]() {
			UE_LOG(LogAGX, Warning, TEXT("Running checks"));
			TestNotNull(TEXT(""), Contents);

			bool Found = false;
			for (FActorIterator It(World); It; ++It)
			{
				if (*It == Contents)
				{
					Found = true;
					break;
				}
			}
			TestTrue(TEXT("Imported actor found in test world."), Found);
		});
	});
}
#endif

/// @todo Cannot (yet) use Spec for tickign tests because I don't know how to make a Latent It
/// that waits until the game world reaches some pre-defiend time. With Latent Commands that is
/// handled automatically by checking the time on each call to Update returning false until the time
/// is right. What I need to know is, how do I abandon an LatentIt until the next tick?
#if 0
BEGIN_DEFINE_SPEC(
	FArchiveImporterToSingleAcgor_SingleSphereSpec,
	"AGXUnreal.ArchiveImporterToSingleActor.SingleSphere.Spec",
	AgxAutomationCommon::DefaultTestFlags)
const TCHAR* MapName = TEXT("Test_ArchiveImport");
const TCHAR* ArchiveName = TEXT("single_sphere.agx");
UWorld* World = nullptr;
AActor* Contents = nullptr;
UAGX_RigidBodyComponent* Sphere = nullptr;
FVector SphereStartPosition;
END_DEFINE_SPEC(FArchiveImporterToSingleAcgor_SingleSphereSpec)

void FArchiveImporterToSingleAcgor_SingleSphereSpec::Define()
{
	Describe(TEXT("Import single sphere"), [this]() {
		// Check world.
		BeforeEach([this]() {
			World = AgxAutomationCommon::GetTestWorld();
			TestNotNull(TEXT("The test world most not be null."), World);
			UE_LOG(LogAGX, Warning, TEXT("Got world %p."), (void*) World);
		});

		// Load map.
		BeforeEach([this]() {
			UE_LOG(LogAGX, Warning, TEXT("Opening map '%s'"), MapName);
			GEngine->Exec(World, *FString::Printf(TEXT("open %s"), MapName));
		});

		// Import archive.
		BeforeEach([this]() {
			FString ArchiveFilePath = AgxAutomationCommon::GetTestScenePath(ArchiveName);
			Contents = AGX_ImporterToSingleActor::ImportAGXArchive(ArchiveFilePath);
			TestNotNull(TEXT("Contents"), Contents);
			TArray<UActorComponent*> Components;
			Contents->GetComponents(Components, false);
			TestEqual(TEXT("Number of imported components"), Components.Num(), 3);
			Sphere =
				AgxAutomationCommon::GetByName<UAGX_RigidBodyComponent>(Components, TEXT("bullet"));
			TestNotNull(TEXT("Sphere body"), Sphere);
			SphereStartPosition = Sphere->GetComponentLocation();
			UE_LOG(LogAGX, Warning, TEXT("Imported archive '%s'."), *ArchiveFilePath);
		});

		// Check contents.
		It(TEXT("should have imported a sphere"),
		   [this]() { TestNotNull(TEXT("Imported sphere"), Sphere); });

		LatentIt(TEXT("should have a falling sphere"), [this](const FDoneDelegate& Done) {

		});
	});
}
#endif

// WITH_DEV_AUTOMATION_TESTS
#endif
