// Copyright 2025, Algoryx Simulation AB.

#include "Contacts/ContactPointBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Contacts/ContactPointEntity.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include "EndAGXIncludes.h"

FContactPointBarrier::FContactPointBarrier()
	: NativeEntity {new FContactPointEntity}
{
}

FContactPointBarrier::FContactPointBarrier(std::unique_ptr<FContactPointEntity> InNativeEntity)
	: NativeEntity(std::move(InNativeEntity))
{
}

FContactPointBarrier::FContactPointBarrier(FContactPointBarrier&& InOther)
	: NativeEntity(std::move(InOther.NativeEntity))
{
}

FContactPointBarrier::FContactPointBarrier(const FContactPointBarrier& InOther)
	: NativeEntity(new FContactPointEntity(InOther.NativeEntity->Native))
{
}

FContactPointBarrier::~FContactPointBarrier()
{
}

FContactPointBarrier& FContactPointBarrier::operator=(const FContactPointBarrier& InOther)
{
	if (InOther.HasNative())
	{
		NativeEntity->Native = InOther.NativeEntity->Native;
	}
	else
	{
		NativeEntity->Native = agxCollide::ContactPoint();
	}
	return *this;
}

//
// Collision detection state getters.
//

bool FContactPointBarrier::IsEnabled() const
{
	check(HasNative());
	return NativeEntity->Native.enabled();
}

FVector FContactPointBarrier::GetLocation() const
{
	check(HasNative());
	return ConvertDisplacement(NativeEntity->Native.point());
}

FVector FContactPointBarrier::GetNormal() const
{
	check(HasNative());
	return ConvertFloatVector(NativeEntity->Native.normal());
}

FVector FContactPointBarrier::GetTangentU() const
{
	check(HasNative());
	return ConvertFloatVector(NativeEntity->Native.tangentU());
}

FVector FContactPointBarrier::GetTangentV() const
{
	check(HasNative());
	return ConvertFloatVector(NativeEntity->Native.tangentV());
}

double FContactPointBarrier::GetDepth() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeEntity->Native.depth());
}

FVector FContactPointBarrier::GetVelocity() const
{
	check(HasNative());
	return ConvertDisplacement(NativeEntity->Native.velocity());
}

double FContactPointBarrier::GetArea() const
{
	check(HasNative());
	return ConvertAreaToUnreal<float>(NativeEntity->Native.area());
}

FVector FContactPointBarrier::GetWitnessPoint(int32 Index) const
{
	check(HasNative());
	return ConvertDisplacement(NativeEntity->Native.getWitnessPoint(Index));
}

//
// Collision detection state setters. May only be called before the solver.
//

void FContactPointBarrier::SetEnabled(bool bEnable)
{
	check(HasNative());
	return NativeEntity->Native->setEnabled(bEnable);
}

void FContactPointBarrier::SetLocation(const FVector& Location)
{
	check(HasNative());
	NativeEntity->Native->setPoint(ConvertDisplacement(Location));
}

void FContactPointBarrier::SetNormal(const FVector& Normal)
{
	check(HasNative());
	NativeEntity->Native->setNormal(ConvertFloatVector(Normal));
}

void FContactPointBarrier::SetTangentU(const FVector& TangentU)
{
	check(HasNative());
	NativeEntity->Native->setTangentU(ConvertFloatVector(TangentU));
}

void FContactPointBarrier::SetTangentV(const FVector& TangentV)
{
	check(HasNative());
	NativeEntity->Native->setTangentV(ConvertFloatVector(TangentV));
}

void FContactPointBarrier::SetDepth(const double Depth)
{
	check(HasNative());
	NativeEntity->Native->setDepth(ConvertDistanceToAGX(Depth));
}

void FContactPointBarrier::SetVelocity(const FVector& Velocity)
{
	check(HasNative());
	NativeEntity->Native->setVelocity(ConvertFloatDisplacement(Velocity));
}

void FContactPointBarrier::SetArea(double Area) const
{
	check(HasNative());
	NativeEntity->Native->setArea(ConvertAreaToAGX<double>(Area));
}

//
// Solver state getters. May only be called after the solver.
//

FVector FContactPointBarrier::GetForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.getForce());
}

double FContactPointBarrier::GetForceMagnitude() const
{
	check(HasNative());
	return NativeEntity->Native.getForceMagnitude();
}

FVector FContactPointBarrier::GetNormalForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.getNormalForce());
}

double FContactPointBarrier::GetNormalForceMagnitude() const
{
	check(HasNative());
	return NativeEntity->Native.getNormalForceMagnitude();
}

FVector FContactPointBarrier::GetTangentialForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.getTangentialForce());
}

double FContactPointBarrier::GetTangentialForceMagnitude() const
{
	check(HasNative());
	return NativeEntity->Native.getTangentialForceMagnitude();
}

double FContactPointBarrier::GetTangentialForceUMagnitude() const
{
	check(HasNative());
	return NativeEntity->Native.getTangentialForceUMagnitude();
}

double FContactPointBarrier::GetTangentialForceVMagnitude() const
{
	check(HasNative());
	return NativeEntity->Native.getTangentialForceVMagnitude();
}

FVector FContactPointBarrier::GetLocalForce() const
{
	check(HasNative());
	return ConvertVector(NativeEntity->Native.localForce());
}

double FContactPointBarrier::GetLocalForce(EAGX_ContactForceComponents Component)
{
	check(HasNative());
	const size_t Index = static_cast<size_t>(Component);
	return NativeEntity->Native.localForce(Index);
}

//
// Native management.
//

bool FContactPointBarrier::HasNative() const
{
	return NativeEntity.get() != nullptr && NativeEntity->Native.isValid();
}

FContactPointEntity* FContactPointBarrier::GetNative()
{
	return NativeEntity.get();
}

const FContactPointEntity* FContactPointBarrier::GetNative() const
{
	return NativeEntity.get();
}
