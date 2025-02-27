// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ObjectUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Engine/Level.h"
#include "Misc/EngineVersionComparison.h"
#include "Misc/PackageName.h"
#include "UObject/SavePackage.h"
#include "Engine/World.h"

void FAGX_ObjectUtilities::GetChildActorsOfActor(AActor* Parent, TArray<AActor*>& ChildActors)
{
	TArray<AActor*> CurrentLevel;

	// Set Parent as root node of the tree
	CurrentLevel.Add(Parent);

	GetActorsTree(CurrentLevel, ChildActors);

	// Remove the parent itself from the ChildActors array
	ChildActors.Remove(Parent);
}

AActor* FAGX_ObjectUtilities::GetRootParentActor(AActor* Actor)
{
	if (!IsValid(Actor))
	{
		return nullptr;
	}

	while (Actor->IsChildActor())
	{
		AActor* Parent = Actor->GetParentActor();
		if (IsValid(Parent))
		{
			Actor = Parent;
		}
		else
		{
			// Needed to avoid infinite loop when Actor is a Child Actor but the Parent Actor is
			// invalid.
			break;
		}
	}

	return Actor;
}

AActor* FAGX_ObjectUtilities::GetRootParentActor(UActorComponent* Component)
{
	if (Component == nullptr)
		return nullptr;
	return GetRootParentActor(*Component);
}

AActor* FAGX_ObjectUtilities::GetRootParentActor(UActorComponent& Component)
{
	return GetRootParentActor(Component.GetTypedOuter<AActor>());
}

bool FAGX_ObjectUtilities::IsTemplateComponent(const UActorComponent& Component)
{
	return Component.HasAnyFlags(RF_ArchetypeObject);
}

bool FAGX_ObjectUtilities::RemoveComponentAndPromoteChildren(
	USceneComponent* Component, AActor* Owner)
{
	if (Component == nullptr || Owner == nullptr || Component->GetOwner() != Owner)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("RemoveComponentAndPromoteChildren: Invalid Component or Owner"));
		return false;
	}

	USceneComponent* ParentComponent = Component->GetAttachParent();

	TArray<USceneComponent*> Children;
	Component->GetChildrenComponents(false, Children);

	for (USceneComponent* Child : Children)
	{
		if (Child == nullptr)
			continue;

		Child->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		if (ParentComponent != nullptr)
			Child->AttachToComponent(
				ParentComponent, FAttachmentTransformRules::KeepWorldTransform);
		else
			Child->AttachToComponent(
				Owner->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);
	}

	Component->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	Owner->RemoveInstanceComponent(Component);
	Component->DestroyComponent();
	return true;
}

bool FAGX_ObjectUtilities::CopyProperties(
	const UObject& Source, UObject& OutDestination, bool UpdateArchetypeInstances)
{
	UClass* Class = Source.GetClass();
	if (OutDestination.GetClass() != Class)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Tried to copy properties from object '%s' of type '%s' to object '%s' of "
				 "type '%s'. Types must match."),
			*Source.GetName(), *Source.GetClass()->GetName(), *OutDestination.GetName(),
			*OutDestination.GetClass()->GetName());
		return false;
	}

	TArray<UObject*> ArchetypeInstances;
	if (UpdateArchetypeInstances)
		OutDestination.GetArchetypeInstances(ArchetypeInstances);

	for (TFieldIterator<FProperty> PropIt(Class); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		// To speed up execution, can we add custom property flags for AGXUnreal properties?
		if (Property && Property->HasAnyPropertyFlags(CPF_Edit))
		{
			const void* SourceValue = Property->ContainerPtrToValuePtr<void>(&Source);
			void* DestValue = Property->ContainerPtrToValuePtr<void>(&OutDestination);
			if (Property->Identical(SourceValue, DestValue))
				continue; // Nothing to do, already equal.

			if (UpdateArchetypeInstances)
			{
				for (UObject* Instance : ArchetypeInstances)
				{
					if (Instance == nullptr)
						continue;

					void* ArchetypeInstanceValue = Property->ContainerPtrToValuePtr<void>(Instance);
					if (Property->Identical(ArchetypeInstanceValue, DestValue)) // In sync; copy!
						Property->CopyCompleteValue(ArchetypeInstanceValue, SourceValue);
				}
			}

			Property->CopyCompleteValue(DestValue, SourceValue);
		}
	}

	return true;
}

