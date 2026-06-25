// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/PLXMarshallingUtilities.h"

// Standard library includes.
#include <memory>

namespace PLXMarshallingUtilities
{
	const openplx::Field* FindField(
		const std::unordered_map<std::string, openplx::Field>& Fields, const std::string& Name)
	{
		const auto It = Fields.find(Name);
		return It != Fields.end() ? &It->second : nullptr;
	}

	bool IsFieldInsideStride(
		const openplx::Field* Field, openplx::FieldType FieldType, size_t FieldSize,
		size_t Stride)
	{
		return Field != nullptr && Field->field_type == FieldType && Field->size == FieldSize &&
			   Field->offset + Field->size <= Stride;
	}

	bool IsFloatFieldInsideStride(const openplx::Field* Field, size_t Stride)
	{
		return IsFieldInsideStride(Field, openplx::FieldType::Real, sizeof(float), Stride);
	}

	bool IsDoubleFieldInsideStride(const openplx::Field* Field, size_t Stride)
	{
		return IsFieldInsideStride(Field, openplx::FieldType::Real, sizeof(double), Stride);
	}

	bool IsInt32FieldInsideStride(const openplx::Field* Field, size_t Stride)
	{
		return IsFieldInsideStride(Field, openplx::FieldType::Int, sizeof(int32_t), Stride);
	}

	bool GetWindowLayout(
		openplx::Marshalling& Marshalling, FWindowLayout& OutLayout, bool bRequireBuffer)
	{
		if (bRequireBuffer && Marshalling.get_buffer_size() > 0 && Marshalling.get_buffer() == nullptr)
			return false;

		std::unique_ptr<openplx::Marshalling>& WindowMarshallingPtr =
			Marshalling.get_or_add_nested_marshalling("window");
		Marshalling.calculate_nested_buffer_sizes();

		openplx::Marshalling* WindowMarshalling = WindowMarshallingPtr.get();
		if (WindowMarshalling == nullptr)
			return false;

		if (bRequireBuffer && WindowMarshalling->get_buffer_size() > 0 &&
			WindowMarshalling->get_buffer() == nullptr)
		{
			return false;
		}

		const size_t WindowStride = WindowMarshalling->get_stride();
		const size_t WindowBufferSize = WindowMarshalling->get_buffer_size();
		if (WindowStride == 0 || WindowBufferSize % WindowStride != 0)
			return false;

		OutLayout.Marshalling = WindowMarshalling;
		OutLayout.Stride = WindowStride;
		OutLayout.BufferSize = WindowBufferSize;
		OutLayout.NumWindows = WindowBufferSize / WindowStride;
		return true;
	}

	bool GetNestedVectorFields(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride,
		const std::string& MarshallingName, FFieldStrideValidator FieldValidator,
		const openplx::Field*& OutXField, const openplx::Field*& OutYField,
		const openplx::Field*& OutZField)
	{
		std::unique_ptr<openplx::Marshalling>& VectorMarshallingPtr =
			WindowMarshalling.get_or_add_nested_marshalling(MarshallingName);
		openplx::Marshalling* VectorMarshalling = VectorMarshallingPtr.get();
		if (VectorMarshalling == nullptr || FieldValidator == nullptr)
			return false;

		const auto& VectorFields = VectorMarshalling->get_field_map();
		OutXField = FindField(VectorFields, "x");
		OutYField = FindField(VectorFields, "y");
		OutZField = FindField(VectorFields, "z");
		return FieldValidator(OutXField, WindowStride) &&
			   FieldValidator(OutYField, WindowStride) &&
			   FieldValidator(OutZField, WindowStride);
	}
}
