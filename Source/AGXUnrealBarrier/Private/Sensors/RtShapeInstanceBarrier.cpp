// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/RtShapeInstanceBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/RtLambertianOpaqueMaterialBarrier.h"
#include "Sensors/RtShapeBarrier.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Sensors/SensorRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/EnvironmentInternalData.h>
#include <agxSensor/RaytraceInstanceData.h>
#include <agxSensor/RaytraceSurfaceMaterial.h>
#include "EndAGXIncludes.h"

FRtShapeInstanceBarrier::FRtShapeInstanceBarrier()
	: NativeRef {new FRtShapeInstance}
{
}

FRtShapeInstanceBarrier::FRtShapeInstanceBarrier(std::unique_ptr<FRtShapeInstance> Native)
	: NativeRef(std::move(Native))
{
}

FRtShapeInstanceBarrier::FRtShapeInstanceBarrier(FRtShapeInstanceBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRtShapeInstance);
}

FRtShapeInstanceBarrier::~FRtShapeInstanceBarrier()
{
}

void FRtShapeInstanceBarrier::AllocateNative(
	FRtShapeBarrier& Shape, FSensorEnvironmentBarrier& Environment)
{
	check(!HasNative());
	check(Shape.HasNative());
	check(Environment.HasNative());

	NativeRef->Native = agxSensor::RtShapeInstance::create(
		Environment.GetNative()->Native->getScene(), Shape.GetNative()->Native, nullptr);
}

void FRtShapeInstanceBarrier::SetLidarSurfaceMaterialOrDefault(
	FRtLambertianOpaqueMaterialBarrier* Material)
{
	check(HasNative());

	if (Material == nullptr)
	{
		// Assign default if setting nullptr Material.
		NativeRef->Native.handle->setMaterial(agxSensor::RtLambertianOpaqueMaterial::create());
	}
	else
	{
		check(Material->HasNative());
		NativeRef->Native.handle->setMaterial(*Material->GetNative()->Native);
	}
}

bool FRtShapeInstanceBarrier::HasNative() const
{
	return NativeRef->Native.handle != nullptr;
}

FRtShapeInstance* FRtShapeInstanceBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRtShapeInstance* FRtShapeInstanceBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FRtShapeInstanceBarrier::SetTransform(const FTransform& Transform)
{
	check(HasNative());
	NativeRef->Native.setTransform(
		ConvertMatrix(Transform.GetLocation(), Transform.GetRotation()),
		Convert(Transform.GetScale3D()));
}
