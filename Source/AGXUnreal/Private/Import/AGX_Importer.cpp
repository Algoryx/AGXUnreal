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
#include "Constraints/AGX_SingleControllerConstraint1DofComponent.h"
#include "Constraints/AnyConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Constraints/SingleControllerConstraint1DOFBarrier.h"
#include "Import/AGX_ImportSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Import/AGXSimObjectsReader.h"
#include "Import/SimulationObjectCollection.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "OpenPLX/PLX_SignalHandlerComponent.h"
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
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/PLXUtilities.h"
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/TrackBarrier.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "UObject/Package.h"

namespace AGX_Importer_helpers
{
	AActor* CreateActor(const FString& Name, const FAGX_ImportContext& Context)
	{
		if (Name.IsEmpty())
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to create an Actor during import, got an invalid empty name."));
			return nullptr;
		}

		const FString UniqueName = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			Context.Outer, Name, AActor::StaticClass());

		AActor* NewActor = NewObject<AActor>(Context.Outer, *UniqueName);
		if (!NewActor)
		{
			UE_LOG(LogAGX, Warning, TEXT("Failed to create new actor during import."));
			return nullptr;
		}

		auto Root = NewObject<USceneComponent>(
			NewActor, USceneComponent::GetDefaultSceneRootVariableName());
		FAGX_ImportRuntimeUtilities::OnComponentCreated(*Root, *NewActor, Context.SessionGuid);
		NewActor->SetRootComponent(Root);

		return NewActor;
	}

	/// Also adds SimpleCollision (Box primitive).
	void BatchBuildStaticMeshes(FAGX_ImportContext& Context)
	{
		TArray<UStaticMesh*> Meshes;
		for (auto M : *Context.RenderStaticMeshes)
			Meshes.Add(M.Value);

		for (auto M : *Context.CollisionStaticMeshes)
			Meshes.Add(M.Value);

		UStaticMesh::BatchBuild(Meshes);
		for (auto M : Meshes)
			AGX_MeshUtilities::AddBoxSimpleCollision(*M);
	}

	bool CreateSimulationObjectCollection(
		const FAGX_ImportSettings& Settings, FSimulationObjectCollection& OutSimObjects)
	{
		auto CheckResult = [&](bool Result) -> bool
		{
			if (!Result)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Unable to import file '%s'. Log category LogAGX in the "
						 "Output Log may contain more information."),
					*Settings.FilePath);
			}

			return Result;
		};

		switch (Settings.ImportType)
		{
			case EAGX_ImportType::Agx:
			{
				return CheckResult(
					FAGXSimObjectsReader::ReadAGXArchive(Settings.FilePath, OutSimObjects));
			}
			case EAGX_ImportType::Urdf:
			{
				return CheckResult(FAGXSimObjectsReader::ReadUrdf(
					Settings.FilePath, Settings.UrdfPackagePath, Settings.UrdfInitialJoints,
					OutSimObjects));
			}
			case EAGX_ImportType::Plx:
			{
				return CheckResult(
					FAGXSimObjectsReader::ReadOpenPLXFile(Settings.FilePath, OutSimObjects));
			}
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("Unsupported import type for file: '%s'. Import will not be possible."),
			*Settings.FilePath);
		return false;
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

	bool CheckFilePath(const FAGX_ImportSettings& Settings)
	{
		if (Settings.ImportType == EAGX_ImportType::Plx)
		{
			if (!Settings.FilePath.StartsWith(FPLXUtilities::GetModelsDirectory()))
			{
				UE_LOG(
					LogAGX, Error, TEXT("OpenPLX file must reside in '%s'."),
					*FPLXUtilities::GetModelsDirectory());
				return false;
			}

			if (Settings.SourceFilePath.StartsWith(FPLXUtilities::GetModelsDirectory()))
			{
				UE_LOG(
					LogAGX, Error,
					TEXT("Original OpenPLX Source File must NOT reside in '%s'. Do not store your original "
						 "OpenPLX models in this directory."),
					*FPLXUtilities::GetModelsDirectory());
				return false;
			}
		}

		if (!FPaths::FileExists(Settings.FilePath))
		{
			UE_LOG(LogAGX, Error, TEXT("File: '%s' does not exist."), *Settings.FilePath);
			return false;
		}

		return true;
	}

	void ConditionallyHideShapes(FAGX_ImportContext& Context)
	{
		// This is the same behavior as for AGXViewer, where if a loaded OpenPLX model has ANY
		// visual geometries, all collision geometries are hidden.
		AGX_CHECK(Context.Settings->ImportType == EAGX_ImportType::Plx);
		if (Context.RenderStaticMeshCom == nullptr || Context.RenderStaticMeshCom->Num() == 0)
			return;

		for (const auto& [Guid, Shape] : *Context.Shapes)
		{
			if (Shape != nullptr)
				Shape->SetVisibility(false, /*bPropagateToChildren*/ false);
		}
	}
}

