// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLXLidarOutputView.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "Utilities/PLXMarshallingUtilities.h"

// Standard library includes.
#include <array>
#include <memory>
#include <string>
#include <utility>

namespace OpenPLXLidarOutputView_helpers
{
	using namespace PLXMarshallingUtilities;

	bool GetPositionFields(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride,
		const openplx::Field*& OutXField, const openplx::Field*& OutYField,
		const openplx::Field*& OutZField)
	{
		return GetNestedVectorFields(
			WindowMarshalling, WindowStride, "position3d", IsFloatFieldInsideStride, OutXField,
			OutYField, OutZField);
	}

	const openplx::Field* GetIntensityField(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride)
	{
		const openplx::Field* Field = FindField(WindowMarshalling.get_field_map(), "intensity");
		return IsFloatFieldInsideStride(Field, WindowStride) ? Field : nullptr;
	}

	const openplx::Field* GetFloatField(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride, const std::string& Name)
	{
		const openplx::Field* Field = FindField(WindowMarshalling.get_field_map(), Name);
		return IsFloatFieldInsideStride(Field, WindowStride) ? Field : nullptr;
	}

	const openplx::Field* GetDoubleField(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride, const std::string& Name)
	{
		const openplx::Field* Field = FindField(WindowMarshalling.get_field_map(), Name);
		return IsDoubleFieldInsideStride(Field, WindowStride) ? Field : nullptr;
	}

	const openplx::Field* GetInt32Field(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride, const std::string& Name)
	{
		const openplx::Field* Field = FindField(WindowMarshalling.get_field_map(), Name);
		return IsInt32FieldInsideStride(Field, WindowStride) ? Field : nullptr;
	}

	bool GetRayPoseFields(
		openplx::Marshalling& WindowMarshalling, size_t WindowStride,
		std::array<const openplx::Field*, 12>& OutFields)
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
				const std::string Name =
					"e" + std::to_string(Row) + std::to_string(Column);
				const openplx::Field* Field = FindField(RayPoseFields, Name);
				if (!IsFloatFieldInsideStride(Field, WindowStride))
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
				TEXT(
					"OpenPLX Lidar Output View: Refusing to read positions because the number of "
					"positions is too large for a TArray."));
			return false;
		}

		const openplx::Field* XField = nullptr;
		const openplx::Field* YField = nullptr;
		const openplx::Field* ZField = nullptr;
		if (!GetPositionFields(*Layout.Marshalling, Layout.Stride, XField, YField, ZField))
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
				static_cast<agx::Real>(X), static_cast<agx::Real>(Y),
				static_cast<agx::Real>(Z));
			if (RelativeTo != nullptr)
				Position = RelativeTo->TransformPositionNoScale(Position);

			OutPositions[I] = Position;
		}

		return true;
	}

	template <typename OutT, typename NativeT, typename ConvertFuncT>
	bool ReadScalarFieldInternal(
		openplx::Marshalling& Marshalling, const std::string& FieldName,
		openplx::FieldType FieldType, TArray<OutT>& OutValues, ConvertFuncT ConvertFunc,
		const TCHAR* FieldDisplayName)
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
		if (!IsFieldInsideStride(Field, FieldType, sizeof(NativeT), Layout.Stride))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("OpenPLX Lidar Output View: Tried to read %s, but this Lidar output does "
					 "not contain %s."),
				FieldDisplayName, FieldDisplayName);
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
}

FOpenPLXLidarOutputView::FOpenPLXLidarOutputView()
	: NativeRef {new FOpenPLXLidarOutputViewRef}
{
}

FOpenPLXLidarOutputView::FOpenPLXLidarOutputView(
	std::shared_ptr<FOpenPLXLidarOutputViewRef> Native)
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

	return CanConvert(Layout.NumWindows) ? static_cast<int32>(Layout.NumWindows) : 0;
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
	return GetPositionFields(*Layout.Marshalling, Layout.Stride, XField, YField, ZField);
}

bool FOpenPLXLidarOutputView::HasIntensities() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   GetIntensityField(*Layout.Marshalling, Layout.Stride) != nullptr;
}

bool FOpenPLXLidarOutputView::HasTimeStamps() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   GetDoubleField(*Layout.Marshalling, Layout.Stride, "timestamp") != nullptr;
}

bool FOpenPLXLidarOutputView::HasDistances() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   GetFloatField(*Layout.Marshalling, Layout.Stride, "distance") != nullptr;
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
	return GetRayPoseFields(*Layout.Marshalling, Layout.Stride, Fields);
}

bool FOpenPLXLidarOutputView::HasIsHits() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   GetInt32Field(*Layout.Marshalling, Layout.Stride, "is_hit") != nullptr;
}

