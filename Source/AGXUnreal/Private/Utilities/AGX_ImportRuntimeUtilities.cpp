// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ImportRuntimeUtilities.h"

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportContext.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ShapeMaterialBarrier.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/MetaData.h"

void FAGX_ImportRuntimeUtilities::WriteSessionGuid(
	UActorComponent& Component, const FGuid& SessionGuid)
{
	Component.ComponentTags.Empty();
	Component.ComponentTags.Add(*SessionGuid.ToString());
}

void FAGX_ImportRuntimeUtilities::WriteSessionGuidToAssetType(
	UObject& Object, const FGuid& SessionGuid)
{
#if WITH_EDITOR
	if (auto MetaData = Object.GetOutermost()->GetMetaData())
		MetaData->SetValue(&Object, TEXT("AGX_ImportSessionGuid"), *SessionGuid.ToString());
#endif
}

void FAGX_ImportRuntimeUtilities::OnComponentCreated(
	UActorComponent& Component, AActor& Owner, const FGuid& SessionGuid)
{
	WriteSessionGuid(Component, SessionGuid);
	Component.SetFlags(RF_Transactional);
	Owner.AddInstanceComponent(&Component);
}

void FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(UObject& Object, const FGuid& SessionGuid)
{
	WriteSessionGuidToAssetType(Object, SessionGuid);
}

UAGX_ShapeMaterial* FAGX_ImportRuntimeUtilities::GetOrCreateShapeMaterial(
	const FShapeMaterialBarrier& Barrier, FAGX_ImportContext* Context)
{
	if (!Barrier.HasNative())
		return nullptr;

	if (Context != nullptr && Context->ShapeMaterials != nullptr)
	{
		if (auto Existing = Context->ShapeMaterials->FindRef(Barrier.GetGuid()))
			return Existing;
	}
	
	UObject* Outer = GetTransientPackage();
	if (Context != nullptr)
		Outer = Context->Outer;

	auto Sm = NewObject<UAGX_ShapeMaterial>(Outer, NAME_None, RF_Public | RF_Standalone);
	Sm->CopyFrom(Barrier, Context);

	if (Context != nullptr && Context->ShapeMaterials != nullptr)
	{
		OnAssetTypeCreated(*Sm, Context->SessionGuid);
		Context->ShapeMaterials->Add(Barrier.GetGuid(), Sm);
	}

	return Sm;
}