FAGX_Importer::FAGX_Importer()
{
	Context.RigidBodies = MakeUnique<TMap<FGuid, UAGX_RigidBodyComponent*>>();
	Context.Shapes = MakeUnique<TMap<FGuid, UAGX_ShapeComponent*>>();
	Context.Constraints = MakeUnique<TMap<FGuid, UAGX_ConstraintComponent*>>();
	Context.Tires = MakeUnique<TMap<FGuid, UAGX_TwoBodyTireComponent*>>();
	Context.Shovels = MakeUnique<TMap<FGuid, UAGX_ShovelComponent*>>();
	Context.Wires = MakeUnique<TMap<FGuid, UAGX_WireComponent*>>();
	Context.Tracks = MakeUnique<TMap<FGuid, UAGX_TrackComponent*>>();
	Context.ObserverFrames = MakeUnique<TMap<FGuid, UAGX_ObserverFrameComponent*>>();
	Context.RenderStaticMeshCom = MakeUnique<TMap<FGuid, UStaticMeshComponent*>>();
	Context.CollisionStaticMeshCom = MakeUnique<TMap<FGuid, UStaticMeshComponent*>>();
	Context.RenderMaterials = MakeUnique<TMap<FGuid, UMaterialInterface*>>();
	Context.RenderStaticMeshes = MakeUnique<TMap<FGuid, UStaticMesh*>>();
	Context.CollisionStaticMeshes = MakeUnique<TMap<FGuid, UStaticMesh*>>();
	Context.MSThresholds = MakeUnique<TMap<FGuid, UAGX_MergeSplitThresholdsBase*>>();
	Context.ShapeMaterials = MakeUnique<TMap<FGuid, UAGX_ShapeMaterial*>>();
	Context.ContactMaterials = MakeUnique<TMap<FGuid, UAGX_ContactMaterial*>>();
	Context.ShovelProperties = MakeUnique<TMap<FGuid, UAGX_ShovelProperties*>>();
	Context.TrackProperties = MakeUnique<TMap<FGuid, UAGX_TrackProperties*>>();
	Context.TrackMergeProperties = MakeUnique<TMap<FGuid, UAGX_TrackInternalMergeProperties*>>();
}

