// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/CylinderShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxCollide/Cylinder.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

namespace
{
	agxCollide::Cylinder* NativeCylinder(FCylinderShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Cylinder>();
	}

	const agxCollide::Cylinder* NativeCylinder(const FCylinderShapeBarrier* Barrier)
	{
		return Barrier->GetNative()->NativeShape->as<agxCollide::Cylinder>();
	}
}

FCylinderShapeBarrier::FCylinderShapeBarrier()
	: FShapeBarrier()
{
}

FCylinderShapeBarrier::FCylinderShapeBarrier(std::shared_ptr<FGeometryAndShapeRef> Native)
	: FShapeBarrier(std::move(Native))
{
	check(NativeRef->NativeShape->is<agxCollide::Cylinder>());
}

void FCylinderShapeBarrier::SetHeight(double Height)
{
	check(HasNative());
	NativeCylinder(this)->setHeight(ConvertDistanceToAGX(Height));
}

double FCylinderShapeBarrier::GetHeight() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCylinder(this)->getHeight());
}

void FCylinderShapeBarrier::SetRadius(double Radius)
{
	check(HasNative());
	NativeCylinder(this)->setRadius(ConvertDistanceToAGX(Radius));
}

namespace CylinderShapeBarrier_helpers
{
	const agx::PropertyContainer* GetProperties(const FCylinderShapeBarrier& Self)
	{
		agxCollide::Geometry* Geometry = Self.GetNative()->NativeGeometry;
		if (!Geometry->hasPropertyContainer())
		{
			return nullptr;
		}
		return Geometry->getPropertyContainer();
	}

	agx::PropertyContainer* GetProperties(FCylinderShapeBarrier& Self)
	{
		agxCollide::Geometry* Geometry = Self.GetNative()->NativeGeometry;
		if (!Geometry->hasPropertyContainer())
		{
			return nullptr;
		}
		return Geometry->getPropertyContainer();
	}

	bool GetPropertyBool(const FCylinderShapeBarrier& Self, const char* Property)
	{
		const agx::PropertyContainer* Properties = GetProperties(Self);
		if (Properties == nullptr)
		{
			return false;
		}
		if (!Properties->hasPropertyBool(Property))
		{
			return false;
		}
		return Properties->getPropertyBool(Property);
	}

	void SetProperty(FCylinderShapeBarrier& Self, const char* Property)
	{
		Self.GetNative()->NativeGeometry->getPropertyContainer()->addPropertyBool(Property, true);
	}

	void RemoveProperty(FCylinderShapeBarrier& Self, const char* Property)
	{
		agx::PropertyContainer* Properties = GetProperties(Self);
		if (Properties == nullptr || !Properties->hasPropertyBool(Property))
		{
			// Not having any properties or not having a particular property is the same as having
			// the property set to false, so no need to do anything more.
			return;
		}
		Properties->removePropertyBool("Pulley");
	}
}

void FCylinderShapeBarrier::SetPulleyProperty(bool bInPulley)
{
	check(HasNative());
	if (bInPulley)
	{
		CylinderShapeBarrier_helpers::SetProperty(*this, "Pulley");
	}
	else
	{
		/// @todo Unclear if we should set the property to false, or remove it.
		CylinderShapeBarrier_helpers::RemoveProperty(*this, "Pulley");
	}
}

bool FCylinderShapeBarrier::GetPulleyProperty() const
{
	check(HasNative());
	return CylinderShapeBarrier_helpers::GetPropertyBool(*this, "Pulley");
}

void FCylinderShapeBarrier::RemovePulleyProperty()
{
	check(HasNative());
	CylinderShapeBarrier_helpers::RemoveProperty(*this, "Pulley");
}

void FCylinderShapeBarrier::SetGypsyProperty(bool bInGypsy)
{
	check(HasNative());
	if (bInGypsy)
	{
		CylinderShapeBarrier_helpers::SetProperty(*this, "Gypsy");
	}
	else
	{
		/// @todo Unclear if we should set the property to false, or remove it.
		CylinderShapeBarrier_helpers::RemoveProperty(*this, "Gypsy");
	}
}

bool FCylinderShapeBarrier::GetGypsyProperty() const
{
	check(HasNative());
	return CylinderShapeBarrier_helpers::GetPropertyBool(*this, "Gypsy");
}

void FCylinderShapeBarrier::RemoveGypsyProperty()
{
	check(HasNative());
	CylinderShapeBarrier_helpers::RemoveProperty(*this, "Gypsy");
}

double FCylinderShapeBarrier::GetRadius() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeCylinder(this)->getRadius());
}

void FCylinderShapeBarrier::AllocateNativeShape()
{
	check(!HasNative());
	NativeRef->NativeShape = new agxCollide::Cylinder(agx::Real(0.5), agx::Real(1.0));
}

void FCylinderShapeBarrier::ReleaseNativeShape()
{
	check(HasNative());
	NativeRef->NativeShape = nullptr;
}
