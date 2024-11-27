// Copyright 2024, Algoryx Simulation AB.

#pragma once

/// \todo This may become a header file with lots of includes, which will make
///       it a compile time hog. Consider splitting it  up.

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_MotionControl.h"
#include "AGX_RealInterval.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "Contacts/AGX_ContactEnums.h"
#include "Materials/AGX_ContactMaterialEnums.h"
#include "RigidBodyBarrier.h"
#include "Sensors/AGX_CustomPatternInterval.h"
#include "Sensors/AGX_LidarEnums.h"
#include "Terrain/AGX_ShovelEnums.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Utilities/DoubleInterval.h"
#include "Vehicle/AGX_TrackEnums.h"
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Interface_CollisionDataProviderCore.h"
#include "Logging/LogVerbosity.h"
#include "Math/Interval.h"
#include "Math/Matrix.h"
#include "Math/Quat.h"
#include "Math/TwoVectors.h"
#include "Math/Vector.h"
#include "Math/Vector2D.h"

// AGX Dynamics includes
#include "BeginAGXIncludes.h"
#include "agxTerrain/Shovel.h"
#include <agx/Constraint.h>
#include <agx/FrictionModel.h>
#include <agx/Line.h>
#include <agx/Notify.h>
#include <agx/Quat.h>
#include <agx/RigidBody.h>
#include <agx/Vec2.h>
#include <agx/Vec3.h>
#include <agxCollide/Contacts.h>
#include <agxModel/TwoBodyTire.h>
#include <agxSensor/LidarModelOusterOS.h>
#include <agxSensor/LidarRayAngleGaussianNoise.h>
#include <agxSensor/LidarRayPatternGenerator.h>
#include <agxSDK/ContactEventListener.h>
#include <agxTerrain/Shovel.h>
#include <agxUtil/agxUtil.h>
#include <agxVehicle/TrackInternalMergeProperties.h>
#include <agxVehicle/TrackWheel.h>
#include <agxWire/Node.h>
#include "EndAGXIncludes.h"

// Standard library includes.
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

// These functions assume that agx::Real and float are different types.
// They also assume that agx::Real has higher (or equal) precision than float.
// We support both float and double on the Unreal Engine side. Function name
// suffixes are used where necessary to disambiguate between return types.
//
// Naming conventions:
//
// Convert
//
// The default conversion function is named Convert. It is overloaded on the parameter type and
// detects if it is an AGX Dynamics type or an Unreal Engine type. double is considered an AGX
// Dynamics type and float an Unreal Engine type. It converts to the other. The conversion is just a
// cast, a plain Convert will never do any unit translations. It can do range checks, which when
// failed will result in an error message being printed and the value truncated. Composite types,
// such as Vector, calls Convert on its members.
//
//
// ConvertDistance
//
// Acts like Convert except that AGX Dynamics types are multiplied by 100 before being converted to
// the corresponding Unreal Engine type, and Unreal Engine types are divided by 100 after being
// converted to the AGX Dynamics type. The unit conversion is always performed using the AGX
// Dynamics types because we assume that agx::Real is at least as precise as float.
//
//
// ConvertAngle
//
// Text... Degrees/radians.
//
//
// Convert<UNIT>ToUnreal / Convert<UNIT>ToAgx
//
// Perform the same operation as Convert<UNIT>, where unit can be e.g., Distance or Angle, but with
// caller control over the return type. Used, for example, when we want to convert from an AGX
// Dynamics unit to an Unreal Engine unit but want the result as a double instead of a float. Also
// used when we have a value in one unit-space stored in the other type-space, e.g., an AGX Dynamics
// distance in an Unreal Engine type.
//
//
// ConvertUNITFloat
//
// The Float-suffix is added when the parameter type based overload produces the correct unit
// conversion but where the default conversion would produce a double, or double composite type, but
// we need a float, or a float composite.

template <typename T>
constexpr T AGX_TO_UNREAL_DISTANCE_FACTOR = T(100.0);

template <typename T>
constexpr T UNREAL_TO_AGX_DISTANCE_FACTOR = T(0.01);

//
// Scalars. AGX Dynamics to Unreal Engine.
//
// We provide untyped (not distance, angle, etc) scalar conversion functions
// because the naive Unreal=float AGX=double isn't true since Unreal Engine 5.
// 'float Convert(double)' and 'double Convert(float)' would be dangerous
// because in some circumstances, when a double is passed from Unreal Engine to
// AGX Dynamics, that could lead to a double -> float -> double round-trip
// through implicit type conversions. We therefore provide templated
// ConvertToUnreal and ConvertToAGX functions. The templating control the Unreal
// type and the AGX type is always agx::Real. So for ConvertToAGX the parameter
// is templated and for ConvertToUnreal the return value is templated.
//

static_assert(
	std::numeric_limits<agx::Real>::max() >= std::numeric_limits<float>::max(),
	"Expecting agx::Real to hold all values that float can hold.");

static_assert(
	std::numeric_limits<agx::Int>::max() >= std::numeric_limits<int32>::max(),
	"Expecting agx::Int to hold all positive values that int32 can hold.");

static_assert(
	std::numeric_limits<std::size_t>::max() >= std::numeric_limits<int32>::max(),
	"Expecting std::size_t to hold all positive values that int32 can hold.");

template <typename TU>
inline TU ConvertToUnreal(agx::Real D)
{
	return static_cast<TU>(D);
}

