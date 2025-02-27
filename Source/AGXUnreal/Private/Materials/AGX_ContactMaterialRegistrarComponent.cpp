// Copyright 2024, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialRegistrarComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "UAGX_ContactMaterialRegistrarComponent"

UAGX_ContactMaterialRegistrarComponent::UAGX_ContactMaterialRegistrarComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_ContactMaterialRegistrarComponent::RemoveContactMaterial(
	UAGX_ContactMaterial* ContactMaterial)
{
	if (ContactMaterial == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World != nullptr && World->IsGameWorld())
	{
		// We assume that the ContactMaterials TArray is filled only with Instances (not Assets).
		UAGX_ContactMaterial* Instance = ContactMaterial->GetInstance();
		if (Instance == nullptr)
		{
			return;
		}
		ContactMaterials.Remove(Instance);
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			Sim->Unregister(*Instance);
		}
	}
	else
	{
		// We assume that the ContactMaterials TArray is filled only with Assets (not Instances).
		if (ContactMaterial->GetAsset() == nullptr)
		{
			return;
		}
		ContactMaterials.Remove(ContactMaterial->GetAsset());
	}
}

void UAGX_ContactMaterialRegistrarComponent::AddContactMaterial(
	UAGX_ContactMaterial* ContactMaterial)
{
	if (ContactMaterial == nullptr)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World != nullptr && World->IsGameWorld())
	{
		// We assume that the ContactMaterials TArray is filled only with Instances (not Assets).
		UAGX_ContactMaterial* Instance = ContactMaterial->GetOrCreateInstance(*this);
		if (Instance == nullptr)
		{
			return;
		}

		ContactMaterials.Add(Instance);
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			Sim->Register(*Instance);
		}
	}
	else
	{
		// We assume that the ContactMaterials TArray is filled only with Assets (not Instances).
		UAGX_ContactMaterial* Asset = ContactMaterial->GetAsset();
		if (Asset == nullptr)
		{
			return;
		}
		ContactMaterials.Add(Asset);
	}
}

const TArray<UAGX_ContactMaterial*>& UAGX_ContactMaterialRegistrarComponent::GetContactMaterials()
	const
{
	return ContactMaterials;
}

namespace AGX_ContactMaterialRegistrarComponent_helpers
{
	UAGX_ContactMaterial* GetOrCreateContactMaterial(
		const FContactMaterialBarrier& Barrier, FAGX_ImportContext& Context)
	{
		AGX_CHECK(Context.ContactMaterials != nullptr);
		if (auto Existing = Context.ContactMaterials->FindRef(Barrier.GetGuid()))
			return Existing;

		auto Cm = NewObject<UAGX_ContactMaterial>(
			GetTransientPackage(), NAME_None, RF_Public | RF_Standalone);
		FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(*Cm, Context.SessionGuid);
		Cm->CopyFrom(Barrier, &Context);

		if (Cm->Material1 == nullptr || Cm->Material2 == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("No Materials set for Contact Material '%s' in "
					 "AGX_ContactMaterialRegistrarComponent_helpers::GetOrCreateContactMaterial. "
					 "The Contact Material will not be added."), *Cm->GetName());
			return nullptr;
		}

		Context.ContactMaterials->Add(Barrier.GetGuid(), Cm);
		return Cm;
	}
}

void UAGX_ContactMaterialRegistrarComponent::CopyFrom(
	const TArray<FContactMaterialBarrier>& Barriers, FAGX_ImportContext* Context)
{
	using namespace AGX_ContactMaterialRegistrarComponent_helpers;
	if (Context == nullptr || Context->ContactMaterials == nullptr)
		return;

	for (const auto& Barrier : Barriers)
	{
		AGX_CHECK(Barrier.HasNative());
		if (!Barrier.HasNative())
			continue;

		if (auto Cm = GetOrCreateContactMaterial(Barrier, *Context))
			ContactMaterials.Add(Cm);
	}
	
	AGX_CHECK(Context->ContactMaterialRegistrar == nullptr);
	Context->ContactMaterialRegistrar = this;
}

void UAGX_ContactMaterialRegistrarComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
	{
		return;
	}

	// Note: BeginPlay is called on Component copy as well if the Component is copied during play.

	// Convert all contact material pointers to point to initialized contact material instances.
	for (UAGX_ContactMaterial*& ContactMaterial : ContactMaterials)
	{
		if (!ContactMaterial)
		{
			continue;
		}

		if (!ContactMaterial->Material1 || !ContactMaterial->Material2)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Contact Material '%s' has at least one material that has not been set."),
				*ContactMaterial->GetName());

			ContactMaterial = nullptr;
			continue;
		}

		UAGX_ContactMaterial* Instance = ContactMaterial->GetOrCreateInstance(*this);
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not create a Contact Material Instance for Contact Material '%s'."),
				*ContactMaterial->GetName());

			ContactMaterial = nullptr;
			return;
		}

		// The Contact Material assets in the ContactMaterials TArray are swapped to Instances
		// during play.
		ContactMaterial = Instance;

		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		{
			Sim->Register(*Instance);
		}
	}
}

void UAGX_ContactMaterialRegistrarComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances)
	{
		return;
	}

	ClearAll();
}

void UAGX_ContactMaterialRegistrarComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (GIsReconstructingBlueprintInstances)
	{
		return;
	}

	// Note: EndPlay is not always called, for example when deleting the Component from the Details
	// Panel during play. Therefore we call ClearAll from here also.
	ClearAll();
}

void UAGX_ContactMaterialRegistrarComponent::ClearAll()
{
	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		for (UAGX_ContactMaterial* ContactMaterial : ContactMaterials)
		{
			if (!ContactMaterial)
			{
				continue;
			}

			if (UAGX_ContactMaterial* Instance = ContactMaterial->GetInstance())
			{
				if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(GetWorld()))
				{
					Sim->Unregister(*Instance);
				}
			}
		}
	}

	ContactMaterials.Empty();
}

#if WITH_EDITOR
bool UAGX_ContactMaterialRegistrarComponent::CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	const UProperty* InProperty
#else
	const FProperty* InProperty
#endif
) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	// Editing the Contact Materials array from the Details Panel during play is not supported.
	// This is because it has proven difficult to keep track of removed array elements since it
	// is too late when PostEditChangeProperty is called (the element is already removed).
	// Elements can still be added/removed during play by using the Add/Remove UFUNCTIONS.
	if (InProperty->GetFName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_ContactMaterialRegistrarComponent, ContactMaterials)))
	{
		UWorld* World = GetWorld();
		return World == nullptr || !World->IsGameWorld();
	}

	return SuperCanEditChange;
}
#endif

#undef LOCTEXT_NAMESPACE
