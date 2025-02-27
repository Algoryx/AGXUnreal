// Copyright 2024, Algoryx Simulation AB.

#include "Import/AGX_Importer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_ObserverFrameComponent.h"
#include "AGX_RigidBodyComponent.h"
#include "CollisionGroups/AGX_CollisionGroupDisablerComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Constraints/AnyConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Import/AGX_ImportSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Import/AGXSimObjectsReader.h"
#include "Import/SimulationObjectCollection.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Terrain/AGX_ShovelComponent.h"
#include "Terrain/ShovelBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/TrackBarrier.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "UObject/Package.h"

namespace AGX_Importer_helpers
{
	AActor* CreateActor(const FString& Name, const FGuid& SessionGuid)
	{
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to create an Actor during import, got an invalid empty name."));
			return nullptr;
		}

		AActor* NewActor = NewObject<AActor>(GetTransientPackage(), *Name);
		if (!NewActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create new actor during import."));
			return nullptr;
		}

		auto Root = NewObject<USceneComponent>(NewActor, FName(TEXT("DefaultSceneRoot")));
		FAGX_ImportRuntimeUtilities::OnComponentCreated(*Root, *NewActor, SessionGuid);
		NewActor->SetRootComponent(Root);

		return NewActor;
	}

	bool CreateSimulationObjectCollection(
		const FAGX_ImportSettings& Settings, FSimulationObjectCollection& OutSimObjects)
	{
		bool Result = false;
		if (Settings.ImportType == EAGX_ImportType::Agx)
		{
			if (FAGXSimObjectsReader::ReadAGXArchive(Settings.FilePath, OutSimObjects))
				Result = true;
		}
		else if (Settings.ImportType == EAGX_ImportType::Urdf)
		{
			if (FAGXSimObjectsReader::ReadUrdf(
					Settings.FilePath, Settings.UrdfPackagePath, Settings.UrdfInitialJoints, OutSimObjects))
			{
				Result = true;
			}
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unsupported import type for file: '%s'. Import will not be possible."),
				*Settings.FilePath);
		}

		if (!Result)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to import file '%s'. Log category LogAGX in the "
					 "Output Log may contain more information."),
				*Settings.FilePath);
		}

		return Result;
	}

	FString GetModelName(const FString& FilePath)
	{
		FString Name =
			FAGX_ObjectUtilities::SanitizeObjectName(FPaths::GetBaseFilename(FilePath), nullptr);
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Unable to generate a valid model name from file '%s'."),
				*FilePath);
			return "";
		}

		return Name;
	}

	template <typename T>
	auto& GetComponentsMapFrom(FAGX_ImportContext& Context)
	{
		if constexpr (std::is_same_v<T, UAGX_RigidBodyComponent>)
			return *Context.RigidBodies.Get();

		if constexpr (std::is_base_of_v<UAGX_ShapeComponent, T>)
			return *Context.Shapes.Get();

		if constexpr (std::is_base_of_v<UAGX_ConstraintComponent, T>)
			return *Context.Constraints.Get();

		if constexpr (std::is_base_of_v<UAGX_TwoBodyTireComponent, T>)
			return *Context.Tires.Get();

		if constexpr (std::is_base_of_v<UAGX_ShovelComponent, T>)
			return *Context.Shovels.Get();

		if constexpr (std::is_base_of_v<UAGX_WireComponent, T>)
			return *Context.Wires.Get();

		if constexpr (std::is_base_of_v<UAGX_TrackComponent, T>)
			return *Context.Tracks.Get();

		// Unsupported types will yield compile errors.
	}

	template <typename TBarrier>
	USceneComponent* GetOwningRigidBodyOrRoot(
		const TBarrier& Barrier, const FAGX_ImportContext& Context, const AActor& Actor)
	{
		FRigidBodyBarrier BodyBarrier = Barrier.GetRigidBody();
		if (!BodyBarrier.HasNative())
			return Actor.GetRootComponent();

		UAGX_RigidBodyComponent* Body = Context.RigidBodies->FindRef(BodyBarrier.GetGuid());
		check(Body != nullptr);
		return Body;
	}
}

