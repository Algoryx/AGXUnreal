#include "AGX_RigidBodyActor.h"

// AGXUnreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_MotionControl.h"

// Unreal Engine includes.
#include "Engine/EngineTypes.h"

AAGX_RigidBodyActor::AAGX_RigidBodyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RigidBodyComponent =
		CreateDefaultSubobject<UAGX_RigidBodyComponent>(TEXT("RigidBodyComponent"));
	RigidBodyComponent->Mobility = RigidBodyComponent->MotionControl == MC_DYNAMICS
									   ? EComponentMobility::Movable
									   : EComponentMobility::Static;
	RootComponent = RigidBodyComponent;
}

void AAGX_RigidBodyActor::BeginPlay()
{
	Super::BeginPlay();
}