FAGX_ImportResult FAGX_Importer::Import(const FAGX_ImportSettings& Settings, UObject& Outer)
{
	using namespace AGX_Importer_helpers;

	if (!CheckFilePath(Settings))
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	Context.Outer = &Outer;
	Context.Settings = &Settings;
	Context.SessionGuid = FGuid::NewGuid();

	const FString Name = GetModelName(Settings.FilePath);
	if (Name.IsEmpty())
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	AActor* Actor = CreateActor(Name, Context);
	if (Actor == nullptr)
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	FSimulationObjectCollection SimObjects;
	if (!CreateSimulationObjectCollection(Settings, SimObjects))
		return FAGX_ImportResult(EAGX_ImportResult::FatalError);

	EAGX_ImportResult Result = AddComponents(Settings, SimObjects, *Actor);
	if (IsUnrecoverableError(Result))
		return FAGX_ImportResult(Result);

#if WITH_EDITOR
	// When WITH_EDITOR, Static Meshes are created but not built during import.
	// That allows us to Batch build them here in paralell.
	// Static Meshes created during Imports in standalone applications (no-editor) are always built
	// when they are created, so no need to Batch build them again here.
	BatchBuildStaticMeshes(Context);
#endif

	PostImport();
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
	Component->FilePath = Context.Settings->FilePath;
	Component->SourceFilePath = Context.Settings->SourceFilePath;
	Component->bRuntimeImport = Context.Settings->bRuntimeImport;
	Component->bIgnoreDisabledTrimeshes = Context.Settings->bIgnoreDisabledTrimeshes;
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
	const FAGX_ImportSettings& Settings, const FSimulationObjectCollection& SimObjects,
	AActor& OutActor)
{
	using namespace AGX_Importer_helpers;
	EAGX_ImportResult Res = EAGX_ImportResult::Success;
	USceneComponent* Root = OutActor.GetRootComponent();
	check(Root != nullptr);

	{
		FScopedSlowTask T((float) SimObjects.GetRigidBodies().Num(), FText::FromString("Bodies"));
		for (const auto& Body : SimObjects.GetRigidBodies())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Body.GetName())));
			Res |= AddComponent<UAGX_RigidBodyComponent, FRigidBodyBarrier>(Body, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T((float) SimObjects.GetBoxShapes().Num(), FText::FromString("Box Shapes"));
		for (const auto& Shape : SimObjects.GetBoxShapes())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Shape.GetName())));
			Res |= AddShape<UAGX_BoxShapeComponent>(Shape, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetCapsuleShapes().Num(), FText::FromString("Capsule Shapes"));
		for (const auto& Shape : SimObjects.GetCapsuleShapes())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Shape.GetName())));
			Res |= AddShape<UAGX_CapsuleShapeComponent>(Shape, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetCylinderShapes().Num(), FText::FromString("Cylinder Shapes"));
		for (const auto& Shape : SimObjects.GetCylinderShapes())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Shape.GetName())));
			Res |= AddShape<UAGX_CylinderShapeComponent>(Shape, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetSphereShapes().Num(), FText::FromString("Sphere Shapes"));
		for (const auto& Shape : SimObjects.GetSphereShapes())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Shape.GetName())));
			Res |= AddShape<UAGX_SphereShapeComponent>(Shape, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetTrimeshShapes().Num(), FText::FromString("Trimesh Shapes"));
		for (const auto& Shape : SimObjects.GetTrimeshShapes())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Shape.GetName())));
			Res |= AddTrimeshShape(Shape, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetBallConstraints().Num(), FText::FromString("Ball Constraints"));
		for (const auto& C : SimObjects.GetBallConstraints())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *C.GetName())));
			Res |=
				AddComponent<UAGX_BallConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetCylindricalConstraints().Num(),
			FText::FromString("Cylindrical Constraints"));
		for (const auto& C : SimObjects.GetCylindricalConstraints())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *C.GetName())));
			Res |= AddComponent<UAGX_CylindricalConstraintComponent, FConstraintBarrier>(
				C, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetDistanceConstraints().Num(),
			FText::FromString("Distance Constraints"));
		for (const auto& C : SimObjects.GetDistanceConstraints())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *C.GetName())));
			Res |= AddComponent<UAGX_DistanceConstraintComponent, FConstraintBarrier>(
				C, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetHingeConstraints().Num(), FText::FromString("Hinge Constraints"));
		for (const auto& C : SimObjects.GetHingeConstraints())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *C.GetName())));
			Res |=
				AddComponent<UAGX_HingeConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetLockConstraints().Num(), FText::FromString("Lock Constraints"));
		for (const auto& C : SimObjects.GetLockConstraints())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *C.GetName())));
			Res |=
				AddComponent<UAGX_LockConstraintComponent, FConstraintBarrier>(C, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetPrismaticConstraints().Num(),
			FText::FromString("Prismatic Constraints"));
		for (const auto& C : SimObjects.GetPrismaticConstraints())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *C.GetName())));
			Res |= AddComponent<UAGX_PrismaticConstraintComponent, FConstraintBarrier>(
				C, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetSingleControllerConstraint1DOFs().Num(),
			FText::FromString("Single Controller Constraint 1DOF Constraints"));
		for (const auto& C : SimObjects.GetSingleControllerConstraint1DOFs())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *C.GetName())));
			Res |= AddComponent<UAGX_SingleControllerConstraint1DofComponent, FConstraintBarrier>(
				C, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetTwoBodyTires().Num(), FText::FromString("TwoBodyTires"));
		for (const auto& Tire : SimObjects.GetTwoBodyTires())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Tire.GetName())));
			Res |=
				AddComponent<UAGX_TwoBodyTireComponent, FTwoBodyTireBarrier>(Tire, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T((float) SimObjects.GetShovels().Num(), FText::FromString("Shovels"));
		for (const auto& Shovel : SimObjects.GetShovels())
		{
			T.EnterProgressFrame(1.f); // Shovel lacks GetName().
			Res |= AddShovel(Shovel, OutActor);
		}
	}

	{
		FScopedSlowTask T((float) SimObjects.GetWires().Num(), FText::FromString("Wires"));
		for (const auto& Wire : SimObjects.GetWires())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Wire.GetName())));
			Res |= AddComponent<UAGX_WireComponent, FWireBarrier>(Wire, *Root, OutActor);
		}
	}

	{
		FScopedSlowTask T((float) SimObjects.GetTracks().Num(), FText::FromString("Tracks"));
		for (const auto& Track : SimObjects.GetTracks())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Track.GetName())));
			Res |= AddComponent<UAGX_TrackComponent, FTrackBarrier>(Track, *Root, OutActor);
		}
	}

	if (SimObjects.GetContactMaterials().Num() > 0)
	{
		Res |= AddContactMaterialRegistrarComponent(SimObjects, OutActor);
	}

	if (SimObjects.GetDisabledCollisionGroups().Num() > 0)
	{
		Res |= AddCollisionGroupDisablerComponent(SimObjects, OutActor);
	}

	{
		FScopedSlowTask T(
			(float) SimObjects.GetObserverFrames().Num(), FText::FromString("ObserverFrames"));
		for (const auto& Frame : SimObjects.GetObserverFrames())
		{
			T.EnterProgressFrame(
				1.f, FText::FromString(FString::Printf(TEXT("Processing: %s"), *Frame.Name)));
			Res |= AddObserverFrame(Frame, SimObjects, OutActor);
		}
	}

	Res |= AddModelSourceComponent(OutActor);

	if (Settings.ImportType == EAGX_ImportType::Plx)
		Res |= AddSignalHandlerComponent(SimObjects, OutActor);

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

EAGX_ImportResult FAGX_Importer::AddSignalHandlerComponent(
	const FSimulationObjectCollection& SimObjects, AActor& OutActor)
{
	const FString Name = "PLX_SignalHandler";
	if (Context.SignalHandler != nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Importer::AddSignalHandlerComponent called, but a "
				 "PLX_SignalHandler has already been added."));
		return EAGX_ImportResult::RecoverableErrorsOccured;
	}

	auto Component = NewObject<UPLX_SignalHandlerComponent>(&OutActor);
	Component->Rename(*Name);
	Component->CopyFrom(SimObjects.GetPLXInputs(), SimObjects.GetPLXOutputs(), &Context);
	FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, OutActor, Context.SessionGuid);
	return EAGX_ImportResult::Success;
}

void FAGX_Importer::PostImport()
{
	if (Context.Settings->ImportType == EAGX_ImportType::Plx)
		AGX_Importer_helpers::ConditionallyHideShapes(Context);
}