FAGX_Importer::FAGX_Importer()
{
	Context.RigidBodies = MakeUnique<decltype(FAGX_ImportContext::RigidBodies)::ElementType>();
	Context.Shapes = MakeUnique<decltype(FAGX_ImportContext::Shapes)::ElementType>();
	Context.Constraints = MakeUnique<decltype(FAGX_ImportContext::Constraints)::ElementType>();
	Context.Tires = MakeUnique<decltype(FAGX_ImportContext::Tires)::ElementType>();
	Context.Shovels = MakeUnique<decltype(FAGX_ImportContext::Shovels)::ElementType>();
	Context.Wires = MakeUnique<decltype(FAGX_ImportContext::Wires)::ElementType>();
	Context.Tracks = MakeUnique<decltype(FAGX_ImportContext::Tracks)::ElementType>();
	Context.ObserverFrames = MakeUnique<TMap<FGuid, UAGX_ObserverFrameComponent*>>();
	Context.RenderStaticMeshCom = MakeUnique<TMap<FGuid, UStaticMeshComponent*>>();
	Context.CollisionStaticMeshCom = MakeUnique<TMap<FGuid, UStaticMeshComponent*>>();
	Context.RenderMaterials = MakeUnique<TMap<FGuid, UMaterialInterface*>>();
	Context.RenderStaticMeshes = MakeUnique<TMap<FGuid, UStaticMesh*>>();
	Context.CollisionStaticMeshes = MakeUnique<TMap<FGuid, UStaticMesh*>>();
	Context.MSThresholds = MakeUnique<decltype(FAGX_ImportContext::MSThresholds)::ElementType>();
	Context.ShapeMaterials = MakeUnique<TMap<FGuid, UAGX_ShapeMaterial*>>();
	Context.ContactMaterials = MakeUnique<TMap<FGuid, UAGX_ContactMaterial*>>();
	Context.ShovelProperties = MakeUnique<TMap<FGuid, UAGX_ShovelProperties*>>();
	Context.TrackProperties = MakeUnique<TMap<FGuid, UAGX_TrackProperties*>>();
	Context.TrackMergeProperties = MakeUnique<TMap<FGuid, UAGX_TrackInternalMergeProperties*>>();
}

FAGX_ImportResult FAGX_Importer::Import(const FAGX_ImportSettings& Settings)
{
	using namespace AGX_Importer_helpers;

	Context.Settings = &Settings;
	Context.SessionGuid = FGuid::NewGuid();

	const FString Name = GetModelName(Settings.FilePath);
	if (Name.IsEmpty())
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	AActor* Actor = CreateActor(Name, Context.SessionGuid);
	if (Actor == nullptr)
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	FSimulationObjectCollection SimObjects;
	if (!CreateSimulationObjectCollection(Settings, SimObjects))
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	EAGX_ImportResult Result = AddComponents(SimObjects, *Actor);
	if (IsUnrecoverableError(Result))
		return FAGX_ImportResult(Result);

	return FAGX_ImportResult(Result, Actor, &Context);
}

const FAGX_ImportContext& FAGX_Importer::GetContext() const
{
	return Context;
}

