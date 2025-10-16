// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "NiagaraCommon.h"

/**
 * Struct to store data from a single coarse particle.
 */ 
struct FCoarseParticle
{
	/**
	 * The position stored at XYZ, the particle radius stored at W.
	 */
	FVector4f PositionAndRadius;

	/**
	 * The velocity stored at XYZ, the particle mass stored at W.
	 */
	FVector4f VelocityAndMass;
};

/**
 * Struct to store data contained in a voxel of the voxel grid.
 */
struct FVoxelEntry
{
	/**
	 * The voxel index stored at XYZ, W represents the voxel's capacity, i.e.,
	 * how many particles can fit in that voxel.
	 */
	FIntVector4 IndexAndRoom;

	/**
	 * The voxel velocity stored at XYZ, the voxel mass stored at W.
	 */
	FVector4f VelocityAndMass;

	/**
	 * The max coordinates of the bounding box surrounding coarse particles
	 * in this voxel stored at XYZ, W is empty.
	 */
	FVector4f MaxBounds;

	/**
	 * The min coordinates of the bounding box surrounding coarse particles 
	 * in this voxel stored at XYZ, W is empty.
	 */
	FVector4f MinBounds;
};

/**
 * Struct for managing GPU buffers used by the particle upsampling data interface.
 * Extends render resources to enable initialization, release, and uploading data
 * to Unordered Access View (UAV) and Shader Resource View (SRV) buffers.
 *
 * UAVs allow for unordered read/write access from multiple threads by supporting
 * atomic operations. This makes it possible to implement data structures like
 * hash tables directly on the GPU, where multiple threads can perform concurrent 
 * inserts and lookups.
 *
 * SRVs provide read-only access to buffers from multiple threads. Although threads
 * can read from the buffer in parallel without conflicts, writing is not allowed,
 * as it would result in exceptions being thrown.
 */
struct FParticleUpsamplingBuffers : public FRenderResource
{
	FParticleUpsamplingBuffers()
		: CoarseParticlesCapacity(0)
		, ActiveVoxelsCapacity(0)
	{
	}

	FParticleUpsamplingBuffers(uint32 InitialCoarseParticleCapacity, uint32 InitialActiveVoxelsCapacity)
		: CoarseParticlesCapacity(InitialCoarseParticleCapacity) 
		, ActiveVoxelsCapacity(InitialActiveVoxelsCapacity)
	{
	}
	
	// ~Begin FRenderResource interface.

	/**
	 * Initialize the different buffers used in this render resource. 
	 */
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override;

	/** 
	 * Release all buffers used in this render resource.  
	 */
	virtual void ReleaseRHI() override;
	virtual FString GetFriendlyName() const override { return TEXT("Particle Upsampling Render Resources") ;};

	// ~End FRenderResource interface.

	/** 
	 * Function for initializing a new Read-only buffer on the GPU 
	 */
	template <typename T>
	FShaderResourceViewRHIRef InitSRVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	/** 
	 * Function for initializing a new Read/Write buffer on the GPU. 
	 */
	template <typename T>
	FUnorderedAccessViewRHIRef InitUAVBuffer(
		FRHICommandListBase& RHICmdList, const TCHAR* InDebugName, uint32 ElementCount);

	/** 
	 * Update the buffers for the coarse particles, releasing the old ones and creating new ones. 
	 */
	void UpdateCoarseParticleBuffer(
		FRHICommandListBase& RHICmdList, const TArray<FCoarseParticle> CoarseParticleData);

	/** 
	 * Update the buffers for the hashtable, releasing the old ones and creating new ones. 
	 */
	void UpdateHashTableBuffers(
		FRHICommandListBase& RHICmdList, const TArray<FIntVector4> ActiveVoxelIndices);

	/** 
	 * Reference to the SRV buffer containing Coarse Particles. 
	 */
	FShaderResourceViewRHIRef CoarseParticles;

	/**
	 * The number of allocated elements that the coarse particle buffer can store.
	 */
	uint32 CoarseParticlesCapacity;

	/** 
	 * Reference to the SRV buffer containing Active Voxels. 
	 */
	FShaderResourceViewRHIRef ActiveVoxelIndices;

	/** 
	 * Reference to the UAV buffer containing data for each voxel in the voxel grid. 
	 */
	FUnorderedAccessViewRHIRef ActiveVoxelsTable;

	/**
	 * Reference to the UAV buffer containing data on which indices are 
	 * occupied in the hashtable buffer.
	 */
	FUnorderedAccessViewRHIRef ActiveVoxelsTableOccupancy;

	/**
	 * The number of elements the buffers hanlding active voxels can store.
	 */
	uint32 ActiveVoxelsCapacity;
};

/**
 * Struct containing the data from the simulation that is used to be able to 
 * perform particle upsampling.
 */
struct FParticleUpsamplingSimulationData
{
	FParticleUpsamplingSimulationData()
	{
	}

	TArray<FCoarseParticle> CoarseParticles;
	TArray<FIntVector4> ActiveVoxelIndices;

	float VoxelSize = 0;
	float FineParticleMass = 0;
	float FineParticleRadius = 0;
	float EaseStepSize = 0;
	float NominalRadius = 0;
	int TableSize = 0;
};

/** 
 * Struct for handling all the data and buffers for the particle upsampling
 * data interface. This struct will be passed as the instanced data between
 * the game thread and render thread.
 */
struct FParticleUpsamplingDataHandler
{
	static const uint32 INITIAL_COARSE_PARTICLE_BUFFER_SIZE = 1024;
	static const uint32 INITIAL_ACTIVE_VOXEL_BUFFER_SIZE = 1024;

	void Init(FNiagaraSystemInstance* SystemInstance);
	void Release();

	FParticleUpsamplingSimulationData Data;

	// We use shared pointer here since both the proxy and the NDI will reference this buffer.
	std::shared_ptr<FParticleUpsamplingBuffers> Buffers;
};
