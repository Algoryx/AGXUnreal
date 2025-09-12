// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/AGX_ParticleUpsamplingDI.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/ParticleRendering/ParticleUpsamplingDataInterface/ParticleUpsamplingDIProxy.h"

// Unreal Engine includes.
#include "NiagaraShaderParametersBuilder.h"
#include "NiagaraSystemInstance.h"
#include "SphereTypes.h"

#define LOCTEXT_NAMESPACE "ParticleUpsamplingDataInterface"

UAGX_ParticleUpsamplingDI::UAGX_ParticleUpsamplingDI(
	FObjectInitializer const& ObjectInitializer)
{
	Proxy.Reset(new FParticleUpsamplingDIProxy());
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
	FParticleUpsamplingDataHandler& Data = DataInterfaceProxy.SystemInstancesToInstanceData_RT.FindChecked(
		Context.GetSystemInstanceID());
	FShaderParameters* ShaderParameters = Context.GetParameterNestedStruct<FShaderParameters>();

	// Particle Shader Parameters
	ShaderParameters->CoarseParticles		= Data.Buffers->CoarseParticles;
	ShaderParameters->NumCoarseParticles	= Data.Data.CoarseParticles.Num();
	ShaderParameters->FineParticleMass		= Data.Data.FineParticleMass;
	ShaderParameters->FineParticleRadius	= Data.Data.FineParticleRadius;
	ShaderParameters->NominalRadius			= Data.Data.NominalRadius;

	// HashTable Shader Parameters
	ShaderParameters->ActiveVoxelIndices	= Data.Buffers->ActiveVoxelIndices;
	ShaderParameters->NumActiveVoxels		= Data.Data.ActiveVoxelIndices.Num();

	ShaderParameters->HashTable				= Data.Buffers->ActiveVoxelsTable;
	ShaderParameters->HTOccupancy			= Data.Buffers->ActiveVoxelsTableOccupancy;

	ShaderParameters->TableSize				= Data.Buffers->ActiveVoxelsCapacity;
	ShaderParameters->VoxelSize				= Data.Data.VoxelSize;

	// Other Variables
	ShaderParameters->AnimationSpeed		= Data.Data.EaseStepSize;
}

bool UAGX_ParticleUpsamplingDI::InitPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FParticleUpsamplingDataHandler* Data = new (PerInstanceData) FParticleUpsamplingDataHandler;
	Data->Init(SystemInstance);
	return true;
}

void UAGX_ParticleUpsamplingDI::DestroyPerInstanceData(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
	FParticleUpsamplingDataHandler* InstanceData = static_cast<FParticleUpsamplingDataHandler*>(PerInstanceData);

	InstanceData->Release();
	InstanceData->~FParticleUpsamplingDataHandler();
	FParticleUpsamplingDIProxy* ThisProxy = GetProxyAs<FParticleUpsamplingDIProxy>();
	ENQUEUE_RENDER_COMMAND(FNiagaraDIDestroyInstanceData)
	(
		[ThisProxy, InstanceID = SystemInstance->GetId()](FRHICommandListImmediate& CmdList)
		{
			FParticleUpsamplingDataHandler* ProxyData =
				ThisProxy->SystemInstancesToInstanceData_RT.Find(InstanceID);
			if (ProxyData != nullptr)
			{
				ThisProxy->SystemInstancesToInstanceData_RT.Remove(InstanceID);
			}
		});
}

bool UAGX_ParticleUpsamplingDI::PerInstanceTick(
	void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
	check(SystemInstance);
	FParticleUpsamplingDataHandler* Data = static_cast<FParticleUpsamplingDataHandler*>(PerInstanceData);
	if (!Data)
	{
		return true;
	}

	// Copy local data to data for instance.
	LocalData.TableSize = Data->Buffers->ActiveVoxelsCapacity;
	Data->Data = LocalData;
	return false;
}

void UAGX_ParticleUpsamplingDI::ProvidePerInstanceDataForRenderThread(
	void* DataForRenderThread, void* PerInstanceData,
	const FNiagaraSystemInstanceID& SystemInstance)
{
	FParticleUpsamplingDIProxy::ProvidePerInstanceDataForRenderThread(DataForRenderThread, PerInstanceData, SystemInstance);
}

void UAGX_ParticleUpsamplingDI::SetCoarseParticles(TArray<FCoarseParticle> NewCoarseParticles)
{
	LocalData.CoarseParticles = NewCoarseParticles;
}

void UAGX_ParticleUpsamplingDI::SetActiveVoxelIndices(TArray<FIntVector4> AVIs)
{
	LocalData.ActiveVoxelIndices = AVIs;
}

int UAGX_ParticleUpsamplingDI::GetHashTableCapacity()
{
	return LocalData.TableSize;
}

void UAGX_ParticleUpsamplingDI::RecalculateFineParticleProperties(
	float Upsampling, float ElementSize, float ParticleDensity)
{
	LocalData.NominalRadius =
		FMath::Pow(3.0f * PACKING_RATIO / (4.0f * PI), 1.0f / 3.0f) * ElementSize;
	LocalData.FineParticleRadius = LocalData.NominalRadius / FMath::Pow(Upsampling, 1.0f / 3.0f);
	float NominalMass = ParticleDensity * UE::Geometry::TSphere3<float>::Volume(LocalData.NominalRadius);
	LocalData.FineParticleMass = NominalMass / Upsampling;
}

void UAGX_ParticleUpsamplingDI::SetStaticVariables(float VoxelSize, float EaseStepSize)
{
	LocalData.VoxelSize = VoxelSize;
	LocalData.EaseStepSize = EaseStepSize;
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
	/**
	 * Since the same data interface can be instantiated multiple times, Niagara needs to know
	 * which functions referes to which instance when it generates the code from the template file. 
	 *
	 * This replaces the text {ParameterName} with the unique HLSL symbol name of each instance of
	 * the data interface. This ensures that the generated functions and variables are correctly 
	 * scoped and does not clash with each other.
	 */
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
	return	FunctionInfo.DefinitionName == UpdateGridName ||
			FunctionInfo.DefinitionName == ApplyParticleMassName ||
			FunctionInfo.DefinitionName == SpawnParticlesName ||
			FunctionInfo.DefinitionName == MoveParticlesName ||
			FunctionInfo.DefinitionName == ClearTableName ||
			FunctionInfo.DefinitionName == GetNominalRadiusName ||
			FunctionInfo.DefinitionName == GetFineParticleRadiusName ||
			FunctionInfo.DefinitionName == IsFineParticleAliveName;
}

#if UE_VERSION_OLDER_THAN(5, 4, 0)
void UAGX_ParticleUpsamplingDI::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
	Super::GetFunctions(OutFunctions);
#else
void UAGX_ParticleUpsamplingDI::GetFunctionsInternal(
	TArray<FNiagaraFunctionSignature>& OutFunctions) const
{
	Super::GetFunctionsInternal(OutFunctions);

#endif

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
		Sig.Name = GetNominalRadiusName;
		Sig.bMemberFunction = true;
		Sig.AddInput(FNiagaraVariable(
			FNiagaraTypeDefinition(GetClass()), TEXT("ParticleUpsamplingInterface")));
		Sig.AddOutput(
			FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("NominalRadius")));

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
}
#endif

#undef LOCTEXT_NAMESPACE