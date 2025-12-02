// Copyright 2025, Algoryx Simulation AB.

#pragma once

#if WITH_EDITOR

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"

/**
 * We identify properties by a (Member, Property) pair. Each such pair is associated with a
 * callback. "Member" is the name of the member on the UObject, the UObject on which
 * PostEditChangeProperty is called. "Property" is the first nesting of the member. This is used
 * when the member is a struct containing many properties and we only wish to update one at the
 * time.
 *
 * For properties directly on the UObject Member and Property will be the same.
 *
 * Note that for nesting deeper than two levels these doesn't directly correspond to the Member and
 * Property parameters passed to PostEditPropertyChanged. They are the first and last properties of
 * a nesting chain while here we store the first and second properties.
 *
 * For examples, consider Constraint1DofComponent > RangeController > Range > Min. Here Member would
 * be "RangeController" and Property would be "Range". The same callback would be called for Min and
 * Max, the other member of Range.
 *
 * The objects at the second level are typically small, such as vectors or ranges, so it is OK to
 * update them in whole. Some restructure of the UpropertyDispatcher will be required if we want
 * to use it for more granular callbacks. Most such cases can be handled directly in the Property
 * Changed callback, without going throught this Property Dispatcher.
 *
 * For immediate member structs that you want to handle with a single callback register a callback
 * on the Member name only and do not register any callback for the nested properties. This is
 * useful for Members that are e.g. a FVector where the (Member, Property) pair would be e.g.
 * (Velocity, X) but we might want to  update the entire velocity as a single chunk. Then add a
 * callback for just "Velocity" and not ("Velocity", "X") etc.
 */
struct FAGX_NamePair
{
	// The name of the root property on the modified instance. This can be a struct or other
	// aggregate type, but it can also be the actual leaf property if the property is a primitive
	// type. If it is the leaf property then Member == Property.
	FName Member;

	// The name of the property that was changed. This can be either equal to Member (for primitive
	// properties), a direct member of Member (for simple struct properties) or the name of a struct
	// (for nested structs).
	FName Property;

	// The Key should be read as:
	//   The property 'Member.Property' was changed, but there can be additional levels of nesting.
};

inline bool operator==(const FAGX_NamePair& LHS, const FAGX_NamePair& RHS)
{
	return LHS.Member == RHS.Member && LHS.Property == RHS.Property;
}

inline uint32 GetTypeHash(const FAGX_NamePair& Pair)
{
	const uint32 Member = GetTypeHash(Pair.Member);
	const uint32 Property = GetTypeHash(Pair.Property);
	const uint32 Hash = HashCombine(Member, Property);
	return Hash;
}

/**
 * The UpropertyDispatcher is a collection of (Property, Callback) pairs. A UObject subclass that
 * need to handle many members in its PostEditChangeProperty or PostEditChangeChainProperty can
 * delegate the name testing to this class and have a callback called for each time a property is
 * edited.
 *
 * @tparam T The type of the object holding this UpropertyDispatcher. A pointer of this type will be
 * passed to each callback.
 */
template <typename T>
struct FAGX_PropertyChangedDispatcher
{
public:
	using UpdatePropertyFunction = TFunction<void(T*)>;

	/**
	 * Get the Property Changed Dispatcher associated with the template type. Each type has its own
	 * instance.
	 */
	static FAGX_PropertyChangedDispatcher<T>& Get();

	/**
	 * @return True if any callbacks has been registered with this dispatcher.
	 */
	bool IsInitialized() const;

	/**
	 * Add a callback for a direct member property. The case where Member and Property have the same
	 * value. In other words, when the member is a primitive type and not a struct, or when you
	 * want the same callback for every member of the struct Property.
	 *
	 * The callback will be called while a Property Changed event is being processed, so it should
	 * not modify the the object further. The intention is that the callback is used to update the
	 * AGX Dynamics object, if any, in response to the change.
	 *
	 * @param MemberAndProperty The name of the property.
	 * @param Function The function to call when the property is changed.
	 */
	void Add(const FName& MemberAndProperty, UpdatePropertyFunction&& Function);

	/**
	 * Add a callback for a struct member property. Member should be the name of the struct that is
	 * the outermost member and Property should be the name of the property within that struct.
	 * There is currently no way to specify per-member callbacks for nesting deeper than a single
	 * struct.
	 *
	 * The callback will be called while a Property Changed event is being processed, so it should
	 * not modify the the object further. The intention is that the callback is used to update the
	 * AGX Dynamics object, if any, in response to the change.
	 *
	 * @param Member The top-most member.
	 * @param Property The member within the top-most member.
	 * @param Function The function to call when the property is changed.
	 */
	void Add(const FName& Member, const FName& Property, UpdatePropertyFunction&& Function);

	/**
	 * Call the callback associated with the given event, if any, on all objects listed as edited by
	 * the event.
	 * \param Event The event to trigger callbacks for.
	 */
	void Trigger(FPropertyChangedEvent& Event);

	/**
	 * Call the callback associated with the given event, if any, on all objects listed as edited by
	 * the event.
	 * \param Event The event to trigger callbacks for.
	 */
	void Trigger(FPropertyChangedChainEvent& Event);

