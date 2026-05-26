// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/AGX_TextureUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/Texture2D.h"
#include "RenderingThread.h"
#include "TextureResource.h"

namespace
{
	bool AreBulkDataEqual(const FByteBulkData& BulkDataA, const FByteBulkData& BulkDataB)
	{
		const int64 SizeA = BulkDataA.GetBulkDataSize();
		const int64 SizeB = BulkDataB.GetBulkDataSize();
		if (SizeA != SizeB)
			return false;

		if (SizeA == 0)
			return true;

		const void* DataA = BulkDataA.LockReadOnly();
		const void* DataB = BulkDataB.LockReadOnly();
		const bool bResult =
			DataA != nullptr && DataB != nullptr && FMemory::Memcmp(DataA, DataB, SizeA) == 0;
		BulkDataA.Unlock();
		BulkDataB.Unlock();
		return bResult;
	}

	bool CopyBulkData(const FByteBulkData& Source, FByteBulkData& Destination)
	{
		const int64 Size = Source.GetBulkDataSize();
		void* DestinationData = Destination.Lock(LOCK_READ_WRITE);
		DestinationData = Destination.Realloc(Size);
		if (Size == 0)
		{
			Destination.Unlock();
			return true;
		}

		const void* SourceData = Source.LockReadOnly();
		if (SourceData == nullptr || DestinationData == nullptr)
		{
			Source.Unlock();
			Destination.Unlock();
			return false;
		}

		FMemory::Memcpy(DestinationData, SourceData, Size);
		Source.Unlock();
		Destination.Unlock();
		return true;
	}

	bool ArePlatformDataEqual(
		const FTexturePlatformData* PlatformDataA, const FTexturePlatformData* PlatformDataB)
	{
		if (PlatformDataA == nullptr || PlatformDataB == nullptr)
			return PlatformDataA == PlatformDataB;

		if (PlatformDataA->SizeX != PlatformDataB->SizeX ||
			PlatformDataA->SizeY != PlatformDataB->SizeY ||
			PlatformDataA->GetNumSlices() != PlatformDataB->GetNumSlices() ||
			PlatformDataA->PixelFormat != PlatformDataB->PixelFormat ||
			PlatformDataA->Mips.Num() != PlatformDataB->Mips.Num())
		{
			return false;
		}

		for (int32 Index = 0; Index < PlatformDataA->Mips.Num(); ++Index)
		{
			const FTexture2DMipMap& MipA = PlatformDataA->Mips[Index];
			const FTexture2DMipMap& MipB = PlatformDataB->Mips[Index];
			if (MipA.SizeX != MipB.SizeX || MipA.SizeY != MipB.SizeY ||
				!AreBulkDataEqual(MipA.BulkData, MipB.BulkData))
			{
				return false;
			}
		}

		return true;
	}

	FTexturePlatformData* CopyPlatformData(const FTexturePlatformData* Source)
	{
		if (Source == nullptr)
			return nullptr;

		FTexturePlatformData* Destination = new FTexturePlatformData();
		Destination->SizeX = Source->SizeX;
		Destination->SizeY = Source->SizeY;
		Destination->SetNumSlices(Source->GetNumSlices());
		Destination->PixelFormat = Source->PixelFormat;

		for (const FTexture2DMipMap& SourceMip : Source->Mips)
		{
			FTexture2DMipMap* DestinationMip = new FTexture2DMipMap();
			Destination->Mips.Add(DestinationMip);
			DestinationMip->SizeX = SourceMip.SizeX;
			DestinationMip->SizeY = SourceMip.SizeY;
			if (!CopyBulkData(SourceMip.BulkData, DestinationMip->BulkData))
			{
				delete Destination;
				return nullptr;
			}
		}

		return Destination;
	}

#if WITH_EDITORONLY_DATA
	bool AreTextureSourcesEqual(const UTexture2D& TextureA, const UTexture2D& TextureB)
	{
		if (TextureA.Source.GetSizeX() != TextureB.Source.GetSizeX() ||
			TextureA.Source.GetSizeY() != TextureB.Source.GetSizeY() ||
			TextureA.Source.GetNumSlices() != TextureB.Source.GetNumSlices() ||
			TextureA.Source.GetNumMips() != TextureB.Source.GetNumMips() ||
			TextureA.Source.GetFormat() != TextureB.Source.GetFormat())
		{
			return false;
		}

		if (TextureA.Source.GetNumMips() == 0)
			return true;

		for (int32 MipIndex = 0; MipIndex < TextureA.Source.GetNumMips(); ++MipIndex)
		{
			TArray64<uint8> DataA;
			TArray64<uint8> DataB;
			FTextureSource& SourceA = const_cast<FTextureSource&>(TextureA.Source);
			FTextureSource& SourceB = const_cast<FTextureSource&>(TextureB.Source);
			if (!SourceA.GetMipData(DataA, MipIndex) || !SourceB.GetMipData(DataB, MipIndex))
			{
				return false;
			}

			if (DataA.Num() != DataB.Num() ||
				(DataA.Num() > 0 &&
				 FMemory::Memcmp(DataA.GetData(), DataB.GetData(), DataA.Num()) != 0))
			{
				return false;
			}
		}

		return true;
	}

