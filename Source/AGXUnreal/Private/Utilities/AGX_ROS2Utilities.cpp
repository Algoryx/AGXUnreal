// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_ROS2Utilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "Sensors/AGX_LidarOutputPosition.h"
#include "Sensors/AGX_LidarOutputPositionIntensity.h"
#include "Sensors/AGX_LidarScanPoint.h"

// Standard library includes.
#include <cstring>
#include <limits>

namespace AGX_ROS2Utilities_helpers
{
	enum class EAGX_PointFieldType : uint8
	{
		Int8 = 1,
		Uint8 = 2,
		Int16 = 3,
		Uint16 = 4,
		Int32 = 5,
		Uint32 = 6,
		Float32 = 7,
		Float64 = 8
	};

	FAGX_BuiltinInterfacesTime Convert(double TimeStamp)
	{
		FAGX_BuiltinInterfacesTime t;
		t.Sec = static_cast<int32>(TimeStamp);
		t.Nanosec = static_cast<int64>(TimeStamp * 1.0E9) % 1000000000;
		return t;
	}

	template <typename PixelType, typename OutputChannelType>
	FAGX_SensorMsgsImage SetAllExceptData(
		const TArray<PixelType>& Image, double TimeStamp, const FIntPoint& Resolution,
		bool Grayscale, const FString& ChannelSize)
	{
		FAGX_SensorMsgsImage Msg;

		Msg.IsBigendian = 0;
		Msg.Header.Stamp = Convert(TimeStamp);
		Msg.Height = static_cast<int64>(Resolution.Y);
		Msg.Width = static_cast<int64>(Resolution.X);

		if (Grayscale)
		{
			Msg.Step = Resolution.X * sizeof(OutputChannelType);
			Msg.Encoding = FString("mono") + ChannelSize;
		}
		else
		{
			Msg.Step = Resolution.X * sizeof(OutputChannelType) * 3;
			Msg.Encoding = FString("rgb") + ChannelSize;
		}

		return Msg;
	}

	FAGX_SensorMsgsPointField MakePointField(
		const FString& Name, int64 Offset, EAGX_PointFieldType Datatype, int64 Count)
	{
		FAGX_SensorMsgsPointField Field;
		Field.Name = Name;
		Field.Offset = Offset;
		Field.Datatype = static_cast<uint8>(Datatype);
		Field.Count = Count;
		return Field;
	};

	void AppendDoubleToUint8Array(double Val, TArray<uint8>& OutData)
	{
		uint64 Bits;
		static_assert(sizeof(Bits) == sizeof(Val));
		std::memcpy(&Bits, &Val, sizeof(Bits));
		for (int i = 0; i < sizeof(Val); i++)
		{
			OutData.Add(static_cast<uint8_t>(Bits & 0xFF));
			Bits >>= 8;
		}
	};

	void AppendFloatToUint8Array(float Val, TArray<uint8>& OutData)
	{
		uint32 Bits;
		static_assert(sizeof(Bits) == sizeof(Val));
		std::memcpy(&Bits, &Val, sizeof(Bits));
		for (int i = 0; i < sizeof(Val); i++)
		{
			OutData.Add(static_cast<uint8_t>(Bits & 0xFF));
			Bits >>= 8;
		}
	};

	auto AppendUint32ToUint8Array(uint32 Val, TArray<uint8>& OutData)
	{
		for (int i = 0; i < sizeof(uint32); i++)
		{
			OutData.Add(static_cast<uint8_t>(Val & 0xFF));
			Val >>= 8;
		}
	};
}

FAGX_SensorMsgsImage FAGX_ROS2Utilities::Convert(
	const TArray<FColor>& Image, double TimeStamp, const FIntPoint& Resolution, bool Grayscale)
{
	static_assert(sizeof(FColor::R) == sizeof(uint8));
	FAGX_SensorMsgsImage Msg = AGX_ROS2Utilities_helpers::SetAllExceptData<FColor, uint8>(
		Image, TimeStamp, Resolution, Grayscale, "8");

	if (Grayscale)
	{
		Msg.Data.Reserve(Image.Num());
		for (const auto& Color : Image)
		{
			const uint16 Sum = static_cast<uint16>(Color.R) + static_cast<uint16>(Color.G) +
							   static_cast<uint16>(Color.B);
			Msg.Data.Add(static_cast<uint8>(Sum / 3));
		}
	}
	else
	{
		Msg.Data.Reserve(Image.Num() * 3);
		for (const auto& Color : Image)
		{
			Msg.Data.Add(Color.R);
			Msg.Data.Add(Color.G);
			Msg.Data.Add(Color.B);
		}
	}

	return Msg;
}

