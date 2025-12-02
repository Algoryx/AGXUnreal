// Copyright 2025, Algoryx Simulation AB.

#include "Tires/AGX_TwoBodyTireActor.h"

// AGX Dynamics for Unreal includes.
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "Engine/EngineTypes.h"

AAGX_TwoBodyTireActor::AAGX_TwoBodyTireActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent =
		CreateDefaultSubobject<USceneComponent>(USceneComponent::GetDefaultSceneRootVariableName());

	TireRigidBodyComponent = CreateDefaultSubobject<UAGX_RigidBodyComponent>(TEXT("TireRigidBody"));
	TireRigidBodyComponent->SetupAttachment(RootComponent);

	HubRigidBodyComponent = CreateDefaultSubobject<UAGX_RigidBodyComponent>(TEXT("HubRigidBody"));
	HubRigidBodyComponent->SetupAttachment(RootComponent);

	TwoBodyTireComponent = CreateDefaultSubobject<UAGX_TwoBodyTireComponent>(TEXT("TwoBodyTire"));

	TwoBodyTireComponent->TireRigidBody.OwningActor = this;
	TwoBodyTireComponent->TireRigidBody.Name = TireRigidBodyComponent->GetFName();

	TwoBodyTireComponent->HubRigidBody.OwningActor = this;
	TwoBodyTireComponent->HubRigidBody.Name = HubRigidBodyComponent->GetFName();
}
