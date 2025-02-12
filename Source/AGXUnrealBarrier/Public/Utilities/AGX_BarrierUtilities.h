// Copyright 2024, Algoryx Simulation AB.

#pragma once

#define AGX_BARRIER_SET_PROPERTY(PropertyName)             \
	if (HasNative())                                       \
	{                                                      \
		NativeBarrier.Set##PropertyName(In##PropertyName); \
	}                                                      \
	PropertyName = In##PropertyName;

#define AGX_BARRIER_GET_PROPERTY(PropertyName)    \
	if (HasNative())                              \
	{                                             \
		return NativeBarrier.Get##PropertyName(); \
	}                                             \
	else                                          \
	{                                             \
		return PropertyName;                      \
	}

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