template <typename TU>
inline TU ConvertDistanceToUnreal(agx::Real D)
{
	return static_cast<TU>(D * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename TU>
inline TU ConvertAreaToUnreal(agx::Real D2)
{
	return static_cast<TU>(
		D2 * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real> * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename TU>
inline TU ConvertVolumeToUnreal(agx::Real D3)
{
	static constexpr agx::Real AGX_TO_UNREAL_VOLUME_FACTOR =
		AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real> * AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real> *
		AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>;

	return static_cast<TU>(D3 * AGX_TO_UNREAL_VOLUME_FACTOR);
}

template <typename TU>
inline TU ConvertDistanceInvToUnreal(agx::Real DInv)
{
	return static_cast<TU>(DInv / AGX_TO_UNREAL_DISTANCE_FACTOR<agx::Real>);
}

template <typename TU>
inline TU ConvertAngleToUnreal(agx::Real A)
{
	return static_cast<TU>(FMath::RadiansToDegrees(A));
}

inline int32 Convert(agx::Int I)
{
	static constexpr agx::Int MaxAllowed = static_cast<agx::Int>(std::numeric_limits<int32>::max());
	if (I > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Too large agx::Int being converted to int32, value is truncated."));
		I = MaxAllowed;
	}
	return static_cast<int32>(I);
}

inline int32 Convert(std::size_t S)
{
	static constexpr std::size_t MaxAllowed =
		static_cast<std::size_t>(std::numeric_limits<int32>::max());
	if (S > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Too large size_t being converted to int32, value is truncated."));
		S = MaxAllowed;
	}
	return static_cast<int32>(S);
}

//
// Scalars. Unreal Engine to AGX Dynamics.
//

template <typename TU>
inline agx::Real ConvertToAGX(TU D)
{
	return static_cast<agx::Real>(D);
}

template <typename TU>
inline agx::Real ConvertDistanceToAGX(TU D)
{
	return static_cast<agx::Real>(D) * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

template <typename TU>
inline agx::Real ConvertAreaToAGX(TU D2)
{
	return static_cast<agx::Real>(D2) * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real> *
		   UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

template <typename TU>
inline agx::Real ConvertVolumeToAGX(TU D3)
{
	static constexpr agx::Real UNREAL_TO_AGX_VOLUME_FACTOR =
		UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real> * UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real> *
		UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;

	return static_cast<agx::Real>(D3) * UNREAL_TO_AGX_VOLUME_FACTOR;
}

template <typename TU>
inline agx::Real ConvertDistanceInvToAGX(TU DInv)
{
	return static_cast<agx::Real>(DInv) / UNREAL_TO_AGX_DISTANCE_FACTOR<agx::Real>;
}

template <typename TU>
inline agx::Real ConvertAngleToAGX(TU A)
{
	return FMath::DegreesToRadians(static_cast<agx::Real>(A));
}

inline agx::Int Convert(int32 I)
{
	return static_cast<agx::Int>(I);
}

//
// Two-dimensional vectors. AGX Dynamics to Unreal Engine.
//

inline FVector2D Convert(const agx::Vec2& V)
{
	return FVector2D(
		ConvertToUnreal<decltype(FVector2D::X)>(V.x()),
		ConvertToUnreal<decltype(FVector2D::X)>(V.y()));
}

inline FVector2D ConvertDistance(const agx::Vec2& V)
{
	return FVector2D(
		ConvertDistanceToUnreal<decltype(FVector2D::X)>(V.x()),
		ConvertDistanceToUnreal<decltype(FVector2D::X)>(V.y()));
}

// No ConvertVector for two-dimensional vectors because there is no handedness here, so it would be
// identical to ConvertDistance.

//
// Two-dimensional vectors. Unreal Engine to AGX Dynamics.
//

inline agx::Vec2 Convert(const FVector2D& V)
{
	return agx::Vec2(ConvertToAGX(V.X), ConvertToAGX(V.Y));
}

inline agx::Vec2 ConvertDistance(const FVector2D& V)
{
	return agx::Vec2(ConvertDistanceToAGX(V.X), ConvertDistanceToAGX(V.Y));
}

//
// Three-dimensional vectors. AGX Dynamics to Unreal Engine.
//

/*
 * There are a few different cases here, characterized by whether we convert cm <> m and
 * whether we flip the Y axis, since Unreal Engine is left-handed and AGX Dynamics is
 * right-handed.
 *
 *             Convert cm <> m
 *       |     No    |    Yes       |
 *     --|-----------|--------------|
 *   F N |           | Convert      |
 *   l o | Convert   | Distance     |
 *   i   |           |              |
 *   p --|-----------|--------------|
 *     Y | Convert   | Convert      |
 *   Y e | Vector    | Displacement |
 *     s |           |              |
 *     --|-----------|--------------|
 *
 *
 * Angular velocity is a beast of its own with a big comment all to itself.
 */

inline FVector Convert(const agx::Vec3& V)
{
	return FVector(
		ConvertToUnreal<decltype(FVector::X)>(V.x()), ConvertToUnreal<decltype(FVector::X)>(V.y()),
		ConvertToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertDistance(const agx::Vec3& V)
{
	return FVector(
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.x()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.y()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertVector(const agx::Vec3& V)
{
	// Negate Y because Unreal is left-handed and AGX Dynamics is right-handed.
	return FVector(
		ConvertToUnreal<decltype(FVector::X)>(V.x()), -ConvertToUnreal<decltype(FVector::X)>(V.y()),
		ConvertToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertDisplacement(const agx::Vec3& V)
{
	// Negate Y because Unreal is left-handed and AGX Dynamics is right-handed.
	return FVector(
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.x()),
		-ConvertDistanceToUnreal<decltype(FVector::X)>(V.y()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.z()));
}

// Float, i.e. agx::Vec3f, versions of the above.

inline FVector Convert(const agx::Vec3f& V)
{
	using ElementType = decltype(FVector::X);
	return FVector(
		static_cast<ElementType>(V.x()), static_cast<ElementType>(V.y()),
		static_cast<ElementType>(V.z()));
}

inline FVector ConvertDistance(const agx::Vec3f& V)
{
	using ElementType = decltype(FVector::X);
	return FVector(
		ConvertDistanceToUnreal<ElementType>(V.x()), ConvertDistanceToUnreal<ElementType>(V.y()),
		ConvertDistanceToUnreal<ElementType>(V.z()));
}

inline FVector ConvertDisplacement(const agx::Vec3f& V)
{
	// Negate Y because Unreal is left-handed and AGX Dynamics is right-handed.
	return FVector(
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.x()),
		-ConvertDistanceToUnreal<decltype(FVector::X)>(V.y()),
		ConvertDistanceToUnreal<decltype(FVector::X)>(V.z()));
}

inline FVector ConvertDisplacement(agx::Real X, agx::Real Y, agx::Real Z)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return FVector(
		ConvertDistanceToUnreal<decltype(X)>(X), -ConvertDistanceToUnreal<decltype(X)>(Y),
		ConvertDistanceToUnreal<decltype(X)>(Z));
}

inline FVector3f ConvertDisplacement(agx::Real32 X, agx::Real32 Y, agx::Real32 Z)
{
	// Negate Y because Unreal is left handed and AGX Dynamics is right handed.
	return FVector3f(
		ConvertDistanceToUnreal<decltype(X)>(X), -ConvertDistanceToUnreal<decltype(X)>(Y),
		ConvertDistanceToUnreal<decltype(X)>(Z));
}

inline FVector ConvertFloatVector(const agx::Vec3f& V)
{
	// Negate Y because Unreal is left-handed and AGX Dynamics is right-handed.
	return FVector(
		ConvertToUnreal<decltype(FVector::X)>(V.x()), -ConvertToUnreal<decltype(FVector::X)>(V.y()),
		ConvertToUnreal<decltype(FVector::X)>(V.z()));
}

// Rotation-related.

inline FVector ConvertAngularVelocity(const agx::Vec3& V)
{
	/*
	 * Angular velocities in Unreal are weird. Even rotations are kind of weird. We're basing this
	 * conversion on the rotation widget in the Details Panel. Unreal Engine uses a left-handed
	 * coordinate system, meaning that thumb=X, index=Y, middle=Z matches the left hand. Normally,
	 * rotations also has a handedness. Imagine gripping the axis around which we rotate with your
	 * thumb pointing towards increasing axis values and look at your (usually) four non-thumb
	 * fingers. Their direction from the knuckles towards the fingertips define the direction of
	 * positive rotation. If you switch hand then the direction of positive rotation is inverted.
	 * Unreal Engine, at least according to the rotation widget in the Details Panel, uses
	 * right-handed rotations for the X and Y axes, and left-handed rotations for the Z axis.
	 *
	 * Axis | Rotation
	 *      | Handedness
	 * ---------------
	 *  X	| Right
	 *  Y	| Right
	 *  Z	| Left
	 *
	 * AGX Dynamics is right-handed throughout. There are two sets of flips going on, one because of
	 * the left-vs-right-handedness of the coordinate system itself and one for the
	 * left-vs-right-handedness of each axis' rotation. The X axis point in the same direction in
	 * both cases and is right-handed in both cases, so we pass it through untouched. The Y axis
	 * should be negated because of the right-to-left switch of the coordinate system, but the
	 * rotations are right-handed in both cases so one negation is enough. The Z axis point in the
	 * same direction in both cases, so no negation there, but the handedness of rotations around Z
	 * is different so we must negate it for that reason.
	 */
	return FVector(
		FMath::RadiansToDegrees(ConvertToUnreal<decltype(FVector::X)>(V.x())),
		FMath::RadiansToDegrees(-ConvertToUnreal<decltype(FVector::X)>(V.y())),
		FMath::RadiansToDegrees(-ConvertToUnreal<decltype(FVector::X)>(V.z())));
}

inline FVector ConvertTorque(const agx::Vec3& V)
{
	/*
	 * Following a similar logic as ConvertAngularVelocity for the axis directions, but no unit
	 * conversion since we use Nm in both AGX Dynamics and Unreal Engine.
	 */
	// clang-format off
	return {
		ConvertToUnreal<decltype(FVector::X)>(V.x()),
		-ConvertToUnreal<decltype(FVector::Y)>(V.y()),
		-ConvertToUnreal<decltype(FVector::Z)>(V.z())};
	// clang-format on
}

//
// Three-dimensional vectors. Unreal Engine to AGX Dynamics.
//

inline agx::Vec3 Convert(const FVector& V)
{
	return agx::Vec3(ConvertToAGX(V.X), ConvertToAGX(V.Y), ConvertToAGX(V.Z));
}

inline agx::Vec3 ConvertDistance(const FVector& V)
{
	return agx::Vec3(
		ConvertDistanceToAGX(V.X), ConvertDistanceToAGX(V.Y), ConvertDistanceToAGX(V.Z));
}

inline agx::Vec3 ConvertVector(const FVector& V)
{
	return agx::Vec3(ConvertToAGX(V.X), -ConvertToAGX(V.Y), ConvertToAGX(V.Z));
}

inline agx::Vec3 ConvertDisplacement(const FVector& V)
{
	// Negate Y because Unreal is left-handed and AGX Dynamics is right-handed.
	return agx::Vec3(
		ConvertDistanceToAGX(V.X), -ConvertDistanceToAGX(V.Y), ConvertDistanceToAGX(V.Z));
}

// Float, i.e. agx::Vec3f, versions for three-dimensional vectors. Unreal Engine to AGX Dynamics.
// Same as the set above, but agx::Vec3f instead of agx::Vec3.

inline agx::Vec3f ConvertFloat(const FVector& V)
{
	return agx::Vec3f(static_cast<float>(V.X), static_cast<float>(V.Y), static_cast<float>(V.Z));
}

inline agx::Vec3f ConvertFloatDistance(const FVector& V)
{
	return agx::Vec3f(
		static_cast<float>(ConvertDistanceToAGX(V.X)),
		static_cast<float>(ConvertDistanceToAGX(V.Y)),
		static_cast<float>(ConvertDistanceToAGX(V.Z)));
}

inline agx::Vec3f ConvertFloatVector(const FVector& V)
{
	// Negate Y because Unreal is left-handed and AGX Dynamics is right-handed.
	// clang-format off
	return agx::Vec3f(
		static_cast<float>(ConvertToAGX(V.X)),
		-static_cast<float>(ConvertToAGX(V.Y)),
		static_cast<float>(ConvertToAGX(V.Z)));
	// clang-format on
}

inline agx::Vec3f ConvertFloatDisplacement(const FVector& V)
{
	// Negate Y because Unreal is left-handed and AGX Dynamics is right-handed.
	// clang-format off
	return agx::Vec3f(
		static_cast<float>(ConvertDistanceToAGX(V.X)),
		-static_cast<float>(ConvertDistanceToAGX(V.Y)),
		static_cast<float>(ConvertDistanceToAGX(V.Z)));
	// clang-format on
}

// Rotation-related.

inline agx::Vec3 ConvertAngularVelocity(const FVector& V)
{
	// See comment in the AGX-to-Unreal version of this function.
	// clang-format off
	return agx::Vec3(
		ConvertToAGX(FMath::DegreesToRadians(V.X)),
		-ConvertToAGX(FMath::DegreesToRadians(V.Y)),
		-ConvertToAGX(FMath::DegreesToRadians(V.Z)));
	// clang-format on
}

inline agx::Vec3 ConvertTorque(const FVector& V)
{
	/*
	 * Following a similar logic as ConvertAngularVelocity for the axis directions, but no unit
	 * conversion since we use Nm in both AGX Dynamics and Unreal Engine.
	 */
	return {ConvertToAGX(V.X), -ConvertToAGX(V.Y), -ConvertToAGX(V.Z)};
}

//
// Four-dimensional vectors. AGX Dynamics to Unreal Engine.
//

inline FVector4 Convert(const agx::Vec4& V)
{
	return FVector4(
		ConvertToUnreal<decltype(FVector4::X)>(V.x()),
		ConvertToUnreal<decltype(FVector4::X)>(V.y()),
		ConvertToUnreal<decltype(FVector4::X)>(V.z()),
		ConvertToUnreal<decltype(FVector4::X)>(V.w()));
}

inline FVector4 Convert(const agx::Vec4f& V)
{
	return FVector4(
		ConvertToUnreal<decltype(FVector4::X)>(V.x()),
		ConvertToUnreal<decltype(FVector4::X)>(V.y()),
		ConvertToUnreal<decltype(FVector4::X)>(V.z()),
		ConvertToUnreal<decltype(FVector4::X)>(V.w()));
}

//
// Four-dimensional vectors. Unreal Engine to AGX Dynamics.
//

inline agx::Vec4 Convert(const FVector4& V)
{
	return agx::Vec4(
		ConvertToAGX<decltype(FVector4::X)>(V.X), ConvertToAGX<decltype(FVector4::X)>(V.Y),
		ConvertToAGX<decltype(FVector4::X)>(V.Z), ConvertToAGX<decltype(FVector4::X)>(V.W));
}

inline agx::Vec4f ConvertFloat(const FVector4& V)
{
	return agx::Vec4f((float) V.X, (float) V.Y, (float) V.Z, (float) V.W);
}

//
// Interval/Range. AGX Dynamics to Unreal Engine.
//
// We've had some issues with the Interval classes built into Unreal Engine. Partly because they
// were float-only in Unreal Engine 4 (not sure about 5 yet), party because Unreal Engine 4.27
// crashes whenever an FFloatInterval Property containing infinity is displayed in a Details panel,
// and partly because they don't support scientific notation in the Details panel.
//
// Because of these we introduced FAGX_DoubleInterval and then FAGX_RealInterval.
//

inline FAGX_RealInterval Convert(const agx::RangeReal& R)
{
	return FAGX_RealInterval {R.lower(), R.upper()};
}

inline FAGX_RealInterval ConvertDistance(const agx::RangeReal& R)
{
	return FAGX_RealInterval {
		ConvertDistanceToUnreal<double>(R.lower()), ConvertDistanceToUnreal<double>(R.upper())};
}

inline FAGX_RealInterval ConvertAngle(const agx::RangeReal& R)
{
	return FAGX_RealInterval {
		ConvertAngleToUnreal<double>(R.lower()), ConvertAngleToUnreal<double>(R.upper())};
}

inline FAGX_RealInterval Convert(const agx::RangeReal32& R)
{
	return FAGX_RealInterval {R.lower(), R.upper()};
}

inline FAGX_RealInterval ConvertDistance(const agx::RangeReal32& R)
{
	return FAGX_RealInterval {
		ConvertDistanceToUnreal<double>(R.lower()), ConvertDistanceToUnreal<double>(R.upper())};
}

inline FAGX_RealInterval ConvertAngle(const agx::RangeReal32& R)
{
	return FAGX_RealInterval {
		ConvertAngleToUnreal<double>(R.lower()), ConvertAngleToUnreal<double>(R.upper())};
}

inline FAGX_CustomPatternInterval Convert(const agxSensor::LidarRayPatternInterval& I)
{
	return FAGX_CustomPatternInterval(I.first, I.numRays);
}

//
// Interval/Range. Unreal Engine to AGX Dynamics.
//

inline agx::RangeReal Convert(const FAGX_RealInterval& I)
{
	return agx::RangeReal(I.Min, I.Max);
}

inline agx::RangeReal ConvertDistance(const FAGX_RealInterval& I)
{
	return agx::RangeReal(ConvertDistanceToAGX(I.Min), ConvertDistanceToAGX(I.Max));
}

inline agx::RangeReal ConvertAngle(const FAGX_RealInterval& I)
{
	return agx::RangeReal(ConvertAngleToAGX(I.Min), ConvertAngleToAGX(I.Max));
}

inline agxSensor::LidarRayPatternInterval Convert(const FAGX_CustomPatternInterval& I)
{
	return agxSensor::LidarRayPatternInterval(I.First, I.NumRays);
}

//
// TwoVectors/Line. Unreal Engine to AGX Dynamics
// TwoVectors may represent other things as well. If that's the case then we'll
// need to do something else.
//

inline agx::Line Convert(const FTwoVectors& Vs)
{
	return {Convert(Vs.v1), Convert(Vs.v2)};
}

inline agx::Line ConvertDistance(const FTwoVectors& Vs)
{
	return {ConvertDistance(Vs.v1), ConvertDistance(Vs.v2)};
}

inline agx::Line ConvertDisplacement(const FTwoVectors& Vs)
{
	return {ConvertDisplacement(Vs.v1), ConvertDisplacement(Vs.v2)};
}

//
// TwoVectors/Line. AGX Dynamics to Unreal Engine.
//

inline FTwoVectors Convert(const agx::Line& Vs)
{
	return {Convert(Vs.p1), Convert(Vs.p2)};
}

inline FTwoVectors ConvertDistance(const agx::Line& Vs)
{
	return {ConvertDistance(Vs.p1), ConvertDistance(Vs.p2)};
}

inline FTwoVectors ConvertDisplacement(const agx::Line& Vs)
{
	return {ConvertDisplacement(Vs.p1), ConvertDisplacement(Vs.p2)};
}

//
// Quaternions.
//

inline FQuat Convert(const agx::Quat& V)
{
	return FQuat(
		ConvertToUnreal<decltype(FQuat::X)>(V.x()), -ConvertToUnreal<decltype(FQuat::X)>(V.y()),
		ConvertToUnreal<decltype(FQuat::X)>(V.z()), -ConvertToUnreal<decltype(FQuat::X)>(V.w()));
}

inline agx::Quat Convert(const FQuat& V)
{
	return agx::Quat(
		ConvertToAGX<decltype(FQuat::X)>(V.X), -ConvertToAGX<decltype(FQuat::X)>(V.Y),
		ConvertToAGX<decltype(FQuat::X)>(V.Z), -ConvertToAGX<decltype(FQuat::X)>(V.W));
}

//
// Transformations.
//

inline FTransform Convert(const agx::AffineMatrix4x4& T)
{
	const FVector Translation = ConvertDisplacement(T.getTranslate());
	const FQuat Rotation = Convert(T.getRotate());
	return FTransform(Rotation, Translation);
}

inline agx::FrameRef ConvertFrame(const FVector& FramePosition, const FQuat& FrameRotation)
{
	return new agx::Frame(
		agx::AffineMatrix4x4(Convert(FrameRotation), ConvertDisplacement(FramePosition)));
}

inline FTransform ConvertLocalFrame(const agx::Frame* Frame)
{
	return FTransform(
		Convert(Frame->getLocalRotate()), ConvertDisplacement(Frame->getLocalTranslate()));
}

inline agx::AffineMatrix4x4 ConvertMatrix(const FVector& FramePosition, const FQuat& FrameRotation)
{
	return agx::AffineMatrix4x4(Convert(FrameRotation), ConvertDisplacement(FramePosition));
}

inline agx::AffineMatrix4x4 Convert(const FTransform& Transform)
{
	return ConvertMatrix(Transform.GetLocation(), Transform.GetRotation());
}

inline agx::AffineMatrix4x4Vector Convert(const TArray<FTransform>& Transforms)
{
	agx::AffineMatrix4x4Vector V;
	V.reserve(Transforms.Num());
	for (const auto& Transform : Transforms)
		V.push_back(Convert(Transform));

	return V;
}

//
// Text.
//

inline FString Convert(const agx::String& StringAGX)
{
	return FString(UTF8_TO_TCHAR(StringAGX.c_str()));
}

inline FString Convert(const agx::Name& NameAGX)
{
	// Due to different memory allocators it is not safe to copy an agx::Name between AGX Dynamics
	// and Unreal Engine shared libraries, or to move ownership of the underlying memory buffer. By
	// passing the return value of c_str() to the FString constructor we create a copy of the
	// characters within the Unreal Engine shared library that is owned by that shared library,
	// and the AGX Dynamics agx::Name retain ownership of the old memory buffer.
	return FString(NameAGX.c_str());
}

inline agx::String Convert(const FString& StringUnreal)
{
	return agx::String(TCHAR_TO_UTF8(*StringUnreal));
}

inline agx::Name Convert(const FName& NameUnreal)
{
	return agx::Name(TCHAR_TO_UTF8(*(NameUnreal.ToString())));
}

inline uint32 StringTo32BitFnvHash(const FString& StringUnreal)
{
	TArray<TCHAR> Chars = StringUnreal.GetCharArray();

	if (Chars.Last() == '\0')
	{
		Chars.Pop();
	}

	uint32 Hash = 2166136261U;
	for (const auto& SingleChar : Chars)
	{
		Hash ^= SingleChar;
		Hash *= 16777619U;
	}

	return Hash;
}

//
// [GU]uid
//

inline FGuid Convert(const agx::Uuid& Uuid)
{
	// To ensure the same textual representations (i.e. if printed out to a log for example) we go
	// via string representations when converting the Uuid. The underlying data storage of each type
	// may not be bitwise equal.
	FString UuidStrUnreal;
	{
		agx::String UuidStrAGX = Uuid.str();
		UuidStrUnreal = Convert(UuidStrAGX);

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(UuidStrAGX);
	}

	return FGuid(UuidStrUnreal);
}

inline agx::Uuid Convert(const FGuid& Guid)
{
	// To ensure the same textual representations (i.e. if printed out to a log for example) we go
	// via string representations when converting the Guid. The underlying data storage of each type
	// may not be bitwise equal.
	const FString GuidStr = Guid.ToString(EGuidFormats::DigitsWithHyphens).ToLower();
	agx::String GuidStrAGX = Convert(GuidStr);

#if 0
	// AGX Dynamics UUIDs require formatting with groupings with '-' separators accordingly.
	GuidStrAGX.insert(20, "-");
	GuidStrAGX.insert(16, "-");
	GuidStrAGX.insert(12, "-");
	GuidStrAGX.insert(8, "-");
#endif

	return agx::Uuid(GuidStrAGX);
}

//
// Triangle data.
//

inline agx::UInt32Vector ConvertIndices(const TArray<FTriIndices>& Indices)
{
	agx::UInt32Vector IndicesAGX;
	IndicesAGX.reserve(Indices.Num() * 3);
	for (const FTriIndices& Index : Indices)
	{
		AGX_CHECK(Index.v0 >= 0);
		AGX_CHECK(Index.v1 >= 0);
		AGX_CHECK(Index.v2 >= 0);

		IndicesAGX.push_back(static_cast<uint32>(Index.v0));
		IndicesAGX.push_back(static_cast<uint32>(Index.v1));
		IndicesAGX.push_back(static_cast<uint32>(Index.v2));
	}

	return IndicesAGX;
}

inline agx::Vec3Vector ConvertVertices(const TArray<FVector>& Vertices)
{
	agx::Vec3Vector VerticesAGX;
	VerticesAGX.reserve(Vertices.Num());
	for (const FVector& Vertex : Vertices)
	{
		VerticesAGX.push_back(ConvertDisplacement(Vertex));
	}

	return VerticesAGX;
}

//
// Enumerations, contacts.
//

inline agxCollide::ContactPoint::ContactForceComponents Convert(
	EAGX_ContactForceComponents Component)
{
	switch (Component)
	{
		case EAGX_ContactForceComponents::NormalForce:
			return agxCollide::ContactPoint::NORMAL_FORCE;
		case EAGX_ContactForceComponents::TangentialForceU:
			return agxCollide::ContactPoint::TANGENTIAL_FORCE_U;
		case EAGX_ContactForceComponents::TangentialForceV:
			return agxCollide::ContactPoint::TANGENTIAL_FORCE_V;
	}
}

inline EAGX_ContactForceComponents Convert(
	agxCollide::ContactPoint::ContactForceComponents Component)
{
	switch (Component)
	{
		case agxCollide::ContactPoint::NORMAL_FORCE:
			return EAGX_ContactForceComponents::NormalForce;
		case agxCollide::ContactPoint::TANGENTIAL_FORCE_U:
			return EAGX_ContactForceComponents::TangentialForceU;
		case agxCollide::ContactPoint::TANGENTIAL_FORCE_V:
			return EAGX_ContactForceComponents::TangentialForceV;
	}
}

inline agxSDK::ContactEventListener::ActivationMask Convert(EAGX_ContactListenerActivationMask Mask)
{
	// This is a mask, meaning the bit patterns must be the same in both types. Cannot do a switch
	// case since there will be too many permutations to test.
	int MaskInt = static_cast<int>(Mask);
	auto MaskAGX = static_cast<agxSDK::ContactEventListener::ActivationMask>(MaskInt);
	return MaskAGX;
}

inline EAGX_ContactListenerActivationMask Convert(agxSDK::ContactEventListener::ActivationMask Mask)
{
	// This is a mask, meaning the bit patterns must be the same in both types. Cannot do a switch
	// case since there will be too many permutations to test.
	int MaskInt = static_cast<int>(Mask);
	auto MaskUnreal = static_cast<EAGX_ContactListenerActivationMask>(MaskInt);
	return MaskUnreal;
}

inline agxSDK::ContactEventListener::KeepContactPolicy Convert(EAGX_KeepContactPolicy Mask)
{
	// This is a mask, meaning the bit patterns must be the same in both types. Cannot do a switch
	// case since there will be too many permutations to test.
	int MaskInt = static_cast<int>(Mask);
	auto MaskAGX = static_cast<agxSDK::ContactEventListener::KeepContactPolicy>(MaskInt);
	return MaskAGX;
}

inline EAGX_KeepContactPolicy Convert(agxSDK::ContactEventListener::KeepContactPolicy Mask)
{
	// This is a mask, meaning the bit patterns must be the same in both types. Cannot do a switch
	// case since there will be too many permutations to test.
	int MaskInt = static_cast<int>(Mask);
	auto MaskUnreal = static_cast<EAGX_KeepContactPolicy>(MaskInt);
	return MaskUnreal;
}

//
// Enumerations, RigidBody.
//

inline agx::RigidBody::MotionControl Convert(EAGX_MotionControl V)
{
	switch (V)
	{
		case MC_STATIC:
			return agx::RigidBody::STATIC;
		case MC_KINEMATICS:
			return agx::RigidBody::KINEMATICS;
		case MC_DYNAMICS:
			return agx::RigidBody::DYNAMICS;
	}
	/// \todo Add UE_LOG(LogAGX, ...) here.
	return agx::RigidBody::DYNAMICS;
}

inline EAGX_MotionControl Convert(agx::RigidBody::MotionControl V)
{
	switch (V)
	{
		case agx::RigidBody::STATIC:
			return MC_STATIC;
		case agx::RigidBody::KINEMATICS:
			return MC_KINEMATICS;
		case agx::RigidBody::DYNAMICS:
			return MC_DYNAMICS;
	}
	/// \todo Add UE_LOG(LogAGX, ...) here.
	return MC_KINEMATICS;
}

//
// Enumerations, Constraint.
//

inline agx::Constraint2DOF::DOF Convert(EAGX_Constraint2DOFFreeDOF Dof)
{
	check(Dof == EAGX_Constraint2DOFFreeDOF::FIRST || Dof == EAGX_Constraint2DOFFreeDOF::SECOND);

	return Dof == EAGX_Constraint2DOFFreeDOF::FIRST ? agx::Constraint2DOF::FIRST
													: agx::Constraint2DOF::SECOND;
}

//
// Enumerations, Lidar.
//

inline agxSensor::LidarRayAngleGaussianNoise::Axis Convert(EAGX_LidarRayAngleDistortionAxis Axis)
{
	switch (Axis)
	{
		case EAGX_LidarRayAngleDistortionAxis::AxisX:
			return agxSensor::LidarRayAngleGaussianNoise::Axis::AXIS_X;
		case EAGX_LidarRayAngleDistortionAxis::AxisY:
			return agxSensor::LidarRayAngleGaussianNoise::Axis::AXIS_Y;
		case EAGX_LidarRayAngleDistortionAxis::AxisZ:
			return agxSensor::LidarRayAngleGaussianNoise::Axis::AXIS_Z;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "EAGX_LidarRayAngleDistortionAxis literal with unknown value to "
			 "an agxSensor::LidarRayAngleGaussianNoise::Axis literal."));
	return agxSensor::LidarRayAngleGaussianNoise::Axis::AXIS_X;
}

inline EAGX_LidarRayAngleDistortionAxis Convert(agxSensor::LidarRayAngleGaussianNoise::Axis Axis)
{
	switch (Axis)
	{
		case agxSensor::LidarRayAngleGaussianNoise::Axis::AXIS_X:
			return EAGX_LidarRayAngleDistortionAxis::AxisX;
		case agxSensor::LidarRayAngleGaussianNoise::Axis::AXIS_Y:
			return EAGX_LidarRayAngleDistortionAxis::AxisY;
		case agxSensor::LidarRayAngleGaussianNoise::Axis::AXIS_Z:
			return EAGX_LidarRayAngleDistortionAxis::AxisZ;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "agxSensor::LidarRayAngleGaussianNoise::Axis literal with unknown value to "
			 "an EAGX_LidarRayAngleDistortionAxis literal."));
	return EAGX_LidarRayAngleDistortionAxis::AxisX;
}

inline agxSensor::LidarModelOusterOS::ChannelCount Convert(EAGX_OusterOSChannelCount Count)
{
	switch (Count)
	{
		case EAGX_OusterOSChannelCount::CH_32:
			return agxSensor::LidarModelOusterOS::ch_32;
		case EAGX_OusterOSChannelCount::CH_64:
			return agxSensor::LidarModelOusterOS::ch_64;
		case EAGX_OusterOSChannelCount::CH_128:
			return agxSensor::LidarModelOusterOS::ch_128;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "EAGX_OusterOSChannelCount literal with unknown value to "
			 "an agxSensor::LidarModelOusterOS::ChannelCount literal."));
	return agxSensor::LidarModelOusterOS::ch_32;
}

inline EAGX_OusterOSChannelCount Convert(agxSensor::LidarModelOusterOS::ChannelCount Count)
{
	switch (Count)
	{
		case agxSensor::LidarModelOusterOS::ch_32:
			return EAGX_OusterOSChannelCount::CH_32;
		case agxSensor::LidarModelOusterOS::ch_64:
			return EAGX_OusterOSChannelCount::CH_64;
		case agxSensor::LidarModelOusterOS::ch_128:
			return EAGX_OusterOSChannelCount::CH_128;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "agxSensor::LidarModelOusterOS::ChannelCount literal with unknown value to "
			 "an EAGX_OusterOSChannelCount literal."));
	return EAGX_OusterOSChannelCount::CH_32;
}

inline agxSensor::LidarModelOusterOS::BeamSpacing Convert(EAGX_OusterOSBeamSpacing BeamSpacing)
{
	switch (BeamSpacing)
	{
		case EAGX_OusterOSBeamSpacing::Uniform:
			return agxSensor::LidarModelOusterOS::Uniform;
		case EAGX_OusterOSBeamSpacing::AboveHorizon:
			return agxSensor::LidarModelOusterOS::AboveHorizon;
		case EAGX_OusterOSBeamSpacing::BelowHorizon:
			return agxSensor::LidarModelOusterOS::BelowHorizon;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "EAGX_OusterOSBeamSpacing literal with unknown value to "
			 "an agxSensor::LidarModelOusterOS::BeamSpacing literal."));
	return agxSensor::LidarModelOusterOS::Uniform;
}

inline EAGX_OusterOSBeamSpacing Convert(agxSensor::LidarModelOusterOS::BeamSpacing Spacing)
{
	switch (Spacing)
	{
		case agxSensor::LidarModelOusterOS::Uniform:
			return EAGX_OusterOSBeamSpacing::Uniform;
		case agxSensor::LidarModelOusterOS::AboveHorizon:
			return EAGX_OusterOSBeamSpacing::AboveHorizon;
		case agxSensor::LidarModelOusterOS::BelowHorizon:
			return EAGX_OusterOSBeamSpacing::BelowHorizon;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "agxSensor::LidarModelOusterOS::BeamSpacing literal with unknown value to "
			 "an EAGX_OusterOSBeamSpacing literal."));
	return EAGX_OusterOSBeamSpacing::Uniform;
}

inline agxSensor::LidarModelOusterOS::LidarMode Convert(EAGX_OusterOSMode Mode)
{
	switch (Mode)
	{
		case EAGX_OusterOSMode::Mode_512x10:
			return agxSensor::LidarModelOusterOS::Mode_512x10;
		case EAGX_OusterOSMode::Mode_512x20:
			return agxSensor::LidarModelOusterOS::Mode_512x20;
		case EAGX_OusterOSMode::Mode_1024x10:
			return agxSensor::LidarModelOusterOS::Mode_1024x10;
		case EAGX_OusterOSMode::Mode_1024x20:
			return agxSensor::LidarModelOusterOS::Mode_1024x20;
		case EAGX_OusterOSMode::Mode_2048x10:
			return agxSensor::LidarModelOusterOS::Mode_2048x10;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "EAGX_OusterOSMode literal with unknown value to "
			 "an agxSensor::LidarModelOusterOS::LidarMode literal."));
	return agxSensor::LidarModelOusterOS::Mode_512x10;
}


inline EAGX_OusterOSMode Convert(agxSensor::LidarModelOusterOS::LidarMode Mode)
{
	switch (Mode)
	{
		case agxSensor::LidarModelOusterOS::Mode_512x10:
			return EAGX_OusterOSMode::Mode_512x10;
		case agxSensor::LidarModelOusterOS::Mode_512x20:
			return EAGX_OusterOSMode::Mode_512x20;
		case agxSensor::LidarModelOusterOS::Mode_1024x10:
			return EAGX_OusterOSMode::Mode_1024x10;
		case agxSensor::LidarModelOusterOS::Mode_1024x20:
			return EAGX_OusterOSMode::Mode_1024x20;
		case agxSensor::LidarModelOusterOS::Mode_2048x10:
			return EAGX_OusterOSMode::Mode_2048x10;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Conversion failed: Tried to convert an "
			 "agxSensor::LidarModelOusterOS::LidarMode literal with unknown value to "
			 "an EAGX_OusterOSMode literal."));
	return EAGX_OusterOSMode::Mode_512x10;
}

//
// Enumerations, Materials.
//

inline agx::FrictionModel::SolveType Convert(EAGX_ContactSolver ContactSolver)
{
	switch (ContactSolver)
	{
		case EAGX_ContactSolver::Direct:
			return agx::FrictionModel::SolveType::DIRECT;
		case EAGX_ContactSolver::Iterative:
			return agx::FrictionModel::SolveType::ITERATIVE;
		case EAGX_ContactSolver::Split:
			return agx::FrictionModel::SolveType::SPLIT;
		case EAGX_ContactSolver::DirectAndIterative:
			return agx::FrictionModel::SolveType::DIRECT_AND_ITERATIVE;
		case EAGX_ContactSolver::NotDefined:
			return agx::FrictionModel::SolveType::NOT_DEFINED;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an "
					 "EAGX_ContactSolver literal with unknown value to "
					 "an agxModel::FrictionModel::SolveType literal."));
			return agx::FrictionModel::SolveType::NOT_DEFINED;
	}
}