FString FAGX_ObjectUtilities::SanitizeObjectName(FString Name, UClass* Class)
{
	if (Class != nullptr && (Name.IsEmpty() || Name.Equals("None")))
		Name = Class->GetName();

	
	Name.RemoveFromEnd("Component");
	return MakeObjectNameFromDisplayLabel(Name, FName(*Name)).ToString();
}

FString FAGX_ObjectUtilities::MakeObjectNameUnique(UObject* Owner, FString Name)
{
	if (Owner == nullptr)
		return Name;

	if (StaticFindObjectFast(UObject::StaticClass(), Owner, FName(*Name)) != nullptr)
		Name = MakeUniqueObjectName(Owner, UObject::StaticClass(), FName(*Name)).ToString();

	return Name;
}

FString FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
	UObject* Owner, const FString& Name, UClass* Class)
{
	return MakeObjectNameUnique(Owner, SanitizeObjectName(Name, Class));
}

void FAGX_ObjectUtilities::GetActorsTree(
	const TArray<AActor*>& CurrentLevel, TArray<AActor*>& ChildActors)
{
	for (AActor* Actor : CurrentLevel)
	{
		if (Actor == nullptr)
		{
			continue;
		}

		ChildActors.Add(Actor);

		TArray<AActor*> NextLevel;
		Actor->GetAttachedActors(NextLevel);
		GetActorsTree(NextLevel, ChildActors);
	}
}

#if WITH_EDITOR
AActor* FAGX_ObjectUtilities::GetActorByLabel(const UWorld& World, const FString Name)
{
	for (ULevel* Level : World.GetLevels())
	{
		for (AActor* Actor : Level->Actors)
		{
			if (Actor != nullptr && Actor->GetActorLabel() == Name)
			{
				return Actor;
			}
		}
	}
	return nullptr;
}
#endif

AActor* FAGX_ObjectUtilities::GetActorByName(const UWorld& World, const FString Name)
{
	// TODO Can we use TActorIterator here?

	for (ULevel* Level : World.GetLevels())
	{
		for (AActor* Actor : Level->Actors)
		{
			if (Actor != nullptr && Actor->GetName() == Name)
			{
				return Actor;
			}
		}
	}
	return nullptr;
}

#if WITH_EDITOR
bool FAGX_ObjectUtilities::SaveAsset(UObject& Asset, bool FullyLoad)
{
	UPackage* Package = Asset.GetPackage();
	if (Package == nullptr || Package->GetPathName().IsEmpty())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("SaveAsset called with Asset '%s' without a valid Package. The asset will not be "
				 "saved."),
			*Asset.GetName());
		return false;
	}

	Asset.MarkPackageDirty();
	Asset.PostEditChange();

	const FString PackageFilename = FPackageName::LongPackageNameToFilename(
		Asset.GetPackage()->GetPathName(), FPackageName::GetAssetPackageExtension());

	// A package must have meta-data in order to be saved. It seems to be created automatically
	// most of the time but sometimes, during unit tests for example, the engine tries to create it
	// on-demand while saving the package which leads to a fatal error because this type of object
	// look-up isn't allowed while saving packages. So try to force it here before calling
	// SavePackage.
	//
	// The error message sometimes printed while within UPackage::SavePackage called below is:
	// Illegal call to StaticFindObjectFast() while serializing object data or garbage collecting!
	Package->GetMetaData();

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	const bool Result = UPackage::SavePackage(Package, &Asset, RF_NoFlags, *PackageFilename);
#else
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	const bool Result = UPackage::SavePackage(Package, &Asset, *PackageFilename, SaveArgs);
#endif

	if (Result && FullyLoad)
		Package->FullyLoad();

	return Result;
}

