// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedPreview.h"
#include "Distributed/AGX_DistributedAsset.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"

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
