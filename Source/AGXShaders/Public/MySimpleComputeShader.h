#pragma once

#include "CoreMinimal.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MySimpleComputeShader.generated.h"

struct AGXSHADERS_API FMySimpleComputeShaderDispatchParams
{
	int X;
	int Y;
	int Z;

	
	int Input[3];
	TArray<float> Output;
	FRenderTarget* RenderTarget;
	

	FMySimpleComputeShaderDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class AGXSHADERS_API FMySimpleComputeShaderInterface {
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList,
		FMySimpleComputeShaderDispatchParams Params,
		TFunction<void(TArray<float> OutputVal)> AsyncCallback
	);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FMySimpleComputeShaderDispatchParams Params,
		TFunction<void(TArray<float> OutputVal)> AsyncCallback
	)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)(
		[Params, AsyncCallback](FRHICommandListImmediate& RHICmdList)
		{
			DispatchRenderThread(RHICmdList, Params, AsyncCallback);
		});
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FMySimpleComputeShaderDispatchParams Params,
		TFunction<void(TArray<float> OutputVal)> AsyncCallback
	)
	{
		if (IsInRenderingThread()) {
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, AsyncCallback);
		}else{
			DispatchGameThread(Params, AsyncCallback);
		}
	}
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMySimpleComputeShaderLibrary_AsyncExecutionCompleted, const TArray<float>&, Value);


UCLASS() // Change the _API to match your project
class AGXSHADERS_API UMySimpleComputeShaderLibrary_AsyncExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Execute the actual load
	virtual void Activate() override {
		// Create a dispatch parameters struct and fill it the input array with our args
		FMySimpleComputeShaderDispatchParams Params(RT->SizeX, RT->SizeY, 1);
		Params.Input[0] = Arg1;
		Params.Input[1] = Arg2;
		Params.Input[2] = Arg3;
		Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
		Params.Output = FloatArr;

		// Dispatch the compute shader and wait until it completes
		FMySimpleComputeShaderInterface::Dispatch(
			Params, [this](TArray<float> OutputVal) {
			this->Completed.Broadcast(OutputVal);
		});
	}

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", Category = "ComputeShader", WorldContext = "WorldContextObject"))
	static UMySimpleComputeShaderLibrary_AsyncExecution* ExecuteBaseComputeShader(
		UObject* WorldContextObject, int Arg1, int Arg2, int Arg3,
		UTextureRenderTarget2D* RenderTarget)
	{
		UMySimpleComputeShaderLibrary_AsyncExecution* Action = NewObject<UMySimpleComputeShaderLibrary_AsyncExecution>();
		Action->Arg1 = Arg1;
		Action->Arg2 = Arg2;
		Action->Arg3 = Arg3;
		Action->RT = RenderTarget;
		Action->RegisterWithGameInstance(WorldContextObject);

		return Action;
	}

	UPROPERTY(BlueprintAssignable)
	FOnMySimpleComputeShaderLibrary_AsyncExecutionCompleted Completed;

	int Arg1;
	int Arg2;
	int Arg3;
	UTextureRenderTarget2D* RT;
	TArray<float> FloatArr;
};
