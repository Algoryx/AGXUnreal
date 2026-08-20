// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraLensSingleElement.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Sensors/CameraLensSingleElementBarrier.h"

void UAGX_CameraLensSingleElement::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_CameraLensSingleElement '%s' whose "
					 "instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior to "
					 "calling this function."),
				*GetName());
			return;
		}
		Instance->GetOrCreateNative();
		return;
	}

	AGX_CHECK(IsInstance());
	if (NativeBarrier != nullptr && NativeBarrier->HasNative())
	{
		NativeBarrier->ReleaseNative();
	}

	NativeBarrier = MakeUnique<FCameraLensSingleElementBarrier>();
	NativeBarrier->AllocateNative();
	check(HasNative());
}
