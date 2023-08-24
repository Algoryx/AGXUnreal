#include "DepthTextureReader.h"


UDepthTextureReader::UDepthTextureReader()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UDepthTextureReader::Execute(const FVector& RefPos, const FQuat& RefRot)
{
	// Todo: read texture pixel data here and extract distances.
}
