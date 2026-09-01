// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXLidarOutputView.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Utilities/PLXMarshallingUtilities.h"

// Standard library includes.
#include <array>
#include <cstring>
#include <limits>
#include <string>
#include <utility>

using namespace PLXMarshallingUtilities;

namespace OpenPLXLidarOutputView_helpers
{
	float ReadFloat(const uint8_t* Data)
	{
		return ReadValue<float>(Data);
	}

	template <typename T>
	struct TOpenPLXFieldType;

	template <>
	struct TOpenPLXFieldType<float>
	{
		static constexpr openplx::FieldType Value = openplx::FieldType::Real;
	};

	template <>
	struct TOpenPLXFieldType<double>
	{
		static constexpr openplx::FieldType Value = openplx::FieldType::Real;
	};

	template <>
	struct TOpenPLXFieldType<int32>
	{
		static constexpr openplx::FieldType Value = openplx::FieldType::Int;
	};

	template <typename T>
	bool DoesFieldMatchNativeType(const openplx::Field& Field)
	{
		return Field.field_type == TOpenPLXFieldType<T>::Value && Field.size == sizeof(T);
	}

	bool GetPositionFields(
		openplx::Marshalling& WindowMarshalling, const openplx::Field*& OutXField,
		const openplx::Field*& OutYField, const openplx::Field*& OutZField)
	{
		openplx::Marshalling* PositionMarshalling =
			WindowMarshalling.get_or_add_nested_marshalling("position3d").get();
		if (PositionMarshalling == nullptr)
			return false;

		const std::unordered_map<std::string, openplx::Field>& PositionFields =
			PositionMarshalling->get_field_map();
		OutXField = FindField(PositionFields, "x");
		OutYField = FindField(PositionFields, "y");
		OutZField = FindField(PositionFields, "z");
		return OutXField != nullptr && OutYField != nullptr && OutZField != nullptr;
	}

	bool GetRayPoseFields(
		openplx::Marshalling& WindowMarshalling, std::array<const openplx::Field*, 12>& OutFields)
	{
		std::unique_ptr<openplx::Marshalling>& RayPoseMarshallingPtr =
			WindowMarshalling.get_or_add_nested_marshalling("raypose");
		openplx::Marshalling* RayPoseMarshalling = RayPoseMarshallingPtr.get();
		if (RayPoseMarshalling == nullptr)
			return false;

		const auto& RayPoseFields = RayPoseMarshalling->get_field_map();
		for (size_t Row = 0; Row < 3; ++Row)
		{
			for (size_t Column = 0; Column < 4; ++Column)
			{
				const std::string Name = "e" + std::to_string(Row) + std::to_string(Column);
				const openplx::Field* Field = FindField(RayPoseFields, Name);
				if (Field == nullptr)
					return false;

				OutFields[Row * 4 + Column] = Field;
			}
		}

		return true;
	}

