// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"

// Unreal Engine includes.
#include "Containers/Array.h"
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Misc/EngineVersionComparison.h"

/**
 * A collection of generic helper functions that can be compiled for Runtime.
 *
 * \see AGX_EditorUtilities.h, which is for WITH_EDITOR builds.
 *
 * There are also other, more specialized, AGX_.+Utilities.h collections.
 */
class AGXUNREAL_API FAGX_ObjectUtilities
{
public:
	/**
	 * Returns array containing all child actors (and all their child actors)
	 * recursively, attached to the Parent actor passed as input argument.
	 * The parent itself is not included in the set.
	 */
	static void GetChildActorsOfActor(AActor* Parent, TArray<AActor*>& ChildActors);

	/**
	 * Walk the Parent Actor chain until a non-Child-Actor is found and return that. If the given
	 * Actor is not a Child Actor then it is returned.
	 *
	 * Required because in some cases the Child Actor is destroyed and replaced so keeping a pointer
	 * to it is dangerous. For example in Component Reference.
	 */
	static AActor* GetRootParentActor(AActor* Actor);

	/**
	 * Walk the Component's Actor outer parent chain until a non-Child-Actor is found and return
	 * that.
	 *
	 * Required because in some cases the Child Actor is destroyed and replaced so keeping a pointer
	 * to it is dangerous. For example when a Component Reference is held by a Child Actor in a
	 * Blueprint class.
	 */
	static AActor* GetRootParentActor(UActorComponent* Component);
	static AActor* GetRootParentActor(UActorComponent& Component);

	/*
	 * Checks whether the component is a template Component, i.e. it may have archetype instances.
	 */
	static bool IsTemplateComponent(const UActorComponent& Component);

	static bool RemoveComponentAndPromoteChildren(USceneComponent* Component, AActor* Owner);

	/**
	 * Copy all properties from an UObject to another. The objects must be of same type.
	 */
	static bool CopyProperties(
		const UObject& Source, UObject& OutDestination, bool UpdateArchetypeInstances = true);

	/**
	 * Returns a sanitized version of the given name that uses only valid characters.
	 * UClass of the object can be passed to give a better default name in case of empty name.
	 */
	static FString SanitizeObjectName(FString Name, UClass* Class);

	/**
	 * Makes an Object Name that is unique in the given context (Owner).
	 */
	static FString MakeObjectNameUnique(UObject* Owner, FString Name);

	/**
	 * Combines SanitizeObjectName and MakeObjectNameUnique to make a valid, final name that can be
	 * used for Components, Assets and Actors.
	 * UClass of the object can be passed to give a better default name in case of empty name.
	 */
	static FString SanitizeAndMakeNameUnique(UObject* Owner, const FString& Name, UClass* Class);

	/**
	 * Give a list of pointer-to-base, return a new list with the elements that
	 * are of a particular derived type.
	 */
	template <typename UDerived, typename UBaseContainer>
	static TArray<UDerived*> Filter(const UBaseContainer& Collection);

	template <typename T>
	static T* FindFirstAncestorOfType(const USceneComponent& Start);

	/**
	 * Returns the number children of type T of the actor.
	 */
	template <typename T>
	static uint32 GetNumComponentsInActor(
		const AActor& Actor, bool bIncludeFromChildActors = false);

	template <typename T>
	static T* GetComponentByName(const AActor& Actor, const TCHAR* Name);

	template <typename T>
	static TArray<T*> GetChildrenOfType(const USceneComponent& Parent, bool Recursive);

	template <typename T>
	static T* Get(const FComponentReference& Reference, const AActor* FallbackOwner);

	static const AActor* GetActor(
		const FComponentReference& Reference, const AActor* FallbackActor = nullptr)
	{
		const AActor* Actor =
#if UE_VERSION_OLDER_THAN(5, 0, 0)
			Reference.OtherActor;
#else
			Reference.OtherActor.Get();
#endif
		return Actor != nullptr ? Actor : FallbackActor;
	}

#if WITH_EDITOR
	/**
	 * Get the first Actor in the level that has the given label.
	 *
	 * The label is the name that is displayed in the World Outliner. Not that this may not be
	 * unique.
	 */
	static AActor* GetActorByLabel(const UWorld& World, const FString Label);
#endif

	static AActor* GetActorByName(const UWorld& World, const FString Name);

	/**
	 * Tries to load and returns an asset given an asset path. Returns nullptr if the asset
	 * could not be loaded.
	 */
	template <typename AssetType>
	static AssetType* GetAssetFromPath(const TCHAR* AssetPath);

