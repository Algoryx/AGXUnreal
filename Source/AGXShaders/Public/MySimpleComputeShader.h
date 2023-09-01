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

	int Width;
	int Height;
	TArray<FVector4> Output;
	FRenderTarget* RenderTarget;

	FMySimpleComputeShaderDispatchParams(int x, int y, int z)
		: X(x)
		, Y(y)
		, Z(z)
	{
	}
};

// This is a public interface that we define so outside code can invoke our compute shader.
class AGXSHADERS_API FMySimpleComputeShaderInterface
{
public:
	// Executes this shader on the render thread
	static void DispatchRenderThread(
		FRHICommandListImmediate& RHICmdList, FMySimpleComputeShaderDispatchParams Params,
		TFunction<void(const TArray<FVector4>& OutputVal)> AsyncCallback);

	// Executes this shader on the render thread from the game thread via EnqueueRenderThreadCommand
	static void DispatchGameThread(
		FMySimpleComputeShaderDispatchParams Params,
		TFunction<void(const TArray<FVector4>& OutputVal)> AsyncCallback)
	{
		ENQUEUE_RENDER_COMMAND(SceneDrawCompletion)
		([Params, AsyncCallback](FRHICommandListImmediate& RHICmdList)
		 { DispatchRenderThread(RHICmdList, Params, AsyncCallback); });
	}

	// Dispatches this shader. Can be called from any thread
	static void Dispatch(
		FMySimpleComputeShaderDispatchParams Params,
		TFunction<void(const TArray<FVector4>& OutputVal)> AsyncCallback)
	{
		if (IsInRenderingThread())
		{
			DispatchRenderThread(GetImmediateCommandList_ForRenderCommand(), Params, AsyncCallback);
		}
		else
		{
			DispatchGameThread(Params, AsyncCallback);
		}
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
	FOnMySimpleComputeShaderLibrary_AsyncExecutionCompleted, const TArray<FVector4>&, Points);

UCLASS() // Change the _API to match your project
class AGXSHADERS_API UMySimpleComputeShaderLibrary_AsyncExecution : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	// Execute the actual load
	virtual void Activate() override
	{
		// Create a dispatch parameters struct and fill it the input array with our args
		FMySimpleComputeShaderDispatchParams Params(RT->SizeX, RT->SizeY, 1);
		Params.RenderTarget = RT->GameThread_GetRenderTargetResource();
		Params.Width = Width;
		Params.Height = Height;

		// Dispatch the compute shader and wait until it completes
		FMySimpleComputeShaderInterface::Dispatch(
			Params,
			[this](const TArray<FVector4>& OutputVal)
			{
				if (!IsValid(this) || this->HasAnyFlags(EObjectFlags::RF_PendingKill) ||
					!IsValid(World) || World->HasAnyFlags(EObjectFlags::RF_PendingKill))
				{
					return;
				}

				DrawDebugPoints(this->World, OutputVal);
				this->Completed.Broadcast(OutputVal);
			});
	}

	UFUNCTION(
		BlueprintCallable, meta =
							   (BlueprintInternalUseOnly = "true", Category = "ComputeShader",
								WorldContext = "WorldContextObject"))
	static UMySimpleComputeShaderLibrary_AsyncExecution* ExecuteBaseComputeShader(
		UObject* WorldContextObject, UTextureRenderTarget2D* RenderTarget)
	{
		UMySimpleComputeShaderLibrary_AsyncExecution* Action =
			NewObject<UMySimpleComputeShaderLibrary_AsyncExecution>();
		Action->RT = RenderTarget;
		Action->Width = RenderTarget->SizeX;
		Action->Height = RenderTarget->SizeY;
		Action->World = WorldContextObject->GetWorld();

		Action->RegisterWithGameInstance(WorldContextObject);
		return Action;
	}

	UPROPERTY(BlueprintAssignable)
	FOnMySimpleComputeShaderLibrary_AsyncExecutionCompleted Completed;

	static void DrawDebugPoints(UWorld* World, const TArray<FVector4>& Points);

	UTextureRenderTarget2D* RT;
	UWorld* World;
	int Width;
	int Height;
};
