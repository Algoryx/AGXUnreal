#include "DepthTextureReader.h"


UDepthTextureReader::UDepthTextureReader()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDepthTextureReader::Execute()
{
	// Todo: read texture pixel data here and extract distances.
}
