// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "Distributed/AGX_DistributedAsset.h"
#include "Distributed/AGX_DistributedPossessionAsync.h"
#include "Distributed/AGX_DistributedScenarioAsset.h"
#include "Distributed/AGX_DistributedSettings.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"
#include "Utilities/TestUtilities.h"

// Unreal Engine includes.
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_Event.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedPossessionBlueprintTest, "AGXUnreal.Editor.Distributed.PossessionBlueprintNode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedPossessionBlueprintTest::RunTest(const FString& Parameters)
{
	auto* Blueprint = FKismetEditorUtilities::CreateBlueprint(
		AActor::StaticClass(), GetTransientPackage(),
		MakeUniqueObjectName(
			GetTransientPackage(), UBlueprint::StaticClass(), TEXT("PossessionNodeTest")),
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	UEdGraph* Graph = Blueprint->UbergraphPages[0];
	auto* Node = NewObject<UK2Node_AsyncAction>(Graph);
	Node->InitializeProxyFromFunction(
		UAGX_DistributedPossessionAsync::StaticClass()->FindFunctionByName(
			TEXT("RequestDistributedPossession")));
	Graph->AddNode(Node);
	Node->AllocateDefaultPins();
	for (FName Name : {FName("Success"), FName("Fail"), FName("Entity"), FName("ErrorMessage")})
		TestNotNull(
			*FString::Printf(TEXT("Completion pin %s"), *Name.ToString()),
			Node->FindPin(Name, EGPD_Output));
	TestNotNull(TEXT("Actor input"), Node->FindPin(TEXT("TargetActor"), EGPD_Input));
	TestNull(
		TEXT("No misleading immediate completion pin"),
		Node->FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output));
	auto* BeginPlay = NewObject<UK2Node_Event>(Graph);
	BeginPlay->EventReference.SetExternalMember(TEXT("ReceiveBeginPlay"), AActor::StaticClass());
	BeginPlay->bOverrideFunction = true;
	Graph->AddNode(BeginPlay);
	BeginPlay->AllocateDefaultPins();
	TestTrue(
		TEXT("Connect execution"), Graph->GetSchema()->TryCreateConnection(
									   BeginPlay->FindPin(UEdGraphSchema_K2::PN_Then),
									   Node->FindPin(UEdGraphSchema_K2::PN_Execute)));
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	TestTrue(TEXT("Async possession node compiles"), Blueprint->Status != BS_Error);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedPossessionCallbackTest, "AGXUnreal.Editor.Distributed.PossessionCallbacks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedPossessionCallbackTest::RunTest(const FString& Parameters)
{
	FString Error;
	const bool bSuccess = FTestUtilities::WithDistributedTestManager(
		17592,
		[&]
		{
			auto* Settings = GetMutableDefault<UAGX_DistributedSettings>();
			const auto Previous = Settings->Scenario;
			auto* Scenario = NewObject<UAGX_DistributedScenarioAsset>();
			auto* Asset = NewObject<UAGX_DistributedAsset>();
			Asset->AssetPath = TEXT("Car.openplx");
			Asset->ActorClass = AActor::StaticClass();
			Scenario->Assets.Add(Asset);
			Scenario->ZenohEndpoint = TEXT("tcp/127.0.0.1:17592");
			Scenario->WorldNamespace = FGuid::NewGuid().ToString();
			Settings->Scenario = Scenario;
			auto* Instance = NewObject<UGameInstance>(GEngine);
			Instance->InitializeStandalone(TEXT("PossessionCallbackTestWorld"));
			UWorld* World = Instance->GetWorld();
			auto* Subsystem = World->GetSubsystem<UAGX_DistributedWorldSubsystem>();
			AActor* Actor = nullptr;
			int32 Completions = 0;
			bool bGranted = false;
			bool bDestroyedCompleted = false;
			int32 ShutdownCompletions = 0;
			auto Callback = FAGX_DistributedPossessionCallback::CreateLambda(
				[&](bool bAccepted, AActor* Target, const FString& Message)
				{
					++Completions;
					bGranted = bAccepted;
					TestTrue(TEXT("Completion identifies Actor"), Target == Actor);
				});
			ON_SCOPE_EXIT
			{
				Subsystem->ShutdownClient();
				World->EndPlay(EEndPlayReason::Quit);
				World->DestroyWorld(false);
				Instance->Shutdown();
				GEngine->DestroyWorldContext(World);
				Settings->Scenario = Previous;
			};
			World->BeginPlay();
			World->SetBegunPlay(true);
			auto Wait = [&](TFunctionRef<bool()> Predicate)
			{
				const double Deadline = FPlatformTime::Seconds() + 10.0;
				while (FPlatformTime::Seconds() < Deadline)
				{
					Subsystem->Tick(0.01f);
					if (Predicate())
						return true;
					FPlatformProcess::Sleep(0.005f);
				}
				return false;
			};
			auto Spawn = [&](const TCHAR* Key) -> AActor*
			{
				FAGX_DistributedSpawnOptions Options;
				Options.Asset = Asset;
				Options.ObjectKey = Key;
				Options.Transform = FTransform::Identity;
				const FGuid Id = Subsystem->QueueSpawn(Options);
				FAGX_DistributedSpawnResult Result;
				const bool bReady = Wait(
					[&]
					{
						return Subsystem->GetSpawnResult(Id, Result) &&
							   Result.State == EAGX_DistributedSpawnState::Succeeded;
					});
				TestTrue(TEXT("Distributed Actor ready"), bReady);
				return bReady ? Result.Entity.Get() : nullptr;
			};
			Actor = Spawn(TEXT("callbacks"));
			if (Actor == nullptr)
				return false;
			TestTrue(
				TEXT("Acquisition submitted"),
				Subsystem->RequestActorPossession(Actor, true, Callback).IsValid());
			TestEqual(TEXT("Completion is not inline"), Completions, 0);
			TestFalse(
				TEXT("Overlapping request rejected"),
				Subsystem->RequestActorPossession(Actor, true, Callback).IsValid());
			if (!TestTrue(
					TEXT("Acquisition completes once"), Wait([&] { return Completions == 1; })))
				return false;
			TestTrue(TEXT("Acquisition accepted"), bGranted);
			TestTrue(
				TEXT("Release submitted"),
				Subsystem->RequestActorPossession(Actor, false, Callback).IsValid());
			if (!TestTrue(TEXT("Release completes once"), Wait([&] { return Completions == 2; })))
				return false;
			TestTrue(TEXT("Release accepted"), bGranted);
			int64 EntityId = -1;
			TestTrue(TEXT("Resolve target ID"), Subsystem->GetEntityIdForActor(Actor, EntityId));
			TestTrue(
				TEXT("Submit before target destruction"),
				Subsystem
					->RequestActorPossession(
						Actor, true,
						FAGX_DistributedPossessionCallback::CreateLambda(
							[&](bool bAccepted, AActor* Target, const FString& Message)
							{
								bDestroyedCompleted = true;
								TestFalse(TEXT("Destroyed target fails"), bAccepted);
								TestNull(TEXT("Destroyed target not returned"), Target);
								TestFalse(TEXT("Failure has a reason"), Message.IsEmpty());
							}))
					.IsValid());
			Actor->Destroy();
			TestTrue(TEXT("Destroyed target completes"), Wait([&] { return bDestroyedCompleted; }));
			TestTrue(
				TEXT("Late grant released"),
				Wait([&] { return !Subsystem->IsPossessed(EntityId); }));
			Actor = Spawn(TEXT("shutdown"));
			if (Actor == nullptr)
				return false;
			TestTrue(
				TEXT("Submit before shutdown"),
				Subsystem
					->RequestActorPossession(
						Actor, true,
						FAGX_DistributedPossessionCallback::CreateLambda(
							[&](bool bAccepted, AActor* Target, const FString& Message)
							{
								++ShutdownCompletions;
								TestFalse(TEXT("Shutdown fails pending request"), bAccepted);
								TestFalse(TEXT("Shutdown has a reason"), Message.IsEmpty());
								TestFalse(
									TEXT("Reentrant submissions rejected during shutdown"),
									Subsystem->RequestActorPossession(Target, true, Callback)
										.IsValid());
							}))
					.IsValid());
			Subsystem->ShutdownClient();
			Subsystem->ShutdownClient();
			TestEqual(TEXT("Shutdown completion exactly once"), ShutdownCompletions, 1);
			return !HasAnyErrors();
		},
		Error);
	if (!bSuccess && !Error.IsEmpty())
		AddError(Error);
	return bSuccess;
}
#endif
