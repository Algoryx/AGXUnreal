// Copyright 2026, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_RenderUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "OpenPLX/OpenPLXMaterialBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/TextureDefines.h"
#include "Engine/Texture2D.h"
#include "PixelFormat.h"
#include "TextureResource.h"

// Standard library includes.
#include <limits>

namespace
{
	TextureCompressionSettings GetTextureCompressionSettings(EOpenPLX_TextureUsage Usage)
	{
		switch (Usage)
		{
			case EOpenPLX_TextureUsage::BaseColor:
				return TC_Default;
			case EOpenPLX_TextureUsage::Scalar:
				return TC_Grayscale;
			case EOpenPLX_TextureUsage::Normal:
				return TC_Normalmap;
			case EOpenPLX_TextureUsage::Raw:
				return TC_Default;
		}

		UE_LOG(
			LogAGX, Warning, TEXT("Unknown OpenPLX texture usage '%d'. Using default compression."),
			static_cast<int32>(Usage));
		return TC_Default;
	}

	bool GetTextureElementCounts(
		const FOpenPLXTextureData& TextureData, int32 ElementsPerPixel, int32& OutNumPixels,
		int32& OutNumElements)
	{
		if (ElementsPerPixel < 1)
			return false;

		const int64 NumPixels =
			static_cast<int64>(TextureData.Width) * static_cast<int64>(TextureData.Height);
		const int64 NumElements = NumPixels * ElementsPerPixel;
		constexpr int64 MaxTArrayElements = static_cast<int64>(std::numeric_limits<int32>::max());
		if (NumElements > MaxTArrayElements)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot create Unreal texture from OpenPLX texture '%s': size=%dx%d with %d "
					 "elements per pixel exceeds the maximum number of elements in a TArray."),
				*TextureData.Name, TextureData.Width, TextureData.Height, ElementsPerPixel);
			return false;
		}

		OutNumPixels = static_cast<int32>(NumPixels);
		OutNumElements = static_cast<int32>(NumElements);
		return true;
	}

	bool ValidateOpenPLXTextureData(const FOpenPLXTextureData& TextureData)
	{
		if (TextureData.Width <= 0 || TextureData.Height <= 0 || TextureData.NumChannels <= 0 ||
			TextureData.NumChannels > 4)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot create Unreal texture from OpenPLX texture '%s': invalid metadata "
					 "size=%dx%d, channels=%d."),
				*TextureData.Name, TextureData.Width, TextureData.Height, TextureData.NumChannels);
			return false;
		}

		int32 NumPixels = 0;
		int32 ExpectedNumElements = 0;
		if (!GetTextureElementCounts(
				TextureData, TextureData.NumChannels, NumPixels, ExpectedNumElements))
		{
			return false;
		}

		if (TextureData.Pixels.Num() != ExpectedNumElements)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot create Unreal texture from OpenPLX texture '%s': got %d elements, "
					 "expected %d."),
				*TextureData.Name, TextureData.Pixels.Num(), ExpectedNumElements);
			return false;
		}

		return true;
	}

	int32 GetSwizzleChannelIndex(TCHAR Channel)
	{
		switch (Channel)
		{
			case 'r':
				return 0;
			case 'g':
				return 1;
			case 'b':
				return 2;
			case 'a':
				return 3;
		}

		UE_LOG(LogAGX, Warning, TEXT("Unsupported OpenPLX texture swizzle channel '%c'."), Channel);
		return INDEX_NONE;
	}

	bool GetTextureSourceChannels(
		const FOpenPLXTextureData& TextureData, TArray<int32>& OutSourceChannels,
		bool bApplySwizzle)
	{
		if (!ValidateOpenPLXTextureData(TextureData))
			return false;

		if (!bApplySwizzle || TextureData.Swizzle.IsEmpty())
		{
			OutSourceChannels.SetNumUninitialized(TextureData.NumChannels);
			for (int32 ChannelIndex = 0; ChannelIndex < TextureData.NumChannels; ++ChannelIndex)
				OutSourceChannels[ChannelIndex] = ChannelIndex;
			return true;
		}

		if (TextureData.Swizzle.Len() > 4)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot create Unreal texture from OpenPLX texture '%s': swizzle '%s' "
					 "has too many channels."),
				*TextureData.Name, *TextureData.Swizzle);
			return false;
		}

		OutSourceChannels.SetNumUninitialized(TextureData.Swizzle.Len());
		for (int32 ChannelIndex = 0; ChannelIndex < TextureData.Swizzle.Len(); ++ChannelIndex)
		{
			const int32 SourceChannelIndex =
				GetSwizzleChannelIndex(TextureData.Swizzle[ChannelIndex]);
			if (SourceChannelIndex == INDEX_NONE || SourceChannelIndex >= TextureData.NumChannels)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Cannot create Unreal texture from OpenPLX texture '%s': swizzle '%s' "
						 "is invalid for %d source channels."),
					*TextureData.Name, *TextureData.Swizzle, TextureData.NumChannels);
				return false;
			}

			OutSourceChannels[ChannelIndex] = SourceChannelIndex;
		}

		return true;
	}

	bool ConvertOpenPLXTextureToG8(const FOpenPLXTextureData& TextureData, TArray<uint8>& OutPixels)
	{
		TArray<int32> SourceChannels;
		if (!GetTextureSourceChannels(TextureData, SourceChannels, true))
			return false;

		int32 NumPixels = 0;
		int32 NumOutputElements = 0;
		if (!GetTextureElementCounts(TextureData, 1, NumPixels, NumOutputElements))
			return false;

		OutPixels.SetNumUninitialized(NumOutputElements);
		for (int32 PixelIndex = 0; PixelIndex < NumPixels; ++PixelIndex)
		{
			const uint8* Source =
				TextureData.Pixels.GetData() + PixelIndex * TextureData.NumChannels;
			OutPixels[PixelIndex] = Source[SourceChannels[0]];
		}

		return true;
	}

	bool ConvertOpenPLXTextureToBGRA8(
		const FOpenPLXTextureData& TextureData, TArray<uint8>& OutPixels,
		EOpenPLX_TextureUsage Usage)
	{
		TArray<int32> SourceChannels;
		if (!GetTextureSourceChannels(
				TextureData, SourceChannels, Usage != EOpenPLX_TextureUsage::Raw))
			return false;

		const int32 NumChannels = SourceChannels.Num();
		if (Usage == EOpenPLX_TextureUsage::Normal && NumChannels < 3)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot create normal map from OpenPLX texture '%s': expected at least 3 "
					 "channels, got %d."),
				*TextureData.Name, NumChannels);
			return false;
		}

		int32 NumPixels = 0;
		int32 NumOutputElements = 0;
		if (!GetTextureElementCounts(TextureData, 4, NumPixels, NumOutputElements))
			return false;

		OutPixels.SetNumUninitialized(NumOutputElements);
		for (int32 PixelIndex = 0; PixelIndex < NumPixels; ++PixelIndex)
		{
			const uint8* Source =
				TextureData.Pixels.GetData() + PixelIndex * TextureData.NumChannels;
			uint8 R = 255;
			uint8 G = 255;
			uint8 B = 255;
			uint8 A = 255;

			switch (NumChannels)
			{
				case 1:
					R = G = B = Source[SourceChannels[0]];
					break;
				case 2:
					R = G = B = Source[SourceChannels[0]];
					A = Source[SourceChannels[1]];
					break;
				case 3:
					R = Source[SourceChannels[0]];
					G = Source[SourceChannels[1]];
					B = Source[SourceChannels[2]];
					break;
				case 4:
					R = Source[SourceChannels[0]];
					G = Source[SourceChannels[1]];
					B = Source[SourceChannels[2]];
					A = Source[SourceChannels[3]];
					break;
			}

			if (NumChannels < 1 || NumChannels > 4)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Cannot create Unreal texture from OpenPLX texture '%s': unsupported "
						 "channel count %d."),
					*TextureData.Name, NumChannels);
				return false;
			}

			uint8* Destination = OutPixels.GetData() + PixelIndex * 4;
			Destination[0] = B;
			Destination[1] = G;
			Destination[2] = R;
			Destination[3] = A;
		}

		return true;
	}
}

