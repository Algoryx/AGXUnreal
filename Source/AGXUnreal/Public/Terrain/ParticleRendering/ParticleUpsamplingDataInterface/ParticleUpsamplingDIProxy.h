// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDataHandler.h"

// Unreal Engine includes.
#include "NiagaraDataInterface.h"

/** This proxy is used to safely copy data between game thread and render thread*/
struct AGXUNREAL_API FParticleUpsamplingDIProxy : FNiagaraDataInterfaceProxy
{
	// ~Begin FNiagaraDataInterfaceProxy interface.

	/** Get the size of the data that will be passed to render. */
	virtual int32 PerInstanceDataPassedToRenderThreadSize() const override;

	/** Get the data that will be passed to render. */
	virtual void ConsumePerInstanceDataFromGameThread(
		void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID) override;

	// ~End FNiagaraDataInterfaceProxy interface.

	/** Initialize the render thread data. */
	static void ProvidePerInstanceDataForRenderThread(
		void* InDataForRenderThread, void* InDataFromGameThread,
		const FNiagaraSystemInstanceID& SystemInstance);

	/** List of proxy data for each system instances*/
	TMap<FNiagaraSystemInstanceID, FParticleUpsamplingDataHandler> SystemInstancesToInstanceData_RT;
};
