// Copyright 2026, Algoryx Simulation AB.

#pragma once

/*
 * This file contains a number of macros that can be used when writing repetitive code involving
 * Barrier types.
 */

//
// Macros dealing with Blueprint Function Libraries.
//

// To set a property, except for bool.
#define AGX_BARRIER_BP_SET_PROPERTY(Barrier, Property)                                    \
	if (!Barrier.HasNative())                                                             \
	{                                                                                     \
		UE_LOG(LogAGX, Error, TEXT("Accessed (None) trying to set %s"), TEXT(#Property)); \
		return;                                                                           \
	}                                                                                     \
                                                                                          \
	Barrier.Set##Property(Property)

// To set a bool property.
#define AGX_BARRIER_BP_SET_PROPERTY_BOOL(Barrier, Property)                               \
	if (!Barrier.HasNative())                                                             \
	{                                                                                     \
		UE_LOG(LogAGX, Error, TEXT("Accessed (None) trying to set %s"), TEXT(#Property)); \
		return;                                                                           \
	}                                                                                     \
                                                                                          \
	Barrier.Set##Property(b##Property)

// To get a property, except for bool.
#define AGX_BARRIER_BP_GET_PROPERTY(Barrier, Property, FallbackValue)                     \
	if (!Barrier.HasNative())                                                             \
	{                                                                                     \
		UE_LOG(LogAGX, Error, TEXT("Accessed (None) trying to get %s"), TEXT(#Property)); \
		return FallbackValue;                                                             \
	}                                                                                     \
                                                                                          \
	return Barrier.Get##Property()

// To get a bool property.
// Only provided for symmetry, the 'b'-prefix doesn't show up in the getter.
#define AGX_BARRIER_BP_GET_PROPERTY_BOOL(Barrier, Property, FallbackValue)                \
	if (!Barrier.HasNative())                                                             \
	{                                                                                     \
		UE_LOG(LogAGX, Error, TEXT("Accessed (None) trying to get %s"), TEXT(#Property)); \
		return FallbackValue;                                                             \
	}                                                                                     \
                                                                                          \
	return Barrier.Get##Property()

//
// Macros to set or get a Property that are backed by a Barrier.
//

// To set a Property.
#define AGX_BARRIER_SET_PROPERTY(PropertyName)             \
	if (HasNative())                                       \
	{                                                      \
		NativeBarrier.Set##PropertyName(In##PropertyName); \
	}                                                      \
	PropertyName = In##PropertyName;

// To get a property.
#define AGX_BARRIER_GET_PROPERTY(PropertyName)    \
	if (HasNative())                              \
	{                                             \
		return NativeBarrier.Get##PropertyName(); \
	}                                             \
	else                                          \
	{                                             \
		return PropertyName;                      \
	}

// Define both a setter and a getter for a property.
#define AGX_BARRIER_SET_GET_PROPERTY(ClassName, PropertyType, PropertyName) \
	void ClassName::Set##PropertyName(PropertyType In##PropertyName)        \
	{                                                                       \
		AGX_BARRIER_SET_PROPERTY(PropertyName);                             \
	}                                                                       \
                                                                            \
	PropertyType ClassName::Get##PropertyName() const                       \
	{                                                                       \
		AGX_BARRIER_GET_PROPERTY(PropertyName);                             \
	}
