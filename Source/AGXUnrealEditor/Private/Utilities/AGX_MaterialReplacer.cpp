// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_MaterialReplacer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetData.h"
#include "Utilities/AGX_StringUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_MaterialReplacer"

void FAGX_MaterialReplacer::SetCurrent(const FAssetData& AssetData)
{
	CurrentMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
}

void FAGX_MaterialReplacer::SetNew(const FAssetData& AssetData)
{
	NewMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
}

FString FAGX_MaterialReplacer::GetCurrentPathName()
{
	return CurrentMaterial.IsValid() ? CurrentMaterial->GetPathName() : FString();
}

FString FAGX_MaterialReplacer::GetNewPathName()
{
	return NewMaterial.IsValid() ? NewMaterial->GetPathName() : FString();
}

bool FAGX_MaterialReplacer::ReplaceMaterials(UBlueprint& EditBlueprint, IPropertyHandle& PropertyHandle)
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return false;
	};

	UClass* EditedBlueprintClass = EditBlueprint.GeneratedClass;
	if (EditedBlueprintClass == nullptr)
	{
		return Bail(
			TEXT("Material replacing failed because the Bluperint doesn't have a generated class. "
				 "It may help to compile the Blueprint first."));
	}

	const UMaterialInterface* const Current = CurrentMaterial.Get();
	UMaterialInterface* const New = NewMaterial.Get();
	// It is OK for the Materials to not be set, i.e. be nullptr. It means that all uses of the
	// default material should be replaced, or that the selected current material should be cleared
	// to the default material.

	FProperty* const Property = UStaticMeshComponent::StaticClass()->FindPropertyByName(
		GET_MEMBER_NAME_CHECKED(UStaticMeshComponent, OverrideMaterials));
	if (Property == nullptr)
	{
		return Bail(
			TEXT("Material replacement could not be completed because Static Mesh Component does "
				 "not have an Override Materials property."));
	}

	FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);

	auto SetMaterial = [New, Property, &Event](UStaticMeshComponent* Mesh, int32 MaterialIndex)
	{
		Mesh->PreEditChange(Property);
		Mesh->Modify();
		Mesh->SetMaterial(MaterialIndex, New);
		Mesh->PostEditChangeProperty(Event);
	};

	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	PropertyHandle.NotifyPreChange();

	// Iterate over all SCS nodes. Not only those in the child Blueprint but also all from parent
	// Blueprints.
	for (UBlueprint* BlueprintIt = &EditBlueprint; BlueprintIt != nullptr;
		 BlueprintIt = FAGX_BlueprintUtilities::GetParent(*BlueprintIt))
	{
		// Iterate over all SCS nodes in the current Blueprint.
		for (auto Node : BlueprintIt->SimpleConstructionScript->GetAllNodes())
		{
			UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Node->ComponentTemplate);
			if (Mesh == nullptr)
				continue;

			// We only want to update the edit Blueprint, so find the instance that is owned by
			// that Blueprint.
			Mesh = FAGX_ObjectUtilities::GetMatchedInstance(Mesh, EditedBlueprintClass);
			if (Mesh == nullptr)
				continue;

			for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetNumMaterials(); ++MaterialIndex)
			{
				UMaterialInterface* Material = Mesh->GetMaterial(MaterialIndex);
				if (Material != Current)
					continue;

				for (UStaticMeshComponent* Instance :
					 FAGX_ObjectUtilities::GetArchetypeInstances(*Mesh))
				{
					if (Instance->GetMaterial(MaterialIndex) == Current)
					{
						SetMaterial(Instance, MaterialIndex);
					}
				}

				SetMaterial(Mesh, MaterialIndex);
			}
		}
	}

	PropertyHandle.NotifyPostChange(EPropertyChangeType::ValueSet);

	return true;
}

bool FAGX_MaterialReplacer::ReplaceMaterials(AActor& Actor)
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return false;
	};

	const UMaterialInterface* const Current = CurrentMaterial.Get();
	UMaterialInterface* const New = NewMaterial.Get();
	// It is OK for the Materials to not be set, i.e. be nullptr. It means that all uses of the
	// default material should be replaced, or that the selected current material should be cleared
	// to the default material.

	FProperty* const Property = UStaticMeshComponent::StaticClass()->FindPropertyByName(
		GET_MEMBER_NAME_CHECKED(UStaticMeshComponent, OverrideMaterials));
	if (Property == nullptr)
	{
		return Bail(
			TEXT("Material replacement could not be completed because Static Mesh Component does "
				 "not have an Override Materials property."));
	}

	FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);

	// If the Actor we are modifying is an instance of a Blueprint then we cannot
	const bool bIsBlueprintInstance = [&Actor]()
	{
		USceneComponent* Root = Actor.GetRootComponent();
		if (Root == nullptr)
			return false; // Are we sure? Can a Blueprint be without a root Component?
		UObject* RootArchetype = Root->GetArchetype();
		if (RootArchetype == nullptr)
			return false;
		return !Root->IsInBlueprint() && RootArchetype->IsInBlueprint();
	}();

	auto SetMaterial = [New, Property, &Event, bIsBlueprintInstance](UStaticMeshComponent* Mesh, int32 MaterialIndex)
	{
		Mesh->PreEditChange(Property);
		Mesh->SetMaterial(MaterialIndex, New);
		if (!bIsBlueprintInstance)
			Mesh->PostEditChangeProperty(Event);
	};

	TArray<UStaticMeshComponent*> Meshes;
	Actor.GetComponents(Meshes);

	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	for (UStaticMeshComponent* Mesh : Meshes)
	{
		UE_LOG(LogAGX, Warning, TEXT("Found static mesh '%s' at %p."), *GetNameSafe(Mesh), Mesh);
		for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetNumMaterials(); ++MaterialIndex)
		{
			UE_LOG(LogAGX, Warning, TEXT("  Checking material index %d."), MaterialIndex);
			UMaterialInterface* Material = Mesh->GetMaterial(MaterialIndex);
			if (Material != Current)
			{
				UE_LOG(LogAGX, Warning, TEXT("  Wrong material, ignoring."));
				continue;
			}

			UE_LOG(LogAGX, Warning, TEXT("  Updating instances."))
			for (UStaticMeshComponent* Instance :
				 FAGX_ObjectUtilities::GetArchetypeInstances(*Mesh))
			{
				UE_LOG(LogAGX, Warning, TEXT("    Found instance '%s' at %p."), *GetNameSafe(Instance), Instance);
				if (Instance->GetMaterial(MaterialIndex) == Current)
				{
					UE_LOG(LogAGX, Warning, TEXT("    Updating material."));
					SetMaterial(Instance, MaterialIndex);
				}
				else
				{
					UE_LOG(LogAGX, Warning, TEXT("    Wrong material, ignoring."));
				}
			}

			UE_LOG(LogAGX, Warning, TEXT("  Setting material on the source mesh: '%s' at %p."), *GetNameSafe(Mesh), Mesh);
			SetMaterial(Mesh, MaterialIndex);
		}
	}

	if (bIsBlueprintInstance)
	{
		Actor.PostEditChange();
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
