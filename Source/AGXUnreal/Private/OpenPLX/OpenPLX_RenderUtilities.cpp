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
			default:
				checkNoEntry();
				return TC_Default;
		}
	}

	bool ValidateOpenPLXTextureData(const FOpenPLXTextureData& TextureData)
	{
		if (TextureData.Width <= 0 || TextureData.Height <= 0 || TextureData.NumChannels <= 0 ||
			TextureData.NumChannels > 4)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Cannot create Unreal texture from OpenPLX texture '%s': invalid metadata "
					"size=%dx%d, channels=%d."),
				*TextureData.Name, TextureData.Width, TextureData.Height,
				TextureData.NumChannels);
			return false;
		}

		const int64 NumPixels = static_cast<int64>(TextureData.Width) * TextureData.Height;
		const int64 ExpectedNumBytes = NumPixels * TextureData.NumChannels;
		if (TextureData.Pixels.Num() != ExpectedNumBytes)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Cannot create Unreal texture from OpenPLX texture '%s': got %d bytes, "
					"expected %lld."),
				*TextureData.Name, TextureData.Pixels.Num(), ExpectedNumBytes);
			return false;
		}

		return true;
	}

	bool ConvertOpenPLXTextureToG8(const FOpenPLXTextureData& TextureData, TArray<uint8>& OutPixels)
	{
		if (!ValidateOpenPLXTextureData(TextureData))
			return false;

		const int64 NumPixels = static_cast<int64>(TextureData.Width) * TextureData.Height;
		OutPixels.SetNumUninitialized(NumPixels);
		for (int64 PixelIndex = 0; PixelIndex < NumPixels; ++PixelIndex)
		{
			const uint8* Source = TextureData.Pixels.GetData() + PixelIndex * TextureData.NumChannels;
			OutPixels[PixelIndex] = Source[0];
		}

		return true;
	}

	bool ConvertOpenPLXTextureToBGRA8(
		const FOpenPLXTextureData& TextureData, TArray<uint8>& OutPixels,
		EOpenPLX_TextureUsage Usage)
	{
		if (!ValidateOpenPLXTextureData(TextureData))
			return false;

		if (Usage == EOpenPLX_TextureUsage::Normal && TextureData.NumChannels < 3)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"Cannot create normal map from OpenPLX texture '%s': expected at least 3 "
					"channels, got %d."),
				*TextureData.Name, TextureData.NumChannels);
			return false;
		}

		const int64 NumPixels = static_cast<int64>(TextureData.Width) * TextureData.Height;
		OutPixels.SetNumUninitialized(NumPixels * 4);
		for (int64 PixelIndex = 0; PixelIndex < NumPixels; ++PixelIndex)
		{
			const uint8* Source = TextureData.Pixels.GetData() + PixelIndex * TextureData.NumChannels;
			uint8 R = 255;
			uint8 G = 255;
			uint8 B = 255;
			uint8 A = 255;

			switch (TextureData.NumChannels)
			{
				case 1:
					R = G = B = Source[0];
					break;
				case 2:
					R = G = B = Source[0];
					A = Source[1];
					break;
				case 3:
					R = Source[0];
					G = Source[1];
					B = Source[2];
					break;
				case 4:
					R = Source[0];
					G = Source[1];
					B = Source[2];
					A = Source[3];
					break;
				default:
					checkNoEntry();
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
	const FOpenPLXTextureData& TextureData, UObject& Owner, EOpenPLX_TextureUsage Usage)
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

	const FString WantedName = TextureData.Name.IsEmpty()
								   ? FString::Printf(
										 TEXT("T_PLXTexture_%s"), *TextureData.Guid.ToString())
								   : FString::Printf(TEXT("T_%s"), *TextureData.Name);
	const FString TextureName = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		&Owner, WantedName, UTexture2D::StaticClass());

	UTexture2D* Texture = NewObject<UTexture2D>(&Owner, *TextureName);
	if (Texture == nullptr)
		return nullptr;

	Texture->SRGB = Usage == EOpenPLX_TextureUsage::BaseColor;

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
			LogAGX, Warning,
			TEXT("Failed to allocate mip data for OpenPLX texture '%s'."),
			*TextureData.Name);
		Mip->BulkData.Unlock();
		return nullptr;
	}

	FMemory::Memcpy(MipData, TexturePixels.GetData(), TexturePixels.Num());
	Mip->BulkData.Unlock();

	Texture->UpdateResource();
	return Texture;
}
