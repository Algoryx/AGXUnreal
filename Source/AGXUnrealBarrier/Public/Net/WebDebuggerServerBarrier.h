// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "WebDebuggerServerBarrier.generated.h"

struct FWebDebuggerServerRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FWebDebuggerServerBarrier
{
	GENERATED_BODY()

	FWebDebuggerServerBarrier();
	FWebDebuggerServerBarrier(std::shared_ptr<FWebDebuggerServerRef> Native);

	bool HasNative() const;
	void AllocateNative(int32 Port);
	FWebDebuggerServerRef* GetNative();
	const FWebDebuggerServerRef* GetNative() const;

	void ReleaseNative();

	/**
	 * Start the HTTP server in a separate thread.
	 * Blocks until server is running and port is assigned.
	 */
	void Start();

	/**
	 * Stop the HTTP server and signal thread to exit.
	 * Safe to call multiple times.
	 * Does not block - use join() to wait for completion.
	 */
	void Stop();

	/**
	 * Wait for the server thread to finish.
	 * Call after stop() to ensure clean shutdown.
	 * Safe to call multiple times.
	 */
	void Join();

	bool IsRunning() const;

	int GetPort() const;

private:
	std::shared_ptr<FWebDebuggerServerRef> NativeRef;
};
