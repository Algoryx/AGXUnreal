// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ImportRuntimeUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Import/AGX_ImportContext.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

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

namespace AGX_ImportRuntimeUtilities_helpers
{
	bool AreComponentsMatching(const UActorComponent& Comp1, const UActorComponent& Comp2)
	{
		return Comp1.GetName() == Comp2.GetName() && Comp1.GetClass() == Comp2.GetClass();
	}

	UActorComponent* GetCorrespondingComponent(
		const UActorComponent& ComponentTemplate, AActor& OutActor)
	{
		const FString Name = ComponentTemplate.GetName();
		for (auto Component : OutActor.GetComponents())
		{
			if (AreComponentsMatching(ComponentTemplate, *Component))
				return Component;
		}

		return nullptr;
	}

	UActorComponent* GetOrCreateCorrespondingComponent(
		const UActorComponent& ComponentTemplate, AActor& OutActor)
	{
		if (auto Existing = GetCorrespondingComponent(ComponentTemplate, OutActor))
			return Existing;

		auto NewComponent = NewObject<UActorComponent>(&OutActor, ComponentTemplate.GetClass());
		NewComponent->Rename(*ComponentTemplate.GetName());
		OutActor.AddInstanceComponent(NewComponent);
		return NewComponent;
	}

	void RemoveUnmatchedComponents(const AActor& Template, AActor& OutActor)
	{
		const TSet<UActorComponent*> OldComponents = OutActor.GetComponents();
		for (auto& Comp : OldComponents)
		{
			bool FoundMatching = false;
			for (const auto& TemplateComponent : Template.GetComponents())
			{
				if (AreComponentsMatching(*TemplateComponent, *Comp))
				{
					FoundMatching = true;
					break;
				}
			}

			if (!FoundMatching)
				Comp->DestroyComponent(/*bPromoteChildren*/ true);
		}
	}

	void EnsureEquivalentAttachments(const AActor& Template, AActor& OutActor)
	{
		for (const auto& TemplateComp : Template.GetComponents())
		{
			if (const auto& TemplateSceneComp = Cast<USceneComponent>(TemplateComp))
			{
				auto SceneComponent =
					Cast<USceneComponent>(GetCorrespondingComponent(*TemplateSceneComp, OutActor));
				AGX_CHECK(SceneComponent != nullptr);
				if (SceneComponent == nullptr)
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT("EnsureEquivalentAttachments: Unable to find matching Component '%s' "
							 "in Actor '%s'. Reimport results may not be as expected."),
						*TemplateSceneComp->GetName(), *OutActor.GetName());
					continue;
				}

				const auto TemplateParent = TemplateSceneComp->GetAttachParent();
				if (TemplateParent == nullptr)
					continue; // Will be true for the root component.

				const auto SceneParent = SceneComponent->GetAttachParent();
				if (SceneParent == nullptr || !AreComponentsMatching(*TemplateParent, *SceneParent))
				{
					auto NewSceneParent =
						Cast<USceneComponent>(GetCorrespondingComponent(*TemplateParent, OutActor));
					if (NewSceneParent == nullptr)
					{
						UE_LOG(
							LogAGX, Warning,
							TEXT("EnsureEquivalentAttachments: Unable to find matching Component "
								 "'%s' in Actor '%s'. Reimport results may not be as expected."),
							*TemplateParent->GetName(), *OutActor.GetName());
						continue;
					}

					SceneComponent->DetachFromComponent(
						FDetachmentTransformRules::KeepWorldTransform);
					SceneComponent->AttachToComponent(
						NewSceneParent, FAttachmentTransformRules::KeepWorldTransform);
				}
			}
		}
	}
}

bool FAGX_ImportRuntimeUtilities::Reimport(const AActor& Template, AActor& OutActor)
{
	using namespace AGX_ImportRuntimeUtilities_helpers;

	if (Template.GetComponentByClass<UAGX_ModelSourceComponent>() == nullptr ||
		OutActor.GetComponentByClass<UAGX_ModelSourceComponent>() == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("One of the Actors passed to FAGX_ImportRuntimeUtilities::Reimport is not an "
				 "Actor created from Import. Reimport aborted."));
		return false;
	}

	RemoveUnmatchedComponents(Template, OutActor);

	for (const auto& TemplateComponent : Template.GetComponents())
	{
		auto Comp = GetOrCreateCorrespondingComponent(*TemplateComponent, OutActor);
		AGX_CHECK(Comp != nullptr);
		if (Comp != nullptr)
			FAGX_ObjectUtilities::CopyProperties(*TemplateComponent, *Comp);
	}

	EnsureEquivalentAttachments(Template, OutActor);

	for (auto Comp : OutActor.GetComponents())
		Comp->RegisterComponent(); // This will trigger BeginPlay.

	return true;
}

FString FAGX_ImportRuntimeUtilities::GetUnsetUniqueImportName()
{
	return FString("AGX_Import_Unnamed_") + FGuid::NewGuid().ToString();
}
