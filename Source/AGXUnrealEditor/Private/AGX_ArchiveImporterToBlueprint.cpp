#include "AGX_ArchiveImporterToBlueprint.h"

// AGXUnreal includes.
#include "AGXArchiveReader.h"
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "CollisionGroups/AGX_DisabledCollisionGroupsComponent.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_ScrewController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "Constraints/Constraint1DOFBarrier.h"
#include "Constraints/Constraint2DOFBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_TrimeshShapeComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "AssetSelection.h"
#include "AssetToolsModule.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "GameFramework/Actor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "UObject/Package.h"
#include "PackageTools.h"

namespace
{
	IAssetTools& GetAssetTools()
	{
		return FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	}

	void PreCreationSetup()
	{
		GEditor->SelectNone(false, false);
	}

	FString Sanitize(const FString& In)
	{
		return UPackageTools::SanitizePackageName(In);
	}

	struct FBlueprintId
	{
		FString ArchiveFilename;
		FString PackagePath;
		FString BlueprintName;

		FBlueprintId(
			const FString& InArchiveFilename, const FString& InPackagePath,
			const FString& InBlueprintName)
			: ArchiveFilename(InArchiveFilename)
			, PackagePath(InPackagePath)
			, BlueprintName(InBlueprintName)
		{
		}
	};

	FBlueprintId CreateBlueprintId(const FString& ArchiveFilepath)
	{
		const FString ArchiveFilename = FPaths::GetBaseFilename(ArchiveFilepath);

		FString ParentPackageName = TEXT("/Game/ImportedBlueprints/");
		FString ParentAssetName = ArchiveFilename;
		IAssetTools& AssetTools = GetAssetTools();
		AssetTools.CreateUniqueAssetName(
			ParentPackageName, ParentAssetName, ParentPackageName, ParentAssetName);
		UPackage* ParentPackage = CreatePackage(nullptr, *ParentPackageName);
		FString Path = FPaths::GetPath(ParentPackage->GetName());

		const FString BlueprintName = TEXT("BP_") + ArchiveFilename;
		FString BasePackagePath = Sanitize(Path + "/" + BlueprintName);
		FString PackagePath = BasePackagePath;

		auto PackageExists = [](const FString& PackagePath) {
			check(!FEditorFileUtils::IsMapPackageAsset(PackagePath));
			return FPackageName::DoesPackageExist(PackagePath) ||
				   FindObject<UPackage>(nullptr, *PackagePath) != nullptr;
		};

		/// \todo Should be possible to use one of the unique name creators here.
		int32 TryCount = 0;
		while (PackageExists(PackagePath))
		{
			++TryCount;
			PackagePath = BasePackagePath + "_" + FString::FromInt(TryCount);
		}

		return FBlueprintId(ArchiveFilename, PackagePath, BlueprintName);
	}

	UPackage* GetPackage(const FBlueprintId& BlueprintId)
	{
		UPackage* Package = CreatePackage(nullptr, *BlueprintId.PackagePath);
		check(Package != nullptr);
		Package->FullyLoad();
		return Package;
	}

	class FBlueprintBody final : public FAGXArchiveBody
	{
	public:
		FBlueprintBody(UAGX_RigidBodyComponent* InBodyComponent, const FString& InArchiveName)
			: BodyComponent(InBodyComponent)
			, ArchiveName(InArchiveName)
		{
		}

		virtual void InstantiateSphere(const FSphereShapeBarrier& Barrier) override
		{
			UAGX_SphereShapeComponent* Component =
				FAGX_EditorUtilities::CreateSphereShape(BodyComponent->GetOwner(), BodyComponent);
			Component->Radius = Barrier.GetRadius();
			FinalizeShape(Component, Barrier);
		}

		virtual void InstantiateBox(const FBoxShapeBarrier& Barrier) override
		{
			UAGX_BoxShapeComponent* Component =
				FAGX_EditorUtilities::CreateBoxShape(BodyComponent->GetOwner(), BodyComponent);
			Component->HalfExtent = Barrier.GetHalfExtents();
			FinalizeShape(Component, Barrier);
		}