FAGX_SensorMsgsImage FAGX_ROS2Utilities::Convert(
	const TArray<FFloat16Color>& Image, double TimeStamp, const FIntPoint& Resolution,
	bool Grayscale)
{
	FAGX_SensorMsgsImage Msg = AGX_ROS2Utilities_helpers::SetAllExceptData<FFloat16Color, uint16>(
		Image, TimeStamp, Resolution, Grayscale, "16");

	static constexpr float MaxUint16f = static_cast<float>(std::numeric_limits<uint16>::max());
	if (Grayscale)
	{
		Msg.Data.Reserve(Image.Num() * 2);
		for (const auto& Color : Image)
		{
			const FLinearColor LColor = Color.GetFloats();

			// Transform from [0..1] to uint16 range.
			const float Valf =
				FMath::Clamp((LColor.R + LColor.G + LColor.B) / 3.f, 0.f, 1.f) * MaxUint16f;
			const uint16 Val = static_cast<uint16>(Valf);

			Msg.Data.Add(static_cast<uint8>(Val & 0xFF)); // Low bits.
			Msg.Data.Add(static_cast<uint8>((Val >> 8) & 0xFF)); // High bits.
		}
	}
	else
	{
		Msg.Data.Reserve(Image.Num() * 2 * 3);
		for (const auto& Color : Image)
		{
			const FLinearColor LColor = Color.GetFloats();

			// Transform from [0..1] to uint16 range.
			const uint16 Rgb[3] = {
				static_cast<uint16>(FMath::Clamp(LColor.R, 0.f, 1.f) * MaxUint16f),
				static_cast<uint16>(FMath::Clamp(LColor.G, 0.f, 1.f) * MaxUint16f),
				static_cast<uint16>(FMath::Clamp(LColor.B, 0.f, 1.f) * MaxUint16f)};

			for (const uint16 C : Rgb)
			{
				Msg.Data.Add(static_cast<uint8>(C & 0xFF)); // Low bits.
				Msg.Data.Add(static_cast<uint8>((C >> 8) & 0xFF)); // High bits.
			}
		}
	}

	return Msg;
}

FAGX_BuiltinInterfacesTime UAGX_ROS2Utilities::ConvertTime(double TimeStamp)
{
	return AGX_ROS2Utilities_helpers::Convert(TimeStamp);
}

FAGX_SensorMsgsPointCloud2 UAGX_ROS2Utilities::ConvertXYZ(
	const TArray<FAGX_LidarScanPoint>& Points, bool DoublePrecision, bool ROSCoordinates,
	const FString& FrameId)
{
	using namespace AGX_ROS2Utilities_helpers;
	FAGX_SensorMsgsPointCloud2 Msg;

	const int32 FirstValidIndex =
		Points.IndexOfByPredicate([](const FAGX_LidarScanPoint& P) { return P.bIsValid; });
	if (FirstValidIndex == INDEX_NONE)
		return Msg;

	Msg.Header.Stamp = ConvertTime(Points[FirstValidIndex].TimeStamp);
	Msg.Header.FrameId = FrameId;
	const EAGX_PointFieldType FieldType =
		DoublePrecision ? EAGX_PointFieldType::Float64 : EAGX_PointFieldType::Float32;
	const int32 ElementStep = DoublePrecision ? 8 : 4;

	Msg.Fields.Add(MakePointField("x", 0, FieldType, 1));
	Msg.Fields.Add(MakePointField("y", 1 * ElementStep, FieldType, 1));
	Msg.Fields.Add(MakePointField("z", 2 * ElementStep, FieldType, 1));
	Msg.Fields.Add(MakePointField("intensity", 3 * ElementStep, FieldType, 1));

	Msg.IsBigendian = false;
	Msg.PointStep = 4 * ElementStep; // Bytes per point.
	Msg.IsDense = true;

	Msg.Data.Reserve(Points.Num() * Msg.PointStep);

	auto AppendToInt8Array = [&](double InData)
	{
		if (FieldType == EAGX_PointFieldType::Float32)
		{
			AppendFloatToUint8Array(static_cast<float>(InData), Msg.Data);
		}
		else if (FieldType == EAGX_PointFieldType::Float64)
		{
			AppendDoubleToUint8Array(InData, Msg.Data);
		}
		else
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Unsupported FieldType in lambda AppendToInt8Array in ConvertXYZ."));
		}
	};

	// Centimeter to meter.
	static constexpr double CtM = 0.01;

	for (int32 i = FirstValidIndex; i < Points.Num(); i++)
	{
		if (!Points[i].bIsValid)
			continue;

		FVector Pos = Points[i].Position;
		if (ROSCoordinates)
		{
			Pos = CtM * Pos;
			Pos.Y = -Pos.Y; // Flip Y due to left vs righ handed coordinates.
		}

		AppendToInt8Array(Pos.X);
		AppendToInt8Array(Pos.Y);
		AppendToInt8Array(Pos.Z);
		AppendToInt8Array(static_cast<double>(Points[i].Intensity));
	}

	// Since the points are unordered, height is 1 and width is the length of the point cloud.
	Msg.Height = 1;
	Msg.Width = Msg.Data.Num() / Msg.PointStep; // Num points.
	Msg.RowStep = Msg.Data.Num(); // Bytes per "row" which is the whole point cloud.

	return Msg;
}

