// Copyright 2026, Algoryx Simulation AB.


#include "Model/AGX_WindAndWaterControllerSubsystemBase.h"

#include "AGX_Simulation.h"
#include "Model/AGX_DynamicWaterComponent.h"
#include "Model/AGX_WindAndWaterAwareShapeMaterial.h"
#include "Model/WindAndWaterControllerBarrier.h"
#include "Model/WindAndWaterParametersEnums.h"
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_HeightFieldShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Wire/AGX_WireComponent.h"

bool UAGX_WindAndWaterControllerSubsystemBase::HasNative() const
{
	return NativeWindAndWaterControllerBarrier.HasNative();
}

uint64 UAGX_WindAndWaterControllerSubsystemBase::GetNativeAddress() const
{
	return static_cast<uint64>(NativeWindAndWaterControllerBarrier.GetNativeAddress());
}

void UAGX_WindAndWaterControllerSubsystemBase::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeWindAndWaterControllerBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

UAGX_WindAndWaterControllerSubsystemBase* UAGX_WindAndWaterControllerSubsystemBase::GetFrom(const UActorComponent* Component)
{
	return Component ? GetFrom(Component->GetOwner()) : nullptr;
}

UAGX_WindAndWaterControllerSubsystemBase* UAGX_WindAndWaterControllerSubsystemBase::GetFrom(const AActor* Actor)
{
	return Actor ? Actor->GetGameInstance()->GetSubsystem<UAGX_WindAndWaterControllerSubsystemBase>() : nullptr;
}

void UAGX_WindAndWaterControllerSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency(UAGX_Simulation::StaticClass());
	if (!HasNative())
	{
		NativeWindAndWaterControllerBarrier.AllocateNative();
	}
	
	if (UAGX_Simulation* Sim = GetGameInstance()->GetSubsystem<UAGX_Simulation>())
	{
		if (Sim->HasNative())
		{
			Sim->GetNative()->Add(NativeWindAndWaterControllerBarrier);
		}
	}
}

void UAGX_WindAndWaterControllerSubsystemBase::Deinitialize()
{
	Super::Deinitialize();
	
	NativeWindAndWaterControllerBarrier.ReleaseNative();
}

FWindAndWaterControllerBarrier* UAGX_WindAndWaterControllerSubsystemBase::GetNative()
{
	return HasNative() ? &NativeWindAndWaterControllerBarrier : nullptr;
}

bool UAGX_WindAndWaterControllerSubsystemBase::AddWater(UAGX_ShapeComponent* Shape)
{
	return HasNative() ? NativeWindAndWaterControllerBarrier.AddWater(Shape->GetOrCreateNative()) : false;
}

bool UAGX_WindAndWaterControllerSubsystemBase::SetEnableAerodynamics(UAGX_ShapeComponent* Shape, bool bEnabled)
{
	return HasNative() ? NativeWindAndWaterControllerBarrier.SetEnableAerodynamics(Shape->GetOrCreateNative(), bEnabled) : false;
}

bool UAGX_WindAndWaterControllerSubsystemBase::SetEnableAerodynamics(UAGX_WireComponent* Wire, bool bEnabled)
{
	return HasNative() ? NativeWindAndWaterControllerBarrier.SetEnableAerodynamics(Wire->GetOrCreateNative(), bEnabled) : false;
}

void UAGX_WindAndWaterControllerSubsystemBase::SetHydrodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterParametersCoefficient Coefficient, double Value)
{
	if (HasNative())
	{
		NativeWindAndWaterControllerBarrier.GetOrCreateHydrodynamicsParameters(Shape->GetOrCreateNative()).SetCoefficient(Coefficient, Value);
	}
}

void UAGX_WindAndWaterControllerSubsystemBase::SetHydrodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterShapeTessellation ShapeTessellation)
{
	if (HasNative())
	{
		NativeWindAndWaterControllerBarrier.GetOrCreateHydrodynamicsParameters(Shape->GetOrCreateNative()).SetShapeTessellation(ShapeTessellation);
	}
}

