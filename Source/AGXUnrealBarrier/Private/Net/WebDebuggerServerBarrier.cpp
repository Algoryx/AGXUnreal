// Copyright 2025, Algoryx Simulation AB.

#include "Net/WebDebuggerServerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/Net/NetRef.h"

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
	NativeRef->Native = std::make_shared<agxNet::agxWebServer::WebDebuggerServer>(
		0);
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
	NativeRef = nullptr;
}

void FWebDebuggerServerBarrier::Start()
{
	check(HasNative());
	NativeRef->Native->start();
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
