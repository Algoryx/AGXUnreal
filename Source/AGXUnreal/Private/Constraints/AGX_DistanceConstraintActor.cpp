#include "Constraints/AGX_DistanceConstraintActor.h"

// AGXUnreal includes.
#include "Constraints/AGX_DistanceConstraintComponent.h"

AAGX_DistanceConstraintActor::AAGX_DistanceConstraintActor()
{
	SetConstraintComponent(
		CreateDefaultSubobject<UAGX_DistanceConstraintComponent>(TEXT("Distance")));
}

AAGX_DistanceConstraintActor::~AAGX_DistanceConstraintActor()
{
}