inline EAGX_ContactSolver Convert(agx::FrictionModel::SolveType SolveType)
{
	switch (SolveType)
	{
		case agx::FrictionModel::SolveType::DIRECT:
			return EAGX_ContactSolver::Direct;
		case agx::FrictionModel::SolveType::ITERATIVE:
			return EAGX_ContactSolver::Iterative;
		case agx::FrictionModel::SolveType::SPLIT:
			return EAGX_ContactSolver::Split;
		case agx::FrictionModel::SolveType::DIRECT_AND_ITERATIVE:
			return EAGX_ContactSolver::DirectAndIterative;
		case agx::FrictionModel::SolveType::NOT_DEFINED:
			return EAGX_ContactSolver::NotDefined;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an "
					 "EAGX_ContactSolver literal with unknown value to "
					 "an agxModel::FrictionModel::SolveType literal."));
			return EAGX_ContactSolver::NotDefined;
	}
}

inline agx::ContactMaterial::ContactReductionMode Convert(EAGX_ContactReductionMode Mode)
{
	switch (Mode)
	{
		case EAGX_ContactReductionMode::None:
			return agx::ContactMaterial::ContactReductionMode::REDUCE_NONE;
		case EAGX_ContactReductionMode::Geometry:
			return agx::ContactMaterial::ContactReductionMode::REDUCE_GEOMETRY;
		case EAGX_ContactReductionMode::All:
			return agx::ContactMaterial::ContactReductionMode::REDUCE_ALL;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an EAGX_ContactReductionMode literal "
					 "with unknown value to an agx::ContactMaterial::ContactReductionMode."))
			return agx::ContactMaterial::ContactReductionMode::REDUCE_NONE;
	}
}