template <typename TComponent, typename TBarrier>
EAGX_ImportResult FAGX_Importer::AddComponent(
	const TBarrier& Barrier, USceneComponent& Parent, AActor& OutActor)
{
	AGX_CHECK(Barrier.HasNative());
	if (!Barrier.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddComponent called on given a barrier that does not have a "
				 "Native. The Component will not be created."));
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	const FGuid Guid = Barrier.GetGuid();
	AGX_CHECK(
		AGX_Importer_helpers::GetComponentsMapFrom<TComponent>(Context).FindRef(Guid) == nullptr);

	TComponent* Component = NewObject<TComponent>(&OutActor);
	FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, OutActor, Context.SessionGuid);
	Component->CopyFrom(Barrier, &Context);

	if constexpr (std::is_base_of_v<USceneComponent, TComponent>)
		Component->AttachToComponent(&Parent, FAttachmentTransformRules::KeepRelativeTransform);

	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddModelSourceComponent(AActor& Owner)
{
	const FString Name = "AGX_ModelSource";
	if (Context.ModelSourceComponent != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddModelSourceComponent called, but a ModelSourceComponent has "
				 "already been added."));
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	UAGX_ModelSourceComponent* Component = NewObject<UAGX_ModelSourceComponent>(&Owner);
	Component->Rename(*Name);

	/*
	 * The Model Source Component cannot be filled here since it relies on things like
	 * asset paths to render materials of the import. Therefore, the Component is only
	 * created and prepared, and any high-level importer using this importer
	 * needs to fill in the data if it is wanted.
	 */

	FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, Owner, Context.SessionGuid);
	AGX_CHECK(Context.ModelSourceComponent == nullptr);
	Context.ModelSourceComponent = Component;
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddContactMaterialRegistrarComponent(
	const FSimulationObjectCollection& SimObjects, AActor& OutActor)
{
	const FString Name = "AGX_ContactMaterialRegistrar";
	if (Context.ContactMaterialRegistrar != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddContactMaterialRegistrarComponent called, but a "
				 "ContactMaterialRegistrarComponent has already been added."));
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	auto Component = NewObject<UAGX_ContactMaterialRegistrarComponent>(&OutActor);
	Component->Rename(*Name);
	Component->CopyFrom(SimObjects.GetContactMaterials(), &Context);
	FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, OutActor, Context.SessionGuid);
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddCollisionGroupDisablerComponent(
	const FSimulationObjectCollection& SimObjects, AActor& OutActor)
{
	const FString Name = "AGX_CollisionGroupDisabler";
	if (Context.CollisionGroupDisabler != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddCollisionGroupDisablerComponent called, but a "
				 "CollisionGroupDisablerComponent has already been added."));
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	auto Component = NewObject<UAGX_CollisionGroupDisablerComponent>(&OutActor);
	Component->Rename(*Name);
	Component->CopyFrom(SimObjects.GetDisabledCollisionGroups(), &Context);

	FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, OutActor, Context.SessionGuid);
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddObserverFrame(
	const FObserverFrameData& Frame, const FSimulationObjectCollection& SimObjects,
	AActor& OutActor)
{
	auto Parent = Context.RigidBodies->FindRef(Frame.BodyGuid);
	if (Parent == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddObserverFrame called for Observer Frame '%s', but the "
				 "owning Rigid Body could not be found. The Observer Frame will not be imported."),
			*Frame.Name);
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	auto Component = NewObject<UAGX_ObserverFrameComponent>(&OutActor);
	Component->CopyFrom(Frame, &Context);
	FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, OutActor, Context.SessionGuid);
	Component->AttachToComponent(Parent, FAttachmentTransformRules::KeepRelativeTransform);
	return EAGX_ImportResult::Success;
}

