// Copyright 2026, Algoryx Simulation AB.


#include "Model/AGX_DynamicWaterComponent.h"

#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Model/AGX_WindAndWaterControllerSubsystemBase.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Utilities/AGX_StringUtilities.h"

namespace
{
	class CustomDynamicWaterBarrier : public FDynamicWaterBarrier
	{
	public:
		CustomDynamicWaterBarrier(const UAGX_DynamicWaterComponent& Owner) : Owner_(Owner)
		{
		}

		virtual double FindHeightFromSurface(const FVector& WorldPoint, const FVector& UpVector, const double& Time) const override
		{
			return Owner_.FindHeightFromSurface(WorldPoint, UpVector, Time);
		}

		virtual double GetDensity() const override
		{
			return Owner_.GetDensity();
		}

		virtual FVector GetVelocity(const FVector& WorldPoint) const override
		{
			return Owner_.GetVelocity(WorldPoint);
		}

		const UAGX_DynamicWaterComponent& Owner_;
	};
}

// Sets default values for this component's properties
UAGX_DynamicWaterComponent::UAGX_DynamicWaterComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

FDynamicWaterBarrier* UAGX_DynamicWaterComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for water wrapper '%s' in '%s' was made "
					"but we are in the middle of a Blueprint Reconstruction and the requested "
					"instance has not yet been restored. The instance cannot be returned, which "
					"may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		InitializeNative();
	}
	check(HasNative()); /// \todo Consider better error handling than 'check'.
	return NativeBarrier.Get();
}

FDynamicWaterBarrier* UAGX_DynamicWaterComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeBarrier.Get();
}

const FDynamicWaterBarrier* UAGX_DynamicWaterComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeBarrier.Get();
}

bool UAGX_DynamicWaterComponent::HasNative() const
{
	return NativeBarrier == nullptr ? false : NativeBarrier->HasNative();
}

uint64 UAGX_DynamicWaterComponent::GetNativeAddress() const
{
	check(!HasNative());
	return static_cast<uint64>(NativeBarrier->GetNativeAddress());
}

void UAGX_DynamicWaterComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier->SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

// Called when the game starts
void UAGX_DynamicWaterComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GIsReconstructingBlueprintInstances) return;

	GetOrCreateNative();
	const UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shape '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				"returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	UAGX_ShapeComponent* ParentShape = Cast<UAGX_ShapeComponent>(GetAttachParent());
	if (ParentShape == nullptr) return;

	if (const auto WindAndWaterControllerSubsystem = UAGX_WindAndWaterControllerSubsystemBase::GetFrom(this))
	{
		WindAndWaterControllerSubsystem->SetWaterWrapper(ParentShape, this);
		WindAndWaterControllerSubsystem->SetWaterFlowGenerator(ParentShape, this);
	}
}

void UAGX_DynamicWaterComponent::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances) return;
	if (HasNative())
	{
		NativeBarrier->ReleaseNative();
	}
}

double UAGX_DynamicWaterComponent::FindHeightFromSurface(const FVector& WorldPoint, const FVector& UpVector, const double& Time) const
{
	return WorldPoint.Z;
}

double UAGX_DynamicWaterComponent::GetDensity() const
{
	return 1000.0;
}

FVector UAGX_DynamicWaterComponent::GetVelocity(const FVector& WorldPoint) const
{
	return FVector::Zero();
}

void UAGX_DynamicWaterComponent::InitializeNative()
{
	check(!HasNative());

	NativeBarrier = MakeUnique<CustomDynamicWaterBarrier>(*this);
	NativeBarrier->AllocateNative();
}
