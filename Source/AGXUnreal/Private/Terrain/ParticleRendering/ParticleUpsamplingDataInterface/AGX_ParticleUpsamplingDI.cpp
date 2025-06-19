// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/AGX_ParticleUpsamplingDI.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDIProxy.h"

// Unreal Engine includes.
#include "NiagaraShaderParametersBuilder.h"
#include "NiagaraSystemInstance.h"

#define LOCTEXT_NAMESPACE "ParticleUpsamplingDataInterface"


UAGX_ParticleUpsamplingDI::UAGX_ParticleUpsamplingDI(
	FObjectInitializer const& ObjectInitializer)
{
	Proxy.Reset(new FParticleUpsamplingDIProxy());
	LocalData = new FPUArrays(
		FParticleUpsamplingData::INITIAL_CP_BUFFER_SIZE, 
		FParticleUpsamplingData::INITIAL_VOXEL_BUFFER_SIZE);
}

void UAGX_ParticleUpsamplingDI::PostInitProperties()
{
	Super::PostInitProperties();

	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		ENiagaraTypeRegistryFlags Flags = 
			ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
		FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
	}
}

void UAGX_ParticleUpsamplingDI::BuildShaderParameters(
	FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
	ShaderParametersBuilder.AddNestedStruct<FShaderParameters>();
}

void UAGX_ParticleUpsamplingDI::SetShaderParameters(
	const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
	FParticleUpsamplingDIProxy& DataInterfaceProxy = Context.GetProxy<FParticleUpsamplingDIProxy>();
	FParticleUpsamplingData& Data = DataInterfaceProxy.SystemInstancesToInstanceData_RT.FindChecked(
		Context.GetSystemInstanceID());
	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();

	// Particle Shader Parameters
	ShaderParameters->CoarseParticles		= Data.PUBuffers->CoarseParticleBufferRef;
	ShaderParameters->NumCoarseParticles	= Data.PUArrays->CoarseParticles.Num();
	ShaderParameters->FineParticleMass		= Data.PUArrays->FineParticleMass;
	ShaderParameters->FineParticleRadius	= Data.PUArrays->FineParticleRadius;
	ShaderParameters->NominalRadius			= Data.PUArrays->NominalRadius;

	// HashTable Shader Parameters
	ShaderParameters->ActiveVoxelIndices	= Data.PUBuffers->ActiveVoxelIndicesBufferRef;
	ShaderParameters->NumActiveVoxels		= Data.PUArrays->ActiveVoxelIndices.Num();

	ShaderParameters->HashTable				= Data.PUBuffers->HashTableBufferRef;
	ShaderParameters->HTOccupancy			=Data.PUBuffers->HashTableOccupancyBufferRef;

	ShaderParameters->TableSize				= Data.PUArrays->NumElementsInActiveVoxelBuffer;
	ShaderParameters->VoxelSize				= Data.PUArrays->VoxelSize;

	// Other Variables
	ShaderParameters->Time					= Data.PUArrays->Time;
	ShaderParameters->AnimationSpeed		= Data.PUArrays->EaseStepSize;
}

bool UAGX_ParticleUpsamplingDI::InitPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FParticleUpsamplingData* Data = static_cast<FParticleUpsamplingData*>(PerInstanceData);
	Data->Init(SystemInstance);
	LocalData = new FPUArrays(
		FParticleUpsamplingData::INITIAL_CP_BUFFER_SIZE,
		FParticleUpsamplingData::INITIAL_VOXEL_BUFFER_SIZE);
	return true;
}

void UAGX_ParticleUpsamplingDI::DestroyPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FParticleUpsamplingData* InstanceData = static_cast<FParticleUpsamplingData*>(PerInstanceData);

	InstanceData->Release();
	InstanceData->~FParticleUpsamplingData();
	FParticleUpsamplingDIProxy* ThisProxy = GetProxyAs<FParticleUpsamplingDIProxy>();
	ENQUEUE_RENDER_COMMAND(FNiagaraDIDestroyInstanceData)
	(
		[ThisProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			FParticleUpsamplingData* ProxyData =
				ThisProxy->SystemInstancesToInstanceData_RT.Find(InstanceID);
			if (ProxyData != nullptr && ProxyData->PUArrays)
			{
				ThisProxy->SystemInstancesToInstanceData_RT.Remove(InstanceID);
				delete ProxyData->PUArrays;
			}
		});
}

bool UAGX_ParticleUpsamplingDI::PerInstanceTick(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	check(SystemInstance);
	FParticleUpsamplingData* Data = static_cast<FParticleUpsamplingData*>(PerInstanceData);
	if (!Data)
	{
		return true;
	}

	Data->Update(SystemInstance, LocalData);
	LocalData->NeedsCPResize = false;
	LocalData->NeedsVoxelResize = false;
	return false;
}

void UAGX_ParticleUpsamplingDI::ProvidePerInstanceDataForRenderThread(
	void* DataForRenderThread, void* PerInstanceData,
	const FNiagaraSystemInstanceID& SystemInstance)
{
	FParticleUpsamplingData* RenderThreadData =
		static_cast<FParticleUpsamplingData*>(DataForRenderThread);
	FParticleUpsamplingData* GameThreadData =
		static_cast<FParticleUpsamplingData*>(PerInstanceData);

	if (RenderThreadData && GameThreadData)
	{
		RenderThreadData->PUBuffers = GameThreadData->PUBuffers;
		RenderThreadData->PUArrays = new FPUArrays(
			FParticleUpsamplingData::INITIAL_CP_BUFFER_SIZE,
			FParticleUpsamplingData::INITIAL_VOXEL_BUFFER_SIZE);
		RenderThreadData->PUArrays->CopyFrom(GameThreadData->PUArrays);
	}
}

