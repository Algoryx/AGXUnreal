// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LidarOutputPosition.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/AGX_LidarSensorComponent.h"

// Unreal Engine includes.
#include "Misc/EngineVersionComparison.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"


void FAGX_LidarOutputPosition::Render(
	const TArray<FAGX_LidarOutputPositionData>& InData, UAGX_LidarSensorComponent* Lidar,
	float LifeTime, float ZeroDistanceSize)
{
	if (Lidar == nullptr)
		return;

	if (!Lidar->bEnableRendering)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPosition::Render called but the given Lidar does "
				 "not have bEnableRendering set to true. Doing nothing."));
		return;
	}

	UNiagaraComponent* Niagara = Lidar->GetSpawnedNiagaraSystemComponent();
	if (Niagara == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_LidarOutputPosition::Render called but the given Lidar does "
				 "not have a spawned Niagara Component. Doing nothing."));
		return;
	}

	// Lidar in AGX Dynamics has output data left in the buffers even after being disabled.
	// Therefore, we check for this explicitly to not render old data after Lidar->setEnable(false).
	const TArray<FAGX_LidarOutputPositionData> NoData;
	const TArray<FAGX_LidarOutputPositionData>& DataToRender =
		Lidar->GetEnabled() ? InData : NoData;

#if UE_VERSION_OLDER_THAN(5, 5, 0)
	RenderPositions.SetNum(0, false);
#else
	RenderPositions.SetNum(0, EAllowShrinking::No);
#endif

	const FTransform& Transform = Lidar->GetComponentTransform();
	for (const auto& Datum : DataToRender)
	{
		RenderPositions.Add(Transform.TransformPositionNoScale(FVector(Datum.Position)));
	}

#if UE_VERSION_OLDER_THAN(5, 3, 0)
	Niagara->SetNiagaraVariableInt("User.NumPoints", DataToRender.Num());
	Niagara->SetNiagaraVariableFloat("User.Lifetime", LifeTime);
	Niagara->SetNiagaraVariableFloat("User.ZeroDistanceSize", ZeroDistanceSize);
#else
	Niagara->SetVariableInt(FName("User.NumPoints"), DataToRender.Num());
	Niagara->SetVariableFloat(FName("User.Lifetime"), LifeTime);
	Niagara->SetVariableFloat(FName("User.ZeroDistanceSize"), ZeroDistanceSize);
#endif

	UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayPosition(
		Niagara, "Positions", RenderPositions);
}

bool FAGX_LidarOutputPosition::HasNative() const
{
	return NativeBarrier.HasNative();
}

FLidarOutputBarrier* FAGX_LidarOutputPosition::GetOrCreateNative()
{
	if (!HasNative())
	{
		NativeBarrier.AllocateNative();
	}

	return GetNative();
}

const FLidarOutputBarrier* FAGX_LidarOutputPosition::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FLidarOutputBarrier* FAGX_LidarOutputPosition::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

FAGX_LidarOutputPosition& FAGX_LidarOutputPosition::operator=(const FAGX_LidarOutputPosition& Other)
{
	// This operator is needed to be able to declare e.g. TArray's containing this struct.
	// It is assumed not to be called during play, therefore no members are copied here.
	return *this;
}

bool FAGX_LidarOutputPosition::operator==(const FAGX_LidarOutputPosition& Other) const
{
	return FAGX_LidarOutputBase::operator==(Other);
}

void FAGX_LidarOutputPosition::GetData(TArray<FAGX_LidarOutputPositionData>& OutData)
{
	if (HasNative())
		NativeBarrier.GetData(OutData);
}
