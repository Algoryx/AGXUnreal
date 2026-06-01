// Copyright 2026, Algoryx Simulation AB.

#include "Wire/AGX_WireController.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Wire/AGX_WireComponent.h"

UAGX_WireController::UAGX_WireController()
{
}

UAGX_WireController* UAGX_WireController::Get()
{
	auto Wc = NewObject<UAGX_WireController>();
	Wc->EnsureNativeIsSet();
	return Wc;
}

bool UAGX_WireController::IsWireWireActive() const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	return NativeBarrier.IsWireWireActive();
}

bool UAGX_WireController::SetCollisionsEnabled(
	UAGX_WireComponent* Wire1, UAGX_WireComponent* Wire2, bool bEnable)
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (Wire1 == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 1 passed to AGX_WireController::SetCollisionsEnabled is None / nullptr. "
				 "This is not allowed."));
		return false;
	}
	if (!Wire1->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 1 passed to AGX_WireController::SetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	if (Wire2 == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 2 passed to AGX_WireController::SetCollisionsEnabled is None / nullptr. "
				 "This is not allowed."));
		return false;
	}
	if (!Wire2->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 2 passed to AGX_WireController::SetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	return NativeBarrier.SetCollisionsEnabled(*Wire1->GetNative(), *Wire2->GetNative(), bEnable);
}

bool UAGX_WireController::GetCollisionsEnabled(
	const UAGX_WireComponent* Wire1, const UAGX_WireComponent* Wire2) const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (Wire1 == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 1 passed to AGX_WireController::SetCollisionsEnabled is None / nullptr. "
				 "This is not allowed."));
		return false;
	}
	if (!Wire1->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 1 passed to AGX_WireController::GetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	if (Wire2 == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 2 passed to AGX_WireController::SetCollisionsEnabled is None / nullptr. "
				 "This is not allowed."));
		return false;
	}
	if (!Wire2->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Wire 2 passed to AGX_WireController::GetCollisionsEnabled does not have a "
				 "Native"));
		return false;
	}
	return NativeBarrier.GetCollisionsEnabled(*Wire1->GetNative(), *Wire2->GetNative());
}

//
// Dynamic wire contacts functions.
//

bool UAGX_WireController::SetDynamicWireContactsEnabled(UAGX_ShapeComponent* Shape, bool bEnable)
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (Shape == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("None / nullptr Shape passed to "
				 "AGX_WireController::SetDynamicWireContactsEnabled. This is not allowed."));
		return false;
	}
	if (!Shape->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape passed to AGX_WireController::SetDynamicWireContactsEnabled does not have "
				 "a native."));
		return false;
	}
	return NativeBarrier.SetDynamicWireContactsEnabled(*Shape->GetNative(), bEnable);
}

bool UAGX_WireController::SetDynamicWireContactsEnabled(
	UAGX_RigidBodyComponent* RigidBody, bool bEnable)
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (RigidBody == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("None / nullptr Rigid Body passed to "
				 "AGX_WireController::SetDynamicWireContactsEnabled. This is not allowed."));
		return false;
	}
	if (!RigidBody->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Rigid Body passed to AGX_WireController::SetDynamicWireContactsEnabled does not "
				 "have a native."));
		return false;
	}
	return NativeBarrier.SetDynamicWireContactsEnabled(*RigidBody->GetNative(), bEnable);
}

bool UAGX_WireController::SetDynamicWireContactsGloballyEnabled(bool bEnable)
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a native"));
		return false;
	}
	NativeBarrier.SetDynamicWireContactsGloballyEnabled(bEnable);
	return true;
}

bool UAGX_WireController::GetDynamicWireContactsEnabled(const UAGX_ShapeComponent* Shape) const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (Shape == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("None / nullptr Shape passed to "
				 "AGX_WireController::GetDynamicWireContactsEnabled. This is not allowed."));
		return false;
	}
	if (!Shape->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape passed to AGX_WireController::GetDynamicWireContactsEnabled does not have "
				 "a native."));
		return false;
	}
	return NativeBarrier.GetDynamicWireContactsEnabled(*Shape->GetNative());
}

bool UAGX_WireController::GetDynamicWireContactsEnabled(
	const UAGX_RigidBodyComponent* RigidBody) const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a Native."));
		return false;
	}
	if (RigidBody == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("None / nullptr Rigid Body passed to "
				 "AGX_WireController::GetDynamicWireContactsEnabled. This is not allowed."));
		return false;
	}
	if (!RigidBody->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Rigid Body passed to AGX_WireController::GetDynamicWireContactsEnabled does not "
				 "have a native."));
		return false;
	}
	return NativeBarrier.GetDynamicWireContactsEnabled(*RigidBody->GetNative());
}

bool UAGX_WireController::GetDynamicWireContactsGloballyEnabled() const
{
	if (!HasNative())
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX_WireController does not have a native"));
		return false;
	}
	return NativeBarrier.GetDynamicWireContactsGloballyEnabled();
}

bool UAGX_WireController::HasNative() const
{
	// Before, we used to call NativeBarrier.InitializeNative() from the Constructor (CDO).
	// This gave us errors on startup with UE5.6 because it was called before the plugin
	// was loaded (on Windows).
	// Therefore, we have moved this call here, to act as a "lazy" native initialization.
	const_cast<UAGX_WireController*>(this)->EnsureNativeIsSet();

	return NativeBarrier.HasNative();
}

void UAGX_WireController::EnsureNativeIsSet()
{
	if (!NativeBarrier.HasNative())
		NativeBarrier.InitializeNative();
}