	bool ReadPositionsInternal(
		openplx::Marshalling& Marshalling, TArray<FVector>& OutPositions,
		const FTransform* RelativeTo)
	{
		OutPositions.Reset();
		if (Marshalling.get_buffer_size() == 0)
			return true;

		FWindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ true))
			return false;

		if (!CanConvert(Layout.NumWindows))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Refusing to read positions because the number of "
					 "positions is too large for a TArray."));
			return false;
		}

		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		if (!GetPositionFields(*Layout.Marshalling, XField, YField, ZField))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Tried to read positions, but this Lidar output "
					 "does not contain positions."));
			return false;
		}

		const size_t LastWindowOffset =
			Layout.NumWindows > 0 ? (Layout.NumWindows - 1) * Layout.Stride : 0;
		const size_t MaxFieldEnd = FMath::Max3(
			XField->offset + XField->size, YField->offset + YField->size,
			ZField->offset + ZField->size);
		if (Layout.NumWindows > 0 && LastWindowOffset + MaxFieldEnd > Layout.BufferSize)
			return false;

		const uint8_t* WindowBuffer = Layout.Marshalling->get_buffer();
		OutPositions.SetNumUninitialized(static_cast<int32>(Layout.NumWindows));
		for (int32 I = 0; I < OutPositions.Num(); ++I)
		{
			const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * Layout.Stride;
			const float X = ReadValue<float>(Window + XField->offset);
			const float Y = ReadValue<float>(Window + YField->offset);
			const float Z = ReadValue<float>(Window + ZField->offset);
			FVector Position = ConvertDisplacement(
				static_cast<agx::Real>(X), static_cast<agx::Real>(Y), static_cast<agx::Real>(Z));
			if (RelativeTo != nullptr)
				Position = RelativeTo->TransformPositionNoScale(Position);

			OutPositions[I] = Position;
		}

		return true;
	}

	template <typename OutT, typename NativeT, typename ConvertFuncT>
	bool ReadScalarFieldInternal(
		openplx::Marshalling& Marshalling, const std::string& FieldName, TArray<OutT>& OutValues,
		ConvertFuncT ConvertFunc, const TCHAR* FieldDisplayName)
	{
		OutValues.Reset();
		if (Marshalling.get_buffer_size() == 0)
			return true;

		FWindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ true))
			return false;

		if (!CanConvert(Layout.NumWindows))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Refusing to read %s because the number of "
					 "values is too large for a TArray."),
				FieldDisplayName);
			return false;
		}

		const openplx::Field* Field = FindField(Layout.Marshalling->get_field_map(), FieldName);
		if (Field == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Tried to read %s, but this Lidar output does "
					 "not contain %s."),
				FieldDisplayName, FieldDisplayName);
			return false;
		}

		if (!DoesFieldMatchNativeType<NativeT>(*Field))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Tried to read %s, but the OpenPLX field "
					 "type or size does not match the expected scalar layout."),
				FieldDisplayName);
			return false;
		}

		const size_t LastWindowOffset =
			Layout.NumWindows > 0 ? (Layout.NumWindows - 1) * Layout.Stride : 0;
		const size_t FieldEnd = Field->offset + Field->size;
		if (Layout.NumWindows > 0 && LastWindowOffset + FieldEnd > Layout.BufferSize)
			return false;

		const uint8_t* WindowBuffer = Layout.Marshalling->get_buffer();
		OutValues.SetNumUninitialized(static_cast<int32>(Layout.NumWindows));
		for (int32 I = 0; I < OutValues.Num(); ++I)
		{
			const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * Layout.Stride;
			OutValues[I] = ConvertFunc(ReadValue<NativeT>(Window + Field->offset));
		}

		return true;
	}

	bool ReadRayPosesInternal(openplx::Marshalling& Marshalling, TArray<FTransform>& OutRayPoses)
	{
		OutRayPoses.Reset();
		if (Marshalling.get_buffer_size() == 0)
			return true;

		FWindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ true))
			return false;

		if (!CanConvert(Layout.NumWindows))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Refusing to read ray poses because the number of "
					 "ray poses is too large for a TArray."));
			return false;
		}

		std::array<const openplx::Field*, 12> Fields;
		if (!GetRayPoseFields(*Layout.Marshalling, Fields))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Tried to read ray poses, but this Lidar output "
					 "does not contain ray poses."));
			return false;
		}

		size_t MaxFieldEnd = 0;
		for (const openplx::Field* Field : Fields)
		{
			MaxFieldEnd = FMath::Max(MaxFieldEnd, Field->offset + Field->size);
		}

		const size_t LastWindowOffset =
			Layout.NumWindows > 0 ? (Layout.NumWindows - 1) * Layout.Stride : 0;
		if (Layout.NumWindows > 0 && LastWindowOffset + MaxFieldEnd > Layout.BufferSize)
			return false;

		const uint8_t* WindowBuffer = Layout.Marshalling->get_buffer();
		OutRayPoses.SetNumUninitialized(static_cast<int32>(Layout.NumWindows));
		for (int32 I = 0; I < OutRayPoses.Num(); ++I)
		{
			const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * Layout.Stride;
			std::array<float, 12> E;
			for (size_t FieldIndex = 0; FieldIndex < Fields.size(); ++FieldIndex)
			{
				E[FieldIndex] = ReadFloat(Window + Fields[FieldIndex]->offset);
			}

			// clang-format off
			const agx::AffineMatrix4x4 RayPoseAGX {
				E[0], E[4], E[8],  0.0,
				E[1], E[5], E[9],  0.0,
				E[2], E[6], E[10], 0.0,
				E[3], E[7], E[11], 1.0};
			// clang-format on
			OutRayPoses[I] = Convert(RayPoseAGX);
		}

		return true;
	}

	void UpdateMaxFieldEnd(const openplx::Field* Field, size_t& InOutMaxFieldEnd)
	{
		InOutMaxFieldEnd = FMath::Max(InOutMaxFieldEnd, Field->offset + Field->size);
	}

	struct RawPointFieldCopy
	{
		const openplx::Field* SourceField = nullptr;
		size_t DestinationOffset = 0;
		size_t Size = 0;
		bool bIsHit = false;
	};

	bool ValidateRawField(
		const openplx::Field* Field, openplx::FieldType ExpectedOpenPLXType,
		size_t ExpectedSize, const TCHAR* FieldDisplayName)
	{
		if (Field->field_type == ExpectedOpenPLXType && Field->size == ExpectedSize)
			return true;

		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Lidar Output View: Tried to read raw %s, but the OpenPLX field "
				 "type or size does not match the expected raw layout."),
			FieldDisplayName);
		return false;
	}

	void AddRawField(
		const TCHAR* Name, EOpenPLXLidarPackedFieldType Type, const openplx::Field* SourceField,
		size_t Size, TArray<FOpenPLXLidarPackedField>& OutFields,
		TArray<RawPointFieldCopy>& OutFieldCopies, size_t& InOutPointStep, bool bIsHit = false)
	{
		FOpenPLXLidarPackedField PackedField;
		PackedField.Name = Name;
		PackedField.Type = Type;
		PackedField.Offset = static_cast<int64>(InOutPointStep);
		OutFields.Add(PackedField);

		RawPointFieldCopy FieldCopy;
		FieldCopy.SourceField = SourceField;
		FieldCopy.DestinationOffset = InOutPointStep;
		FieldCopy.Size = Size;
		FieldCopy.bIsHit = bIsHit;
		OutFieldCopies.Add(FieldCopy);

		InOutPointStep += Size;
	}

	bool AddRequestedRawScalarField(
		openplx::Marshalling& WindowMarshalling, bool bRequested, const std::string& SourceName,
		const TCHAR* PackedName, const TCHAR* FieldDisplayName, openplx::FieldType OpenPLXType,
		EOpenPLXLidarPackedFieldType PackedType, size_t Size,
		TArray<FOpenPLXLidarPackedField>& OutFields, TArray<RawPointFieldCopy>& OutFieldCopies,
		size_t& InOutPointStep, size_t& InOutMaxFieldEnd, bool bIsHit = false)
	{
		if (!bRequested)
			return true;

		const openplx::Field* SourceField = FindField(WindowMarshalling.get_field_map(), SourceName);
		if (SourceField == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Tried to read raw point data, but this Lidar "
					 "output does not contain %s."),
				FieldDisplayName);
			return false;
		}

		if (!ValidateRawField(SourceField, OpenPLXType, Size, FieldDisplayName))
			return false;

		UpdateMaxFieldEnd(SourceField, InOutMaxFieldEnd);
		AddRawField(
			PackedName, PackedType, SourceField, Size, OutFields, OutFieldCopies, InOutPointStep,
			bIsHit);
		return true;
	}

	bool ReadRawPointDataInternal(
		openplx::Marshalling& Marshalling, const FOpenPLXLidarPointReadFlags& ReadFlags,
		TArray<uint8>& OutData, TArray<FOpenPLXLidarPackedField>& OutFields, int64& OutPointStep,
		bool& bOutAllHits)
	{
		OutData.Reset();
		OutFields.Reset();
		OutPointStep = 0;
		bOutAllHits = true;

		FWindowLayout Layout;
		if (!GetWindowLayout(Marshalling, Layout, /*bRequireBuffer*/ true))
			return false;

		if (!CanConvert(Layout.NumWindows))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"OpenPLX Lidar Output View: Refusing to read raw point data because the "
					"number of points is too large for an int32."));
			return false;
		}

		size_t MaxFieldEnd = 0;
		size_t PointStep = 0;
		TArray<RawPointFieldCopy> FieldCopies;
		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		if (ReadFlags.bPositions)
		{
			if (!GetPositionFields(*Layout.Marshalling, XField, YField, ZField))
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT(
						"OpenPLX Lidar Output View: Tried to read raw point data, but this "
						"Lidar output does not contain positions."));
				return false;
			}

			if (!ValidateRawField(XField, openplx::FieldType::Real, sizeof(float), TEXT("x")) ||
				!ValidateRawField(YField, openplx::FieldType::Real, sizeof(float), TEXT("y")) ||
				!ValidateRawField(ZField, openplx::FieldType::Real, sizeof(float), TEXT("z")))
			{
				return false;
			}

			UpdateMaxFieldEnd(XField, MaxFieldEnd);
			UpdateMaxFieldEnd(YField, MaxFieldEnd);
			UpdateMaxFieldEnd(ZField, MaxFieldEnd);
			AddRawField(
				TEXT("x"), EOpenPLXLidarPackedFieldType::Float32, XField, sizeof(float), OutFields,
				FieldCopies, PointStep);
			AddRawField(
				TEXT("y"), EOpenPLXLidarPackedFieldType::Float32, YField, sizeof(float), OutFields,
				FieldCopies, PointStep);
			AddRawField(
				TEXT("z"), EOpenPLXLidarPackedFieldType::Float32, ZField, sizeof(float), OutFields,
				FieldCopies, PointStep);
		}

		if (!AddRequestedRawScalarField(
				*Layout.Marshalling, ReadFlags.bIntensities, "intensity", TEXT("intensity"),
				TEXT("intensities"), openplx::FieldType::Real, EOpenPLXLidarPackedFieldType::Float32,
				sizeof(float), OutFields, FieldCopies, PointStep, MaxFieldEnd))
		{
			return false;
		}

		if (!AddRequestedRawScalarField(
				*Layout.Marshalling, ReadFlags.bTimeStamps, "timestamp", TEXT("timestamp"),
				TEXT("timestamps"), openplx::FieldType::Real, EOpenPLXLidarPackedFieldType::Float64,
				sizeof(double), OutFields, FieldCopies, PointStep, MaxFieldEnd))
		{
			return false;
		}

		if (!AddRequestedRawScalarField(
				*Layout.Marshalling, ReadFlags.bDistances, "distance", TEXT("distance"),
				TEXT("distances"), openplx::FieldType::Real, EOpenPLXLidarPackedFieldType::Float32,
				sizeof(float), OutFields, FieldCopies, PointStep, MaxFieldEnd))
		{
			return false;
		}

		std::array<const openplx::Field*, 12> RayPoseFields;
		if (ReadFlags.bRayPoses)
		{
			if (!GetRayPoseFields(*Layout.Marshalling, RayPoseFields))
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT(
						"OpenPLX Lidar Output View: Tried to read raw point data, but this "
						"Lidar output does not contain ray poses."));
				return false;
			}

			const size_t RayPoseOffset = PointStep;
			FOpenPLXLidarPackedField RayPoseField;
			RayPoseField.Name = TEXT("raypose");
			RayPoseField.Type = EOpenPLXLidarPackedFieldType::Float32;
			RayPoseField.Offset = static_cast<int64>(RayPoseOffset);
			RayPoseField.Count = static_cast<int64>(RayPoseFields.size());
			OutFields.Add(RayPoseField);

			for (size_t FieldIndex = 0; FieldIndex < RayPoseFields.size(); ++FieldIndex)
			{
				const openplx::Field* Field = RayPoseFields[FieldIndex];
				if (!ValidateRawField(
						Field, openplx::FieldType::Real, sizeof(float), TEXT("ray poses")))
				{
					return false;
				}

				UpdateMaxFieldEnd(Field, MaxFieldEnd);

				RawPointFieldCopy FieldCopy;
				FieldCopy.SourceField = Field;
				FieldCopy.DestinationOffset = RayPoseOffset + FieldIndex * sizeof(float);
				FieldCopy.Size = sizeof(float);
				FieldCopies.Add(FieldCopy);
			}

			PointStep += RayPoseFields.size() * sizeof(float);
		}

		if (!AddRequestedRawScalarField(
				*Layout.Marshalling, ReadFlags.bIsHits, "is_hit", TEXT("is_hit"), TEXT("hit flags"),
				openplx::FieldType::Int, EOpenPLXLidarPackedFieldType::Int32, sizeof(int32),
				OutFields, FieldCopies, PointStep, MaxFieldEnd, /*bIsHit*/ true))
		{
			return false;
		}

		if (!AddRequestedRawScalarField(
				*Layout.Marshalling, ReadFlags.bEntityIds, "entity_id", TEXT("entity_id"),
				TEXT("entity IDs"), openplx::FieldType::Int, EOpenPLXLidarPackedFieldType::Int32,
				sizeof(int32), OutFields, FieldCopies, PointStep, MaxFieldEnd))
		{
			return false;
		}

		const size_t LastWindowOffset =
			Layout.NumWindows > 0 ? (Layout.NumWindows - 1) * Layout.Stride : 0;
		if (Layout.NumWindows > 0 && LastWindowOffset + MaxFieldEnd > Layout.BufferSize)
			return false;

		if (Layout.NumWindows > 0 && PointStep > 0 &&
			Layout.NumWindows > std::numeric_limits<size_t>::max() / PointStep)
		{
			return false;
		}

		const size_t DataSize = Layout.NumWindows * PointStep;
		if (!CanConvert(DataSize))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Refusing to read raw point data because the "
					 "output data is too large for a TArray."));
			return false;
		}

		OutPointStep = static_cast<int64>(PointStep);
		OutData.SetNumUninitialized(static_cast<int32>(DataSize));
		if (PointStep == 0)
			return true;

		const uint8_t* WindowBuffer = Layout.Marshalling->get_buffer();
		uint8* DestinationBuffer = OutData.GetData();
		for (size_t WindowIndex = 0; WindowIndex < Layout.NumWindows; ++WindowIndex)
		{
			const uint8_t* Window = WindowBuffer + WindowIndex * Layout.Stride;
			uint8* DestinationPoint = DestinationBuffer + WindowIndex * PointStep;
			for (const RawPointFieldCopy& FieldCopy : FieldCopies)
			{
				const uint8_t* Source = Window + FieldCopy.SourceField->offset;
				std::memcpy(DestinationPoint + FieldCopy.DestinationOffset, Source, FieldCopy.Size);
				if (FieldCopy.bIsHit && ReadValue<int32>(Source) == 0)
					bOutAllHits = false;
			}
		}

		return true;
	}
}