	/*
	 * Returns any Archetype instances of the passed object. If the passed object is not an
	 * archetype, an empty TArray is returned.
	 * Looks recursively in the archetype tree and returns all Instances in it.
	 * Note that an Archetype Instance may itself be an Archetype.
	 */
	template <typename T>
	static TArray<T*> GetArchetypeInstances(T& Object);

	/**
	 * Finds archetype instance with the given outer object. If the TemplateComponent has Outer as
	 * its outer, then TemplateComponent is returned. If non could be found, nullptr is returned.
	 */
	template <typename T>
	static T* GetMatchedInstance(T* TemplateComponent, UObject* Outer);

#if WITH_EDITOR
	/**
	 * Saves (or re-saves) an asset to disk. The asset must have a valid Package setup before
	 * passing it to this function.
	 * Setting FullyLoad to true will call Package->FullyLoad after save. This has been noted
	 * necessary when building cooked build on Linux in some situations.
	 */
	static bool SaveAsset(UObject& Asset, bool FullyLoad = false);

	/**
	 * Mark the package holding the given asset dirty.
	 *
	 * Should be called after making changes to the asset that should lead to an asterisk emblem
	 * being shown on the asset in the Content Browser and a Save? question asked when closing
	 * Unreal Editor.
	 *
	 * First tries to mark the package dirty with UPackage::MarkPackageDirty. If that fails then
	 * we set the dirty bit explicitly.
	 */
	static bool MarkAssetDirty(UObject& Asset);
#endif

	/**
	 * Get the transform of any Component, even template Components residing in a Blueprint.
	 */
	static FTransform GetAnyComponentWorldTransform(const USceneComponent& Component);

	/**
	 * Set the transform of any Component, even template Components residing in a Blueprint. If the
	 * Component resides in a Blueprint and is a Component template, any archetype instances
	 * currently "in sync" with the Component will be updated as well. The archetype instances
	 * update only happens in editor builds.
	 */
	static void SetAnyComponentWorldTransform(
		USceneComponent& Component, const FTransform& Transform,
		bool ForceOverwriteInstances = false);

	/**
	 * Unreal Engine, at least Unreal Engine 5.0, uses a string-based communication protocol between
	 * the Details panel and the actual objects. For floating-point values the value-to-string
	 * conversion is done with printf("%f", Value), which is highly destructive, meaning the value
	 * that is stored is not the same as the value that is shown in the UI. Worse, if the value is
	 * set from C++ then it may be set to a value that the UI cannot represent. The effect of this
	 * is that the value becomes uneditable since during value propagation through the template type
	 * instance hierarchy only values that compare equal to the old value are updated, but no value
	 * will compare equal since in the round-trip to the UI the value was changed.
	 *
	 * C++ > Value > printf("%f") > UI > Edit > scanf("%f") > Compare with Value, not equal.
	 *
	 * To prevent the value from becoming uneditable we must ensure that all writes made to a
	 * property is Details panel compatible, i.e. does not have more precision than printf("%s")
	 * provides. That is the purpose of TruncateForDetailsPanel. It does the same Value > String >
	 * Value round-trip to throw away any excess precision and ensure that future string comparisons
	 * will compare equal.
	 *
	 * It many cases it is not acceptable to destroy information like this. FAGX_Real is a custom
	 * floating-point type that uses "%g" instead of "%f" for the string conversion which helps a
	 * bit, but cannot be used with composite types such as FVector, FQuat, and FRotator.
	 *
	 * See FPropertyHandleFloat::SetValue in PropertyHandleImpl.cpp.
	 * See Expose_TFormatSpecifier(double in UnrealTypeTraits.h.
	 * See
	 * https://udn.unrealengine.com/s/question/0D54z0000773duUCAQ/data-loss-for-nearzero-values-in-details-panel
	 * "Data loss for near-zero values in Details Panel"
	 */
	static void TruncateForDetailsPanel(double& Value);

	/** See TruncateForDetailsPanel(double& Value)*/
	static void TruncateForDetailsPanel(FVector& Values);

	/** See TruncateForDetailsPanel(double& Value)*/
	static void TruncateForDetailsPanel(FRotator& Values);

	static bool HasChainPrefixPath(
		FEditPropertyChain::TDoubleLinkedListNode* Node, const TArray<const TCHAR*>& Path);

	template <typename T>
	static T* SetIfNullptr(T*& Storage, T* Value);

	/**
	 * Set Storage to New if its current value is Expected.
	 *
	 * @param Storage Variable to maybe set.
	 * @param Expected Value Storage should have for the set to happen.
	 * @param New The value Storage may be set to.
	 */
	template <typename T>
	static void SetIfEqual(T& Storage, T Expected, T New);

