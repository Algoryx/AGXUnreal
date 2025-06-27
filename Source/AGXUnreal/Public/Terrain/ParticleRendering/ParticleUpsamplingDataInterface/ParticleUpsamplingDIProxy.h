// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "NiagaraDataInterface.h"

struct FParticleUpsamplingData;

/** This proxy is used to safely copy data between game thread and render thread*/
struct FParticleUpsamplingDIProxy : FNiagaraDataInterfaceProxy
{
	// ~Begin FNiagaraDataInterfaceProxy interface.

	/** Get the size of the data that will be passed to render. */
	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override;

	/** Get the data that will be passed to render. */
	virtual void ConsumePerInstanceDataFromGameThread(
		void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID) override;

	// ~End FNiagaraDataInterfaceProxy interface.

	/** Initialize the Proxy data buffer */
	void InitializePerInstanceData(const FNiagaraSystemInstanceID& SystemInstance);

	/** Destroy the proxy data if necessary */
	void DestroyPerInstanceData(const FNiagaraSystemInstanceID& SystemInstance);

	/** List of proxy data for each system instances*/
	TMap<FNiagaraSystemInstanceID, FParticleUpsamplingData> SystemInstancesToInstanceData_RT;
};
