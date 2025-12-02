// Copyright 2025, Algoryx Simulation AB.

#include "Utilities/AGX_MaterialReplacer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "AssetRegistry/AssetData.h"
#include "Components/MeshComponent.h"
#include "ScopedTransaction.h"
#include "Materials/MaterialInterface.h"

#define LOCTEXT_NAMESPACE "FAGX_MaterialReplacer"

void FAGX_MaterialReplacer::SetCurrent(const FAssetData& AssetData)
{
	CurrentMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
}

void FAGX_MaterialReplacer::SetNew(const FAssetData& AssetData)
{
	NewMaterial = Cast<UMaterialInterface>(AssetData.GetAsset());
}

namespace AGX_MaterialReplacer_helpers
{
	FString GetPathName(TWeakObjectPtr<UMaterialInterface>& Material)
	{
		// Unreal Engine 5.5 got TWeakObjectPtr::Pin, which is used to guarantee that the pointed-to
		// object isn't destroyed while we are using it. On earlier Unreal Engine versions we can
		// only hope.
#if UE_VERSION_OLDER_THAN(5, 5, 0)
		TWeakObjectPtr<UMaterialInterface>& Mat = Material;
#else
		TStrongObjectPtr<UMaterialInterface> Mat = Material.Pin();
#endif
		return Mat.IsValid() ? Mat->GetPathName() : FString();
	}
}

FString FAGX_MaterialReplacer::GetCurrentPathName()
{
	return AGX_MaterialReplacer_helpers::GetPathName(CurrentMaterial);
}

FString FAGX_MaterialReplacer::GetNewPathName()
{
	return AGX_MaterialReplacer_helpers::GetPathName(NewMaterial);
}

namespace AGX_MaterialReplacer_helpers
{
	TArray<UMeshComponent*> GetMeshTemplates(UBlueprint& Blueprint, UClass& BlueprintClass)
	{
		TArray<UMeshComponent*> Templates;
		Templates.Reserve(32);

		// Iterate over the inheritance chain, starting at the given Blueprint.
		for (UBlueprint* Parent = &Blueprint; Parent != nullptr;
			 Parent = FAGX_BlueprintUtilities::GetParent(*Parent))
		{
			// Iterate over all SCS nodes in the Blueprint.
			for (auto Node : Parent->SimpleConstructionScript->GetAllNodes())
			{
				if (Node == nullptr)
					continue;

				// Only interested in Mesh Components.
				UMeshComponent* Mesh = Cast<UMeshComponent>(Node->ComponentTemplate);
				if (Mesh == nullptr)
					continue;

				// We only want to update the given Blueprint, not any parent Blueprints, so find
				// the instance that is owned by that Blueprint.
				Mesh = FAGX_ObjectUtilities::GetMatchedInstance(Mesh, &BlueprintClass);
				if (Mesh == nullptr)
					continue;

				Templates.Add(Mesh);
			}
		}

		return Templates;
	}
}

bool FAGX_MaterialReplacer::ReplaceMaterials(UBlueprint& Blueprint)
{
	using namespace AGX_MaterialReplacer_helpers;

	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return false;
	};

	UClass* BlueprintClass = Blueprint.GeneratedClass;
	if (BlueprintClass == nullptr)
	{
		return Bail(
			TEXT("Material replacing failed because the Blueprint doesn't have a generated class. "
				 "It may help to compile the Blueprint first."));
	}

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
	auto CurrentMaterialPin = CurrentMaterial.Pin();
	auto NewMaterilPin = NewMaterial.Pin();