	bool CopyTextureSource(const UTexture2D& Source, UTexture2D& Destination)
	{
		if (Source.Source.GetNumMips() == 0)
		{
			Destination.Source.Reset();
			return true;
		}

		if (Source.Source.GetNumMips() != 1)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to copy texture source data from '%s': expected exactly one source "
					 "mip, got %d."),
				*Source.GetName(), Source.Source.GetNumMips());
			return false;
		}

		TArray64<uint8> SourceData;
		FTextureSource& SourceTextureSource = const_cast<FTextureSource&>(Source.Source);
		if (!SourceTextureSource.GetMipData(SourceData, 0))
			return false;

		Destination.Source.Init(
			Source.Source.GetSizeX(), Source.Source.GetSizeY(), Source.Source.GetNumSlices(),
			Source.Source.GetNumMips(), Source.Source.GetFormat(), SourceData.GetData());
		return true;
	}
#endif
}

bool AGX_TextureUtilities::CopyTexture(UTexture2D* Source, UTexture2D* Destination)
{
	if (Source == nullptr || Destination == nullptr)
		return false;

	FTexturePlatformData* PlatformData = CopyPlatformData(Source->GetPlatformData());
	if (Source->GetPlatformData() != nullptr && PlatformData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Unable to copy texture platform data from '%s' to '%s'."),
			*Source->GetName(), *Destination->GetName());
		return false;
	}

#if WITH_EDITORONLY_DATA
	if (!CopyTextureSource(*Source, *Destination))
	{
		delete PlatformData;
		UE_LOG(
			LogAGX, Warning, TEXT("Unable to copy texture source data from '%s' to '%s'."),
			*Source->GetName(), *Destination->GetName());
		return false;
	}
#endif

	Destination->ReleaseResource();
	FlushRenderingCommands();

	Destination->SRGB = Source->SRGB;
	Destination->NeverStream = Source->NeverStream;
	Destination->CompressionSettings = Source->CompressionSettings;
	Destination->MipGenSettings = Source->MipGenSettings;
	Destination->SetPlatformData(PlatformData);
	Destination->UpdateResource();
	return true;
}

bool AGX_TextureUtilities::AreTexturesEqual(UTexture2D* TextureA, UTexture2D* TextureB)
{
	if (TextureA == nullptr || TextureB == nullptr)
		return false;

	if (TextureA == TextureB)
		return true;

	if (TextureA->SRGB != TextureB->SRGB || TextureA->NeverStream != TextureB->NeverStream ||
		TextureA->CompressionSettings != TextureB->CompressionSettings ||
		TextureA->MipGenSettings != TextureB->MipGenSettings)
	{
		return false;
	}

#if WITH_EDITORONLY_DATA
	if (!AreTextureSourcesEqual(*TextureA, *TextureB))
	{
		return false;
	}
#endif

	return ArePlatformDataEqual(TextureA->GetPlatformData(), TextureB->GetPlatformData());
}
