// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/PLXMarshallingUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Math/UnrealMathUtility.h"

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
		openplx::Marshalling& InnerMarshalling, const std::string& MarshallingName,
		const openplx::Field*& OutXField, const openplx::Field*& OutYField,
		const openplx::Field*& OutZField)
	{
		openplx::Marshalling* VectorMarshalling =
			InnerMarshalling.get_or_add_nested_marshalling(MarshallingName).get();
		if (VectorMarshalling == nullptr)
			return false;

		const auto& VectorFields = VectorMarshalling->get_field_map();
		OutXField = FindField(VectorFields, "x");
		OutYField = FindField(VectorFields, "y");
		OutZField = FindField(VectorFields, "z");
		return OutXField != nullptr && OutYField != nullptr && OutZField != nullptr;
	}

	bool ReadVector(
		openplx::Marshalling& InnerMarshalling, const std::string& MarshallingName,
		FVector& OutValue, TFunctionRef<FVector(const agx::Vec3&)> ConvertFunc,
		const TCHAR* DisplayName)
	{
		OutValue = FVector::ZeroVector;
		if (InnerMarshalling.get_buffer_size() == 0)
			return true;

		if (InnerMarshalling.get_buffer() == nullptr)
			return false;

		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		if (!GetNestedVectorFields(InnerMarshalling, MarshallingName, XField, YField, ZField))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX: Tried to read %s, but the marshalling does not contain its vector "
					 "fields."),
				DisplayName);
			return false;
		}

		const size_t MaxFieldEnd = FMath::Max3(
			XField->offset + XField->size, YField->offset + YField->size,
			ZField->offset + ZField->size);
		if (MaxFieldEnd > InnerMarshalling.get_buffer_size())
			return false;

		const uint8_t* Buffer = InnerMarshalling.get_buffer();
		const agx::Vec3 ValueAGX {
			static_cast<agx::Real>(ReadValue<double>(Buffer + XField->offset)),
			static_cast<agx::Real>(ReadValue<double>(Buffer + YField->offset)),
			static_cast<agx::Real>(ReadValue<double>(Buffer + ZField->offset))};
		OutValue = ConvertFunc(ValueAGX);

		return true;
	}
}
