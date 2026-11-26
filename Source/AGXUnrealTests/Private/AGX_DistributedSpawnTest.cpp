// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedAsset.h"
#include "Distributed/AGX_DistributedGarageWidget.h"
#include "Components/ComboBoxString.h"
#include "Distributed/AGX_DistributedPlacementComponent.h"
#include "Distributed/AGX_DistributedSpawnBubbleComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Distributed/AGX_DistributedScenarioAsset.h"
#include "Distributed/AGX_DistributedSettings.h"
#include "Distributed/AGX_DistributedSpawnAsync.h"
#include "Distributed/AGX_DistributedSpawnPoint.h"
#include "Distributed/AGX_DistributedSpawnPointVisualizerComponent.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"
#include "AGX_RigidBodyActor.h"
#include "AGX_RigidBodyComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_Event.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedSpawnQueueTest, "AGXUnreal.Editor.Distributed.SpawnQueue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDistributedSpawnQueueTest::RunTest(const FString& Parameters)
{
	auto* Settings = GetMutableDefault<UAGX_DistributedSettings>();
	const auto Previous = Settings->Scenario;
	auto* Scenario = NewObject<UAGX_DistributedScenarioAsset>();
	auto* Asset = NewObject<UAGX_DistributedAsset>();
	Asset->AssetPath = TEXT("Car.openplx");
	Asset->ActorClass = AActor::StaticClass();
	Scenario->Assets.Add(Asset);
	Settings->Scenario = Scenario;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("DistributedQueueTest"));
	auto* Subsystem = World->GetSubsystem<UAGX_DistributedWorldSubsystem>();
	TestNotNull(TEXT("Game world has distributed subsystem"), Subsystem);
	if (Subsystem != nullptr)
	{
		FAGX_DistributedSpawnOptions Options;
		Options.Asset = Asset;
		int32 Completions = 0;
		FAGX_DistributedSpawnResult Completed;
		const FGuid Id = Subsystem->QueueSpawn(
			Options, FAGX_DistributedSpawnCallback::CreateLambda(
						 [&](const FAGX_DistributedSpawnResult& Result)
						 {
							 ++Completions;
							 Completed = Result;
						 }));
		TestTrue(TEXT("Can queue before connection"), Id.IsValid());
		TestEqual(TEXT("Queue does not complete inline"), Completions, 0);
		Subsystem->Tick(0.0f);
		FAGX_DistributedSpawnResult Pending;
		TestTrue(TEXT("Request is observable"), Subsystem->GetSpawnResult(Id, Pending));
		TestEqual(
			TEXT("Still queued while disconnected"), Pending.State,
			EAGX_DistributedSpawnState::Queued);
		Subsystem->ReleaseSpawn(Id);
		Subsystem->ReleaseSpawn(Id);
		TestEqual(TEXT("Cancellation notifies once"), Completions, 1);
		TestEqual(
			TEXT("Cancellation state"), Completed.State, EAGX_DistributedSpawnState::Cancelled);
		Subsystem->Tick(0.0f);
		TestFalse(TEXT("Cancelled unsent request removed"), Subsystem->GetSpawnResult(Id, Pending));
		auto* Owner = World->SpawnActor<AActor>();
		auto* Bubble = NewObject<UAGX_DistributedSpawnBubbleComponent>(Owner);
		Bubble->RegisterComponent();
		Bubble->SpawnRadius = 100;
		Bubble->ReleaseRadius = 200;
		Subsystem->RegisterSpawnBubble(Bubble);
		TestTrue(
			TEXT("Activate inside bubble"),
			Subsystem->IsInsideSpawnBubble(FVector(50, 0, 0), false));
		TestFalse(
			TEXT("Do not activate in hysteresis band"),
			Subsystem->IsInsideSpawnBubble(FVector(150, 0, 0), false));
		TestTrue(
			TEXT("Keep active in hysteresis band"),
			Subsystem->IsInsideSpawnBubble(FVector(150, 0, 0), true));
		TestFalse(
			TEXT("Release outside bubble"),
			Subsystem->IsInsideSpawnBubble(FVector(250, 0, 0), true));
		Subsystem->UnregisterSpawnBubble(Bubble);
		TestFalse(
			TEXT("Unregistered bubble has no interest"),
			Subsystem->IsInsideSpawnBubble(FVector(50, 0, 0), true));
		Options.Transform.SetScale3D(FVector(2));
		TestFalse(
			TEXT("Reject unsupported scale before submission"),
			Subsystem->QueueSpawn(Options).IsValid());
		Options.Transform = FTransform::Identity;
		Options.Asset = nullptr;
		TestFalse(TEXT("Reject absent descriptor"), Subsystem->QueueSpawn(Options).IsValid());
	}
	World->DestroyWorld(false);
	Settings->Scenario = Previous;
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedSpawnBlueprintTest, "AGXUnreal.Editor.Distributed.SpawnBlueprintNode",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDistributedSpawnBlueprintTest::RunTest(const FString& Parameters)
{
	auto* Blueprint = FKismetEditorUtilities::CreateBlueprint(
		AActor::StaticClass(), GetTransientPackage(),
		MakeUniqueObjectName(
			GetTransientPackage(), UBlueprint::StaticClass(), TEXT("DistributedSpawnNodeTest")),
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	UEdGraph* Graph = Blueprint->UbergraphPages[0];
	auto* Node = NewObject<UK2Node_AsyncAction>(Graph);
	Node->InitializeProxyFromFunction(
		UAGX_DistributedSpawnAsync::StaticClass()->FindFunctionByName(
			TEXT("SpawnDistributedEntity")));
	Graph->AddNode(Node);
	Node->AllocateDefaultPins();
	for (FName Name :
		 {FName("Queued"), FName("Success"), FName("Fail"), FName("QueuedResult"),
		  FName("EntityId"), FName("Entity"), FName("ErrorMessage"), FName("RequestId")})
		TestNotNull(
			*FString::Printf(TEXT("Output pin %s exists"), *Name.ToString()),
			Node->FindPin(Name, EGPD_Output));
	auto* Event = NewObject<UK2Node_Event>(Graph);
	Event->EventReference.SetExternalMember(TEXT("ReceiveBeginPlay"), AActor::StaticClass());
	Event->bOverrideFunction = true;
	Graph->AddNode(Event);
	Event->AllocateDefaultPins();
	TestTrue(
		TEXT("Connect BeginPlay to async request"),
		Graph->GetSchema()->TryCreateConnection(
			Event->FindPin(UEdGraphSchema_K2::PN_Then),
			Node->FindPin(UEdGraphSchema_K2::PN_Execute)));
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	TestTrue(TEXT("Blueprint with async spawn compiles"), Blueprint->Status != BS_Error);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedSpawnIntegrationTest, "AGXUnreal.Editor.Distributed.SpawnIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDistributedSpawnIntegrationTest::RunTest(const FString& Parameters)
{
	if (!FParse::Param(FCommandLine::Get(), TEXT("DistributedIntegration")))
	{
		AddInfo(TEXT(
			"External-manager integration skipped: supply -DistributedIntegration and the test "
			"manager at tcp/127.0.0.1:17589."));
		return true;
	}
	auto* Settings = GetMutableDefault<UAGX_DistributedSettings>();
	const auto Previous = Settings->Scenario;
	auto* Scenario = NewObject<UAGX_DistributedScenarioAsset>();
	auto* Asset = NewObject<UAGX_DistributedAsset>();
	Asset->AssetPath = TEXT("Car.openplx");
	Asset->ActorClass = AAGX_RigidBodyActor::StaticClass();
	Scenario->Assets.Add(Asset);
	Scenario->ZenohEndpoint = TEXT("tcp/127.0.0.1:17589");
	Scenario->WorldNamespace = FGuid::NewGuid().ToString();
	Settings->Scenario = Scenario;
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone(TEXT("DistributedIntegrationWorld"));
	UWorld* World = Instance->GetWorld();
	auto* Subsystem = World->GetSubsystem<UAGX_DistributedWorldSubsystem>();
	FAGX_DistributedSpawnOptions Options;
	Options.Asset = Asset;
	Options.ObjectKey = TEXT("shared-placement");
	Options.Transform = FTransform(FVector(100, 200, 300));
	int32 Completions = 0;
	const FGuid A = Subsystem->QueueSpawn(
		Options, FAGX_DistributedSpawnCallback::CreateLambda([&](const FAGX_DistributedSpawnResult&)
															 { ++Completions; }));
	const FGuid B = Subsystem->QueueSpawn(Options);
	TestTrue(TEXT("Queue before initialization"), A.IsValid() && B.IsValid());
	World->BeginPlay();
	World->SetBegunPlay(true);
	auto Wait = [&](TFunctionRef<bool()> Predicate)
	{
		const double Deadline = FPlatformTime::Seconds() + 15.0;
		while (FPlatformTime::Seconds() < Deadline)
		{
			Subsystem->Tick(0.01f);
			if (Predicate())
				return true;
			FPlatformProcess::Sleep(0.01f);
		}
		return false;
	};
	FAGX_DistributedSpawnResult ResultA, ResultB;
	const bool bReady = Wait(
		[&]
		{
			Subsystem->GetSpawnResult(A, ResultA);
			Subsystem->GetSpawnResult(B, ResultB);
			return ResultA.State == EAGX_DistributedSpawnState::Succeeded &&
				   ResultB.State == EAGX_DistributedSpawnState::Succeeded;
		});
	TestTrue(TEXT("Both requests complete with local Actors"), bReady);
	if (bReady)
	{
		TestEqual(TEXT("Only one distributed entity"), ResultA.EntityId, ResultB.EntityId);
		TestEqual(TEXT("Only one local Actor"), ResultA.Entity.Get(), ResultB.Entity.Get());
		TestEqual(TEXT("Completion callback exactly once"), Completions, 1);
		auto* Entity = Cast<AAGX_RigidBodyActor>(ResultA.Entity);
		TestNotNull(TEXT("Mapped Actor class created"), Entity);
		if (Entity != nullptr)
		{
			TestEqual(
				TEXT("Remote entity starts kinematic"),
				Entity->RigidBodyComponent->GetMotionControl().GetValue(),
				EAGX_MotionControl::MC_KINEMATICS);
			Subsystem->RequestSimulationAuthority(ResultA.EntityId, true);
			TestTrue(
				TEXT("Granted authority restores authored dynamics"),
				Wait(
					[&]
					{
						return Entity->RigidBodyComponent->GetMotionControl() ==
							   EAGX_MotionControl::MC_DYNAMICS;
					}));
			Subsystem->RequestSimulationAuthority(ResultA.EntityId, false);
			TestTrue(
				TEXT("Release switches back to kinematics"),
				Wait(
					[&]
					{
						return Entity->RigidBodyComponent->GetMotionControl() ==
							   EAGX_MotionControl::MC_KINEMATICS;
					}));
		}
		Subsystem->ReleaseSpawn(A);
		for (int32 I = 0; I < 20; ++I)
		{
			Subsystem->Tick(0.01f);
			FPlatformProcess::Sleep(0.01f);
		}
		TestTrue(TEXT("Other local interest keeps Actor alive"), IsValid(ResultB.Entity));
		Subsystem->ReleaseSpawn(B);
		TestTrue(
			TEXT("Last release destroys local entity Actor"),
			Wait([&] { return !IsValid(ResultB.Entity); }));
	}
	Subsystem->ShutdownClient();
	World->EndPlay(EEndPlayReason::Quit);
	World->DestroyWorld(false);
	Instance->Shutdown();
	GEngine->DestroyWorldContext(World);
	Settings->Scenario = Previous;
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedAuthoringTest, "AGXUnreal.Editor.Distributed.Authoring",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDistributedAuthoringTest::RunTest(const FString& Parameters)
{
	UWorld* World =
		UWorld::CreateWorld(EWorldType::Editor, false, TEXT("DistributedAuthoringWorld"));
	auto* Asset = NewObject<UAGX_DistributedAsset>();
	Asset->PreviewMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	Asset->ActorClass = AAGX_RigidBodyActor::StaticClass();
	Asset->AssetPath = TEXT("Car.openplx");
	auto* Marker = World->SpawnActor<AAGX_DistributedSpawnPoint>();
	TestEqual(
		TEXT("Default proximity radius is 600 cm"), Asset->SimulationAuthoritySensorRadius, 600.0);
	auto* Visualizer =
		Marker->FindComponentByClass<UAGX_DistributedSpawnPointVisualizerComponent>();
	TestNotNull(TEXT("Spawn point owns a proximity visualizer"), Visualizer);
	if (Visualizer != nullptr)
		TestTrue(TEXT("Proximity visualization is editor-only"), Visualizer->IsEditorOnly());
	Marker->Asset = Asset;
	Marker->RefreshPreview();
	TArray<UAGX_RigidBodyComponent*> Bodies;
	Marker->GetComponents(Bodies);
	TestEqual(TEXT("Visual twin has no AGX bodies"), Bodies.Num(), 0);
	TArray<UStaticMeshComponent*> Meshes;
	Marker->GetComponents(Meshes);
	TestEqual(TEXT("Visual twin displays supplied preview"), Meshes.Num(), 1);
	if (Meshes.Num() == 1)
		TestEqual(
			TEXT("Preview has no collision"), Meshes[0]->GetCollisionEnabled(),
			ECollisionEnabled::NoCollision);

	auto* VisualBlueprint = FKismetEditorUtilities::CreateBlueprint(
		AAGX_RigidBodyActor::StaticClass(), GetTransientPackage(),
		MakeUniqueObjectName(
			GetTransientPackage(), UBlueprint::StaticClass(), TEXT("DistributedVisualTest")),
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	auto* VisualNode = VisualBlueprint->SimpleConstructionScript->CreateNode(
		UStaticMeshComponent::StaticClass(), TEXT("Visual"));
	auto* VisualTemplate = CastChecked<UStaticMeshComponent>(VisualNode->ComponentTemplate);
	VisualTemplate->SetStaticMesh(Asset->PreviewMesh);
	VisualTemplate->SetRelativeLocation(FVector(25, 50, 75));
	VisualBlueprint->SimpleConstructionScript->AddNode(VisualNode);
	FKismetEditorUtilities::CompileBlueprint(VisualBlueprint);
	Asset->PreviewMesh = nullptr;
	Asset->ActorClass = VisualBlueprint->GeneratedClass;
	Marker->RefreshPreview();
	Marker->GetComponents(Meshes);
	Marker->GetComponents(Bodies);
	TestEqual(TEXT("Template-derived visual twin has no AGX bodies"), Bodies.Num(), 0);
	TestEqual(TEXT("Blueprint static-mesh template is previewed"), Meshes.Num(), 1);
	if (Meshes.Num() == 1)
		TestTrue(
			TEXT("Preview preserves mesh-local placement"),
			Meshes[0]->GetRelativeLocation().Equals(FVector(25, 50, 75)));
	Asset->PreviewMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	auto* Copy = DuplicateObject<AAGX_DistributedSpawnPoint>(
		Marker, World, MakeUniqueObjectName(World, Marker->GetClass()));
	TestTrue(
		TEXT("Normal duplication creates distinct placement identity"),
		Copy->PlacementId != Marker->PlacementId);
	FObjectDuplicationParameters PIEParams(Marker, World);
	PIEParams.DuplicateMode = EDuplicateMode::PIE;
	PIEParams.DestName = MakeUniqueObjectName(World, Marker->GetClass());
	auto* PIECopy = Cast<AAGX_DistributedSpawnPoint>(StaticDuplicateObjectEx(PIEParams));
	TestEqual(
		TEXT("PIE duplication preserves placement identity"), PIECopy->PlacementId,
		Marker->PlacementId);
	auto* Owner = World->SpawnActor<AActor>();
	auto* Instances = NewObject<UInstancedStaticMeshComponent>(Owner, TEXT("SourceInstances"));
	Instances->SetStaticMesh(Asset->PreviewMesh);
	Instances->RegisterComponent();
	const FTransform First(FVector(100, 0, 0)), Second(FVector(200, 0, 0));
	Instances->AddInstance(First);
	Instances->AddInstance(Second);
	auto* Placements = NewObject<UAGX_DistributedPlacementComponent>(Owner);
	Placements->RegisterComponent();
	Placements->Source.SetComponent(Instances);
	Placements->CaptureInstances();
	TestEqual(TEXT("Capture one placement per instance"), Placements->Placements.Num(), 2);
	if (Placements->Placements.Num() == 2)
	{
		const FGuid FirstId = Placements->Placements[0].Id, SecondId = Placements->Placements[1].Id;
		TestTrue(
			TEXT("Each placement has a distinct identity"),
			FirstId.IsValid() && SecondId.IsValid() && FirstId != SecondId);
		Instances->ClearInstances();
		Instances->AddInstance(Second);
		Instances->AddInstance(First);
		Placements->CaptureInstances();
		TestEqual(
			TEXT("Reordered source retains second object's ID"), Placements->Placements[0].Id,
			SecondId);
		TestEqual(
			TEXT("Reordered source retains first object's ID"), Placements->Placements[1].Id,
			FirstId);
	}
	World->DestroyWorld(false);
	return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedProximityCenterTest, "AGXUnreal.Editor.Distributed.ProximityCenter",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDistributedProximityCenterTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Editor, false, TEXT("DistributedCenterWorld"));
	auto* Parent = FKismetEditorUtilities::CreateBlueprint(
		AAGX_RigidBodyActor::StaticClass(), GetTransientPackage(),
		MakeUniqueObjectName(
			GetTransientPackage(), UBlueprint::StaticClass(), TEXT("CenterParent")),
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	auto* Offset = Parent->SimpleConstructionScript->CreateNode(
		USceneComponent::StaticClass(), TEXT("Offset"));
	auto* OffsetTemplate = CastChecked<USceneComponent>(Offset->ComponentTemplate);
	OffsetTemplate->SetRelativeLocation(FVector(-300, 100, 0));
	OffsetTemplate->SetRelativeRotation(FRotator(0, 90, 0));
	Parent->SimpleConstructionScript->AddNode(Offset);
	Offset->SetParent(Parent->GeneratedClass->GetDefaultObject<AActor>()->GetRootComponent());
	auto AddBody = [](USimpleConstructionScript* SCS, const TCHAR* Name, const FVector& Location)
	{
		USCS_Node* Node = SCS->CreateNode(UAGX_RigidBodyComponent::StaticClass(), FName(Name));
		CastChecked<UAGX_RigidBodyComponent>(Node->ComponentTemplate)
			->SetRelativeLocation(Location);
		return Node;
	};
	Offset->AddChildNode(
		AddBody(Parent->SimpleConstructionScript, TEXT("First"), FVector(100, 0, 0)));
	Offset->AddChildNode(
		AddBody(Parent->SimpleConstructionScript, TEXT("Second"), FVector(0, 100, 0)));
	FKismetEditorUtilities::CompileBlueprint(Parent);
	if (Parent->Status != BS_UpToDate)
	{
		AddError(TEXT("Parent Blueprint did not compile."));
		World->DestroyWorld(false);
		return false;
	}

	auto* Child = FKismetEditorUtilities::CreateBlueprint(
		Parent->GeneratedClass, GetTransientPackage(),
		MakeUniqueObjectName(GetTransientPackage(), UBlueprint::StaticClass(), TEXT("CenterChild")),
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	auto* Third = AddBody(Child->SimpleConstructionScript, TEXT("Third"), FVector(0, 0, 300));
	Child->SimpleConstructionScript->AddNode(Third);
	Third->SetParent(Offset);
	FKismetEditorUtilities::CompileBlueprint(Child);
	if (Child->Status != BS_UpToDate)
	{
		AddError(TEXT("Child Blueprint did not compile."));
		World->DestroyWorld(false);
		return false;
	}

	auto* Asset = NewObject<UAGX_DistributedAsset>();
	Asset->ActorClass = Child->GeneratedClass;
	const FTransform Placement(FRotator(0, 37, 0), FVector(1200, -800, 250));
	auto* Marker = World->SpawnActor<AAGX_DistributedSpawnPoint>(
		AAGX_DistributedSpawnPoint::StaticClass(), Placement);
	Marker->Asset = Asset;

	// Four equally weighted origins: native root, two nested bodies, and a child-Blueprint body
	// attached to the inherited, rotated Offset component. Their Actor-space mean is known.
	const FVector Expected = Placement.TransformPosition(FVector(-250, 100, 75));
	TestTrue(
		TEXT("Preview averages complete native and inherited template transforms"),
		Marker->GetProximitySensorCenter().Equals(Expected, 1.e-6));
	AActor* Actual = World->SpawnActor<AActor>(Child->GeneratedClass, Placement);
	if (Actual == nullptr)
	{
		AddError(TEXT("Could not instantiate the model to compare its rigid-body origins."));
		World->DestroyWorld(false);
		return false;
	}
	TArray<UAGX_RigidBodyComponent*> Bodies;
	Actual->GetComponents(Bodies, true);
	TestEqual(TEXT("Fixture has four instantiated bodies"), Bodies.Num(), 4);
	FVector Sum = FVector::ZeroVector;
	for (const auto* Body : Bodies)
	{
		TestFalse(TEXT("Editor inspection creates no native bodies"), Body->HasNative());
		Sum += Body->GetPosition();
	}
	if (!Bodies.IsEmpty())
		TestTrue(
			TEXT("Preview matches mean used by runtime registration"),
			Marker->GetProximitySensorCenter().Equals(
				Sum / static_cast<double>(Bodies.Num()), 1.e-6));

	Asset->ActorClass = AActor::StaticClass();
	TestTrue(
		TEXT("No bodies falls back to spawn origin"),
		Marker->GetProximitySensorCenter().Equals(Placement.GetLocation()));
	Marker->Asset = nullptr;
	TestTrue(
		TEXT("No asset falls back to spawn origin"),
		Marker->GetProximitySensorCenter().Equals(Placement.GetLocation()));
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDistributedGarageWidgetTest, "AGXUnreal.Editor.Distributed.GarageWidget",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDistributedGarageWidgetTest::RunTest(const FString& Parameters)
{
	auto* Instance = NewObject<UGameInstance>(GEngine);
	Instance->InitializeStandalone(TEXT("DistributedGarageWidgetWorld"));
	UWorld* World = Instance->GetWorld();
	auto* Location = World->SpawnActor<AAGX_DistributedSpawnPoint>();
	Location->bSpawnOnBeginPlay = false;
	auto* Widget = CreateWidget<UAGX_DistributedGarageWidget>(
		Instance, UAGX_DistributedGarageWidget::StaticClass());
	Widget->Assets.Add(NewObject<UAGX_DistributedAsset>());
	Widget->TakeWidget();
	auto* AssetPicker = Cast<UComboBoxString>(Widget->GetWidgetFromName(TEXT("AssetPicker")));
	auto* LocationPicker = Cast<UComboBoxString>(Widget->GetWidgetFromName(TEXT("LocationPicker")));
	TestNotNull(TEXT("Garage constructs asset selector"), AssetPicker);
	TestNotNull(TEXT("Garage constructs location selector"), LocationPicker);
	if (AssetPicker != nullptr)
		TestEqual(TEXT("Garage lists configured assets"), AssetPicker->GetOptionCount(), 1);
	if (LocationPicker != nullptr)
		TestEqual(
			TEXT("Garage lists designated spawn locations"), LocationPicker->GetOptionCount(), 1);
	Widget->ReleaseSlateResources(true);
	World->DestroyWorld(false);
	Instance->Shutdown();
	GEngine->DestroyWorldContext(World);
	return true;
}
#endif
