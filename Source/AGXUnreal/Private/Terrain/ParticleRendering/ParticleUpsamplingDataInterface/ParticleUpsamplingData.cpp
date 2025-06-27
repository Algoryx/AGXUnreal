// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"

FPUBuffers::FPUBuffers(uint32 InitialCPBufferSize, uint32 InitialActiveVoxelBuffer)
{
	NumElementsInCoarseParticleBuffer = InitialCPBufferSize;
	NumElementsInActiveVoxelBuffer = InitialActiveVoxelBuffer;
}

void FPUBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Init SRV Buffers
	CoarseParticleBufferRef = InitSRVBuffer<FCoarseParticle>(
		RHICmdList, TEXT("CPPositionsAndRadiusBuffer"), NumElementsInCoarseParticleBuffer);
	ActiveVoxelIndicesBufferRef = InitSRVBuffer<FIntVector4>(
		RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), NumElementsInActiveVoxelBuffer);
	
	// Init UAV Buffers
	HashTableBufferRef = InitUAVBuffer<FVoxelEntry>(
		RHICmdList, TEXT("HashTableBuffer"), NumElementsInActiveVoxelBuffer * 2);
	HashTableOccupancyBufferRef = InitUAVBuffer<int>(
		RHICmdList, TEXT("HashTableOccupancyBuffer"), NumElementsInActiveVoxelBuffer * 2);
}

void FPUBuffers::ReleaseRHI()
{
	CoarseParticleBufferRef.SafeRelease();
	ActiveVoxelIndicesBufferRef.SafeRelease();
	HashTableBufferRef.SafeRelease();
	HashTableOccupancyBufferRef.SafeRelease();
}

template <typename T>
FShaderResourceViewRHIRef FPUBuffers::InitSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(T), sizeof(T) * ElementCount, BUF_ShaderResource, CreateInfo);
	return RHICmdList.CreateShaderResourceView(
		BufferRef, FRHIViewDesc::CreateBufferSRV().SetType(FRHIViewDesc::EBufferType::Structured));
}

template <typename T>
FUnorderedAccessViewRHIRef FPUBuffers::InitUAVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef = RHICmdList.CreateStructuredBuffer(
		sizeof(T), sizeof(T) * ElementCount, BUF_UnorderedAccess, CreateInfo);
	return RHICmdList.CreateUnorderedAccessView(
		BufferRef, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Structured));
}

