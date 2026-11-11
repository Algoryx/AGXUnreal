// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedGarageWidget.h"
#include "Distributed/AGX_DistributedAsset.h"
#include "Distributed/AGX_DistributedSpawnPoint.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

TSharedRef<SWidget> UAGX_DistributedGarageWidget::RebuildWidget()
{
	if (WidgetTree->RootWidget == nullptr)
	{
		auto* Box = WidgetTree->ConstructWidget<UVerticalBox>();
		WidgetTree->RootWidget = Box;
		auto* Title = WidgetTree->ConstructWidget<UTextBlock>();
		Title->SetText(FText::FromString(TEXT("AGX Distributed Garage")));
		Box->AddChildToVerticalBox(Title);
		AssetPicker = WidgetTree->ConstructWidget<UComboBoxString>(
			UComboBoxString::StaticClass(), TEXT("AssetPicker"));
		Box->AddChildToVerticalBox(AssetPicker);
		LocationPicker = WidgetTree->ConstructWidget<UComboBoxString>(
			UComboBoxString::StaticClass(), TEXT("LocationPicker"));
		Box->AddChildToVerticalBox(LocationPicker);
		auto* Button = WidgetTree->ConstructWidget<UButton>();
		auto* Label = WidgetTree->ConstructWidget<UTextBlock>();
		Label->SetText(FText::FromString(TEXT("Spawn vehicle")));
		Button->AddChild(Label);
		Button->OnClicked.AddDynamic(this, &ThisClass::SpawnSelected);
		Box->AddChildToVerticalBox(Button);
		Status = WidgetTree->ConstructWidget<UTextBlock>();
		Box->AddChildToVerticalBox(Status);
	}
	return Super::RebuildWidget();
}
void UAGX_DistributedGarageWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (AssetPicker == nullptr || LocationPicker == nullptr)
		return;
	AssetPicker->ClearOptions();
	for (const UAGX_DistributedAsset* Asset : Assets)
		AssetPicker->AddOption(Asset != nullptr ? Asset->GetName() : TEXT("Invalid asset"));
	LocationPicker->ClearOptions();
	Locations.Reset();
	for (TActorIterator<AAGX_DistributedSpawnPoint> It(GetWorld()); It; ++It)
		if (!It->bSpawnOnBeginPlay)
		{
			Locations.Add(*It);
			LocationPicker->AddOption(It->GetActorNameOrLabel());
		}
	AssetPicker->SetSelectedIndex(0);
	LocationPicker->SetSelectedIndex(0);
	Status->SetText(FText::FromString(TEXT("Select an asset and garage location.")));
}
void UAGX_DistributedGarageWidget::SpawnSelected()
{
	const int32 AssetIndex = AssetPicker->GetSelectedIndex();
	const int32 LocationIndex = LocationPicker->GetSelectedIndex();
	if (!Assets.IsValidIndex(AssetIndex) || !Locations.IsValidIndex(LocationIndex) ||
		!IsValid(Locations[LocationIndex]))
	{
		Status->SetText(
			FText::FromString(
				TEXT("Choose a valid asset and a spawn point with Spawn On Begin Play disabled.")));
		return;
	}
	FAGX_DistributedSpawnOptions Options;
	Options.Asset = Assets[AssetIndex];
	Options.Transform = Locations[LocationIndex]->GetActorTransform();
	Options.bRequestSimulationAuthority = bRequestSimulationAuthority;
	auto* Subsystem = GetWorld()->GetSubsystem<UAGX_DistributedWorldSubsystem>();
	const FGuid Id = Subsystem->QueueSpawn(
		Options, FAGX_DistributedSpawnCallback::CreateUObject(this, &ThisClass::Completed));
	Status->SetText(
		FText::FromString(
			Id.IsValid() ? TEXT("Queued; waiting for the distributed entity and local Actor...")
						 : TEXT("Could not queue spawn. Check scenario and asset configuration.")));
}
void UAGX_DistributedGarageWidget::Completed(const FAGX_DistributedSpawnResult& Result)
{
	Status->SetText(
		FText::FromString(
			Result.State == EAGX_DistributedSpawnState::Succeeded
				? FString::Printf(
					  TEXT("Created entity %lld."),
					  Result.EntityId)
				: Result.ErrorMessage));
	OnSpawnCompleted.Broadcast(Result);
}
void AAGX_DistributedGarageHUD::BeginPlay()
{
	Super::BeginPlay();
	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	if (Controller == nullptr || !Controller->IsLocalController())
		return;
	Widget = CreateWidget<UAGX_DistributedGarageWidget>(
		Controller, UAGX_DistributedGarageWidget::StaticClass());
	Widget->Assets = Assets;
	Widget->bRequestSimulationAuthority = bRequestSimulationAuthority;
	Widget->AddToViewport();
	Widget->SetPositionInViewport(FVector2D(24, 24));
	Widget->SetDesiredSizeInViewport(FVector2D(460, 180));
	bPreviousCursor = Controller->bShowMouseCursor;
	Controller->bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Controller->SetInputMode(Mode);
}
void AAGX_DistributedGarageHUD::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Widget != nullptr)
	{
		Widget->RemoveFromParent();
		if (APlayerController* Controller = GetWorld()->GetFirstPlayerController())
			Controller->bShowMouseCursor = bPreviousCursor;
	}
	Super::EndPlay(Reason);
}