#endif
	const UMaterialInterface* const CurrentMat = CurrentMaterial.Get();
	UMaterialInterface* const NewMat = NewMaterial.Get();
	// It is OK for the Materials to not be set, i.e. be nullptr. It means that all uses of the
	// default material should be replaced, or that the selected current material should be cleared
	// to the default material.

	FProperty* const Property = UMeshComponent::StaticClass()->FindPropertyByName(
		GET_MEMBER_NAME_CHECKED(UMeshComponent, OverrideMaterials));
	if (Property == nullptr)
	{
		return Bail(
			TEXT("Material replacement could not be completed because Mesh Component does not have "
				 "an Override Materials property."));
	}

	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	TArray<UMeshComponent*> ChangedBlueprintMeshes;
	ChangedBlueprintMeshes.Reserve(32);

	// Iterate over all Mesh templates. Not only those in the current Blueprint but also all Meshes
	// inherited from parent Blueprints.
	for (UMeshComponent* Template : GetMeshTemplates(Blueprint, *BlueprintClass))
	{
		for (int32 MaterialIndex = 0; MaterialIndex < Template->GetNumMaterials(); ++MaterialIndex)
		{
			// Only replace matching Materials.
			if (Template->GetMaterial(MaterialIndex) != CurrentMat)
				continue;

			FEditPropertyChain EditPropertyChain;
			EditPropertyChain.AddHead(Property);
			EditPropertyChain.SetActivePropertyNode(Property);

			// PreEditChange is virtual and overloaded in UObject. However, UActorComponent only
			// overrides one overload of PreEditChange, which hides the other due to C++ overloading
			// rules. To call the overload we need, we must explicitly specify its scope.
			Template->UObject::PreEditChange(EditPropertyChain);
			Template->SetMaterial(MaterialIndex, NewMat);
			ChangedBlueprintMeshes.Add(Template);
			// Post edit change is deferred to after all templates has been updated.
			// Not sure why, but it is what FComponentMaterialCategory::OnMaterialChanged does and
			// was recommended by Unreal Developer Network.
			// https://udn.unrealengine.com/s/question/0D5QP00000j1F1e0AE/how-do-i-implement-undoredo-when-modifying-a-blueprint-from-a-details-panel-customization

			// Update the archetype instances. The Pre and PostEditChange calls are required
			// even when not actually changing anything. Not sure why, but not calling them causes
			// the meshes disappear from the viewport.
			FPropertyChangedEvent PropertyChangedEvent(Property);
			for (UMeshComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Template))
			{
				Instance->PreEditChange(Property);
				if (Instance->GetMaterial(MaterialIndex) == CurrentMat)
				{
					Instance->SetMaterial(MaterialIndex, NewMat);
				}
				Instance->PostEditChangeProperty(PropertyChangedEvent);
			}
		}
	}

	// Call the post-change callback on all modified template Components.
	for (UMeshComponent* Mesh : ChangedBlueprintMeshes)
	{
		FPropertyChangedEvent PropertyChangedEvent(Property, EPropertyChangeType::ValueSet);
		Mesh->PostEditChangeProperty(PropertyChangedEvent);
	}

	return true;
}

bool FAGX_MaterialReplacer::ReplaceMaterials(AActor& Actor)
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return false;
	};

#if !UE_VERSION_OLDER_THAN(5, 5, 0)
	auto CurrentMaterialPin = CurrentMaterial.Pin();
	auto NewMaterilPin = NewMaterial.Pin();
#endif
	const UMaterialInterface* const CurrentMat = CurrentMaterial.Get();
	UMaterialInterface* const NewMat = NewMaterial.Get();
	// It is OK for the Materials to not be set, i.e. be nullptr. It means that all uses of the
	// default material should be replaced, or that the selected current material should be cleared
	// to the default material.

	FProperty* const Property = UMeshComponent::StaticClass()->FindPropertyByName(
		GET_MEMBER_NAME_CHECKED(UMeshComponent, OverrideMaterials));
	if (Property == nullptr)
	{
		return Bail(
			TEXT("Material replacement could not be completed because Mesh Component does not have "
				 "an Override Materials property."));
	}

	FPropertyChangedEvent Event(Property, EPropertyChangeType::ValueSet);

	// If the Actor we are modifying is an instance of a Blueprint then we must defer all Post Edit
	// Change calls so that we can do all our changes before Blueprint Reconstruction happens and
	// destroys all the Components we have pointers to.
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

	auto SetMaterial =
		[NewMat, Property, &Event, bIsBlueprintInstance](UMeshComponent* Mesh, int32 MaterialIndex)
	{
		Mesh->PreEditChange(Property);
		Mesh->SetMaterial(MaterialIndex, NewMat);
		if (!bIsBlueprintInstance)
			Mesh->PostEditChangeProperty(Event);
	};

	TArray<UMeshComponent*> Meshes;
	Actor.GetComponents(Meshes);

	FScopedTransaction Transaction(
		LOCTEXT("ReplaceRenderMaterialsUndo", "Replace Render Materials"));

	for (UMeshComponent* Mesh : Meshes)
	{
		for (int32 MaterialIndex = 0; MaterialIndex < Mesh->GetNumMaterials(); ++MaterialIndex)
		{
			if (Mesh->GetMaterial(MaterialIndex) != CurrentMat)
				continue;

			for (UMeshComponent* Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Mesh))
			{
				if (Instance->GetMaterial(MaterialIndex) == CurrentMat)
				{
					SetMaterial(Instance, MaterialIndex);
				}
			}

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