	template <typename T, typename FPredicate>
	static T* FindComponentByPredicate(UWorld& World, FPredicate Predicate);

	/**
	 * Iterator for registered Components on an Actor.
	 *
	 * Based on TComponentIterator in GameFrameWorkComponent.h. Not using that directly because
	 * it is in a plugin, Modular Gameplay, that we cannot depend on. Is there some other convenient
	 * way to iterate over Components in an Actor that we can use?
	 */
	template <typename T>
	class TComponentIterator
	{
	public:
		explicit TComponentIterator(AActor* OwnerActor)
		{
			if (IsValid(OwnerActor))
			{
				OwnerActor->GetComponents(AllComponents);
			}

			Advance();
		}

		FORCEINLINE void operator++()
		{
			Advance();
		}

		FORCEINLINE explicit operator bool() const
		{
			return AllComponents.IsValidIndex(Index);
		}

		FORCEINLINE bool operator!() const
		{
			return !(bool) *this;
		}

		FORCEINLINE T* operator*() const
		{
			return GetComponent();
		}

		FORCEINLINE T* operator->() const
		{
			return GetComponent();
		}

	protected:
		/// Get the current component.
		FORCEINLINE T* GetComponent() const
		{
			return AllComponents[Index];
		}

		/// Moves the iterator to the next valid component.
		FORCEINLINE bool Advance()
		{
			while (++Index < AllComponents.Num())
			{
				T* Component = GetComponent();
				if (Component == nullptr || !IsValid(Component))
					continue;
				if (Component->IsRegistered())
					return true;
			}
			return false;
		}

	private:
		/// Results from GetComponents.
		TInlineComponentArray<T*> AllComponents;

		// Index of the current element in the All Components array.
		int32 Index {-1};

		FORCEINLINE bool operator==(const TComponentIterator& Other) const
		{
			return Index == Other.Index;
		}

		FORCEINLINE bool operator!=(const TComponentIterator& Other) const
		{
			return Index != Other.Index;
		}
	};

private:
	static void GetActorsTree(const TArray<AActor*>& CurrentLevel, TArray<AActor*>& ChildActors);
};

template <typename T>
T* FAGX_ObjectUtilities::FindFirstAncestorOfType(const USceneComponent& Start)
{
	USceneComponent* Parent = Start.GetAttachParent();
	while (Parent != nullptr)
	{
		if (Parent->IsA<T>())
		{
			return Cast<T>(Parent);
		}
		Parent = Parent->GetAttachParent();
	}
	return nullptr;
}

template <typename UDerived, typename UBaseContainer>
TArray<UDerived*> FAGX_ObjectUtilities::Filter(const UBaseContainer& Collection)
{
	using UBase = typename std::remove_pointer<typename UBaseContainer::ElementType>::type;
	TArray<UDerived*> Result;
	for (UBase* Element : Collection)
	{
		if (UDerived* Match = Cast<UDerived>(Element))
		{
			Result.Add(Match);
		}
	}
	return Result;
}

template <typename T>
uint32 FAGX_ObjectUtilities::GetNumComponentsInActor(
	const AActor& Actor, bool bIncludeFromChildActors)
{
	TArray<T*> Components;
	Actor.GetComponents<T>(Components, bIncludeFromChildActors);
	return Components.Num();
}

template <typename T>
T* FAGX_ObjectUtilities::GetComponentByName(const AActor& Actor, const TCHAR* Name)
{
	// The Components are stored in a TSet but I don't know how to search a TSet with a predicate
	// So copying all the pointers to a TArray. Is there a better way?
	//
	// That question can be asked in general, this seems like a complicated way to find a Component
	// in an Actor.
	TArray<T*> Components;
	Actor.GetComponents(Components);
	auto It = Components.FindByPredicate([Name](const T* Component)
										 { return Component->GetName() == Name; });
	return It ? *It : nullptr;
}

template <typename T>
inline TArray<T*> FAGX_ObjectUtilities::GetChildrenOfType(
	const USceneComponent& Parent, bool Recursive)
{
	TArray<USceneComponent*> Children;
	Parent.GetChildrenComponents(Recursive, Children);
	TArray<T*> Result;
	for (USceneComponent* Child : Children)
	{
		if (auto C = Cast<T>(Child))
			Result.Add(C);
	}

	return Result;
}

