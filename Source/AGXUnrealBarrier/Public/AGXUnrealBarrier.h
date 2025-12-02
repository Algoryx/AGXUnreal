// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

#include "NotifyBarrier.h"

class FAGXNotifyListener;

class FAGXUnrealBarrierModule : public IModuleInterface
{
public:
	~FAGXUnrealBarrierModule() = default;

	// ~Begin IModuleInterface interface.
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// ~End IModuleInterface interface.

	static FAGXUnrealBarrierModule& Get();

	/// Add a Notify Listener that will be notified of log messages from AGX Dynamics.
	void AddNotifyListener(FAGXNotifyListener* Listener);

	/// Remove a Notify Listener.
	void RemoveNotifyListener(FAGXNotifyListener* Listener);

	/// Called by FAGXNotify when AGX Dynamics prints a log message.
	void RelayNotifyMessage(const FString& Message, ELogVerbosity::Type Verbosity);

private:
	FNotifyBarrier NotifyBarrier;
	TArray<FAGXNotifyListener*> NotifyListeners;
};

/**
 * Base class for listening in on log messages from AGX Dynamics.
 */
class AGXUNREALBARRIER_API FAGXNotifyListener
{
public:
	FAGXNotifyListener();
	virtual ~FAGXNotifyListener();

	virtual void OnMessage(const FString& Message, ELogVerbosity::Type Verbosity) = 0;
};
