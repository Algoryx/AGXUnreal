// Copyright 2025, Algoryx Simulation AB.

#include "Contacts/AGX_ContactPoint.h"

#include "AGX_LogCategory.h"

FAGX_ContactPoint::FAGX_ContactPoint(const FAGX_ContactPoint& InOther)
	: Barrier(InOther.Barrier)
{
}

FAGX_ContactPoint::FAGX_ContactPoint(FContactPointBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
}

FAGX_ContactPoint& FAGX_ContactPoint::operator=(const FAGX_ContactPoint& Other)
{
	Barrier = Other.Barrier;
	return *this;
}

bool FAGX_ContactPoint::HasNative() const
{
	return Barrier.HasNative();
}

namespace
{
	bool TestHasNative(
		const FAGX_ContactPoint& ContactPoint, const TCHAR* Operation, const TCHAR* AttributeName)
	{
		if (ContactPoint.HasNative())
		{
			return true;
		}
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot %s %s with a ContactPoint that doesn't have a native AGX Dynamics "
				 "representation"),
			Operation, AttributeName);
		return false;
	}

	bool TestHasNativeSet(const FAGX_ContactPoint& Point, const TCHAR* Attribute)
	{
		return TestHasNative(Point, TEXT("set"), Attribute);
	}

	bool TestHasNativeGet(const FAGX_ContactPoint& Point, const TCHAR* Attribute)
	{
		return TestHasNative(Point, TEXT("get"), Attribute);
	}
}

//
// Collision detection state.
//

void FAGX_ContactPoint::SetEnabled(bool bEnable)
{
	if (!TestHasNativeSet(*this, TEXT("Enabled")))
		return;
	Barrier.SetEnabled(bEnable);
}

bool FAGX_ContactPoint::IsEnabled() const
{
	if (!TestHasNativeGet(*this, TEXT("Enabled")))
		return false;
	return Barrier.IsEnabled();
}

void FAGX_ContactPoint::SetLocation(const FVector& Location)
{
	if (!TestHasNativeGet(*this, TEXT("Location")))
		return;
	Barrier.SetLocation(Location);
}

FVector FAGX_ContactPoint::GetLocation() const
{
	if (!TestHasNativeGet(*this, TEXT("Location")))
		return FVector::ZeroVector;
	return Barrier.GetLocation();
}

void FAGX_ContactPoint::SetNormal(const FVector& Normal)
{
	if (!TestHasNativeSet(*this, TEXT("Normal")))
		return;
	Barrier.SetNormal(Normal);
}

FVector FAGX_ContactPoint::GetNormal() const
{
	if (!TestHasNativeGet(*this, TEXT("Normal")))
		return FVector::ZeroVector;
	return Barrier.GetNormal();
}

void FAGX_ContactPoint::SetTangentU(const FVector& TangentU)
{
	if (!TestHasNativeSet(*this, TEXT("Tangent U")))
		return;
	Barrier.SetTangentU(TangentU);
}

FVector FAGX_ContactPoint::GetTangentU() const
{
	if (!TestHasNativeGet(*this, TEXT("Tangent U")))
		return FVector::ZeroVector;
	return Barrier.GetTangentU();
}

void FAGX_ContactPoint::SetTangentV(const FVector& TangentV)
{
	if (!TestHasNativeSet(*this, TEXT("Tangent V")))
		return;
	Barrier.SetTangentV(TangentV);
}

FVector FAGX_ContactPoint::GetTangentV() const
{
	if (!TestHasNativeGet(*this, TEXT("Tangent V")))
		return FVector::ZeroVector;
	return Barrier.GetTangentV();
}

void FAGX_ContactPoint::SetDepth(double Depth)
{
	if (!TestHasNativeGet(*this, TEXT("Depth")))
		return;
	Barrier.SetDepth(Depth);
}

double FAGX_ContactPoint::GetDepth() const
{
	if (!TestHasNativeGet(*this, TEXT("Depth")))
		return 0.0;
	return Barrier.GetDepth();
}

FVector FAGX_ContactPoint::GetWitnessPoint(int32 Index) const
{
	if (!TestHasNativeGet(*this, TEXT("WitnessPoint")))
		return FVector::ZeroVector;
	return Barrier.GetWitnessPoint(Index);
}

void FAGX_ContactPoint::SetArea(double Area) const
{
	if (!TestHasNativeSet(*this, TEXT("Area")))
		return;
	Barrier.SetArea(Area);
}

double FAGX_ContactPoint::GetArea() const
{
	if (!TestHasNativeGet(*this, TEXT("Area")))
		return 0.0;
	return Barrier.GetArea();
}

void FAGX_ContactPoint::SetSurfaceVelocity(const FVector& SurfaceVelocity)
{
	if (!TestHasNativeGet(*this, TEXT("Surface Velocity")))
		return;
	Barrier.SetVelocity(SurfaceVelocity);
}

FVector FAGX_ContactPoint::GetSurfaceVelocity() const
{
	if (!TestHasNativeGet(*this, TEXT("Surface Velocity")))
		return FVector::ZeroVector;
	return Barrier.GetVelocity();
}

//
// Solver state getters. May only be called after the solver.
//

FVector FAGX_ContactPoint::GetForce() const
{
	if (!TestHasNativeGet(*this, TEXT("Force")))
		return FVector::ZeroVector;
	return Barrier.GetForce();
}

double FAGX_ContactPoint::GetForceMagnitude() const
{
	if (!TestHasNativeGet(*this, TEXT("Force Magnitude")))
		return 0.0;
	return Barrier.GetForceMagnitude();
}

FVector FAGX_ContactPoint::GetNormalForce() const
{
	if (!TestHasNativeGet(*this, TEXT("Normal Force")))
		return FVector::ZeroVector;
	return Barrier.GetNormalForce();
}

double FAGX_ContactPoint::GetNormalForceMagnitude() const
{
	if (!TestHasNativeGet(*this, TEXT("Normal Force Magnitude")))
		return 0.0;
	return Barrier.GetNormalForceMagnitude();
}

FVector FAGX_ContactPoint::GetTangentialForce() const
{
	if (!TestHasNativeGet(*this, TEXT("Tangential Force")))
		return FVector::ZeroVector;
	return Barrier.GetTangentialForce();
}

double FAGX_ContactPoint::GetTangentialForceMagnitude() const
{
	if (!TestHasNativeGet(*this, TEXT("Tangential Force Magnitude")))
		return 0.0;
	return Barrier.GetTangentialForceMagnitude();
}

double FAGX_ContactPoint::GetTangentialForceUMagnitude() const
{
	if (!TestHasNativeGet(*this, TEXT("Tangential Force U Magnitude")))
		return 0.0;
	return Barrier.GetTangentialForceUMagnitude();
}

double FAGX_ContactPoint::GetTangentialForceVMagnitude() const
{
	if (!TestHasNativeGet(*this, TEXT("Tangential Force V Magnitude")))
		return 0.0;
	return Barrier.GetTangentialForceVMagnitude();
}

FVector FAGX_ContactPoint::GetLocalForce() const
{
	if (!TestHasNativeGet(*this, TEXT("LocalForce")))
		return FVector::ZeroVector;
	return Barrier.GetLocalForce();
}

double FAGX_ContactPoint::GetLocalForce(EAGX_ContactForceComponents Component)
{
	if (!TestHasNativeGet(*this, TEXT("Local Force")))
		return 0.0;
	return Barrier.GetLocalForce(Component);
}