template <typename T>
T* FAGX_ObjectUtilities::Get(const FComponentReference& Reference, const AActor* FallbackOwner)
{
	// Search among the Properties.
	// const_cast because FComponentReference require a non-const AActor. For now we assume that's
	// ok.
	AActor* FallbackOwner_ = const_cast<AActor*>(FallbackOwner);
	UActorComponent* ActorComponent = Reference.GetComponent(FallbackOwner_);
	if (T* Component = Cast<T>(ActorComponent))
	{
		// Found a Component of the correct type in a Property with the correct name.
		return Component;
	}

	// Search among all Components with the correct type.
	const AActor* SearchActor = FAGX_ObjectUtilities::GetActor(Reference, FallbackOwner);
	if (SearchActor == nullptr)
	{
		return nullptr;
	}
	TArray<T*> AllComponents;
	SearchActor->GetComponents(AllComponents, false);
	T** It = AllComponents.FindByPredicate(
		[Reference](T* Component) { return Component->GetFName() == Reference.ComponentProperty; });
	if (It == nullptr)
	{
		return nullptr;
	}
	return *It;
}

template <typename AssetType>
inline AssetType* FAGX_ObjectUtilities::GetAssetFromPath(const TCHAR* AssetPath)
{
	UObject* LoadResult =
		StaticLoadObject(AssetType::StaticClass(), nullptr, AssetPath, nullptr, LOAD_NoWarn);
	return Cast<AssetType>(LoadResult);
}

template <typename T>
TArray<T*> FAGX_ObjectUtilities::GetArchetypeInstances(T& Object)
{
	TArray<T*> Arr;
	if (!Object.HasAnyFlags(RF_ArchetypeObject))
	{
		return Arr;
	}

	TArray<UObject*> ArchetypeInstances;
	Object.GetArchetypeInstances(ArchetypeInstances);
	for (UObject* Obj : ArchetypeInstances)
	{
		T* Instance = Cast<T>(Obj);
		if (Instance == nullptr)
		{
			continue;
		}

		Arr.Add(Instance);
	}

	return Arr;
}

template <typename T>
T* FAGX_ObjectUtilities::GetMatchedInstance(T* TemplateComponent, UObject* Outer)
{
	if (TemplateComponent == nullptr || Outer == nullptr)
		return nullptr;

	if (TemplateComponent->GetOuter() == Outer)
		return TemplateComponent;

	for (auto Instance : GetArchetypeInstances(*TemplateComponent))
	{
		if (Instance->GetOuter() == Outer)
		{
			return Instance;
		}
	}

	return nullptr;
}

#define AGX_COPY_PROPERTY_FROM(                                                          \
	UpropertyName, GetterExpression, Component, ForceOverwriteInstances)                 \
	{                                                                                    \
		if (FAGX_ObjectUtilities::IsTemplateComponent(Component))                        \
		{                                                                                \
			for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component)) \
			{                                                                            \
				if (ForceOverwriteInstances ||                                           \
					Instance->UpropertyName == (Component).UpropertyName)                \
				{                                                                        \
					Instance->UpropertyName = GetterExpression;                          \
				}                                                                        \
			}                                                                            \
		}                                                                                \
		(Component).UpropertyName = GetterExpression;                                    \
	}

#define AGX_COPY_ASSET_PROPERTY_FROM(                                                         \
	AssetName, PropertyName, GetterExpression, Component, ForceOverwriteInstances)            \
	{                                                                                         \
		if (FAGX_ObjectUtilities::IsTemplateComponent(Component))                             \
		{                                                                                     \
			for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(Component))      \
			{                                                                                 \
				if (ForceOverwriteInstances ||                                                \
					Instance->AssetName->PropertyName == (Component).AssetName->PropertyName) \
				{                                                                             \
					Instance->AssetName->PropertyName = GetterExpression;                     \
				}                                                                             \
			}                                                                                 \
		}                                                                                     \
		(Component).AssetName->PropertyName = GetterExpression;                               \
	}

template <typename T>
T* FAGX_ObjectUtilities::SetIfNullptr(T*& Storage, T* const Value)
{
	if (Storage == nullptr)
	{
		Storage = Value;
	}
	return Storage;
}

template <typename T>
void FAGX_ObjectUtilities::SetIfEqual(T& Storage, T Expected, T New)
{
	if (Storage == Expected)
	{
		Storage = New;
	}
}

template <typename T, typename FPredicate>
T* FAGX_ObjectUtilities::FindComponentByPredicate(UWorld& World, FPredicate Predicate)
{
	for (TActorIterator<AActor> ActorIt(&World); ActorIt; ++ActorIt)
	{
		for (TComponentIterator<T> ComponentIt(*ActorIt); ComponentIt; ++ComponentIt)
		{
			if (Predicate(*ComponentIt))
			{
				return *ComponentIt;
			}
		}
	}
	return nullptr;
}
