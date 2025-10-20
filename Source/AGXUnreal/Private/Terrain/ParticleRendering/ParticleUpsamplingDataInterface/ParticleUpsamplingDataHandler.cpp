// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDataHandler.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"

void FParticleUpsamplingBuffers::InitRHI(FRHICommandListBase& RHICmdList)
{
	// Init SRV Buffers (Shader Resource View - read only).
	CoarseParticles = InitSRVBuffer<FCoarseParticle>(
		RHICmdList, TEXT("CoarseParticles"), CoarseParticlesCapacity);
	ActiveVoxelIndices =
		InitSRVBuffer<FIntVector4>(RHICmdList, TEXT("ActiveVoxelIndices"), ActiveVoxelsCapacity);

	// Init UAV Buffers (Unordered Access View - read/write).
	ActiveVoxelsTable =
		InitUAVBuffer<FVoxelEntry>(RHICmdList, TEXT("ActiveVoxelsTable"), ActiveVoxelsCapacity);
	ActiveVoxelsTableOccupancy =
		InitUAVBuffer<int>(RHICmdList, TEXT("ActiveVoxelsTableOccupancy"), ActiveVoxelsCapacity);
}

void FParticleUpsamplingBuffers::ReleaseRHI()
{
	CoarseParticles.SafeRelease();
	ActiveVoxelIndices.SafeRelease();
	ActiveVoxelsTable.SafeRelease();
	ActiveVoxelsTableOccupancy.SafeRelease();
}

template <typename T>
FShaderResourceViewRHIRef FParticleUpsamplingBuffers::InitSRVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	const uint32 Stride = sizeof(T);
	const uint32 Size = Stride * ElementCount;
#if UE_VERSION_OLDER_THAN(5, 6, 0)
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef =
		RHICmdList.CreateStructuredBuffer(Stride, Size, BUF_ShaderResource, CreateInfo);
	return RHICmdList.CreateShaderResourceView(
		BufferRef, FRHIViewDesc::CreateBufferSRV().SetType(FRHIViewDesc::EBufferType::Structured));
#else
	FRHIBufferCreateDesc BufferDesc =
		FRHIBufferCreateDesc::Create(
			InDebugName, Size, Stride, BUF_ShaderResource | BUF_StructuredBuffer)
			.SetInitialState(RHIGetDefaultResourceState(
				EBufferUsageFlags(BUF_ShaderResource | BUF_StructuredBuffer), false));

	FBufferRHIRef BufferRef = RHICmdList.CreateBuffer(BufferDesc);
	return RHICmdList.CreateShaderResourceView(
		BufferRef, FRHIViewDesc::CreateBufferSRV().SetType(FRHIViewDesc::EBufferType::Structured));
#endif
}

template <typename T>
FUnorderedAccessViewRHIRef FParticleUpsamplingBuffers::InitUAVBuffer(
	FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount)
{
	const uint32 Stride = sizeof(T);
	const uint32 Size = Stride * ElementCount;
#if UE_VERSION_OLDER_THAN(5, 6, 0)
	FRHIResourceCreateInfo CreateInfo(InDebugName);
	FBufferRHIRef BufferRef =
		RHICmdList.CreateStructuredBuffer(Stride, Size, BUF_UnorderedAccess, CreateInfo);
	return RHICmdList.CreateUnorderedAccessView(
		BufferRef, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Structured));
#else
	FRHIBufferCreateDesc BufferDesc =
		FRHIBufferCreateDesc::Create(
			InDebugName, Size, Stride, BUF_UnorderedAccess | BUF_StructuredBuffer)
			.SetInitialState(RHIGetDefaultResourceState(
				EBufferUsageFlags(BUF_UnorderedAccess | BUF_StructuredBuffer), false));

	FBufferRHIRef BufferRef = RHICmdList.CreateBuffer(BufferDesc);
	return RHICmdList.CreateUnorderedAccessView(
		BufferRef, FRHIViewDesc::CreateBufferUAV().SetType(FRHIViewDesc::EBufferType::Structured));
#endif
}

void FParticleUpsamplingBuffers::UpdateCoarseParticleBuffer(
	FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData)
{
	uint32 ElementCount = CoarseParticleData.Num();
	if (ElementCount == 0 || !CoarseParticles.IsValid() || !CoarseParticles->GetBuffer()->IsValid())
	{
		return;
	}

	if (CoarseParticlesCapacity < ElementCount)
	{
		// Release old buffer.
		CoarseParticles.SafeRelease();

		// Create new, larger buffer.
		CoarseParticlesCapacity *= 2;
		CoarseParticles = InitSRVBuffer<FCoarseParticle>(
			RHICmdList, TEXT("CoarseParticles"), CoarseParticlesCapacity);
	}

	const uint32 BufferBytes = sizeof(FCoarseParticle) * ElementCount;
	void* OutputData =
		RHICmdList.LockBuffer(CoarseParticles->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

	FMemory::Memcpy(OutputData, CoarseParticleData.GetData(), BufferBytes);
	RHICmdList.UnlockBuffer(CoarseParticles->GetBuffer());
}

void FParticleUpsamplingBuffers::UpdateHashTableBuffers(
	FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndicesArray)
{
	uint32 ElementCount = ActiveVoxelIndicesArray.Num();
	if (ElementCount == 0 || !ActiveVoxelIndices.IsValid() ||
		!ActiveVoxelIndices->GetBuffer()->IsValid())
	{
		return;
	}

	if (ActiveVoxelsCapacity < ElementCount)
	{
		// Release old buffers.
		ActiveVoxelIndices.SafeRelease();
		ActiveVoxelsTable.SafeRelease();
		ActiveVoxelsTableOccupancy.SafeRelease();

		// Create new, larger buffers.
		ActiveVoxelsCapacity *= 2;

		ActiveVoxelIndices = InitSRVBuffer<FIntVector4>(
			RHICmdList, TEXT("ActiveVoxelIndices"), ActiveVoxelsCapacity);
		ActiveVoxelsTable =
			InitUAVBuffer<FVoxelEntry>(RHICmdList, TEXT("ActiveVoxelsTable"), ActiveVoxelsCapacity);
		ActiveVoxelsTableOccupancy = InitUAVBuffer<int>(
			RHICmdList, TEXT("ActiveVoxelsTableOccupancy"), ActiveVoxelsCapacity);
	}

	const uint32 BufferBytes = sizeof(FIntVector4) * ElementCount;
	void* OutputData =
		RHICmdList.LockBuffer(ActiveVoxelIndices->GetBuffer(), 0, BufferBytes, RLM_WriteOnly);

	FMemory::Memcpy(OutputData, ActiveVoxelIndicesArray.GetData(), BufferBytes);
	RHICmdList.UnlockBuffer(ActiveVoxelIndices->GetBuffer());
}

void FParticleUpsamplingDataHandler::Init(FNiagaraSystemInstance* SystemInstance)
{
	if (!SystemInstance)
	{
		return;
	}

	Buffers.reset(new FParticleUpsamplingBuffers(
		INITIAL_COARSE_PARTICLE_BUFFER_SIZE, INITIAL_ACTIVE_VOXEL_BUFFER_SIZE));
	BeginInitResource(Buffers.get());
}

void FParticleUpsamplingDataHandler::Release()
{
	if (Buffers)
	{
		ReleaseResourceAndFlush(Buffers.get());
	}
}