inline agx::UInt8 ConvertContactReductionLevelToAGX(
	EAGX_ContactReductionLevel ContactReductionLevel)
{
	switch (ContactReductionLevel)
	{
		case EAGX_ContactReductionLevel::Default:
			return 0;
		case EAGX_ContactReductionLevel::Aggressive:
			return 1;
		case EAGX_ContactReductionLevel::Moderate:
			return 2;
		case EAGX_ContactReductionLevel::Minimal:
			return 3;
		default:
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an EAGX_ContactReductionLevel literal "
					 "with unknown value to an agx::UInt8. Default contact reduction level is "
					 "returned."))
			return 0;
		}
	}
}

inline EAGX_ContactReductionMode Convert(agx::ContactMaterial::ContactReductionMode Mode)
{
	switch (Mode)
	{
		case agx::ContactMaterial::REDUCE_NONE:
			return EAGX_ContactReductionMode::None;
		case agx::ContactMaterial::ContactReductionMode::REDUCE_GEOMETRY:
			return EAGX_ContactReductionMode::Geometry;
		case agx::ContactMaterial::ContactReductionMode::REDUCE_ALL:
			return EAGX_ContactReductionMode::All;
		default:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Conversion failed: Tried to convert an "
					 "agx::ContactMaterial::ContactReductionMode "
					 "with unknown value to an EAGX_ContactReductionMode."));
			return EAGX_ContactReductionMode::None;
	}
}