FAGX_SensorMsgsPointCloud2 UAGX_ROS2Utilities::ConvertAnglesTOF(
	const TArray<FAGX_LidarScanPoint>& Points, const FString& FrameId)
{
	using namespace AGX_ROS2Utilities_helpers;
	FAGX_SensorMsgsPointCloud2 Msg;

	const int32 FirstValidIndex =
		Points.IndexOfByPredicate([](const FAGX_LidarScanPoint& P) { return P.bIsValid; });
	if (FirstValidIndex == INDEX_NONE)
		return Msg;

	Msg.Header.Stamp = ConvertTime(Points[FirstValidIndex].TimeStamp);
	Msg.Header.FrameId = FrameId;
	Msg.Fields.Add(MakePointField("angle_x", 0, EAGX_PointFieldType::Float64, 1));
	Msg.Fields.Add(MakePointField("angle_y", 8, EAGX_PointFieldType::Float64, 1));
	Msg.Fields.Add(MakePointField("tof", 16, EAGX_PointFieldType::Uint32, 1));
	Msg.Fields.Add(MakePointField("intensity", 20, EAGX_PointFieldType::Float64, 1));

	Msg.IsBigendian = false;
	Msg.PointStep = 28; // Size of a point.
	Msg.IsDense = true;

	// TimePiko = DistanceMeters / C * 1.0e12 * 2.
	// Where C is the speed of light. The factor 2 is because the ray travels to the object and back
	// again.
	// We collect the constants and get TimePiko = DistanceMeters * K.
	static constexpr double K = 2.0 * 1.0e12 / 299792458.0;

	Msg.Data.Reserve(Points.Num() * Msg.PointStep);
	for (int32 i = FirstValidIndex; i < Points.Num(); i++)
	{
		if (!Points[i].bIsValid)
			continue;

		const double AngleX = FMath::Atan2(Points[i].Position.Y, Points[i].Position.X);
		const double AngleY = FMath::Atan2(
			Points[i].Position.Z,
			FMath::Sqrt(FMath::Pow(Points[i].Position.X, 2) + FMath::Pow(Points[i].Position.Y, 2)));

		const double Distance = 0.01 * Points[i].Position.Length(); // In meters.
		const double TimePikoSecondsd = Distance * K;
		const uint32 TimePikoSeconds = [TimePikoSecondsd]()
		{
			if (TimePikoSecondsd > std::numeric_limits<uint32>::max())
			{
				// This means we have a measurement that is more than 643799 meters away which is
				// unlikely for a Lidar Sensor. We could use uint64 here, but that's not part of the
				// specification for sensor_msgs::PointField, so we stick with the supported uint32
				// until we need to represent larger values than this.
				UE_LOG(
					LogAGX, Warning,
					TEXT("ConvertAnglesTOF got time in pikoseconds: %f which is too large to store "
						 "in "
						 "an uint32. std::numeric_limits<uint32>::max() is used instead."),
					TimePikoSecondsd);
				return std::numeric_limits<uint32>::max();
			}
			return static_cast<uint32>(TimePikoSecondsd);
		}();

		// Append data to Msg.Data
		AppendDoubleToUint8Array(AngleX, Msg.Data);
		AppendDoubleToUint8Array(AngleY, Msg.Data);
		AppendUint32ToUint8Array(TimePikoSeconds, Msg.Data);
		AppendDoubleToUint8Array(Points[i].Intensity, Msg.Data);
	}

	// Since the points are unordered, height is 1 and width is the length of the point cloud.
	Msg.Height = 1;
	Msg.Width = Msg.Data.Num() / Msg.PointStep; // Num points.
	Msg.RowStep = Msg.Data.Num(); // Bytes per "row" which is the whole point cloud.

	return Msg;
}

