// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDIProxy.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDataHandler.h"

/** 
 * Get the size of the data that will be passed to render.
 */
int32 FParticleUpsamplingDIProxy::PerInstanceDataPassedToRenderThreadSize() const
{
	return sizeof(FParticleUpsamplingDataHandler);
}

void FParticleUpsamplingDIProxy::ProvidePerInstanceDataForRenderThread(
	void* InDataForRenderThread, void* InDataFromGameThread,
	const FNiagaraSystemInstanceID& SystemInstance)
{
	// Initialize the render thread instance data into the pre-allocated memory
	FParticleUpsamplingDataHandler* DataForRenderThread =
		new (InDataForRenderThread) FParticleUpsamplingDataHandler;

	// Copy the game thread data to the render thread data
	const FParticleUpsamplingDataHandler* DataFromGameThread =
		static_cast<FParticleUpsamplingDataHandler*>(InDataFromGameThread);
	*DataForRenderThread = *DataFromGameThread;
}

void FParticleUpsamplingDIProxy::ConsumePerInstanceDataFromGameThread(
	void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID)
{
	FParticleUpsamplingDataHandler* InstanceDataFromGT =
		static_cast<FParticleUpsamplingDataHandler*>(PerInstanceData);
	FParticleUpsamplingDataHandler* InstanceData =
		&SystemInstancesToInstanceData_RT.FindOrAdd(InstanceID);
	InstanceData->Buffers = InstanceDataFromGT->Buffers;
	InstanceData->Data = InstanceDataFromGT->Data;

	if (InstanceData != nullptr && InstanceData->Buffers)
	{
		if (InstanceData->Data.CoarseParticles.Num() != 0 &&
			InstanceData->Data.ActiveVoxelIndices.Num() != 0)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();

			InstanceData->Buffers->UpdateCoarseParticleBuffer(
				RHICmdList, InstanceData->Data.CoarseParticles);

			InstanceData->Buffers->UpdateHashTableBuffers(
				RHICmdList, InstanceData->Data.ActiveVoxelIndices);
		}
	}

	// we call the destructor here to clean up the GT data. Without this we could be leaking
	// memory.
	InstanceDataFromGT->~FParticleUpsamplingDataHandler();
}