UTexture2D* FOpenPLX_RenderUtilities::CreateTexture(
	const FOpenPLXTextureData& TextureData, UObject& Owner, EOpenPLX_TextureUsage Usage,
	bool bCreateRenderResource)
{
	const bool bScalarTexture = Usage == EOpenPLX_TextureUsage::Scalar;
	TArray<uint8> TexturePixels;
	if (bScalarTexture)
	{
		if (!ConvertOpenPLXTextureToG8(TextureData, TexturePixels))
			return nullptr;
	}
	else if (!ConvertOpenPLXTextureToBGRA8(TextureData, TexturePixels, Usage))
	{
		return nullptr;
	}

	FString WantedName = TextureData.Name.IsEmpty()
							 ? FString::Printf(TEXT("T_Texture_%s"), *TextureData.Guid.ToString())
							 : FString::Printf(TEXT("T_%s"), *TextureData.Name);
	if (WantedName.StartsWith(TEXT("T_T_")))
		WantedName.RemoveFromStart(TEXT("T_")); // Texture files may start with T_ already.

	const FString TextureName = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		&Owner, WantedName, UTexture2D::StaticClass());

	UTexture2D* Texture = NewObject<UTexture2D>(&Owner, *TextureName);
	if (Texture == nullptr)
		return nullptr;

	Texture->SRGB = Usage == EOpenPLX_TextureUsage::BaseColor;
	Texture->NeverStream = true;

#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
	Texture->CompressionSettings = GetTextureCompressionSettings(Usage);
	Texture->Source.Init(
		TextureData.Width, TextureData.Height, 1, 1, bScalarTexture ? TSF_G8 : TSF_BGRA8,
		TexturePixels.GetData());
#endif

	Texture->SetPlatformData(new FTexturePlatformData());
	FTexturePlatformData* PlatformData = Texture->GetPlatformData();

	PlatformData->SizeX = TextureData.Width;
	PlatformData->SizeY = TextureData.Height;
	PlatformData->SetNumSlices(1);
	PlatformData->PixelFormat = bScalarTexture ? PF_G8 : PF_B8G8R8A8;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	PlatformData->Mips.Add(Mip);
	Mip->SizeX = TextureData.Width;
	Mip->SizeY = TextureData.Height;
	void* MipData = Mip->BulkData.Lock(LOCK_READ_WRITE);
	MipData = Mip->BulkData.Realloc(TexturePixels.Num());
	if (MipData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Failed to allocate mip data for OpenPLX texture '%s'."),
			*TextureData.Name);
		Mip->BulkData.Unlock();
		return nullptr;
	}

	FMemory::Memcpy(MipData, TexturePixels.GetData(), TexturePixels.Num());
	Mip->BulkData.Unlock();

	if (bCreateRenderResource)
		Texture->UpdateResource();

	return Texture;
}