inline EAGX_ContactReductionLevel ConvertContactReductionLevelToUnreal(agx::UInt8 Level)
{
	switch (Level)
	{
		case 0:
			return EAGX_ContactReductionLevel::Default;
		case 1:
			return EAGX_ContactReductionLevel::Aggressive;
		case 2:
			return EAGX_ContactReductionLevel::Moderate;
		case 3:
			return EAGX_ContactReductionLevel::Minimal;
		default:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Tried to convert an agx::UInt8: %d to an EAGX_ContactReductionLevel, but the "
					 "value is larger than the corresponding largest enum literal. Returning "
					 "EAGX_ContactReductionLevel::Minimal."),
				Level);
			AGX_CHECK(Level > static_cast<agx::UInt8>(EAGX_ContactReductionLevel::Minimal));
			return EAGX_ContactReductionLevel::Minimal;
	}
}

//
// Enumerations, Tire.
//

inline agxModel::TwoBodyTire::DeformationMode Convert(FTwoBodyTireBarrier::DeformationMode Mode)
{
	switch (Mode)
	{
		case FTwoBodyTireBarrier::RADIAL:
			return agxModel::TwoBodyTire::RADIAL;
		case FTwoBodyTireBarrier::LATERAL:
			return agxModel::TwoBodyTire::LATERAL;
		case FTwoBodyTireBarrier::BENDING:
			return agxModel::TwoBodyTire::BENDING;
		case FTwoBodyTireBarrier::TORSIONAL:
			return agxModel::TwoBodyTire::TORSIONAL;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an FTwoBodyTireBarrier::DeformationMode "
					 "literal of unknown type to an agxModel::TwoBodyTire::DeformationMode "
					 "literal. Returning agxModel::TwoBodyTire::RADIAL."));
			return agxModel::TwoBodyTire::RADIAL;
	}
}