bool FOpenPLXLidarOutputView::HasEntityIds() const
{
	using namespace OpenPLXLidarOutputView_helpers;

	if (!HasNative())
		return false;

	FWindowLayout Layout;
	return GetWindowLayout(*NativeRef->Marshalling, Layout, /*bRequireBuffer*/ false) &&
		   GetInt32Field(*Layout.Marshalling, Layout.Stride, "entity_id") != nullptr;
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
	using namespace OpenPLXLidarOutputView_helpers;

	OutIntensities.Reset();
	if (!HasNative())
		return false;

	openplx::Marshalling* Marshalling = NativeRef->Marshalling.get();
	if (Marshalling->get_buffer_size() == 0)
		return true;

	FWindowLayout Layout;
	if (!GetWindowLayout(*Marshalling, Layout, /*bRequireBuffer*/ true))
		return false;

	if (!CanConvert(Layout.NumWindows))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"OpenPLX Lidar Output View: Refusing to read intensities because the number of "
				"intensities is too large for a TArray."));
		return false;
	}

	const openplx::Field* IntensityField =
		GetIntensityField(*Layout.Marshalling, Layout.Stride);
	if (IntensityField == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("OpenPLX Lidar Output View: Tried to read intensities, but this Lidar output "
				 "does not contain intensities."));
		return false;
	}

	const size_t LastWindowOffset =
		Layout.NumWindows > 0 ? (Layout.NumWindows - 1) * Layout.Stride : 0;
	const size_t FieldEnd = IntensityField->offset + IntensityField->size;
	if (Layout.NumWindows > 0 && LastWindowOffset + FieldEnd > Layout.BufferSize)
		return false;

	const uint8_t* WindowBuffer = Layout.Marshalling->get_buffer();
	OutIntensities.SetNumUninitialized(static_cast<int32>(Layout.NumWindows));
	for (int32 I = 0; I < OutIntensities.Num(); ++I)
	{
		const uint8_t* Window = WindowBuffer + static_cast<size_t>(I) * Layout.Stride;
		OutIntensities[I] = ReadValue<float>(Window + IntensityField->offset);
	}

	return true;
}

bool FOpenPLXLidarOutputView::ReadTimeStamps(TArray<double>& OutTimeStamps)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<double, double>(
		*NativeRef->Marshalling, "timestamp", openplx::FieldType::Real, OutTimeStamps,
		[](double Value) { return Value; }, TEXT("timestamps"));
}

bool FOpenPLXLidarOutputView::ReadDistances(TArray<double>& OutDistances)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<double, float>(
		*NativeRef->Marshalling, "distance", openplx::FieldType::Real, OutDistances,
		[](float Value)
		{
			return ConvertDistanceToUnreal<double>(static_cast<agx::Real>(Value));
		},
		TEXT("distances"));
}

bool FOpenPLXLidarOutputView::ReadRayPoses(TArray<FTransform>& OutRayPoses)
{
	using namespace OpenPLXLidarOutputView_helpers;

	OutRayPoses.Reset();
	if (!HasNative())
		return false;

	openplx::Marshalling* Marshalling = NativeRef->Marshalling.get();
	if (Marshalling->get_buffer_size() == 0)
		return true;

	FWindowLayout Layout;
	if (!GetWindowLayout(*Marshalling, Layout, /*bRequireBuffer*/ true))
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
	if (!GetRayPoseFields(*Layout.Marshalling, Layout.Stride, Fields))
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
			E[FieldIndex] = ReadValue<float>(Window + Fields[FieldIndex]->offset);
		}

		const agx::AffineMatrix4x4 RayPoseAGX {
			E[0], E[4], E[8], 0.0, E[1], E[5], E[9], 0.0,
			E[2], E[6], E[10], 0.0, E[3], E[7], E[11], 1.0};
		OutRayPoses[I] = Convert(RayPoseAGX);
	}

	return true;
}

bool FOpenPLXLidarOutputView::ReadIsHits(TArray<bool>& OutIsHits)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<bool, int32>(
		*NativeRef->Marshalling, "is_hit", openplx::FieldType::Int, OutIsHits,
		[](int32 Value) { return Value != 0; }, TEXT("hit flags"));
}

bool FOpenPLXLidarOutputView::ReadEntityIds(TArray<int32>& OutEntityIds)
{
	if (!HasNative())
		return false;

	return OpenPLXLidarOutputView_helpers::ReadScalarFieldInternal<int32, int32>(
		*NativeRef->Marshalling, "entity_id", openplx::FieldType::Int, OutEntityIds,
		[](int32 Value) { return Value; }, TEXT("entity IDs"));
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
