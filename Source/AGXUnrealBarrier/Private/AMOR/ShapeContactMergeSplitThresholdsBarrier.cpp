// Copyright 2025, Algoryx Simulation AB.

#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSDK/MergeSplitHandler.h>
#include "EndAGXIncludes.h"

FShapeContactMergeSplitThresholdsBarrier::FShapeContactMergeSplitThresholdsBarrier()
	: FMergeSplitThresholdsBarrier()
{
}

FShapeContactMergeSplitThresholdsBarrier::FShapeContactMergeSplitThresholdsBarrier(
	std::unique_ptr<FMergeSplitThresholdsRef>&& Native)
	: FMergeSplitThresholdsBarrier(std::move(Native))
{
}

FShapeContactMergeSplitThresholdsBarrier::~FShapeContactMergeSplitThresholdsBarrier()
{
}

void FShapeContactMergeSplitThresholdsBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSDK::GeometryContactMergeSplitThresholds();
}

namespace ShapeContactMergeSplitThresholds_helpers
{
	agxSDK::GeometryContactMergeSplitThresholds* CastToGeometryContactThresholds(
		agxSDK::MergeSplitThresholds* Thresholds, const FString& Operation)
	{
		agxSDK::GeometryContactMergeSplitThresholds* GeomContThresholds =
			dynamic_cast<agxSDK::GeometryContactMergeSplitThresholds*>(Thresholds);
		if (GeomContThresholds == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Operation %s failed, could not cast native MergeSplitThresholds to "
					 "ShapeContactMergeSplitThresholds."),
				*Operation);
		}

		return GeomContThresholds;
	}

	const agx::RigidBody* GetFrom(const FRigidBodyBarrier& Barrier)
	{
		AGX_CHECK(Barrier.HasNative());
		return Barrier.GetNative()->Native;
	}

	const agxCollide::Geometry* GetFrom(const FShapeBarrier& Barrier)
	{
		AGX_CHECK(Barrier.HasNativeGeometry());
		return Barrier.GetNative()->NativeGeometry;
	}

	template <typename BodyOrShape>
	FShapeContactMergeSplitThresholdsBarrier CreateFrom(const BodyOrShape& Barrier)
	{
		if (!Barrier.HasNative())
		{
			return FShapeContactMergeSplitThresholdsBarrier();
		}

		const auto Msp = agxSDK::MergeSplitHandler::getProperties(GetFrom(Barrier));
		if (Msp == nullptr)
		{
			return FShapeContactMergeSplitThresholdsBarrier();
		}

		auto Mst = Msp->getContactThresholds();
		if (Mst == nullptr)
		{
			return FShapeContactMergeSplitThresholdsBarrier();
		}

		return FShapeContactMergeSplitThresholdsBarrier(
			std::make_unique<FMergeSplitThresholdsRef>(Mst));
	}
}

void FShapeContactMergeSplitThresholdsBarrier::SetMaxImpactSpeed(double InMaxImpactSpeed)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "SetMaxImpactSpeed"))
	{
		Native->setMaxImpactSpeed(ConvertDistanceToAGX(InMaxImpactSpeed));
	}
}

double FShapeContactMergeSplitThresholdsBarrier::GetMaxImpactSpeed() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "GetMaxImpactSpeed"))
	{
		return ConvertDistanceToUnreal<double>(Native->getMaxImpactSpeed());
	}

	// Error message printed above.
	return 0.0;
}

void FShapeContactMergeSplitThresholdsBarrier::SetMaxRelativeNormalSpeed(
	double InMaxRelativeNormalSpeed)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native =
			CastToGeometryContactThresholds(NativeRef->Native, "SetMaxRelativeNormalSpeed"))
	{
		Native->setMaxRelativeNormalSpeed(ConvertDistanceToAGX(InMaxRelativeNormalSpeed));
	}
}

double FShapeContactMergeSplitThresholdsBarrier::GetMaxRelativeNormalSpeed() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native =
			CastToGeometryContactThresholds(NativeRef->Native, "GetMaxRelativeNormalSpeed"))
	{
		return ConvertDistanceToUnreal<double>(Native->getMaxRelativeNormalSpeed());
	}

	// Error message printed above.
	return 0.0;
}