FOpenPLXLidarOutputView::FOpenPLXLidarOutputView()
	: NativeRef {new FOpenPLXLidarOutputViewRef}
{
}

FOpenPLXLidarOutputView::FOpenPLXLidarOutputView(std::shared_ptr<FOpenPLXLidarOutputViewRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

bool FOpenPLXLidarOutputView::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Marshalling != nullptr;
}

int32 FOpenPLXLidarOutputView::GetNumPoints() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return 0;

	FWindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return 0;

	if (!CanConvert(Layout.NumWindows))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Lidar Output View: Refusing to return the number of points because the "
				 "number of points is too large for an int32."));
		return 0;
	}

	return static_cast<int32>(Layout.NumWindows);
}

bool FOpenPLXLidarOutputView::HasPositions() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	const openplx::Field* XField = nullptr;
	const openplx::Field* YField = nullptr;
	const openplx::Field* ZField = nullptr;
	return GetPositionFields(*Layout.Marshalling, XField, YField, ZField);
}

bool FOpenPLXLidarOutputView::HasIntensities() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   PLXMarshallingUtilities::FindField(
			   Layout.Marshalling->get_field_map(), "intensity") != nullptr;
}

bool FOpenPLXLidarOutputView::HasTimeStamps() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   PLXMarshallingUtilities::FindField(
			   Layout.Marshalling->get_field_map(), "timestamp") != nullptr;
}