FAGX_SensorMsgsPointCloud2 UAGX_ROS2Utilities::ConvertPositionData(
	const TArray<FAGX_LidarOutputPositionData>& Data, double TimeStamp, const FString& FrameId)
{
	using namespace AGX_ROS2Utilities_helpers;
	FAGX_SensorMsgsPointCloud2 Msg;

	Msg.Header.Stamp = ConvertTime(TimeStamp);
	Msg.Header.FrameId = FrameId;
	Msg.Fields.Add(MakePointField("x", 0, EAGX_PointFieldType::Float32, 1));
	Msg.Fields.Add(MakePointField("y", 4, EAGX_PointFieldType::Float32, 1));
	Msg.Fields.Add(MakePointField("z", 8, EAGX_PointFieldType::Float32, 1));

	Msg.IsBigendian = false;
	Msg.PointStep = 12; // Bytes per point.
	Msg.IsDense = true;

	// Centimeter to meter.
	static constexpr float CtM = 0.01f;

	Msg.Data.Reserve(Data.Num() * Msg.PointStep);
	for (const auto& D : Data)
	{
		FVector3f Pos = D.Position;
		Pos = CtM * Pos;
		Pos.Y = -Pos.Y; // Flip Y due to left vs righ handed coordinates.
		AppendFloatToUint8Array(D.Position.X, Msg.Data);
		AppendFloatToUint8Array(D.Position.Y, Msg.Data);
		AppendFloatToUint8Array(D.Position.Z, Msg.Data);
	}

	// Since the points are unordered, height is 1 and width is the length of the point cloud.
	Msg.Height = 1;
	Msg.Width = Msg.Data.Num() / Msg.PointStep; // Num points.
	Msg.RowStep = Msg.Data.Num(); // Bytes per "row" which is the whole point cloud.

	return Msg;
}

FAGX_SensorMsgsPointCloud2 UAGX_ROS2Utilities::ConvertPositionIntensityData(
	const TArray<FAGX_LidarOutputPositionIntensityData>& Data, double TimeStamp,
	const FString& FrameId)
{
	using namespace AGX_ROS2Utilities_helpers;
	FAGX_SensorMsgsPointCloud2 Msg;

	Msg.Header.Stamp = ConvertTime(TimeStamp);
	Msg.Header.FrameId = FrameId;
	Msg.Fields.Add(MakePointField("x", 0, EAGX_PointFieldType::Float32, 1));
	Msg.Fields.Add(MakePointField("y", 4, EAGX_PointFieldType::Float32, 1));
	Msg.Fields.Add(MakePointField("z", 8, EAGX_PointFieldType::Float32, 1));
	Msg.Fields.Add(MakePointField("intensity", 12, EAGX_PointFieldType::Float32, 1));

	Msg.IsBigendian = false;
	Msg.PointStep = 16; // Bytes per point.
	Msg.IsDense = true;

	// Centimeter to meter.
	static constexpr float CtM = 0.01f;

	Msg.Data.Reserve(Data.Num() * Msg.PointStep);
	for (const auto& D : Data)
	{
		FVector3f Pos = D.Position;
		Pos = CtM * Pos;
		Pos.Y = -Pos.Y; // Flip Y due to left vs righ handed coordinates.
		AppendFloatToUint8Array(D.Position.X, Msg.Data);
		AppendFloatToUint8Array(D.Position.Y, Msg.Data);
		AppendFloatToUint8Array(D.Position.Z, Msg.Data);
		AppendFloatToUint8Array(D.Intensity, Msg.Data);
	}

	// Since the points are unordered, height is 1 and width is the length of the point cloud.
	Msg.Height = 1;
	Msg.Width = Msg.Data.Num() / Msg.PointStep; // Num points.
	Msg.RowStep = Msg.Data.Num(); // Bytes per "row" which is the whole point cloud.

	return Msg;
}
