// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingData.h"

// Unreal Engine includes.
#include "NiagaraCommon.h"
#include "NiagaraDataInterface.h"
#include "Misc/EngineVersionComparison.h"

#include "AGX_ParticleUpsamplingDI.generated.h"

struct FPUArrays;
struct FCoarseParticle;

UCLASS(EditInlineNew, Category = "Data Interface", CollapseCategories, meta = (DisplayName = "Particle Upsampling Data Interface"))
class AGXUNREAL_API UAGX_ParticleUpsamplingDI : public UNiagaraDataInterface
{
	GENERATED_UCLASS_BODY()

	BEGIN_SHADER_PARAMETER_STRUCT(FShaderParameters, )
		// Particle Buffers
		SHADER_PARAMETER_SRV(StructuredBuffer<FCoarseParticle>,	CoarseParticles)
		SHADER_PARAMETER(int,									NumCoarseParticles)
		SHADER_PARAMETER(float,									FineParticleMass)
		SHADER_PARAMETER(float,									FineParticleRadius)
		SHADER_PARAMETER(float,									NominalRadius)

		// HashTable Buffers
		SHADER_PARAMETER_SRV(StructuredBuffer<FIntVector4>,		ActiveVoxelIndices)
		SHADER_PARAMETER(int,									NumActiveVoxels)

		SHADER_PARAMETER_UAV(RWStructuredBuffer<FVoxelEntry>,	HashTable)
		SHADER_PARAMETER_UAV(RWStructuredBuffer<int>,			HTOccupancy)

		SHADER_PARAMETER(int,									TableSize)
		SHADER_PARAMETER(float,									VoxelSize)

		// Other Variables
		SHADER_PARAMETER(int,									Time)
		SHADER_PARAMETER(float,									AnimationSpeed)
	END_SHADER_PARAMETER_STRUCT()

public:

	// ~Begin UObject Interface.

	/** Funcition for registering our custom DI with Niagara. */
	virtual void PostInitProperties() override;

	// ~End UObject Interface.

	// ~Begin UNiagaraDataInterface interface.

	virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override { return true; };

	/** This fills in the parameters with data to send to the GPU. */
	virtual void SetShaderParameters(
		const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;

	/**
	 * This function initializes the data of an instance of this NDI, meaning that this
	 * function will run when hitting the start button for each instance of this NDI that
	 * are present.
	 */
	virtual bool InitPerInstanceData(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;

	/**
	 * This function removes the data of an instance of this NDI and the proxy, meaning that 
	 * this function will run when hitting the stop button for each instance of this NDI that
	 * are present.
	 */
	virtual void DestroyPerInstanceData(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;

	virtual int32 PerInstanceDataSize() const override{ return sizeof(FParticleUpsamplingData); };
	virtual bool HasPreSimulateTick() const override { return true; };

	/** This function runs every tick for every instance of this NDI. */
	virtual bool PerInstanceTick(
		void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
	virtual void ProvidePerInstanceDataForRenderThread(
		void* DataForRenderThread, void* PerInstanceData,
		const FNiagaraSystemInstanceID& SystemInstance) override;

#if WITH_EDITORONLY_DATA
	/**
	 * This lets the Niagara compiler know that it needs to recompile an effect when
	 * our HLSL file changes.
	 */
	virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;

	/** Loads our HLSL template script file and replaces all template arguments accordingly. */
	virtual void GetParameterDefinitionHLSL(
		const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;

	/**
	 * This function can be used to provide the hlsl code for gpu scripts. If the DI supports only cpu
	 * implementations, this is not needed. We don't need to actually print our function code to
	 * OutHLSL here because we use a template file that gets appended in
	 * GetParameterDefinitionHLSL().
	 */
	virtual bool GetFunctionHLSL(
		const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
		const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
		FString& OutHLSL) override;
#endif
	// ~End UNiagaraDataInterface Interface.

	// ~Begin UNiagaraDataInterfaceBase Interface.

	/** This fills in the expected parameter bindings we use to send data to the GPU. */
	virtual void BuildShaderParameters(
		FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;

	// ~End UNiagaraDataInterfaceBase Interface.

	/** Sets the CoarseParticles in local data storage. */
	void SetCoarseParticles(TArray<FCoarseParticle> NewCoarseParticles);

	/** Sets the Active Voxel Indices in local data storage. */
	void SetActiveVoxelIndices(TArray<FIntVector4> AVIs);

	/** Gets the elements in the active voxel buffer reference. */
	int GetElementsInActiveVoxelBuffer();
	void RecalculateFineParticleProperties(
		float Upsampling, float ElementSize, float ParticleDensity);
	void SetStaticVariables(float VoxelSize, float EaseStepSize);

protected:

	// ~Begin UNiagaraDataInterface interface.

#if WITH_EDITORONLY_DATA
	/**
	 * Lists all the functions that will be visible when using the NDI in the
	 * Niagara Blueprint system.
	 */
#if UE_VERSION_OLDER_THAN(5, 4, 0)
	virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
#else
	virtual void GetFunctionsInternal(
		TArray<FNiagaraFunctionSignature>& OutFunctions) const override;
#endif
#endif

	// ~End UNiagaraDataInterface interface.

private:

	FPUArrays* LocalData;

	/** The names of the different functions this NDI will call on the GPU */
	const FName UpdateGridName = TEXT("UpdateGrid");
	const FName ApplyParticleMassName = TEXT("ApplyParticleMass");
	const FName SpawnParticlesName = TEXT("SpawnParticles");
	const FName MoveParticlesName = TEXT("MoveParticles");
	const FName ClearTableName = TEXT("ClearTable");
	const FName GetVoxelPositionAndRoomName = TEXT("GetVoxelPositionAndRoom");
	const FName GetNominalRadiusName = TEXT("GetNominalRadius");
	const FName GetFineParticleRadiusName = TEXT("GetFineParticleRadius");
	const FName IsFineParticleAliveName = TEXT("IsFineParticleAlive");
	const FName GetCoarseParticleInfoName = TEXT("GetCoarseParticleInfo");

	const TCHAR* PUUnrealShaderHeaderFile = TEXT("/ParticleRenderingShaders/ParticleUpsampling.ush");

	const float PACKING_RATIO = 0.67f;
};