bool FOpenPLXLidarOutputView::HasDistances() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   PLXMarshallingUtilities::FindField(
			   Layout.Marshalling->get_field_map(), "distance") != nullptr;
}

bool FOpenPLXLidarOutputView::HasRayPoses() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	if (!GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false))
		return false;

	std::array<const openplx::Field*, 12> Fields;
	return GetRayPoseFields(*Layout.Marshalling, Fields);
}

bool FOpenPLXLidarOutputView::HasIsHits() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   PLXMarshallingUtilities::FindField(
			   Layout.Marshalling->get_field_map(), "is_hit") != nullptr;
}

bool FOpenPLXLidarOutputView::HasEntityIds() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   PLXMarshallingUtilities::FindField(
			   Layout.Marshalling->get_field_map(), "entity_id") != nullptr;
}

bool FOpenPLXLidarOutputView::ReadPositions(TArray<FVector>& OutPositions)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadPositionsInternal(
		*NativeRef->Marshalling, OutPositions, nullptr);
}

bool FOpenPLXLidarOutputView::ReadPositionsTransformed(
	const FTransform& RelativeTo, TArray<FVector>& OutPositions)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadPositionsInternal(
		*NativeRef->Marshalling, OutPositions, &RelativeTo);
}

bool FOpenPLXLidarOutputView::ReadIntensities(TArray<float>& OutIntensities)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<float, float>(
		*NativeRef->Marshalling, "intensity", OutIntensities, [](float Value) { return Value; },
		TEXT("intensities"));
}

