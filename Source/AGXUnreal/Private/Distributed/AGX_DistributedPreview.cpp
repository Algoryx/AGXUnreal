// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedPreview.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Distributed/AGX_DistributedAsset.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"

FVector AGX_GetDistributedProximityCenter(const UAGX_DistributedAsset* Asset)
{
	UClass* Class = Asset != nullptr ? Asset->ActorClass.LoadSynchronous() : nullptr;
	if (Class == nullptr || !Class->IsChildOf(AActor::StaticClass()))
		return FVector::ZeroVector;

	struct FComponentTemplate
	{
		const USceneComponent* Component;
		FName Parent;
	};
	TMap<FName, FComponentTemplate> Templates;
	const AActor* Default = Class->GetDefaultObject<AActor>();
	TArray<USceneComponent*> NativeComponents;
	Default->GetComponents(NativeComponents);
	for (const USceneComponent* Component : NativeComponents)
		Templates.Add(
			Component->GetFName(), {Component, Component->GetAttachParent() != nullptr
												   ? Component->GetAttachParent()->GetFName()
												   : NAME_None});

	auto* MostDerived = Cast<UBlueprintGeneratedClass>(Class);
	TFunction<void(USCS_Node*, FName)> AddNode = [&](USCS_Node* Node, FName Parent)
	{
		auto* Component = Cast<USceneComponent>(Node->GetActualComponentTemplate(MostDerived));
		FName ChildParent = Parent;
		if (Component != nullptr)
		{
			ChildParent = Node->GetVariableName();
			Templates.Add(ChildParent, {Component, Parent});
		}
		for (USCS_Node* Child : Node->GetChildNodes())
			AddNode(Child, ChildParent);
	};
	for (UClass* Base = Class; Base != nullptr; Base = Base->GetSuperClass())
		if (auto* Blueprint = Cast<UBlueprintGeneratedClass>(Base))
			if (Blueprint->SimpleConstructionScript != nullptr)
				for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetRootNodes())
				{
					FName Parent = Node->ParentComponentOrVariableName;
					if (Parent.IsNone() && Default->GetRootComponent() != nullptr)
						Parent = Default->GetRootComponent()->GetFName();
					AddNode(Node, Parent);
				}

	// Resolve attachments after collecting all classes, so inherited parents and overrides
	// contribute their complete transforms. The Actor root itself has identity in Actor space.
	TMap<FName, FTransform> Transforms;
	TFunction<FTransform(FName)> Resolve = [&](FName Name)
	{
		if (const FTransform* Cached = Transforms.Find(Name))
			return *Cached;
		const FComponentTemplate* Template = Templates.Find(Name);
		const FTransform Transform =
			Template != nullptr && !Template->Parent.IsNone()
				? Template->Component->GetRelativeTransform() * Resolve(Template->Parent)
				: FTransform::Identity;
		Transforms.Add(Name, Transform);
		return Transform;
	};
	FVector Sum = FVector::ZeroVector;
	int32 Count = 0;
	for (const auto& Entry : Templates)
		if (Entry.Value.Component->IsA<UAGX_RigidBodyComponent>())
		{
			Sum += Resolve(Entry.Key).GetLocation();
			++Count;
		}
	return Count > 0 ? Sum / static_cast<double>(Count) : FVector::ZeroVector;
}

TArray<FAGX_DistributedPreviewMesh> AGX_GetDistributedPreview(const UAGX_DistributedAsset* Asset)
{
	TArray<FAGX_DistributedPreviewMesh> Result;
	if (Asset == nullptr)
		return Result;
	if (Asset->PreviewMesh != nullptr)
	{
		FAGX_DistributedPreviewMesh Preview;
		Preview.Mesh = Asset->PreviewMesh;
		Result.Add(Preview);
		return Result;
	}
	UClass* Class = Asset->ActorClass.LoadSynchronous();
	if (Class == nullptr || !Class->IsChildOf(AActor::StaticClass()))
		return Result;
	auto Add = [&](UStaticMeshComponent* Mesh, const FTransform& Transform)
	{
		if (Mesh == nullptr || Mesh->GetStaticMesh() == nullptr || !Mesh->IsVisible())
			return;
		FAGX_DistributedPreviewMesh Preview;
		Preview.Mesh = Mesh->GetStaticMesh();
		Preview.Transform = Transform;
		for (int32 I = 0; I < Mesh->GetNumMaterials(); ++I)
			Preview.Materials.Add(Mesh->GetMaterial(I));
		Result.Add(MoveTemp(Preview));
	};
	const AActor* Default = Class->GetDefaultObject<AActor>();
	TArray<UStaticMeshComponent*> Meshes;
	Default->GetComponents(Meshes);
	for (auto* Mesh : Meshes)
	{
		FTransform Transform = Mesh->GetRelativeTransform();
		for (auto* Parent = Mesh->GetAttachParent(); Parent != nullptr;
			 Parent = Parent->GetAttachParent())
			Transform *= Parent->GetRelativeTransform();
		Add(Mesh, Transform);
	}
	auto* MostDerived = Cast<UBlueprintGeneratedClass>(Class);
	TFunction<void(USCS_Node*, const FTransform&)> Visit =
		[&](USCS_Node* Node, const FTransform& Parent)
	{
		UActorComponent* Template = MostDerived != nullptr
										? Node->GetActualComponentTemplate(MostDerived)
										: Node->ComponentTemplate.Get();
		auto* Scene = Cast<USceneComponent>(Template);
		const FTransform Local = Scene != nullptr ? Scene->GetRelativeTransform() * Parent : Parent;
		Add(Cast<UStaticMeshComponent>(Template), Local);
		for (USCS_Node* Child : Node->GetChildNodes())
			Visit(Child, Local);
	};
	for (UClass* Base = Class; Base != nullptr; Base = Base->GetSuperClass())
		if (auto* Blueprint = Cast<UBlueprintGeneratedClass>(Base))
			if (Blueprint->SimpleConstructionScript != nullptr)
				for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetRootNodes())
					Visit(Node, FTransform::Identity);
	return Result;
}
