// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDIProxy.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"

/** 
 * Get the size of the data that will be passed to render.
 */
int32 FParticleUpsamplingDIProxy::PerInstanceDataPassedToRenderThreadSize() const
{
	return sizeof(FParticleUpsamplingData);
}

/**
 * Get the data that will be passed to render.
 */
void FParticleUpsamplingDIProxy::ConsumePerInstanceDataFromGameThread(
	void* PerInstanceData, const FNiagaraSystemInstanceID& InstanceID)
{
	FParticleUpsamplingData* InstanceDataFromGT = static_cast<FParticleUpsamplingData*>(PerInstanceData);
	FParticleUpsamplingData* InstanceData = &SystemInstancesToInstanceData_RT.FindOrAdd(InstanceID);
	InstanceData->PUBuffers = InstanceDataFromGT->PUBuffers;
	InstanceData->PUArrays = new FPUArrays(
		FParticleUpsamplingData::INITIAL_CP_BUFFER_SIZE,
		FParticleUpsamplingData::INITIAL_VOXEL_BUFFER_SIZE);
	InstanceData->PUArrays->CopyFrom(InstanceDataFromGT->PUArrays);

	if (InstanceData != nullptr && InstanceData->PUBuffers)
	{
		if (InstanceData->PUArrays->CoarseParticles.Num() != 0 &&
			InstanceData->PUArrays->ActiveVoxelIndices.Num() != 0)
		{
			FRHICommandListBase& RHICmdList = FRHICommandListImmediate::Get();

			InstanceData->PUBuffers->UpdateCoarseParticleBuffers(
				RHICmdList, InstanceData->PUArrays->CoarseParticles,
				InstanceData->PUArrays->NumElementsInCoarseParticleBuffer,
				InstanceData->PUArrays->NeedsCPResize);

			InstanceData->PUBuffers->UpdateHashTableBuffers(
				RHICmdList, InstanceData->PUArrays->ActiveVoxelIndices,
				InstanceData->PUArrays->NumElementsInActiveVoxelBuffer,
				InstanceData->PUArrays->NeedsVoxelResize);
		}
	}

	// we call the destructor here to clean up the GT data. Without this we could be leaking
	// memory.
	InstanceDataFromGT->~FParticleUpsamplingData();
}

/** 
 * Initialize the Proxy data buffer.
 */
void FParticleUpsamplingDIProxy::InitializePerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
{
	check(IsInRenderingThread());
	check(!SystemInstancesToInstanceData_RT.Contains(SystemInstance));

	SystemInstancesToInstanceData_RT.Add(SystemInstance);
}

/**
 * Destroy the proxy data if necessary.
 */ 
void FParticleUpsamplingDIProxy::DestroyPerInstanceData(
	const FNiagaraSystemInstanceID& SystemInstance)
{
	check(IsInRenderingThread());

	SystemInstancesToInstanceData_RT.Remove(SystemInstance);
}

