// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/SteeringBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/Vehicle/SteeringRef.h"
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"
#include "Vehicle/AGX_SteeringParametersData.h"
#include "Vehicle/WheelJointBarrier.h"


FSteeringBarrier::FSteeringBarrier()
	: NativeRef {new FSteeringRef}
{
}

FSteeringBarrier::FSteeringBarrier(std::shared_ptr<FSteeringRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

namespace SteeringBarrier_helpers
{
	agxVehicle::SteeringParameters CreateSteeringParameters(
		const FAGX_SteeringParametersData& Params)
	{
		agxVehicle::SteeringParameters ParamsAGX;
		ParamsAGX.phi0 = ConvertAngleToAGX(Params.Phi0);
		ParamsAGX.l = ConvertDistanceToAGX(Params.L);
		ParamsAGX.alpha0 = ConvertAngleToAGX(Params.Alpha0);
		ParamsAGX.lc = ConvertDistanceToAGX(Params.Lc);
		ParamsAGX.lr = ConvertDistanceToAGX(Params.Lr);
		ParamsAGX.gear = Params.Gear;
		ParamsAGX.side = static_cast<agx::UInt>(Params.Side);
		return ParamsAGX;
	}

	FAGX_SteeringParametersData CreateSteeringParameters(
		const agxVehicle::SteeringParameters& ParamsAGX)
	{
		FAGX_SteeringParametersData Params;
		Params.Phi0 = ConvertAngleToUnreal<double>(ParamsAGX.phi0);
		Params.L = ConvertDistanceToUnreal<double>(ParamsAGX.l);
		Params.Alpha0 = ConvertAngleToUnreal<double>(ParamsAGX.alpha0);
		Params.Lc = ConvertDistanceToUnreal<double>(ParamsAGX.lc);
		Params.Lr = ConvertDistanceToUnreal<double>(ParamsAGX.lr);
		Params.Gear = ParamsAGX.gear;
		Params.Side = static_cast<int32>(ParamsAGX.side);

		return Params;
	}

	template <typename T>
	agxVehicle::SteeringRef AllocateImpl(
		FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
		const FAGX_SteeringParametersData& Params)
	{
		check(LeftWheel.HasNative());
		check(RightWheel.HasNative());
		auto WheelLeftAGX =
			dynamic_cast<agxVehicle::WheelJoint*>(LeftWheel.GetNative()->Native.get());
		auto WheelRightAGX =
			dynamic_cast<agxVehicle::WheelJoint*>(RightWheel.GetNative()->Native.get());
		const auto ParamsAGX = SteeringBarrier_helpers::CreateSteeringParameters(Params);
		return new T(WheelLeftAGX, WheelRightAGX, ParamsAGX);
	}
}

void FSteeringBarrier::AllocateAckermann(
	FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
	const FAGX_SteeringParametersData& Params)
{
	using namespace SteeringBarrier_helpers;
	check(!HasNative());
	NativeRef->Native = AllocateImpl<agxVehicle::Ackermann>(LeftWheel, RightWheel, Params);
}

void FSteeringBarrier::AllocateBellCrank(
	FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
	const FAGX_SteeringParametersData& Params)
{
	using namespace SteeringBarrier_helpers;
	check(!HasNative());
	NativeRef->Native = AllocateImpl<agxVehicle::BellCrank>(LeftWheel, RightWheel, Params);
}

void FSteeringBarrier::AllocateRackPinion(
	FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
	const FAGX_SteeringParametersData& Params)
{
	using namespace SteeringBarrier_helpers;
	check(!HasNative());
	NativeRef->Native = AllocateImpl<agxVehicle::RackPinion>(LeftWheel, RightWheel, Params);
}

void FSteeringBarrier::AllocateDavis(
	FWheelJointBarrier& LeftWheel, FWheelJointBarrier& RightWheel,
	const FAGX_SteeringParametersData& Params)
{
	using namespace SteeringBarrier_helpers;
	check(!HasNative());
	NativeRef->Native = AllocateImpl<agxVehicle::Davis>(LeftWheel, RightWheel, Params);
}

void FSteeringBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FSteeringBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FSteeringBarrier::SetSteeringAngle(double Angle)
{
	check(HasNative());
	return NativeRef->Native->setSteeringAngle(ConvertAngleToAGX(Angle));
}

double FSteeringBarrier::GetSteeringAngle() const
{
	check(HasNative());
	return ConvertAngleToUnreal<double>(NativeRef->Native->getSteeringAngle());
}

void FSteeringBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->Native->setName(Convert(Name));
}

FString FSteeringBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

FGuid FSteeringBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

EAGX_SteeringType FSteeringBarrier::GetType() const
{
	check(HasNative());

	if (NativeRef->Native->is<agxVehicle::Ackermann>())
		return EAGX_SteeringType::Ackermann;
	if (NativeRef->Native->is<agxVehicle::BellCrank>())
		return EAGX_SteeringType::BellCrank;
	if (NativeRef->Native->is<agxVehicle::Davis>())
		return EAGX_SteeringType::Davis;
	if (NativeRef->Native->is<agxVehicle::RackPinion>())
		return EAGX_SteeringType::RackPinion;

	return EAGX_SteeringType::Invalid;
}

FAGX_SteeringParametersData FSteeringBarrier::GetSteeringParameters() const
{
	check(HasNative());
	const auto& SteeringAGX = NativeRef->Native->getSteeringParameters();
	return SteeringBarrier_helpers::CreateSteeringParameters(SteeringAGX);
}

bool FSteeringBarrier::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Native != nullptr;
}

FSteeringRef* FSteeringBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FSteeringRef* FSteeringBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FSteeringBarrier::GetNativeAddress() const
{
	if (!HasNative())
		return 0;

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FSteeringBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
		return;

	if (HasNative())
		ReleaseNative();

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		return;
	}

	NativeRef->Native = reinterpret_cast<agxVehicle::Steering*>(NativeAddress);
}

void FSteeringBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}