	/**
	 * Run the function associated with the property.
	 * @param Member The name of the direct member that was changed.
	 * @param Property The name of the nested property that was changed.
	 * @param Event The event containing the objects that were modified. Each is passed to the
	 * callback.
	 */
	void Trigger(const FName& Member, const FName& Property, FPropertyChangedEvent& Event);

	/**
	 * Get the callback associated with the given (Member, Property) pair.
	 * \param Member
	 * \param Property
	 * \return
	 */
	UpdatePropertyFunction* GetFunction(const FName& Member, const FName& Property);

private:
	bool FunctionExists(const FName& Member, const FName& Property)
	{
		return GetFunction(Member, Property) != nullptr;
	}

	TMap<FAGX_NamePair, UpdatePropertyFunction> Functions;
};

/// Short version of GET_MEMBER_NAME_CHECKED when we want a member in the current class.
#define AGX_MEMBER_NAME(MemberName) GET_MEMBER_NAME_CHECKED(ThisClass, MemberName)

template <typename T>
FAGX_PropertyChangedDispatcher<T>& FAGX_PropertyChangedDispatcher<T>::Get()
{
	static FAGX_PropertyChangedDispatcher<T> Instance;
	return Instance;
}

template <typename T>
void FAGX_PropertyChangedDispatcher<T>::Add(
	const FName& MemberAndProperty, UpdatePropertyFunction&& Function)
{
	Functions.Add(FAGX_NamePair {MemberAndProperty, MemberAndProperty}, std::move(Function));
}

template <typename T>
void FAGX_PropertyChangedDispatcher<T>::Add(
	const FName& Member, const FName& Property, UpdatePropertyFunction&& Function)
{
	Functions.Add(FAGX_NamePair {Member, Property}, std::move(Function));
}

template <typename T>
void FAGX_PropertyChangedDispatcher<T>::Trigger(FPropertyChangedEvent& Event)
{
	const FName Property = GetFNameSafe(Event.Property);
	const FName Member = GetFNameSafe(Event.MemberProperty);
	Trigger(Member, Property, Event);
}

template <typename T>
void FAGX_PropertyChangedDispatcher<T>::Trigger(struct FPropertyChangedChainEvent& Event)
{
	if (Event.PropertyChain.Num() == 0)
	{
		return;
	}

	FEditPropertyChain::TDoubleLinkedListNode* Node = Event.PropertyChain.GetHead();

	while (Node != nullptr)
	{
		// The name of the rest of the nodes doesn't matter, we set all elements at level two each
		// time. These are small objects such as FVector or FFloatInterval. Some rewrite of Property
		// Changed Dispatcher will be required to support more detailed nesting callbacks.
		// We search through the Event-chain until we find a match given Member/Property pair. One
		// example where this is important is for our AGX Actor types, where the AGX Component might
		// be edited from the Actor's root component in the Details Panel. In that case the
		// event-chain head will NOT point to the Component, but rather the Actor from within the
		// Component's PostEditChangeChainProperty.
		const FName Member = Node->GetValue()->GetFName();
		Node = Node->GetNextNode();
		const FName Property = Node != nullptr ? Node->GetValue()->GetFName() : Member;

		if (!FunctionExists(Member, Property))
		{
			// Continue trying to find match further down the Property Chain.
			continue;
		}

		Trigger(Member, Property, Event);
		break;
	}
}

template <typename T>
void FAGX_PropertyChangedDispatcher<T>::Trigger(
	const FName& Member, const FName& Property, FPropertyChangedEvent& Event)
{
	UpdatePropertyFunction* Function = GetFunction(Member, Property);
	if (Function == nullptr)
	{
		return;
	}

	for (int32 I = 0; I < Event.GetNumObjectsBeingEdited(); ++I)
	{
		// const_cast because there is no other way of getting the modified
		// object. We get a const object because the developers at Epic Games
		// really don't want us to modify the object while were still processing
		// the previous change. This means that the callback MAY NOT EDIT the
		// object passed to it, it may only update the native AGX Dynamics
		// object. Or at least, it may not edit the Unreal Engine object in a
		// way that is visible to Unreal Engine.
		T* Object = Cast<T>(const_cast<UObject*>(Event.GetObjectBeingEdited(I)));
		if (Object != nullptr)
		{
			(*Function)(Object);
		}
	}
}

template <typename T>
bool FAGX_PropertyChangedDispatcher<T>::IsInitialized() const
{
	return Functions.Num() != 0;
}

template <typename T>
typename FAGX_PropertyChangedDispatcher<T>::UpdatePropertyFunction*
FAGX_PropertyChangedDispatcher<T>::GetFunction(const FName& Member, const FName& Property)
{
	// First see if we  have a callback for this specific property.
	UpdatePropertyFunction* Function = Functions.Find({Member, Property});
	if (Function != nullptr)
	{
		return Function;
	}

	// Did not have a callback for the specific property, see if we have a callback for the entire
	// member.
	Function = Functions.Find({Member, Member});
	if (Function != nullptr)
	{
		return Function;
	}

	return nullptr;
}

#endif
