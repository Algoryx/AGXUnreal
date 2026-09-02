// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraEnums.h"
#include "Sensors/AGX_CameraOutputBase.h"
#include "Sensors/AGX_ColorMappingMatrix.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_CameraOutputColor.generated.h"

class UAGX_CameraSensorComponent;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_CameraOutputColor : public FAGX_CameraOutputBase
{
	GENERATED_BODY()

public:
	virtual ~FAGX_CameraOutputColor() override = default;

	/**
	 * Output channel element type.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera")
	EAGX_CameraOutputChannelType ChannelType {EAGX_CameraOutputChannelType::U8};

	void SetChannelType(EAGX_CameraOutputChannelType InChannelType);
	EAGX_CameraOutputChannelType GetChannelType() const;

	/**
	 * Output gamma correction value.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera", Meta = (ClampMin = "0.0"))
	double Gamma {1.0};

	void SetGamma(double InGamma);
	double GetGamma() const;

	/**
	 * Matrix for remapping color channels from linear RGB to the output linear space.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Camera")
	FAGX_ColorMappingMatrix ColorMappingMatrix;

	void SetColorMappingMatrix(FAGX_ColorMappingMatrix InColorMappingMatrix);
	FAGX_ColorMappingMatrix GetColorMappingMatrix() const;

	/**
	 * Number of output channels per pixel, e.g. RGB is 3, RGBA is 4.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Camera",
		Meta = (ClampMin = "1", ClampMax = "4"))
	uint8 ChannelCount {4};

	void SetChannelCount(uint8 InChannelCount);
	uint8 GetChannelCount() const;

	void GetData(TArray<FColor>& OutData);
	void GetDataU8(TArray<uint8>& OutData);
	void GetDataF32(TArray<float>& OutData);

	FAGX_CameraOutputColor& operator=(const FAGX_CameraOutputColor& Other);
	bool operator==(const FAGX_CameraOutputColor& Other) const;

protected:
	virtual TUniquePtr<FCameraOutputBarrier> CreateNativeBarrier() const override;
};

/**
 * This class acts as an API that exposes functions of FAGX_CameraOutputColor in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_CameraOutputColor_LF : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void AddTo(
		UPARAM(ref) FAGX_CameraOutputColor& Output, UAGX_CameraSensorComponent* Camera)
	{
		Output.AddTo(Camera);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void SetResolution(UPARAM(ref) FAGX_CameraOutputColor& Output, FIntPoint Resolution)
	{
		Output.SetResolution(Resolution);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	static FIntPoint GetResolution(UPARAM(ref) const FAGX_CameraOutputColor& Output)
	{
		return Output.GetResolution();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void SetFrameRate(UPARAM(ref) FAGX_CameraOutputColor& Output, double FrameRate)
	{
		Output.SetFrameRate(FrameRate);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	static double GetFrameRate(UPARAM(ref) const FAGX_CameraOutputColor& Output)
	{
		return Output.GetFrameRate();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void SetConstantCapture(
		UPARAM(ref) FAGX_CameraOutputColor& Output, bool bConstantCapture)
	{
		Output.SetConstantCapture(bConstantCapture);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	static bool GetConstantCapture(UPARAM(ref) const FAGX_CameraOutputColor& Output)
	{
		return Output.GetConstantCapture();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void SetChannelType(
		UPARAM(ref) FAGX_CameraOutputColor& Output, EAGX_CameraOutputChannelType ChannelType)
	{
		Output.SetChannelType(ChannelType);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	static EAGX_CameraOutputChannelType GetChannelType(
		UPARAM(ref) const FAGX_CameraOutputColor& Output)
	{
		return Output.GetChannelType();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void SetGamma(UPARAM(ref) FAGX_CameraOutputColor& Output, double Gamma)
	{
		Output.SetGamma(Gamma);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	static double GetGamma(UPARAM(ref) const FAGX_CameraOutputColor& Output)
	{
		return Output.GetGamma();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void SetColorMappingMatrix(
		UPARAM(ref) FAGX_CameraOutputColor& Output,
		FAGX_ColorMappingMatrix ColorMappingMatrix)
	{
		Output.SetColorMappingMatrix(ColorMappingMatrix);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	static FAGX_ColorMappingMatrix GetColorMappingMatrix(
		UPARAM(ref) const FAGX_CameraOutputColor& Output)
	{
		return Output.GetColorMappingMatrix();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void SetChannelCount(UPARAM(ref) FAGX_CameraOutputColor& Output, uint8 ChannelCount)
	{
		Output.SetChannelCount(ChannelCount);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Camera")
	static uint8 GetChannelCount(UPARAM(ref) const FAGX_CameraOutputColor& Output)
	{
		return Output.GetChannelCount();
	}

	/**
	 * Get the latest Camera Color Output data as FColor values.
	 *
	 * Only valid for UInt8 output data. Supports 1 to 4 channels by filling missing RGB channels
	 * with 0 and missing alpha with 255.
	 * This is an expensive operation because it copies the Camera output data into a TArray<FColor>.
	 * For displaying the Camera output, prefer UAGX_CameraSensorComponent::GetOutputRenderTarget().
	 * When converting to a ROS2 message, prefer the appropriate FAGX_ROS2Utilities conversion
	 * function that operates directly on the underlying Camera output data buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void GetData(UPARAM(ref) FAGX_CameraOutputColor& Output, TArray<FColor>& OutData)
	{
		Output.GetData(OutData);
	}

	/**
	 * Get the latest Camera Color Output data as flat UInt8 channel values.
	 *
	 * Only valid for UInt8 output data. The array contains Width * Height * ChannelCount values.
	 * This is an expensive operation because it copies the Camera output data into a TArray<uint8>.
	 * For displaying the Camera output, prefer UAGX_CameraSensorComponent::GetOutputRenderTarget().
	 * When converting to a ROS2 message, prefer the appropriate FAGX_ROS2Utilities conversion
	 * function that operates directly on the underlying Camera output data buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void GetDataU8(UPARAM(ref) FAGX_CameraOutputColor& Output, TArray<uint8>& OutData)
	{
		Output.GetDataU8(OutData);
	}

	/**
	 * Get the latest Camera Color Output data as flat Float32 channel values.
	 *
	 * Only valid for Float32 output data. The array contains Width * Height * ChannelCount values.
	 * This is an expensive operation because it copies the Camera output data into a TArray<float>.
	 * For displaying the Camera output, prefer UAGX_CameraSensorComponent::GetOutputRenderTarget().
	 * When converting to a ROS2 message, prefer the appropriate FAGX_ROS2Utilities conversion
	 * function that operates directly on the underlying Camera output data buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	static void GetDataF32(UPARAM(ref) FAGX_CameraOutputColor& Output, TArray<float>& OutData)
	{
		Output.GetDataF32(OutData);
	}
};
