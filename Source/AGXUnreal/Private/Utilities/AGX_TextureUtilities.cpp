#include "Utilities/AGX_TextureUtilities.h"

// AGXUnreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/TextureRenderTarget2D.h"
#include "RHI.h"
#include "RHICommandList.h"

bool AGX_TextureUtilities::UpdateRenderTextureRegions(
	UTextureRenderTarget2D& RenderTarget, uint32 NumRegions, FUpdateTextureRegion2D* Regions,
	uint32 SourcePitch, uint32 SourceBitsPerPixel, uint8* SourceData, bool bFreeData)
{
	FTextureRenderTarget2DResource* Resource =
		(FTextureRenderTarget2DResource*) (RenderTarget.Resource);
	if (Resource == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("TextureRenderTarget doesn't have a resource."));
		return false;
	}

	auto WriteTexture = [Resource, NumRegions, Regions, SourcePitch, SourceBitsPerPixel, SourceData,
						 bFreeData](FRHICommandListImmediate& RHICmdList) {
		FRHITexture2D* Texture = Resource->GetTextureRHI();
		for (uint32 RegionIndex = 0; RegionIndex < NumRegions; ++RegionIndex)
		{
			FUpdateTextureRegion2D& Region = Regions[RegionIndex];
			uint8* Bits = SourceData + Region.SrcY * SourcePitch + Region.SrcX * SourceBitsPerPixel;
			RHIUpdateTexture2D(Texture, /*MipIndex*/ 0, Region, SourcePitch, Bits);
		}
		if (bFreeData)
		{
			FMemory::Free(Regions);
			FMemory::Free(SourceData);
		}
	};

	ENQUEUE_RENDER_COMMAND(UpdateRenderTextureRegionsData)(std::move(WriteTexture));

	return true;
}