bool FOpenPLXLidarOutputView::ReadTimeStamps(TArray<double>& OutTimeStamps)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<double, double>(
		*NativeRef->Marshalling, "timestamp", OutTimeStamps, [](double Value) { return Value; },
		TEXT("timestamps"));
}

bool FOpenPLXLidarOutputView::ReadDistances(TArray<double>& OutDistances)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<double, float>(
		*NativeRef->Marshalling, "distance", OutDistances,
		[](float Value) { return ConvertDistanceToUnreal<double>(static_cast<agx::Real>(Value)); },
		TEXT("distances"));
}

bool FOpenPLXLidarOutputView::ReadRayPoses(TArray<FTransform>& OutRayPoses)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadRayPosesInternal(
		*NativeRef->Marshalling, OutRayPoses);
}

bool FOpenPLXLidarOutputView::ReadIsHits(TArray<bool>& OutIsHits)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<bool, int32>(
		*NativeRef->Marshalling, "is_hit", OutIsHits, [](int32 Value) { return Value != 0; },
		TEXT("hit flags"));
}

bool FOpenPLXLidarOutputView::ReadEntityIds(TArray<int32>& OutEntityIds)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<int32, int32>(
		*NativeRef->Marshalling, "entity_id", OutEntityIds, [](int32 Value) { return Value; },
		TEXT("entity IDs"));
}

bool FOpenPLXLidarOutputView::ReadRawPointData(
	const FOpenPLXLidarPointReadFlags& ReadFlags, TArray<uint8>& OutData,
	TArray<FOpenPLXLidarPackedField>& OutFields, int64& OutPointStep, bool& bOutAllHits)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadRawPointDataInternal(
		*NativeRef->Marshalling, ReadFlags, OutData, OutFields, OutPointStep, bOutAllHits);
}

bool FOpenPLXLidarOutputView::MakePersistant()
{
	if (!HasNative())
		return false;

	std::shared_ptr<openplx::Marshalling> Detached = NativeRef->Marshalling->detach_copy();
	if (Detached == nullptr)
		return false;

	NativeRef->Marshalling = std::move(Detached);
	return true;
}

FOpenPLXLidarOutputViewRef* FOpenPLXLidarOutputView::GetNative()
{
	check(NativeRef);
	return NativeRef.get();
}

const FOpenPLXLidarOutputViewRef* FOpenPLXLidarOutputView::GetNative() const
{
	check(NativeRef);
	return NativeRef.get();
}
