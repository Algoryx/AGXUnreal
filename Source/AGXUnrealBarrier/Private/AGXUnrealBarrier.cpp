// Copyright 2025, Algoryx Simulation AB.

#include "AGXUnrealBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Environment.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/agx.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FAGXUnrealBarrierModule"

namespace
{
	void LogAgxDynamicsLicenseStatus()
	{
		FString Status;
		if (FAGX_Environment::GetInstance().EnsureAGXDynamicsLicenseValid(&Status) == false)
		{
			UE_LOG(LogAGX, Warning, TEXT("AGX Dynamics license is invalid. Status: %s"), *Status);
		}
		else
		{
			UE_LOG(LogAGX, Log, TEXT("AGX Dynamics license is valid."));
		}
	}
}

void FAGXUnrealBarrierModule::StartupModule()
{
	FAGX_Environment::GetInstance().EnsureEnvironmentSetup();

	agx::init();

	// Start AGX logging.
	NotifyBarrier.StartAgxNotify(ELogVerbosity::Log);

	LogAgxDynamicsLicenseStatus();
}

void FAGXUnrealBarrierModule::ShutdownModule()
{
	// Stop AGX logging.
	NotifyBarrier.StopAgxNotify();

	agx::shutdown();
}

FAGXUnrealBarrierModule& FAGXUnrealBarrierModule::Get()
{
	return FModuleManager::GetModuleChecked<FAGXUnrealBarrierModule>("AGXUnrealBarrier");
}

void FAGXUnrealBarrierModule::AddNotifyListener(FAGXNotifyListener* Listener)
{
	NotifyListeners.Add(Listener);
}

void FAGXUnrealBarrierModule::RemoveNotifyListener(FAGXNotifyListener* Listener)
{
	NotifyListeners.Remove(Listener);
}

void FAGXUnrealBarrierModule::RelayNotifyMessage(
	const FString& Message, ELogVerbosity::Type Verbosity)
{
	for (auto& Listener : NotifyListeners)
	{
		Listener->OnMessage(Message, Verbosity);
	}
}

FAGXNotifyListener::FAGXNotifyListener()
{
	FAGXUnrealBarrierModule::Get().AddNotifyListener(this);
}

FAGXNotifyListener::~FAGXNotifyListener()
{
	FAGXUnrealBarrierModule::Get().RemoveNotifyListener(this);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAGXUnrealBarrierModule, AGXUnrealBarrier)