inline FTwoBodyTireBarrier::DeformationMode Convert(agxModel::TwoBodyTire::DeformationMode Mode)
{
	switch (Mode)
	{
		case agxModel::TwoBodyTire::RADIAL:
			return FTwoBodyTireBarrier::RADIAL;
		case agxModel::TwoBodyTire::LATERAL:
			return FTwoBodyTireBarrier::LATERAL;
		case agxModel::TwoBodyTire::BENDING:
			return FTwoBodyTireBarrier::BENDING;
		case agxModel::TwoBodyTire::TORSIONAL:
			return FTwoBodyTireBarrier::TORSIONAL;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an "
					 "agxModel::TwoBodyTire::DeformationMode "
					 "literal of unknown type to an FTwoBodyTireBarrier::DeformationMode "
					 "literal. Returning FTwoBodyTireBarrier::DeformationMode::RADIAL."));
			return FTwoBodyTireBarrier::DeformationMode::RADIAL;
	}
}

//
// Enumerations, Terrain.
//

inline EAGX_ExcavationMode Convert(agxTerrain::Shovel::ExcavationMode Mode)
{
	switch (Mode)
	{
		case agxTerrain::Shovel::ExcavationMode::PRIMARY:
			return EAGX_ExcavationMode::Primary;
		case agxTerrain::Shovel::ExcavationMode::DEFORM_BACK:
			return EAGX_ExcavationMode::DeformBack;
		case agxTerrain::Shovel::ExcavationMode::DEFORM_RIGHT:
			return EAGX_ExcavationMode::DeformRight;
		case agxTerrain::Shovel::ExcavationMode::DEFORM_LEFT:
			return EAGX_ExcavationMode::DeformLeft;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown "
					 "agxTerrain::Shovel::ExcavationMode "
					 "literal to an EAGX_ExcavationMode."));
			return EAGX_ExcavationMode::Primary;
	}
}