void FPUBuffers::UpdateCoarseParticleBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData,
	uint32 NewElementCount, bool NeedsResize)
{
	uint32 ElementCount = CoarseParticleData.Num();
	if (ElementCount > 0 && CoarseParticleBufferRef.IsValid() &&
		CoarseParticleBufferRef->GetBuffer()->IsValid())
	{
		if (NeedsResize)
		{
			// Release old buffer.
			CoarseParticleBufferRef.SafeRelease();

			// Create new, larger buffer.
			CoarseParticleBufferRef = InitSRVBuffer<FCoarseParticle>(
				RHICmdList, TEXT("CPPositionsAndRadiusBuffer"), NewElementCount);
		}

		const uint32 BufferBytes = sizeof(FCoarseParticle) * ElementCount;
		if (BufferBytes < CoarseParticleBufferRef->GetBuffer()->GetSize())
		{
			void* OutputData = RHICmdList.LockBuffer(
				CoarseParticleBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

			FMemory::Memcpy(OutputData, CoarseParticleData.GetData(), BufferBytes);
			RHICmdList.UnlockBuffer(CoarseParticleBufferRef->GetBuffer());
		}
	}
}

void FPUBuffers::UpdateHashTableBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices,
	uint32 NewElementCount, bool NeedsResize)
{
	uint32 ElementCount = ActiveVoxelIndices.Num();
	if (ElementCount > 0 && ActiveVoxelIndicesBufferRef.IsValid() &&
		ActiveVoxelIndicesBufferRef->GetBuffer()->IsValid())
	{
		if (NeedsResize)
		{
			// Release old buffers.
			ActiveVoxelIndicesBufferRef.SafeRelease();
			HashTableBufferRef.SafeRelease();
			HashTableOccupancyBufferRef.SafeRelease();

			// Create new, larger buffers.
			ActiveVoxelIndicesBufferRef = InitSRVBuffer<FIntVector4>(
				RHICmdList, TEXT("ActiveVoxelIndicesBuffer"), NewElementCount);
			HashTableBufferRef = InitUAVBuffer<FVoxelEntry>(
				RHICmdList, TEXT("HashtableBuffer"), NewElementCount * 2);
			HashTableOccupancyBufferRef = InitUAVBuffer<int>(
				RHICmdList, TEXT("HTOccupancy"), NewElementCount * 2);
		}

		const uint32 BufferBytes = sizeof(FIntVector4) * ElementCount;
		if (BufferBytes < ActiveVoxelIndicesBufferRef->GetBuffer()->GetSize())
		{
			void* OutputData = RHICmdList.LockBuffer(
				ActiveVoxelIndicesBufferRef->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

			FMemory::Memcpy(OutputData, ActiveVoxelIndices.GetData(), BufferBytes);
			RHICmdList.UnlockBuffer(ActiveVoxelIndicesBufferRef->GetBuffer());
		}
	
	}
}

FPUArrays::FPUArrays(uint32 InitialCPBufferSize, uint32 InitialActiveVoxelBuffer)
{
	CoarseParticles.SetNumZeroed(InitialCPBufferSize);
	ActiveVoxelIndices.SetNumZeroed(InitialActiveVoxelBuffer);

	NumElementsInCoarseParticleBuffer = InitialCPBufferSize;
	NumElementsInActiveVoxelBuffer = InitialActiveVoxelBuffer;
}

void FPUArrays::CopyFrom(const FPUArrays* Other)
{
	CoarseParticles = Other->CoarseParticles;
	ActiveVoxelIndices = Other->ActiveVoxelIndices;

	NumElementsInCoarseParticleBuffer = Other->NumElementsInCoarseParticleBuffer;
	NumElementsInActiveVoxelBuffer = Other->NumElementsInActiveVoxelBuffer;

	Time = Other->Time;
	VoxelSize = Other->VoxelSize;
	FineParticleMass = Other->FineParticleMass;
	FineParticleRadius = Other->FineParticleRadius;
	EaseStepSize = Other->EaseStepSize;
	NominalRadius = Other->NominalRadius;
	TableSize = Other->TableSize;

	NeedsCPResize = Other->NeedsCPResize;
	NeedsVoxelResize = Other->NeedsVoxelResize;
}

void FParticleUpsamplingData::Init(FNiagaraSystemInstance* SystemInstance)
{
	PUBuffers = nullptr;
	if (SystemInstance)
	{
		PUArrays = new FPUArrays(INITIAL_CP_BUFFER_SIZE, INITIAL_VOXEL_BUFFER_SIZE);
		PUBuffers = new FPUBuffers(INITIAL_CP_BUFFER_SIZE, INITIAL_VOXEL_BUFFER_SIZE);
		BeginInitResource(PUBuffers);
	}
}

void FParticleUpsamplingData::Update(
	FNiagaraSystemInstance* SystemInstance, FPUArrays* OtherData)
{
	if (SystemInstance)
	{
		PUArrays->CopyFrom(OtherData);
		PUArrays->Time = (int) std::time(0);
	}
}

void FParticleUpsamplingData::Release()
{
	if (PUBuffers)
	{
		ENQUEUE_RENDER_COMMAND(DeleteResource)(
			[ParamPointerToRelease = PUBuffers](FRHICommandListImmediate& RHICmdList)
			{
				ParamPointerToRelease->ReleaseResource();
				delete ParamPointerToRelease;
			});
		PUBuffers = nullptr;
	}
}
