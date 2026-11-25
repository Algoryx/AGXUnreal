// Copyright 2026, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "Distributed/AGX_DistributedAsset.h"
#include "Distributed/AGX_DistributedScenarioAsset.h"
#include "Distributed/AGX_DistributedSettings.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"
#include "Distributed/DistributedApplicationBarrier.h"
#include "SimulationBarrier.h"
#include "Utilities/TestUtilities.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedAssetRegistrationTest, "AGXUnreal.Editor.Distributed.AssetRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedAssetRegistrationTest::RunTest(const FString& Parameters)
{
	FString Error;
	const bool bSuccess = FTestUtilities::WithDistributedTestManager(
		17594,
		[&]
		{
			FSimulationBarrier SimulationA, SimulationB;
			SimulationA.AllocateNative();
			SimulationB.AllocateNative();
			FDistributedApplicationBarrier A, B;
			if (!TestTrue(
					TEXT("Initialize two clients"),
					A.Initialize(SimulationA, TEXT("tcp/127.0.0.1:17594")) &&
						B.Initialize(SimulationB, TEXT("tcp/127.0.0.1:17594"))))
				return false;
			A.StartAndLoad(5);
			B.StartAndLoad(5);
			TArray<FAGXDistributedEvent> EventsA, EventsB;
			auto Wait = [&](TFunctionRef<bool()> Predicate)
			{
				const double Deadline = FPlatformTime::Seconds() + 8.0;
				while (FPlatformTime::Seconds() < Deadline)
				{
					A.Poll(EventsA);
					B.Poll(EventsB);
					if (Predicate())
						return true;
					FPlatformProcess::Sleep(0.005f);
				}
				return false;
			};
			if (!TestTrue(
					TEXT("Join empty manager"), Wait([&] { return A.IsReady() && B.IsReady(); })))
				return false;
			uint64 Id = MAX_uint64;
			TestFalse(
				TEXT("Joining does not register assets"), A.ResolveAsset(TEXT("Car.openplx"), Id));
			TArray<FGuid> Requests;
			for (int32 I = 0; I < 16; ++I)
			{
				Requests.Add(FGuid::NewGuid());
				TestTrue(
					TEXT("Concurrent registration accepted"),
					(I % 2 == 0 ? A : B)
						.RegisterAsset(
							Requests.Last(), TEXT("Car.openplx"),
							EAGX_DistributedAssetType::Possessable));
			}
			TestTrue(
				TEXT("All duplicate registrations complete"),
				Wait(
					[&]
					{
						int32 Completed = 0;
						for (const auto* Events : {&EventsA, &EventsB})
							for (const auto& Event : *Events)
								if (Event.Type ==
										EAGXDistributedEventType::AssetRegistrationResult &&
									Requests.Contains(Event.RequestId))
								{
									TestTrue(
										TEXT("Authoritative ID is identical"),
										Event.bAccepted && Event.AssetId == 0);
									++Completed;
								}
						return Completed == Requests.Num();
					}));
			const FGuid Conflict = FGuid::NewGuid();
			A.RegisterAsset(Conflict, TEXT("Car.openplx"), EAGX_DistributedAssetType::Dynamic);
			TestTrue(
				TEXT("Cached path still validates category"),
				Wait(
					[&]
					{
						return EventsA.ContainsByPredicate(
							[&](const auto& Event)
							{
								return Event.Type ==
										   EAGXDistributedEventType::AssetRegistrationResult &&
									   Event.RequestId == Conflict && !Event.bAccepted &&
									   !Event.Message.IsEmpty();
							});
					}));
			// A distinct key must get ID 1: duplicate requests did not append extra entries.
			const FGuid Other = FGuid::NewGuid();
			B.RegisterAsset(Other, TEXT("Ground.openplx"), EAGX_DistributedAssetType::Static);
			TestTrue(
				TEXT("Distinct asset receives next ID and reaches other client"),
				Wait(
					[&]
					{
						return A.ResolveAsset(TEXT("Ground.openplx"), Id) && Id == 1 &&
							   EventsB.ContainsByPredicate(
								   [&](const auto& Event)
								   {
									   return Event.Type == EAGXDistributedEventType::
																AssetRegistrationResult &&
											  Event.RequestId == Other && Event.bAccepted &&
											  Event.AssetId == 1;
								   });
					}));
			// FString equality normally ignores case. AGX keys must stay distinct even while
			// registrations are being shared, and long keys cross the native allocator boundary.
			const FString UpperPath = TEXT("Models/CaseSensitiveVehicle.openplx");
			const FString LowerPath = TEXT("Models/casesensitivevehicle.openplx");
			const FGuid UpperRequest = FGuid::NewGuid(), LowerRequest = FGuid::NewGuid();
			A.RegisterAsset(UpperRequest, UpperPath, EAGX_DistributedAssetType::Dynamic);
			A.RegisterAsset(LowerRequest, LowerPath, EAGX_DistributedAssetType::Dynamic);
			TestTrue(
				TEXT("Case-distinct Dynamic assets register independently"),
				Wait(
					[&]
					{
						int32 Completed = 0;
						for (const auto& Event : EventsA)
							if (Event.Type == EAGXDistributedEventType::AssetRegistrationResult &&
								(Event.RequestId == UpperRequest ||
								 Event.RequestId == LowerRequest) &&
								Event.bAccepted)
								++Completed;
						return Completed == 2;
					}));
			uint64 UpperId = MAX_uint64, LowerId = MAX_uint64;
			TestTrue(
				TEXT("Exact keys have different authoritative IDs"),
				A.ResolveAsset(UpperPath, UpperId) && A.ResolveAsset(LowerPath, LowerId) &&
					UpperId != LowerId);

			TestTrue(
				TEXT("Legacy spawn lazily registers unknown model"),
				A.RequestSpawn(TEXT("Legacy.openplx"), FTransform::Identity, false));
			TestTrue(
				TEXT("Legacy spawn is instantiated on both nodes"),
				Wait(
					[&]
					{
						for (const auto* Events : {&EventsA, &EventsB})
							if (!Events->ContainsByPredicate(
									[](const auto& Event)
									{
										return Event.Type ==
												   EAGXDistributedEventType::SpawnEntity &&
											   Event.AssetPath == TEXT("Legacy.openplx");
									}))
								return false;
						return true;
					}));
			FSimulationBarrier SimulationLate;
			SimulationLate.AllocateNative();
			FDistributedApplicationBarrier Late;
			Late.Initialize(SimulationLate, TEXT("tcp/127.0.0.1:17594"));
			Late.StartAndLoad(5);
			TestTrue(
				TEXT("Late join receives updated catalog"),
				Wait(
					[&]
					{
						TArray<FAGXDistributedEvent> Events;
						Late.Poll(Events);
						return Late.IsReady() && Late.ResolveAsset(TEXT("Car.openplx"), Id) &&
							   Id == 0 && Late.ResolveAsset(TEXT("Ground.openplx"), Id) && Id == 1;
					}));
			return true;
		},
		Error, true);
	if (!bSuccess)
		AddError(Error.IsEmpty() ? TEXT("Asset registration fixture failed.") : Error);
	return bSuccess;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedLazySpawnTest, "AGXUnreal.Editor.Distributed.LazySpawnRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedLazySpawnTest::RunTest(const FString& Parameters)
{
	FString Error;
	const bool bSuccess = FTestUtilities::WithDistributedTestManager(
		17595,
		[&]
		{
			auto* Settings = GetMutableDefault<UAGX_DistributedSettings>();
			const auto Previous = Settings->Scenario;
			auto* Scenario = NewObject<UAGX_DistributedScenarioAsset>();
			auto MakeAsset = [&](const TCHAR* Path)
			{
				auto* Asset = NewObject<UAGX_DistributedAsset>(Scenario);
				Asset->AssetPath = Path;
				Asset->ActorClass = AActor::StaticClass();
				Scenario->Assets.Add(Asset);
				return Asset;
			};
			auto* Car = MakeAsset(TEXT("Car.openplx"));
			auto* CancelledAsset = MakeAsset(TEXT("Cancelled.openplx"));
			auto* TimedOutAsset = MakeAsset(TEXT("TimedOut.openplx"));
			Scenario->ZenohEndpoint = TEXT("tcp/127.0.0.1:17595");
			Scenario->WorldNamespace = FGuid::NewGuid().ToString();
			Settings->Scenario = Scenario;
			auto* InstanceA = NewObject<UGameInstance>(GEngine);
			InstanceA->InitializeStandalone(TEXT("LazySpawnA"));
			auto* InstanceB = NewObject<UGameInstance>(GEngine);
			InstanceB->InitializeStandalone(TEXT("LazySpawnB"));
			UWorld* WorldA = InstanceA->GetWorld();
			UWorld* WorldB = InstanceB->GetWorld();
			auto* A = WorldA->GetSubsystem<UAGX_DistributedWorldSubsystem>();
			auto* B = WorldB->GetSubsystem<UAGX_DistributedWorldSubsystem>();
			ON_SCOPE_EXIT
			{
				for (auto* Instance : {InstanceB, InstanceA})
				{
					UWorld* World = Instance->GetWorld();
					World->GetSubsystem<UAGX_DistributedWorldSubsystem>()->ShutdownClient();
					World->EndPlay(EEndPlayReason::Quit);
					World->DestroyWorld(false);
					Instance->Shutdown();
					GEngine->DestroyWorldContext(World);
				}
				Settings->Scenario = Previous;
			};
			WorldA->BeginPlay();
			WorldA->SetBegunPlay(true);
			WorldB->BeginPlay();
			WorldB->SetBegunPlay(true);
			auto Wait = [&](TFunctionRef<bool()> Predicate)
			{
				const double Deadline = FPlatformTime::Seconds() + 10.0;
				while (FPlatformTime::Seconds() < Deadline)
				{
					A->Tick(0.01f);
					B->Tick(0.01f);
					if (Predicate())
						return true;
					FPlatformProcess::Sleep(0.005f);
				}
				return false;
			};
			if (!TestTrue(
					TEXT("Both Unreal worlds connect"),
					Wait([&] { return A->IsConnected() && B->IsConnected(); })))
				return false;
			FAGX_DistributedSpawnOptions Options;
			Options.Asset = Car;
			Options.ObjectKey = TEXT("shared-car");
			const FGuid SharedA = A->QueueSpawn(Options);
			const FGuid DuplicateA = A->QueueSpawn(Options);
			const FGuid SharedB = B->QueueSpawn(Options);
			A->Tick(0.0f);
			FAGX_DistributedSpawnResult Result;
			A->GetSpawnResult(SharedA, Result);
			TestEqual(
				TEXT("Registration is a separate asynchronous phase"), Result.State,
				EAGX_DistributedSpawnState::Registering);
			FAGX_DistributedSpawnResult RA, RD, RB;
			if (!TestTrue(
					TEXT("Both worlds finish with Actors"),
					Wait(
						[&]
						{
							return A->GetSpawnResult(SharedA, RA) &&
								   A->GetSpawnResult(DuplicateA, RD) &&
								   B->GetSpawnResult(SharedB, RB) &&
								   RA.State == EAGX_DistributedSpawnState::Succeeded &&
								   RD.State == EAGX_DistributedSpawnState::Succeeded &&
								   RB.State == EAGX_DistributedSpawnState::Succeeded;
						})))
				return false;
			TestEqual(
				TEXT("Shared object uses same entity across nodes"), RA.EntityId, RB.EntityId);
			TestEqual(
				TEXT("Duplicate interest reuses local Actor"), RA.Entity.Get(), RD.Entity.Get());
			TestTrue(
				TEXT("Each node creates its own Actor"),
				IsValid(RA.Entity) && IsValid(RB.Entity) && RA.Entity != RB.Entity);
			Options.ObjectKey = TEXT("independent-car");
			const FGuid Independent = B->QueueSpawn(Options);
			TestTrue(
				TEXT("Same asset can create independent instance"),
				Wait(
					[&]
					{
						return B->GetSpawnResult(Independent, Result) &&
							   Result.State == EAGX_DistributedSpawnState::Succeeded;
					}));
			TestTrue(
				TEXT("Different instance key gets different entity"),
				Result.EntityId != RA.EntityId);

			Options.Asset = CancelledAsset;
			Options.ObjectKey = TEXT("cancel-during-registration");
			int32 CancelCompletions = 0;
			const FGuid Cancel = A->QueueSpawn(
				Options, FAGX_DistributedSpawnCallback::CreateLambda(
							 [&](const auto& Completed)
							 {
								 ++CancelCompletions;
								 Result = Completed;
							 }));
			A->Tick(0.0f);
			A->GetSpawnResult(Cancel, Result);
			TestEqual(
				TEXT("Cancellation starts during registration"), Result.State,
				EAGX_DistributedSpawnState::Registering);
			A->ReleaseSpawn(Cancel);
			TestEqual(TEXT("Cancelled once"), CancelCompletions, 1);
			TestEqual(
				TEXT("Cancellation result"), Result.State, EAGX_DistributedSpawnState::Cancelled);
			// Reuse the same catalog registration, but only spawn this new instance key.
			Options.ObjectKey = TEXT("after-cancel");
			const FGuid AfterCancel = A->QueueSpawn(Options);
			TestTrue(
				TEXT("Another request survives shared registration cancellation"),
				Wait(
					[&]
					{
						return A->GetSpawnResult(AfterCancel, Result) &&
							   Result.State == EAGX_DistributedSpawnState::Succeeded;
					}));
			TestNull(
				TEXT("Cancelled instance never spawned locally"),
				A->FindWorldObjectActor(TEXT("cancel-during-registration")));
			TestNull(
				TEXT("Cancelled instance never spawned remotely"),
				B->FindWorldObjectActor(TEXT("cancel-during-registration")));

			Options.Asset = TimedOutAsset;
			Options.ObjectKey = TEXT("timeout-during-registration");
			Options.TimeoutSeconds = 0.1;
			int32 TimeoutCompletions = 0;
			const FGuid Timeout = A->QueueSpawn(
				Options, FAGX_DistributedSpawnCallback::CreateLambda(
							 [&](const auto& Completed)
							 {
								 ++TimeoutCompletions;
								 Result = Completed;
							 }));
			A->Tick(0.0f);
			A->GetSpawnResult(Timeout, Result);
			TestEqual(
				TEXT("Timeout starts during registration"), Result.State,
				EAGX_DistributedSpawnState::Registering);
			FPlatformProcess::Sleep(0.15f);
			AddExpectedError(
				TEXT("Distributed spawn failed: Timed out waiting"),
				EAutomationExpectedErrorFlags::Contains, 1);
			A->Tick(0.0f);
			TestEqual(TEXT("Timeout completes once"), TimeoutCompletions, 1);
			TestEqual(
				TEXT("Registration counts toward spawn deadline"), Result.State,
				EAGX_DistributedSpawnState::Failed);
			TestNull(
				TEXT("Expired request never creates an Actor"),
				B->FindWorldObjectActor(TEXT("timeout-during-registration")));

			auto* WrongType = NewObject<UAGX_DistributedAsset>(Scenario);
			WrongType->AssetPath = Car->AssetPath;
			WrongType->ActorClass = Car->ActorClass;
			WrongType->AssetType = EAGX_DistributedAssetType::Dynamic;
			Options.Asset = WrongType;
			Options.ObjectKey = TEXT("wrong-category");
			Options.TimeoutSeconds = 5.0;
			bool bRejected = false;
			AddExpectedError(
				TEXT("Distributed spawn failed: Could not register asset 'Car.openplx'"),
				EAutomationExpectedErrorFlags::Contains, 1);
			A->QueueSpawn(
				Options, FAGX_DistributedSpawnCallback::CreateLambda(
							 [&](const auto& Completed)
							 {
								 bRejected =
									 Completed.State == EAGX_DistributedSpawnState::Failed &&
									 Completed.ErrorMessage.Contains(TEXT("different type"));
							 }));
			TestTrue(
				TEXT("Category conflict fails through spawn completion"),
				Wait([&] { return bRejected; }));
			return true;
		},
		Error, true);
	if (!bSuccess)
		AddError(Error.IsEmpty() ? TEXT("Lazy spawn fixture failed.") : Error);
	return bSuccess;
}
#endif