		virtual void InstantiateCylinder(const FCylinderShapeBarrier& Barrier) override
		{
			UAGX_CylinderShapeComponent* Component =
				FAGX_EditorUtilities::CreateCylinderShape(BodyComponent->GetOwner(), BodyComponent);
			Component->Height = Barrier.GetHeight();
			Component->Radius = Barrier.GetRadius();
			FinalizeShape(Component, Barrier);
		}

		virtual void InstantiateTrimesh(const FTrimeshShapeBarrier& Barrier) override
		{
			AActor* Owner = BodyComponent->GetOwner();
			UAGX_TrimeshShapeComponent* Component =
				FAGX_EditorUtilities::CreateTrimeshShape(Owner, BodyComponent);
			Component->MeshSourceLocation =
				EAGX_TrimeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
			UStaticMeshComponent* MeshComponent =
				FAGX_EditorUtilities::CreateStaticMeshAsset(Owner, Component, Barrier, ArchiveName);

			FString Name = MeshComponent->GetName() + "Shape";
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(
						   Owner, UAGX_TrimeshShapeComponent::StaticClass(), FName(*Name))
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Trimesh '%s' imported with name '%s' because of name conflict."),
					*OldName, *Name);
			}
			Component->Rename(*Name, nullptr, REN_DontCreateRedirectors);
			FinalizeShape(Component, Barrier);
		}

	private:
		void FinalizeShape(UAGX_ShapeComponent* Component, const FShapeBarrier& Barrier)
		{
			Component->SetFlags(RF_Transactional);
			Component->bCanCollide = Barrier.GetEnableCollisions();
			for (const FName& Group : Barrier.GetCollisionGroups())
			{
				Component->AddCollisionGroup(Group);
			}
			Component->SetRelativeLocation(Barrier.GetLocalPosition());
			Component->SetRelativeRotation(Barrier.GetLocalRotation());
			Component->UpdateVisualMesh();
			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(Component->GetOwner(), Component->GetClass(), *Name)
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Shape '%s' imported with name '%s' because of name conflict."), *OldName,
					*Name);
			}
			Component->Rename(*Name);
		}

	private:
		UAGX_RigidBodyComponent* BodyComponent;
		const FString& ArchiveName;
	};

	class FBlueprintInstantiator final : public FAGXArchiveInstantiator
	{
	public:
		FBlueprintInstantiator(AActor* InBlueprintTemplate, const FString& InArchiveName)
			: BlueprintTemplate(InBlueprintTemplate)
			, ArchiveName(InArchiveName)
		{
		}

		virtual FAGXArchiveBody* InstantiateBody(const FRigidBodyBarrier& Barrier) override
		{
			UAGX_RigidBodyComponent* Component =
				NewObject<UAGX_RigidBodyComponent>(BlueprintTemplate, NAME_None);

			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(
						   BlueprintTemplate, UAGX_RigidBodyComponent::StaticClass(), *Name)
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("RigidBody '%s' imported with name '%s' because of name conflict."),
					*OldName, *Name);
			}
			Component->Rename(*Name);

			Component->SetWorldLocation(Barrier.GetPosition());
			Component->SetWorldRotation(Barrier.GetRotation());
			Component->Mass = Barrier.GetMass();
			Component->MotionControl = Barrier.GetMotionControl();

			Component->SetFlags(RF_Transactional);
			BlueprintTemplate->AddInstanceComponent(Component);
			Component->RegisterComponent();