bool FAGX_ObjectUtilities::MarkAssetDirty(UObject& Asset)
{
	UPackage* Package = Asset.GetPackage();
	if (Package == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot mark asset '%s' dirty because it does not have a package."),
			*Asset.GetPathName());
		return false;
	}

	if (Package->MarkPackageDirty())
	{
		return true;
	}

	Package->SetDirtyFlag(true);
	if (Package->IsDirty())
	{
		Package->PackageMarkedDirtyEvent.Broadcast(Package, true);
	}
	else
	{
		const FString Message = FString::Printf(
			TEXT("Could not mark package '%s' dirty. It should be saved manually."),
			*Asset.GetPathName());
		FAGX_NotificationUtilities::ShowNotification(
			Message, SNotificationItem::ECompletionState::CS_Fail);
	}

	return Package->IsDirty();
}
#endif

FTransform FAGX_ObjectUtilities::GetAnyComponentWorldTransform(const USceneComponent& Component)
{
#if WITH_EDITOR
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		return FAGX_BlueprintUtilities::GetTemplateComponentWorldTransform(&Component);
	}
	else
	{
		return Component.GetComponentTransform();
	}
#else
	return Component.GetComponentTransform();
#endif
}

void FAGX_ObjectUtilities::SetAnyComponentWorldTransform(
	USceneComponent& Component, const FTransform& Transform, bool ForceOverwriteInstances)
{
#if WITH_EDITOR
	if (FAGX_ObjectUtilities::IsTemplateComponent(Component))
	{
		FAGX_BlueprintUtilities::SetTemplateComponentWorldTransform(
			&Component, Transform, true, ForceOverwriteInstances);
	}
	else
	{
		Component.SetWorldTransform(Transform);
	}
#else
	Component.SetWorldTransform(Transform);
#endif
}

void FAGX_ObjectUtilities::TruncateForDetailsPanel(double& Value)
{
	// See comment in header file.
	// At the time of writing the format specifier exposed for double in UnrealTypeTraits.h is %f.
	// Value = FCString::Atod(*FString::Printf(TEXT("%f"), Value));
	//
	// We should keep an eye out for changes to this setup in Unreal Engine. The UDN reply mentioned
	// improvements made in CL# 14346058 and CL# 15516611, which I believe corresponds to
	// - https://github.com/EpicGames/UnrealEngine/commit/065d8d227321ca364b7edc3cdfc9539cc01fadcb
	//   "Widget: Modify SpinBox to support double and int64"
	// - https://github.com/EpicGames/UnrealEngine/commit/60cd75894a720fa8cc97d3f8424f0dd42742a92c
	//   "Add support for displaying floats greater than e18 & remove cast losing double precision."
	// both of which were released with Unreal Engine 5.0.
	Value = FCString::Atod(*FString::Printf(TFormatSpecifier<double>::GetFormatSpecifier(), Value));
}

void FAGX_ObjectUtilities::TruncateForDetailsPanel(FVector& Values)
{
	TruncateForDetailsPanel(Values.X);
	TruncateForDetailsPanel(Values.Y);
	TruncateForDetailsPanel(Values.Z);
}

void FAGX_ObjectUtilities::TruncateForDetailsPanel(FRotator& Values)
{
	TruncateForDetailsPanel(Values.Pitch);
	TruncateForDetailsPanel(Values.Yaw);
	TruncateForDetailsPanel(Values.Roll);
}

bool FAGX_ObjectUtilities::HasChainPrefixPath(
	FEditPropertyChain::TDoubleLinkedListNode* Node, const TArray<const TCHAR*>& Path)
{
	// Walk the two lists in parallel, i.e. do a zip, and bail as soon as we find a name mismatch.
	FString NodeName;
	int32 I = 0;
	for (; I < Path.Num() && Node != nullptr; ++I, Node = Node->GetNextNode())
	{
		Node->GetValue()->GetName(NodeName);
		if (NodeName != Path[I])
		{
			return false;
		}
	}
	return I == Path.Num();
}