void UAGX_WindAndWaterControllerSubsystemBase::SetAerodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterParametersCoefficient Coefficient, double Value)
{
	if (HasNative())
	{
		NativeWindAndWaterControllerBarrier.GetOrCreateAerodynamicsParameters(Shape->GetOrCreateNative()).SetCoefficient(Coefficient, Value);
	}
}

void UAGX_WindAndWaterControllerSubsystemBase::SetAerodynamicParameters(UAGX_ShapeComponent* Shape, EWindAndWaterShapeTessellation ShapeTessellation)
{
	if (HasNative())
	{
		NativeWindAndWaterControllerBarrier.GetOrCreateAerodynamicsParameters(Shape->GetOrCreateNative()).SetShapeTessellation(ShapeTessellation);
	}
}

bool UAGX_WindAndWaterControllerSubsystemBase::UpdateNativeWindAndWaterParameters(UAGX_ShapeComponent* Shape)
{
	if (!Shape) return false;

	const UAGX_WindAndWaterAwareShapeMaterial* ShapeMaterial = Cast<UAGX_WindAndWaterAwareShapeMaterial>(Shape->ShapeMaterial);
	if (!ShapeMaterial) return false;

	if (ShapeMaterial->bIsWaterGeometry)
	{
		if (Shape->IsA(UAGX_BoxShapeComponent::StaticClass()) ||
			Shape->IsA(UAGX_HeightFieldShapeComponent::StaticClass()) ||
			Shape->IsA(UAGX_SphereShapeComponent::StaticClass()))
		{
			Shape->SetCanCollide(false);
			return AddWater(Shape);
		}
	}

	SetHydrodynamicParameters(Shape, EWindAndWaterParametersCoefficient::PRESSURE_DRAG, ShapeMaterial->HydroParameters.PressureDrag_);
	SetHydrodynamicParameters(Shape, EWindAndWaterParametersCoefficient::VISCOUS_DRAG, ShapeMaterial->HydroParameters.ViscousDrag_);
	SetHydrodynamicParameters(Shape, EWindAndWaterParametersCoefficient::LIFT, ShapeMaterial->HydroParameters.Lift_);
	SetHydrodynamicParameters(Shape, EWindAndWaterParametersCoefficient::BUOYANCY, ShapeMaterial->HydroParameters.Buoyancy_);
	SetHydrodynamicParameters(Shape, ShapeMaterial->HydroParameters.ShapeTessellation_);

	SetAerodynamicParameters(Shape, EWindAndWaterParametersCoefficient::PRESSURE_DRAG, ShapeMaterial->AeroParameters.PressureDrag_);
	SetAerodynamicParameters(Shape, EWindAndWaterParametersCoefficient::VISCOUS_DRAG, ShapeMaterial->AeroParameters.ViscousDrag_);
	SetAerodynamicParameters(Shape, EWindAndWaterParametersCoefficient::LIFT, ShapeMaterial->AeroParameters.Lift_);
	SetAerodynamicParameters(Shape, ShapeMaterial->AeroParameters.ShapeTessellation_);
	return true;
}

void UAGX_WindAndWaterControllerSubsystemBase::SetWaterWrapper(UAGX_ShapeComponent* Shape, UAGX_DynamicWaterComponent* WaterWrapper)
{
	if (NativeWindAndWaterControllerBarrier.HasNative())
	{
		NativeWindAndWaterControllerBarrier.SetWaterWrapper(Shape->GetOrCreateNative(), WaterWrapper->GetOrCreateNative());
	}
}

void UAGX_WindAndWaterControllerSubsystemBase::SetWaterFlowGenerator(UAGX_ShapeComponent* ParentShape, UAGX_DynamicWaterComponent* DynamicWater)
{
	if (NativeWindAndWaterControllerBarrier.HasNative())
	{
		NativeWindAndWaterControllerBarrier.SetWaterFlowGenerator(ParentShape->GetOrCreateNative(), DynamicWater->GetOrCreateNative());
	}
}