void UAGX_ParticleUpsamplingDI::SetCoarseParticles(TArray<FCoarseParticle> NewCoarseParticles)
{
	uint32 ArraySize = NewCoarseParticles.Num();
	if (LocalData->NumElementsInCoarseParticleBuffer < ArraySize)
	{
		LocalData->NumElementsInCoarseParticleBuffer *= 2;
		LocalData->NeedsCPResize = true;
	}
	LocalData->CoarseParticles = NewCoarseParticles;
}

void UAGX_ParticleUpsamplingDI::SetActiveVoxelIndices(TArray<FIntVector4> AVIs)
{
	uint32 ArraySize = AVIs.Num();
	if (LocalData->NumElementsInActiveVoxelBuffer < ArraySize)
	{
		LocalData->NumElementsInActiveVoxelBuffer *= 2;
		LocalData->NeedsVoxelResize = true;
	}
	LocalData->ActiveVoxelIndices = AVIs;
}

int UAGX_ParticleUpsamplingDI::GetElementsInActiveVoxelBuffer()
{
	return LocalData->NumElementsInActiveVoxelBuffer;
}

void UAGX_ParticleUpsamplingDI::RecalculateFineParticleProperties(
	float Upsampling, float ElementSize, float ParticleDensity)
{
	LocalData->NominalRadius =
		FMath::Pow(3.0f * PACKING_RATIO / (4.0f * PI), 1.0f / 3.0f) * ElementSize;
	LocalData->FineParticleRadius = LocalData->NominalRadius / FMath::Pow(Upsampling, 1.0f / 3.0f);
	float NominalMass =
		ParticleDensity * 4.0f / 3.0f * PI * FMath::Pow(LocalData->NominalRadius, 3.0f);
	LocalData->FineParticleMass = NominalMass / Upsampling;
}

void UAGX_ParticleUpsamplingDI::SetStaticVariables(float VoxelSize, float EaseStepSize)
{
	LocalData->VoxelSize = VoxelSize;
	LocalData->EaseStepSize = EaseStepSize;
}

#if WITH_EDITORONLY_DATA

bool UAGX_ParticleUpsamplingDI::AppendCompileHash(
	FNiagaraCompileHashVisitor* InVisitor) const
{
	bool bSuccess = Super::AppendCompileHash(InVisitor);
	bSuccess &= InVisitor->UpdateShaderFile(PUUnrealShaderHeaderFile);
	bSuccess &= InVisitor->UpdateShaderParameters<FShaderParameters>();
	return bSuccess;
}

void UAGX_ParticleUpsamplingDI::GetParameterDefinitionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
	const TMap<FString, FStringFormatArg> TemplateArgs = {
		{TEXT("ParameterName"), ParamInfo.DataInterfaceHLSLSymbol},
	};
	AppendTemplateHLSL(OutHLSL, PUUnrealShaderHeaderFile, TemplateArgs);
}

bool UAGX_ParticleUpsamplingDI::GetFunctionHLSL(
	const FNiagaraDataInterfaceGPUParamInfo& ParamInfo,
	const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex,
	FString& OutHLSL)
{
	return FunctionInfo.DefinitionName == UpdateGridName ||
		   FunctionInfo.DefinitionName == ApplyParticleMassName ||
		   FunctionInfo.DefinitionName == SpawnParticlesName ||
		   FunctionInfo.DefinitionName == MoveParticlesName ||
		   FunctionInfo.DefinitionName == GetVoxelPositionAndRoomName ||
		   FunctionInfo.DefinitionName == ClearTableName ||
		   FunctionInfo.DefinitionName == GetFineParticleRadiusName ||
		   FunctionInfo.DefinitionName == IsFineParticleAliveName ||
		   FunctionInfo.DefinitionName == GetNominalRadiusName ||
		   FunctionInfo.DefinitionName == GetCoarseParticleInfoName;
}

void UAGX_ParticleUpsamplingDI::GetFunctionsInternal(
	TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	Super::GetFunctionsInternal(OutFunctions);

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = UpdateGridName;

		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));

		Sig.bMemberFunction = true;
		Sig.Description = LOCTEXT(
			"UpdateGridNameFunctionDescription",
			"Updates the Voxel Grid with sampled values from the Coarse Particles.");
		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = ApplyParticleMassName;

		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticlePosition")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEase")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEaseNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsAlive")));
		
		Sig.bMemberFunction = true;
		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = SpawnParticlesName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("ShouldSpawn")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("ParticleRoom")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("VoxelIndex")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MaxBounds")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MinBounds")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = MoveParticlesName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("ParticleIndex")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticlePosition")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEase")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("DeltaTime")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticlePositionNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("ParticleEaseNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticleVelocityNew")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsAlive")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = ClearTableName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetVoxelPositionAndRoomName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("VoxelId")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("VoxelPosition")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("Room")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MaxBounds")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("MinBounds")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetFineParticleRadiusName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("FineParticleRadius")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = IsFineParticleAliveName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Ease")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetBoolDef(), TEXT("IsAlive")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetCoarseParticleInfoName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition::GetIntDef(), TEXT("CoarseParticleIndex")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec4Def(), TEXT("ParticlePositionAndRadius")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetVec3Def(), TEXT("ParticleVelocity")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}

	{
		FNiagaraFunctionSignature Sig;
		Sig.Name = GetNominalRadiusName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("NominalRadius")));

		Sig.ModuleUsageBitmask = ENiagaraScriptUsageMask::Particle;
		Sig.bExperimental = true;
		Sig.bSupportsCPU = false;
		Sig.bSupportsGPU = true;
		OutFunctions.Add(Sig);
	}
}
#endif

#undef LOCTEXT_NAMESPACE