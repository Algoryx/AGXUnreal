// Copyright 2026, Algoryx Simulation AB.

#pragma once

// OpenPLX includes.
#include "BeginAGXIncludes.h"
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

	bool GetNestedVectorFields(
		openplx::Marshalling& WindowMarshalling, const std::string& MarshallingName,
		const openplx::Field*& OutXField, const openplx::Field*& OutYField,
		const openplx::Field*& OutZField);

	template <typename T>
	T ReadValue(const uint8_t* Data)
	{
		T Value;
		std::memcpy(&Value, Data, sizeof(Value));
		return Value;
	}
}