EAGX_ImportResult FAGX_Importer::AddComponents(
	const FSimulationObjectCollection& SimObjects, AActor& OutActor)
{
	using namespace AGX_Importer_helpers;
	EAGX_ImportResult Res = EAGX_ImportResult::Success;
	USceneComponent* Root = OutActor.GetRootComponent();
	check(Root != nullptr);

	for (const auto& Body : SimObjects.GetRigidBodies())
		Res |= AddComponent<UAGX_RigidBodyComponent, FRigidBodyBarrier>(Body, *Root, OutActor);

	for (const auto& Shape : SimObjects.GetBoxShapes())
		Res |= AddShape<UAGX_BoxShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetCapsuleShapes())
		Res |= AddShape<UAGX_CapsuleShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetCylinderShapes())
		Res |= AddShape<UAGX_CylinderShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetSphereShapes())
		Res |= AddShape<UAGX_SphereShapeComponent>(Shape, OutActor);

	for (const auto& Shape : SimObjects.GetTrimeshShapes())
		Res |= AddTrimeshShape(Shape, OutActor);

	for (const auto& C : SimObjects.GetBallConstraints())
		Res |= AddComponent<UAGX_BallConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);

	for (const auto& C : SimObjects.GetCylindricalConstraints())
		Res |= AddComponent<UAGX_CylindricalConstraintComponent, FConstraintBarrier>(
			C, *Root, OutActor);

	for (const auto& C : SimObjects.GetDistanceConstraints())
		Res |=
			AddComponent<UAGX_DistanceConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);

	for (const auto& C : SimObjects.GetHingeConstraints())
		Res |= AddComponent<UAGX_HingeConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);

	for (const auto& C : SimObjects.GetLockConstraints())
		Res |= AddComponent<UAGX_LockConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);

	for (const auto& C : SimObjects.GetPrismaticConstraints())
		Res |=
			AddComponent<UAGX_PrismaticConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);

	for (const auto& Tire : SimObjects.GetTwoBodyTires())
		Res |= AddComponent<UAGX_TwoBodyTireComponent, FTwoBodyTireBarrier>(Tire, *Root, OutActor);

	for (const auto& Shovel : SimObjects.GetShovels())
		Res |= AddShovel(Shovel, OutActor);

	for (const auto& Wire : SimObjects.GetWires())
		Res |= AddComponent<UAGX_WireComponent, FWireBarrier>(Wire, *Root, OutActor);

	for (const auto& Track : SimObjects.GetTracks())
		Res |= AddComponent<UAGX_TrackComponent, FTrackBarrier>(Track, *Root, OutActor);

	if (SimObjects.GetContactMaterials().Num() > 0)
		Res |= AddContactMaterialRegistrarComponent(SimObjects, OutActor);

	if (SimObjects.GetDisabledCollisionGroups().Num() > 0)
		Res |= AddCollisionGroupDisablerComponent(SimObjects, OutActor);

	for (const auto& Frame : SimObjects.GetObserverFrames())
		Res |= AddObserverFrame(Frame, SimObjects, OutActor);

	Res |= AddModelSourceComponent(OutActor);

	return Res;
}

template <typename TShapeComponent>
EAGX_ImportResult FAGX_Importer::AddShape(const FShapeBarrier& Shape, AActor& OutActor)
{
	using namespace AGX_Importer_helpers;
	auto Parent = GetOwningRigidBodyOrRoot(Shape, Context, OutActor);
	return AddComponent<TShapeComponent, FShapeBarrier>(Shape, *Parent, OutActor);
}

EAGX_ImportResult FAGX_Importer::AddTrimeshShape(const FShapeBarrier& Shape, AActor& OutActor)
{
	auto Result = AddShape<UAGX_TrimeshShapeComponent>(Shape, OutActor);

	if (Context.Settings->bIgnoreDisabledTrimeshes && !Shape.GetEnableCollisions())
	{
		const FGuid Guid = Shape.GetShapeGuid();
		// We don't want to import the Trimesh but we do want to import the Render Data.
		// For simplicity, the have imported the Trimesh as usual above, but will now remove the
		// Trimesh Component. The Trimesh Component will know to not create a collision Mesh, so we
		// don't need to consider that.
		auto Trimesh = Context.Shapes->FindRef(Guid);
		AGX_CHECK(Trimesh != nullptr);
		if (Trimesh == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not find Trimesh '%s' that should have been imported."),
				*Shape.GetName());
			return EAGX_ImportResult::RecoverableErrorsOccured;
		}

		Context.Shapes->Remove(Guid);
		auto Res = FAGX_ObjectUtilities::RemoveComponentAndPromoteChildren(Trimesh, &OutActor);
		AGX_CHECK(Res);
	}

	return Result;
}

EAGX_ImportResult FAGX_Importer::AddShovel(const FShovelBarrier& Shovel, AActor& OutActor)
{
	using namespace AGX_Importer_helpers;
	auto Parent = GetOwningRigidBodyOrRoot(Shovel, Context, OutActor);
	return AddComponent<UAGX_ShovelComponent, FShovelBarrier>(Shovel, *Parent, OutActor);
}
