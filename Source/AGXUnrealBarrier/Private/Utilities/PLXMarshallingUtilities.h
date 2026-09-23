// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Math/Vector.h"
#include "Templates/Function.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include <agx/Vec3.h>
#include "openplx/Marshalling.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

namespace PLXMarshallingUtilities
{
	struct FWindowLayout
	{
		openplx::Marshalling* Marshalling = nullptr;
		size_t Stride = 0;
		size_t BufferSize = 0;
		size_t NumWindows = 0;
	};

	const openplx::Field* FindField(
		const std::unordered_map<std::string, openplx::Field>& Fields, const std::string& Name);

	bool GetWindowLayout(
		openplx::Marshalling& Marshalling, FWindowLayout& OutLayout, bool bRequireBuffer);

	/**
	 * Get fields for a vector nested in the given marshalling object.
	 * The named vector's x, y, and z fields must be readable from InnerMarshalling.
	 */
	bool GetNestedVectorFields(
		openplx::Marshalling& InnerMarshalling, const std::string& MarshallingName,
		const openplx::Field*& OutXField, const openplx::Field*& OutYField,
		const openplx::Field*& OutZField);

	/**
	 * Read a vector from the given marshalling object.
	 * The named vector's x, y, and z fields must be readable from InnerMarshalling.
	 */
	bool ReadVector(
		openplx::Marshalling& InnerMarshalling, const std::string& MarshallingName,
		FVector& OutValue, TFunctionRef<FVector(const agx::Vec3&)> ConvertFunc,
		const TCHAR* DisplayName);

	template <typename T>
	T ReadValue(const uint8_t* Data)
	{
		T Value;
		std::memcpy(&Value, Data, sizeof(Value));
		return Value;
	}
}
