#include "AGX_ArchiveImporter.h"
#include "AGXArchiveReader.h"
#include "AGX_EditorUtilities.h"
#include "RigidBodyBarrier.h"

#include "AGX_RigidBodyComponent.h"
#include "AGX_SphereShapeComponent.h"
#include "AGX_BoxShapeComponent.h"
#include "AGX_TrimeshShapeComponent.h"

#include "Math/Transform.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Internationalization/Text.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealEditorModule"

#include <iostream>


namespace
{
	std::tuple<AActor*, USceneComponent*> InstantiateBody(const FRigidBodyBarrier& Body, UWorld& World)
	{
		UE_LOG(LogTemp, Log, TEXT("Loaded AGX body with name '%s'."), *Body.GetName());
		AActor* NewActor;
		USceneComponent* Root;
		FTransform Transform(Body.GetRotation(), Body.GetPosition());
		std::tie(NewActor, Root) = FAGX_EditorUtilities::CreateEmptyActor(Transform, &World);
		if (NewActor == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create Actor for body '%s'."), *Body.GetName());
			return {nullptr, nullptr};
		}
		if (Root == nullptr)
		{
			/// \todo Do we need to destroy the Actor here?
			UE_LOG(LogTemp, Log, TEXT("Could not create SceneComponent for body '%s'."), *Body.GetName());
			return {nullptr, nullptr};
		}

		NewActor->SetActorLabel(Body.GetName());

		/// \todo For some reason the actor location must be set again after
		/// creating the root SceneComponent, or else the Actor remain at the
		/// origin. I'm assuming we must set rotation as well, but haven't
		/// tested yet.
		NewActor->SetActorLocation(Body.GetPosition());

		UAGX_RigidBodyComponent* NewBody = FAGX_EditorUtilities::CreateRigidBody(NewActor);
		if (NewBody == nullptr)
		{
			UE_LOG(LogTemp, Log, TEXT("Could not create AGX RigidBodyComponenet for body '%s'."), *Body.GetName());
			/// \todo Do we need to destroy the Actor and the RigidBodyComponent here?
			return {nullptr, nullptr};
		}

		NewBody->Rename(TEXT("AGX_RigidBodyComponent"));
		NewBody->Mass = Body.GetMass();
		NewBody->MotionControl = Body.GetMotionControl();

		return {NewActor, Root};
	}


}

AActor* AGX_ArchiveImporter::ImportAGXArchive(const FString& ArchivePath)
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	if (World == nullptr)
	{
		return nullptr;
	}

	// The ImportGroup AActor will contain all objects created while reading
	// the AGX Dynamics archive.
	AActor* ImportGroup;
	USceneComponent* ImportRoot;
	/// \todo Consider placing ImportedRoot at the center if the imported bodies.
	std::tie(ImportGroup, ImportRoot) = FAGX_EditorUtilities::CreateEmptyActor(FTransform::Identity, World);
	if (ImportGroup == nullptr || ImportRoot == nullptr)
	{
		return nullptr;
	}

	FString Filename;
	/// \todo What about platforms that don't use / as a path separator?
	ArchivePath.Split(TEXT("/"), nullptr, &Filename, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	ImportGroup->SetActorLabel(Filename);

	// Archive instantiator that creates sub-objects under RigidBody. Knows how
	// to create various subclasses of AGX_ShapesComponent in Unreal Editor.
	class EditorBody final : public FAGXArchiveBody
	{
	public:
		EditorBody(AActor& InActor, USceneComponent& InRoot, UWorld& InWorld)
			: Actor(InActor), Root(InRoot), World(InWorld)
		{
		}

		virtual void InstantiateSphere(const FSphereShapeBarrier& Sphere) override
		{
			UAGX_SphereShapeComponent* ShapeComponent = FAGX_EditorUtilities::CreateSphereShape(&Actor, &Root);
			ShapeComponent->Radius = Sphere.GetRadius();
			FinalizeShape(ShapeComponent, Sphere);
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Box) override
		{
			UAGX_BoxShapeComponent* ShapeComponent = FAGX_EditorUtilities::CreateBoxShape(&Actor, &Root);
			ShapeComponent->HalfExtent = Box.GetHalfExtents();
			FinalizeShape(ShapeComponent, Box);
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Trimesh) override
		{
			UAGX_TrimeshShapeComponent* ShapeComponent = FAGX_EditorUtilities::CreateTrimeshShape(&Actor, &Root);
			ShapeComponent->MeshSourceLocation = EAGX_TrimeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
			FAGX_EditorUtilities::CreateStaticMesh(&Actor, ShapeComponent, Trimesh);
			FinalizeShape(ShapeComponent, Trimesh);
		}

	private:
		void FinalizeShape(UAGX_ShapeComponent* Component, const FShapeBarrier& Barrier)
		{
			FVector Location;
			FQuat Rotation;
			std::tie(Location, Rotation) = Barrier.GetLocalPositionAndRotation();
			Component->SetRelativeLocationAndRotation(Location, Rotation);
			Component->UpdateVisualMesh();
		}

		AActor& Actor;
		USceneComponent& Root;
		UWorld& World;
	};

	// Archive instantiator that creates top-level objects. Knows how to create
	// UAGX_RigidBodyComponent in Unreal Editor.
	class EditorInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		EditorInstantiator(AActor& InImportedRoot, UWorld& InWorld)
			: ImportedRoot(InImportedRoot), World(InWorld)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			AActor* NewActor;
			USceneComponent* ActorRoot;
			std::tie(NewActor, ActorRoot) = ::InstantiateBody(Barrier, World);
			if (NewActor == nullptr || ActorRoot == nullptr)
			{
				return nullptr;
			}
			NewActor->AttachToActor(&ImportedRoot, FAttachmentTransformRules::KeepWorldTransform);
			return new EditorBody(*NewActor, *ActorRoot, World);
		}
	private:
		AActor& ImportedRoot;
		UWorld& World;
	};


	EditorInstantiator Instantiator(*ImportGroup, *World);
	FAGXArchiveReader::Read(ArchivePath, Instantiator);

	return ImportGroup;
}

#undef LOCTEXT_NAMESPACE