inline agxTerrain::Shovel::ExcavationMode Convert(EAGX_ExcavationMode Mode)
{
	switch (Mode)
	{
		case EAGX_ExcavationMode::Primary:
			return agxTerrain::Shovel::ExcavationMode::PRIMARY;
		case EAGX_ExcavationMode::DeformBack:
			return agxTerrain::Shovel::ExcavationMode::DEFORM_BACK;
		case EAGX_ExcavationMode::DeformRight:
			return agxTerrain::Shovel::ExcavationMode::DEFORM_RIGHT;
		case EAGX_ExcavationMode::DeformLeft:
			return agxTerrain::Shovel::ExcavationMode::DEFORM_LEFT;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown EAGX_ExcavationMode "
					 "literal to an agxTerrain::Shovel::ExcavationMode."));
			return agxTerrain::Shovel::ExcavationMode::PRIMARY;
	}
}

//
// Enumerations, Track.
//

inline EAGX_TrackWheelModel Convert(agxVehicle::TrackWheel::Model Model)
{
	switch (Model)
	{
		case agxVehicle::TrackWheel::IDLER:
			return EAGX_TrackWheelModel::Idler;
		case agxVehicle::TrackWheel::ROLLER:
			return EAGX_TrackWheelModel::Roller;
		case agxVehicle::TrackWheel::SPROCKET:
			return EAGX_TrackWheelModel::Sprocket;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown agxVehicle::TrackWheel::Model "
					 "literal to an EAGX_TrackWheelModel."));
			return EAGX_TrackWheelModel::Idler;
	}
}

inline agxVehicle::TrackWheel::Model Convert(EAGX_TrackWheelModel Model)
{
	switch (Model)
	{
		case EAGX_TrackWheelModel::Idler:
			return agxVehicle::TrackWheel::IDLER;
		case EAGX_TrackWheelModel::Roller:
			return agxVehicle::TrackWheel::ROLLER;
		case EAGX_TrackWheelModel::Sprocket:
			return agxVehicle::TrackWheel::SPROCKET;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown EAGX_TrackWheelModel "
					 "literal to an agxVehicle::TrackWheel::Model."));
			return agxVehicle::TrackWheel::IDLER;
	}
}

inline EAGX_MergedTrackNodeContactReduction Convert(
	agxVehicle::TrackInternalMergeProperties::ContactReduction Resolution)
{
	switch (Resolution)
	{
		case agxVehicle::TrackInternalMergeProperties::NONE:
			return EAGX_MergedTrackNodeContactReduction::None;
		case agxVehicle::TrackInternalMergeProperties::MINIMAL:
			return EAGX_MergedTrackNodeContactReduction::Minimal;
		case agxVehicle::TrackInternalMergeProperties::MODERATE:
			return EAGX_MergedTrackNodeContactReduction::Moderate;
		case agxVehicle::TrackInternalMergeProperties::AGGRESSIVE:
			return EAGX_MergedTrackNodeContactReduction::Aggressive;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown "
					 "agxVehicle::TrackInternalMergeProperties::ContactReduction "
					 "literal to an EAGX_MergedTrackNodeContactReduction."));
			return EAGX_MergedTrackNodeContactReduction::None;
	}
}