// This is the attach part of the RootComponent strangeness. I would like to
// call AttachToComponent here, but I don't have a RootComponent. See comment in
// CreateTemplate.
#if 0
			Component->AttachToComponent(
				BlueprintTemplate->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
#endif
			Component->PostEditChange();
			RestoredBodies.Add(Barrier.GetGuid(), Component);
			return new FBlueprintBody(Component, ArchiveName);
		}

		virtual void InstantiateHinge(const FHingeBarrier& Barrier) override
		{
			InstantiateConstraint1Dof(Barrier, UAGX_HingeConstraintComponent::StaticClass());
		}

		virtual void InstantiatePrismatic(const FPrismaticBarrier& Barrier) override
		{
			InstantiateConstraint1Dof(Barrier, UAGX_PrismaticConstraintComponent::StaticClass());
		}

		virtual void InstantiateBallJoint(const FBallJointBarrier& Barrier) override
		{
			InstantiateConstraint<UAGX_ConstraintComponent>(
				Barrier, UAGX_BallConstraintComponent::StaticClass());
		}

		virtual void InstantiateCylindricalJoint(const FCylindricalJointBarrier& Barrier) override
		{
			InstantiateConstraint2Dof(Barrier, UAGX_CylindricalConstraintComponent::StaticClass());
		}

		virtual void InstantiateDistanceJoint(const FDistanceJointBarrier& Barrier) override
		{
			InstantiateConstraint1Dof(Barrier, UAGX_DistanceConstraintComponent::StaticClass());
		}

		virtual void InstantiateLockJoint(const FLockJointBarrier& Barrier) override
		{
			InstantiateConstraint<UAGX_ConstraintComponent>(
				Barrier, UAGX_LockConstraintComponent::StaticClass());
		}

		virtual void DisabledCollisionGroups(
			const TArray<std::pair<FString, FString>>& DisabledPairs) override
		{
			if (DisabledPairs.Num() == 0)
			{
				return;
			}

			UAGX_DisabledCollisionGroupsComponent* Component =
				NewObject<UAGX_DisabledCollisionGroupsComponent>(
					BlueprintTemplate, TEXT("DisabledCollisionGroupPairs"));
			Component->SetFlags(RF_Transactional);
			BlueprintTemplate->AddInstanceComponent(Component);
			Component->RegisterComponent();
			for (const std::pair<FString, FString>& DisabledPair : DisabledPairs)
			{
				Component->DisabledCollisionGroupPairs.Add(
					{FName(*DisabledPair.first), FName(*DisabledPair.second)});
			}
		}

		virtual ~FBlueprintInstantiator() = default;

	private:
		using FBodyPair = std::pair<UAGX_RigidBodyComponent*, UAGX_RigidBodyComponent*>;

	private:
		void InstantiateConstraint1Dof(const FConstraint1DOFBarrier& Barrier, UClass* Type)
		{
			UAGX_Constraint1DofComponent* Component =
				InstantiateConstraint<UAGX_Constraint1DofComponent>(Barrier, Type);
			if (Component == nullptr)
			{
				// No need to log here, done by InstantiateConstraint.
				return;
			}

			/// \todo This is copy/paste from AGX_ArchiveImporterToActorTree. Find a sensible shared
			/// location.
			StoreElectricMotorController(Barrier, Component->ElectricMotorController);
			StoreFrictionController(Barrier, Component->FrictionController);
			StoreLockController(Barrier, Component->LockController);
			StoreRangeController(Barrier, Component->RangeController);
			StoreTargetSpeedController(Barrier, Component->TargetSpeedController);
		}

		void InstantiateConstraint2Dof(const FConstraint2DOFBarrier& Barrier, UClass* Type)
		{
			UAGX_Constraint2DofComponent* Component =
				InstantiateConstraint<UAGX_Constraint2DofComponent>(Barrier, Type);
			if (Component == nullptr)
			{
				// No need to log here, done by InstantiateConstraint.
				return;
			}

			/// \todo This is copy/paste from AGX_ArchiveImporter. Find a sensible shared location.
			const EAGX_Constraint2DOFFreeDOF First = EAGX_Constraint2DOFFreeDOF::FIRST;
			const EAGX_Constraint2DOFFreeDOF Second = EAGX_Constraint2DOFFreeDOF::SECOND;

			StoreElectricMotorController(Barrier, Component->ElectricMotorController1, First);
			StoreElectricMotorController(Barrier, Component->ElectricMotorController2, Second);

			StoreFrictionController(Barrier, Component->FrictionController1, First);
			StoreFrictionController(Barrier, Component->FrictionController2, Second);

			StoreLockController(Barrier, Component->LockController1, First);
			StoreLockController(Barrier, Component->LockController2, Second);

			StoreRangeController(Barrier, Component->RangeController1, First);
			StoreRangeController(Barrier, Component->RangeController2, Second);

			StoreTargetSpeedController(Barrier, Component->TargetSpeedController1, First);
			StoreTargetSpeedController(Barrier, Component->TargetSpeedController2, Second);
		}

		template <typename UConstraint>
		UConstraint* InstantiateConstraint(const FConstraintBarrier& Barrier, UClass* Type)
		{
			FBodyPair Bodies = GetBodies(Barrier);
			if (Bodies.first == nullptr)
			{
				// Not having a second body is fine, means that the first body is constrained to the
				// world. Not having a first body is bad.
				UE_LOG(
					LogAGX, Warning, TEXT("Constraint '%s' does not have a first body. Ignoring."),
					*Barrier.GetName());
				return nullptr;
			}

			UConstraint* Component = FAGX_EditorUtilities::CreateConstraintComponent<UConstraint>(
				BlueprintTemplate, Bodies.first, Bodies.second, Type);
			if (Component == nullptr)
			{
				return nullptr;
			}

			// By default the BodyAttachments are created with the OwningActor set to the owner of
			// the RigidBodyComponents passed to CreateConstraintComponent. In this case the
			// OwningActor points to the temporary template actor and Unreal doesn't update the
			// pointers to instead point to the actor that is created when the Blueprint is
			// instantiated. The best we can do is to set them to nullptr and rely on the body
			// names only.
			Component->BodyAttachment1.RigidBody.OwningActor = nullptr;
			Component->BodyAttachment2.RigidBody.OwningActor = nullptr;

			StoreFrames(Barrier, *Component);

			FString Name = Barrier.GetName();
			if (!Component->Rename(*Name, nullptr, REN_Test))
			{
				FString OldName = Name;
				Name = MakeUniqueObjectName(
						   BlueprintTemplate, UConstraint::StaticClass(), FName(*Name))
						   .ToString();
				UE_LOG(
					LogAGX, Warning,
					TEXT("Constraint '%s' imported with name '%s' because of name collision."),
					*OldName, *Name);
			}
			Component->Rename(*Name, nullptr, REN_DontCreateRedirectors);

			return Component;
		}

		/// \todo There is copy/paste from AGX_ArchiveImporter. Find a sensible shared location.

		void StoreElectricMotorController(
			const FConstraint1DOFBarrier& Barrier,
			FAGX_ConstraintElectricMotorController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetElectricMotorController());
		}

		void StoreElectricMotorController(
			const FConstraint2DOFBarrier& Barrier,
			FAGX_ConstraintElectricMotorController& Controller, EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetElectricMotorController(Dof));
		}

		void StoreFrictionController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetFrictionController());
		}

		void StoreFrictionController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintFrictionController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetFrictionController(Dof));
		}

		void StoreLockController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetLockController());
		}

		void StoreLockController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintLockController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetLockController(Dof));
		}

		void StoreRangeController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetRangeController());
		}

		void StoreRangeController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintRangeController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetRangeController(Dof));
		}

		void StoreTargetSpeedController(
			const FConstraint1DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller)
		{
			Controller.CopyFrom(*Barrier.GetTargetSpeedController());
		}

		void StoreTargetSpeedController(
			const FConstraint2DOFBarrier& Barrier, FAGX_ConstraintTargetSpeedController& Controller,
			EAGX_Constraint2DOFFreeDOF Dof)
		{
			Controller.CopyFrom(*Barrier.GetTargetSpeedController(Dof));
		}

		UAGX_RigidBodyComponent* GetBody(const FRigidBodyBarrier& Barrier)
		{
			if (!Barrier.HasNative())
			{
				// Not an error. Means constrained with world.
				return nullptr;
			}
			FGuid Guid = Barrier.GetGuid();
			UAGX_RigidBodyComponent* Component = RestoredBodies.FindRef(Guid);
			if (Component == nullptr)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Found a constraint to body '%s', but that body isn't known."),
					*Barrier.GetName());
				return nullptr;
			}
			return Component;
		}

		FBodyPair GetBodies(const FConstraintBarrier& Constraint)
		{
			return {GetBody(Constraint.GetFirstBody()), GetBody(Constraint.GetSecondBody())};
		}

		/// \todo The two StoreFrame(s) member functions are copy/paste from AGX_ArchiveImporter.
		/// Find a reasonable shared location to put them in.

		void StoreFrame(
			const FConstraintBarrier& Barrier, FAGX_ConstraintBodyAttachment& Attachment,
			int32 BodyIndex)
		{
			Attachment.FrameDefiningComponent.Clear();
			Attachment.LocalFrameLocation = Barrier.GetLocalLocation(BodyIndex);
			Attachment.LocalFrameRotation = Barrier.GetLocalRotation(BodyIndex);
		}

		void StoreFrames(const FConstraintBarrier& Barrier, UAGX_ConstraintComponent& Component)
		{
			StoreFrame(Barrier, Component.BodyAttachment1, 0);
			StoreFrame(Barrier, Component.BodyAttachment2, 1);
		}

	private:
		AActor* BlueprintTemplate;
		const FString& ArchiveName;
		TMap<FGuid, UAGX_RigidBodyComponent*> RestoredBodies;
	};

	void AddComponentsFromArchive(const FString& ArchivePath, AActor* ImportedActor)
	{
		FString ArchiveName = FPaths::GetCleanFilename(ArchivePath);
		FBlueprintInstantiator Instantiator(ImportedActor, ArchiveName);
		FAGXArchiveReader::Read(ArchivePath, Instantiator);
	}

	AActor* CreateTemplate(const FString& BlueprintName, const FString ArchivePath)
	{
		UActorFactory* Factory =
			GEditor->FindActorFactoryByClass(UActorFactoryEmptyActor::StaticClass());
		FAssetData EmptyActorAssetData = FAssetData(Factory->GetDefaultActorClass(FAssetData()));
		UObject* EmptyActorAsset = EmptyActorAssetData.GetAsset();
		AActor* RootActorContainer =
			FActorFactoryAssetProxy::AddActorForAsset(EmptyActorAsset, false);
		check(RootActorContainer != nullptr);
		RootActorContainer->SetFlags(RF_Transactional);
		RootActorContainer->SetActorLabel(BlueprintName);

// I would like to be able to create and configure the RootComponent here, but
// the way Blueprint creation has been done in Unreal Engine makes this
// impossible. A new RootComponent is always created and the DefaultSceneRoot I
// create here is made a child of that new SceneComponent. Not what I want. My
// work-around for now is to rely on the implicitly created RootComponent and
// hoping it does what we want in all cases. I leave SceneComponents that should
// be attached to the RootComponent unconnected, they are implicitly connected
// to the implicit RootComponent by the Blueprint creator code. This produces a
// weird/invalid template actor so I'm worried that the it-happens-to-work state
// we now have won't survive for long.
#if 0
		USceneComponent* ActorRootComponent = NewObject<USceneComponent>(
			RootActorContainer, USceneComponent::GetDefaultSceneRootVariableName());
		check(ActorRootComponent != nullptr);
		ActorRootComponent->Mobility = EComponentMobility::Movable;
		ActorRootComponent->bVisualizeComponent = true;
		ActorRootComponent->SetFlags(RF_Transactional);
		ActorRootComponent->RegisterComponent();
		RootActorContainer->AddInstanceComponent(ActorRootComponent);
		RootActorContainer->SetRootComponent(ActorRootComponent);
#endif

		AddComponentsFromArchive(ArchivePath, RootActorContainer);
		return RootActorContainer;
	}

	UBlueprint* CreateBlueprint(UPackage* Package, AActor* Template)
	{
		UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprintFromActor(
			Package->GetName(), Template, false, true);
		check(Blueprint);
		return Blueprint;
	}

	void PostCreationTeardown(
		AActor* Template, UPackage* Package, UBlueprint* Blueprint, const FString& PackagePath)
	{
		Template->Destroy();
		GEngine->BroadcastLevelActorListChanged();

		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			PackagePath, FPackageName::GetAssetPackageExtension());

		UPackage::SavePackage(
			Package, Blueprint, RF_Public | RF_Standalone | RF_MarkAsRootSet, *PackageFilename,
			GError, nullptr, true, true, SAVE_NoError);
	}
}

UBlueprint* AGX_ArchiveImporterToBlueprint::ImportAGXArchive(const FString& ArchivePath)
{
	PreCreationSetup();
	FBlueprintId Id = CreateBlueprintId(ArchivePath);
	UPackage* Package = GetPackage(Id);
	AActor* Template = CreateTemplate(Id.BlueprintName, ArchivePath);
	UBlueprint* Blueprint = CreateBlueprint(Package, Template);
	PostCreationTeardown(Template, Package, Blueprint, Id.PackagePath);
	return Blueprint;
}