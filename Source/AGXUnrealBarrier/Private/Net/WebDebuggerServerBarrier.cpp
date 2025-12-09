// Copyright 2025, Algoryx Simulation AB.

#include "Net/WebDebuggerServerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/Net/NetRef.h"
#include "TypeConversions.h"

// Unreal Engine includes.
#include "Misc/Paths.h"

FWebDebuggerServerBarrier::FWebDebuggerServerBarrier()
	: NativeRef(new FWebDebuggerServerRef())
{
}

FWebDebuggerServerBarrier::FWebDebuggerServerBarrier(std::shared_ptr<FWebDebuggerServerRef> Native)
	: NativeRef(std::move(Native))
{
}

bool FWebDebuggerServerBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FWebDebuggerServerBarrier::AllocateNative(int32 Port)
{
	check(!HasNative());
	const FString AGXResources = FAGX_Environment::GetAGXDynamicsResourcesPath();
#if defined(_WIN64)
	const FString BinSubDir = FAGX_Environment::IsSetupEnvRun() ? TEXT("x64") : TEXT("Win64");
	const FString WebDebuggerDir =
		FPaths::Combine(AGXResources, FString("bin"), BinSubDir, FString("WebDebugger"));
#else
	const FString WebDebuggerDir =
		FPaths::Combine(AGXResources, FString("lib"), FString("Linux"), FString("WebDebugger"));
#endif

	NativeRef->Native =
		std::make_shared<agxNet::agxWebServer::WebDebuggerServer>(Port, ToStdString(WebDebuggerDir));
}

FWebDebuggerServerRef* FWebDebuggerServerBarrier::GetNative()
{
	return NativeRef.get();
}

const FWebDebuggerServerRef* FWebDebuggerServerBarrier::GetNative() const
{
	return NativeRef.get();
}

void FWebDebuggerServerBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

void FWebDebuggerServerBarrier::Start()
{
	check(HasNative());
	try
	{
		NativeRef->Native->start();
	}
	catch (const std::exception& E)
	{
		// Log standard exceptions with message
		UE_LOG(
			LogAGX, Error, TEXT("Exception in FWebDebuggerServerBarrier::Start: %s"),
			*FString(E.what()));
	}
	catch (...)
	{
		// Log all other unexpected exceptions
		UE_LOG(
			LogAGX, Error, TEXT("Unknown exception caught in FWebDebuggerServerBarrier::Start."));
	}
}

void FWebDebuggerServerBarrier::Stop()
{
	check(HasNative());
	NativeRef->Native->stop();
}

void FWebDebuggerServerBarrier::Join()
{
	check(HasNative());
	NativeRef->Native->join();
}

bool FWebDebuggerServerBarrier::IsRunning() const
{
	check(HasNative());
	return NativeRef->Native->isRunning();
}

int FWebDebuggerServerBarrier::GetPort() const
{
	check(HasNative());
	return NativeRef->Native->getPort();
}