inline agxVehicle::TrackInternalMergeProperties::ContactReduction Convert(
	EAGX_MergedTrackNodeContactReduction Resolution)
{
	switch (Resolution)
	{
		case EAGX_MergedTrackNodeContactReduction::None:
			return agxVehicle::TrackInternalMergeProperties::NONE;
		case EAGX_MergedTrackNodeContactReduction::Minimal:
			return agxVehicle::TrackInternalMergeProperties::MINIMAL;
		case EAGX_MergedTrackNodeContactReduction::Moderate:
			return agxVehicle::TrackInternalMergeProperties::MODERATE;
		case EAGX_MergedTrackNodeContactReduction::Aggressive:
			return agxVehicle::TrackInternalMergeProperties::AGGRESSIVE;
		default:
			UE_LOG(
				LogAGX, Error,
				TEXT("Conversion failed: Tried to convert an unknown "
					 "EAGX_MergedTrackNodeContactReduction"
					 "literal to an agxVehicle::TrackInternalMergeProperties::ContactReduction."));
			return agxVehicle::TrackInternalMergeProperties::NONE;
	}
}

//
// Enumerations, Wire.
//

inline EWireNodeType Convert(agxWire::Node::Type Type)
{
	switch (Type)
	{
		case agxWire::Node::FREE:
			return EWireNodeType::Free;
		case agxWire::Node::EYE:
			return EWireNodeType::Eye;
		case agxWire::Node::BODY_FIXED:
			return EWireNodeType::BodyFixed;
		case agxWire::Node::CONNECTING:
			return EWireNodeType::Connecting;
		case agxWire::Node::STOP:
			return EWireNodeType::Stop;
		case agxWire::Node::CONTACT:
			return EWireNodeType::Contact;
		case agxWire::Node::SHAPE_CONTACT:
			return EWireNodeType::ShapeContact;
		case agxWire::Node::MISSING:
		case agxWire::Node::NOT_DEFINED:
			return EWireNodeType::Other;
	}

	UE_LOG(
		LogAGX, Warning, TEXT("Unknown AGX Dynamics wire node type %d. Defaulting to Other."),
		static_cast<int>(Type));
	return EWireNodeType::Other;
}

inline agxWire::Node::Type Convert(EWireNodeType Type)
{
	switch (Type)
	{
		case EWireNodeType::Free:
			return agxWire::Node::FREE;
		case EWireNodeType::Eye:
			return agxWire::Node::EYE;
		case EWireNodeType::BodyFixed:
			return agxWire::Node::BODY_FIXED;
		case EWireNodeType::Connecting:
			return agxWire::Node::CONNECTING;
		case EWireNodeType::Stop:
			return agxWire::Node::STOP;
		case EWireNodeType::Contact:
			return agxWire::Node::CONTACT;
		case EWireNodeType::ShapeContact:
			return agxWire::Node::SHAPE_CONTACT;
		case EWireNodeType::Other:
		case EWireNodeType::NUM_USER_CREATABLE:
			return agxWire::Node::NOT_DEFINED;
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT("Unknown Unreal Engine wire node type %d. Defaulting to NOT_DEFINED."),
		static_cast<int>(Type));
	return agxWire::Node::NOT_DEFINED;
}

inline EWireNodeNativeType ConvertNative(agxWire::Node::Type Type)
{
	// The values in EWireNodeNativeType must match those in agxWire::Node::Type.
	return static_cast<EWireNodeNativeType>(Type);
}

inline agxWire::Node::Type ConvertNative(EWireNodeNativeType Type)
{
	// The values in EWireNodeNativeType must match those in agxWire::Node::Type.
	return static_cast<agxWire::Node::Type>(Type);
}

//
// Enumerations, Logging.
//

inline agx::Notify::NotifyLevel ConvertLogLevelVerbosity(ELogVerbosity::Type LogVerbosity)
{
	switch (LogVerbosity)
	{
		case ELogVerbosity::VeryVerbose:
			return agx::Notify::NOTIFY_DEBUG;
		case ELogVerbosity::Verbose:
			return agx::Notify::NOTIFY_DEBUG;
		case ELogVerbosity::Log:
			return agx::Notify::NOTIFY_INFO;
		case ELogVerbosity::Display:
			return agx::Notify::NOTIFY_WARNING;
		case ELogVerbosity::Warning:
			return agx::Notify::NOTIFY_WARNING;
		case ELogVerbosity::Error:
			return agx::Notify::NOTIFY_ERROR;
		case ELogVerbosity::Fatal:
			return agx::Notify::NOTIFY_ERROR;
		default:
			UE_LOG(
				LogAGX, Warning,
				TEXT("ConvertLogLevelVerbosity: unknown verbosity level: %d. Verbosity level "
					 "'NOTIFY_INFO' will be used instead."),
				LogVerbosity);

			// Use NOTIFY_INFO as default, if unknown log verbosity is given
			return agx::Notify::NOTIFY_INFO;
	}
}

inline ELogVerbosity::Type ConvertLogLevelVerbosity(agx::Notify::NotifyLevel Level)
{
	switch (Level)
	{
		case agx::Notify::NOTIFY_DEBUG:
			return ELogVerbosity::VeryVerbose;
		case agx::Notify::NOTIFY_INFO:
			return ELogVerbosity::Verbose;
		case agx::Notify::NOTIFY_WARNING:
			return ELogVerbosity::Warning;
		case agx::Notify::NOTIFY_ERROR:
			return ELogVerbosity::Error;

		// The following are not actual verbosity levels.
		case agx::Notify::NOTIFY_CLEAR:
		case agx::Notify::NOTIFY_END:
		case agx::Notify::NOTIFY_LOGONLY:
		case agx::Notify::NOTIFY_PUSH:
			return ELogVerbosity::VeryVerbose;
	}

	UE_LOG(
		LogAGX, Warning, TEXT("Unknown AGX Dynamics log verbosity %d. Defaulting to Warning."),
		static_cast<int>(Level));
	return ELogVerbosity::Warning;
}

//
// Standard Library to Unreal.
//

inline FString Convert(const std::string& Str)
{
	return FString(Str.c_str());
}

template <typename SourceT, typename DestinationT>
inline TArray<DestinationT> ToUnrealArray(const std::vector<SourceT>& V)
{
	TArray<DestinationT> Arr;
	Arr.Reserve(V.size());
	for (const auto& Val : V)
		Arr.Add(Val);

	return Arr;
}

inline TArray<FString> ToUnrealStringArray(const std::vector<std::string>& V)
{
	TArray<FString> Arr;
	Arr.Reserve(V.size());
	for (const auto& Val : V)
		Arr.Add(Convert(Val));

	return Arr;
}

//
// Unreal to Standard Library.
//

inline std::string ToStdString(const FString& Str)
{
	return std::string(TCHAR_TO_UTF8(*Str));
}

template <typename SourceT, typename DestinationT>
inline std::vector<DestinationT> ToStdArray(const TArray<SourceT>& A)
{
	std::vector<DestinationT> Arr;
	Arr.reserve(A.Num());
	for (const auto& Val : A)
		Arr.push_back(Val);

	return Arr;
}

inline std::vector<std::string> ToStdStringArray(const TArray<FString>& A)
{
	std::vector<std::string> Arr;
	Arr.reserve(A.Num());
	for (const auto& Val : A)
		Arr.push_back(ToStdString(Val));

	return Arr;
}