void FShapeContactMergeSplitThresholdsBarrier::SetMaxRelativeTangentSpeed(
	double InMaxRelativeTangentSpeed)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native =
			CastToGeometryContactThresholds(NativeRef->Native, "SetMaxRelativeTangentSpeed"))
	{
		Native->setMaxRelativeTangentSpeed(ConvertDistanceToAGX(InMaxRelativeTangentSpeed));
	}
}

double FShapeContactMergeSplitThresholdsBarrier::GetMaxRelativeTangentSpeed() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native =
			CastToGeometryContactThresholds(NativeRef->Native, "GetMaxRelativeTangentSpeed"))
	{
		return ConvertDistanceToUnreal<double>(Native->getMaxRelativeTangentSpeed());
	}

	// Error message printed above.
	return 0.0;
}

void FShapeContactMergeSplitThresholdsBarrier::SetMaxRollingSpeed(double InMaxRollingSpeed)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "SetMaxRollingSpeed"))
	{
		Native->setMaxRollingSpeed(ConvertDistanceToAGX(InMaxRollingSpeed));
	}
}

double FShapeContactMergeSplitThresholdsBarrier::GetMaxRollingSpeed() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "GetMaxRollingSpeed"))
	{
		return ConvertDistanceToUnreal<double>(Native->getMaxRollingSpeed());
	}

	// Error message printed above.
	return 0.0;
}

void FShapeContactMergeSplitThresholdsBarrier::SetNormalAdhesion(double InNormalAdhesion)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "SetNormalAdhesion"))
	{
		Native->setNormalAdhesion(InNormalAdhesion);
	}
}

double FShapeContactMergeSplitThresholdsBarrier::GetNormalAdhesion() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "GetNormalAdhesion"))
	{
		return Native->getNormalAdhesion();
	}

	// Error message printed above.
	return 0.0;
}

void FShapeContactMergeSplitThresholdsBarrier::SetTangentialAdhesion(double InTangentialAdhesion)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "SetTangentialAdhesion"))
	{
		Native->setTangentialAdhesion(InTangentialAdhesion);
	}
}

double FShapeContactMergeSplitThresholdsBarrier::GetTangentialAdhesion() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "GetTangentialAdhesion"))
	{
		return Native->getTangentialAdhesion();
	}

	// Error message printed above.
	return 0.0;
}

void FShapeContactMergeSplitThresholdsBarrier::SetMaySplitInGravityField(
	bool bMaySplitInGravityField)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native =
			CastToGeometryContactThresholds(NativeRef->Native, "SetMaySplitInGravityField"))
	{
		Native->setMaySplitInGravityField(bMaySplitInGravityField);
	}
}

bool FShapeContactMergeSplitThresholdsBarrier::GetMaySplitInGravityField() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native =
			CastToGeometryContactThresholds(NativeRef->Native, "GetMaySplitInGravityField"))
	{
		return Native->getMaySplitInGravityField();
	}

	// Error message printed above.
	return false;
}

void FShapeContactMergeSplitThresholdsBarrier::SetSplitOnLogicalImpact(bool bSplitOnLogicalImpact)
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "SetSplitOnLogicalImpact"))
	{
		Native->setSplitOnLogicalImpact(bSplitOnLogicalImpact);
	}
}

bool FShapeContactMergeSplitThresholdsBarrier::GetSplitOnLogicalImpact() const
{
	check(HasNative());
	using namespace ShapeContactMergeSplitThresholds_helpers;
	if (auto Native = CastToGeometryContactThresholds(NativeRef->Native, "GetSplitOnLogicalImpact"))
	{
		return Native->getSplitOnLogicalImpact();
	}

	// Error message printed above.
	return false;
}

FShapeContactMergeSplitThresholdsBarrier FShapeContactMergeSplitThresholdsBarrier::CreateFrom(
	const FRigidBodyBarrier& Barrier)
{
	return ShapeContactMergeSplitThresholds_helpers::CreateFrom(Barrier);
}

FShapeContactMergeSplitThresholdsBarrier FShapeContactMergeSplitThresholdsBarrier::CreateFrom(
	const FShapeBarrier& Barrier)
{
	return ShapeContactMergeSplitThresholds_helpers::CreateFrom(Barrier);
}
