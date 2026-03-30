// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "Utilities/AGX_ObjectUtilities.h"

// Standard library includes.
#include <type_traits>

/*
 * This file contains a number of helper-macros useful for dealing with our asset/instance type
 * and property updates in general.
 */

/**
 * The C++ preprocessor makes it difficult to embed #if blocks within multi-line macros, so this
 * namespace defines wrapper for functions that are WITH_EDITOR only, where the non-WITH_EDITOR
 * versions does nothing.
 */
namespace AGX_WithEditorWrappers
{
#if WITH_EDITOR
	template <typename T>
	inline void Modify(T& Object)
	{
		Object.Modify();
	}

	inline void MarkAssetDirty(UObject& Asset)
	{
		FAGX_ObjectUtilities::MarkAssetDirty(Asset);
	}
#else
	template <typename T>
	inline void Modify(T&)
	{
	}

	inline void MarkAssetDirty(UObject& Asset)
	{
	}
#endif
}

// clang-format off

/**
 * @brief Final expansion of the various AGX_ASSET_SETTER macros.
 *
 * The rules are as follows:
 * - If the object that is modified is a runtime instance:
 *   - Modify that instance and the native, if there is one.
 *   - Do not propagate changes from instance to asset when the source is a Set function.
 * - If the object that is modified is an asset:
 *   - Prefer to modify that instance instead of the asset.
 *     - Runtime logic should only affect the runtime state, not the persistent asset.
 *   - If there is not instance, then modify that asset itself.
 *     - When modifying the asset behave as-if the change was made by an Unreal Editor user:
 *       - Mark the package dirty.
 *       - Support undo / redo.
 *
 * In some places it is not possible to use this macro directly, but we still want the logic to be
 * the same. If we decide to change the logic here then also update the following places:
 * - SetAndPropagateShovelProperty in AGX_ShovelProperties.cpp.
 * - SetAndPropagateShovelExcavationProperty in AGX_ShovelProperties.cpp.
 *
 * @param PropertyName The name of the property to set. May be a StructName.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 * @param HasNativeFunc Function to call in order to determine if the Barrier has a Native.
 * @param NativeName The name of the Barrier member variable.
 * @param BarrierMemberAccess The operator to use to access member functions in the Barrier, either '.' or '->'.
 */
 #define AGX_ASSET_SETTER_IMPL_INTERNAL( \
	PropertyName, InVar, SetFunc, HasNativeFunc, NativeName, BarrierMemberAccess) \
{ \
	if (IsInstance()) \
	{ \
		PropertyName = InVar; \
		if (HasNativeFunc()) \
		{ \
			NativeName BarrierMemberAccess SetFunc(InVar); \
		} \
	} \
	else \
	{ \
		if (Instance != nullptr) \
		{ \
			Instance->SetFunc(InVar); \
		} \
		else \
		{ \
			AGX_WithEditorWrappers::Modify(*this); \
			PropertyName = InVar; \
			AGX_WithEditorWrappers::MarkAssetDirty(*this); \
		} \
	} \
}

/**
 * @brief Set a new Property value on one of our asset/instance types, where the NativeBarrier is a pointer.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_IMPL_POINTER(PropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNative, NativeBarrier, ->)

/**
 * @brief Set a new Property value on one of our asset/instance types, where the NativeBarrier is held by-value.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_IMPL_VALUE(PropertyName, InVar, SetFunc) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNative, NativeBarrier, .)

/**
 * @brief Set a new Property value on one of our asset/instance types, where there are two NativeBarriers
 * and the one we want is a pointer.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_POINTER(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName, ->)

/**
 * @brief Set a new Property value on one of our asset/instance types, where there are two NativeBarriers
 * and the one we want is held by-value.
 * @param PropertyName The name of the property to set. May be a StructNAme.MemberVariableName identifier.
 * @param InVar The new value to assign to the property.
 * @param SetFunc The name of the function to call to set the value, both on an instance and a Barrier.
 */
#define AGX_ASSET_SETTER_DUAL_NATIVE_IMPL_VALUE(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_SETTER_IMPL_INTERNAL(PropertyName, InVar, SetFunc, HasNativeFunc, NativeName, .)


/**
 * @brief Final expansion of the various AGX_ASSET_GETTER macros.
 *
 * The rules are as follows:
 *  - If the object being modified is an asset that has an instance:
 *    - Let that instance's Get function decide what to do.
 *    - It will read from the native if there is one, otherwise the instance's property member.
 * - If the object being modified has a native:
 *    - Read from the native.
 *    - This can only happen for runtime instances, assets never have a native.
 * - If the object don't have an instance and don't have a native:
 *   - The only thing we can do is return the property member.
 */
#define AGX_ASSET_GETTER_IMPL_INTERNAL( \
	PropertyName, GetFunc, HasNativeFunc, NativeName, BarrierMemberAccess) \
{ \
	if (Instance != nullptr) \
	{ \
		return Instance->GetFunc(); \
	} \
	if (HasNativeFunc()) \
	{ \
		return NativeName BarrierMemberAccess GetFunc(); \
	} \
	return PropertyName; \
}

#define AGX_ASSET_GETTER_IMPL_POINTER(PropertyName, GetFunc) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNative, NativeBarrier, ->)

#define AGX_ASSET_GETTER_IMPL_VALUE(PropertyName, GetFunc) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNative, NativeBarrier, .)

#define AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_POINTER(PropertyName, GetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNativeFunc, NativeName, ->)

#define AGX_ASSET_GETTER_DUAL_NATIVE_IMPL_VALUE(PropertyName, GetFunc, HasNativeFunc, NativeName) \
	AGX_ASSET_GETTER_IMPL_INTERNAL(PropertyName, GetFunc, HasNativeFunc, NativeName, .)

// clang-format on
