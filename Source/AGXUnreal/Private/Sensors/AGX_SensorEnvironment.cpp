// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_SensorEnvironment.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Shapes/AGX_SimpleMeshComponent.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Sensors/AGX_IMUSensorComponent.h"
#include "Sensors/AGX_LidarAmbientMaterial.h"
#include "Sensors/AGX_LidarLambertianOpaqueMaterial.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_SensorEnvironmentSpriteComponent.h"
#include "Sensors/AGX_SurfaceMaterialAssetUserData.h"
#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Wire/AGX_WireComponent.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "Landscape.h"

#include <algorithm>

namespace AGX_SensorEnvironment_helpers
{
	bool GetVerticesIndices(
		UStaticMeshComponent* Mesh, TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices,
		int32 Lod)
	{
		if (Mesh == nullptr)
			return false;

		const UStaticMesh* StaticMesh = Mesh->GetStaticMesh();
		if (StaticMesh == nullptr)
			return false;

		// Default LOD is LodMax, if not set explicitly.
		const uint32 LodMax = StaticMesh->GetNumLODs() - 1;
		const uint32 LodIndex = Lod < 0 ? LodMax : std::min(static_cast<uint32>(Lod), LodMax);
		if (!StaticMesh->HasValidRenderData(/*bCheckLODForVerts*/ true, LodIndex))
			return false;

		FAGX_MeshWithTransform MeshWTransform(StaticMesh, Mesh->GetComponentTransform());
		return AGX_MeshUtilities::GetStaticMeshCollisionData(
			MeshWTransform, Mesh->GetComponentTransform(), OutVertices, OutIndices, &LodIndex);
	}

	bool GetVerticesIndices(
		UAGX_SimpleMeshComponent* Mesh, TArray<FVector>& OutVertices,
		TArray<FTriIndices>& OutIndices)
	{
		if (Mesh == nullptr)
			return false;

		const FAGX_SimpleMeshData* MeshData = Mesh->GetMeshData();
		if (MeshData == nullptr)
			return false;

		OutVertices.Reserve(MeshData->Vertices.Num());
		for (const FVector3f& V : MeshData->Vertices)
		{
			OutVertices.Add(
				{static_cast<double>(V.X), static_cast<double>(V.Y), static_cast<double>(V.Z)});
		}

		OutIndices.Reserve(MeshData->Indices.Num() / 3);
		for (int32 I = 2; I < MeshData->Indices.Num(); I += 3)
		{
			FTriIndices TriInd;
			TriInd.v0 = static_cast<int32>(MeshData->Indices[I - 2]);
			TriInd.v1 = static_cast<int32>(MeshData->Indices[I - 1]);
			TriInd.v2 = static_cast<int32>(MeshData->Indices[I - 0]);
			OutIndices.Add(TriInd);
		}

		AGX_CHECK(OutIndices.Num() == MeshData->Indices.Num() / 3);
		return true;
	}

	void UpdateCollisionSphere(const UAGX_LidarSensorComponent* Lidar, USphereComponent* Sphere)
	{
		if (Lidar == nullptr || Sphere == nullptr)
			return;

		// Chosen arbitrarily, too large will cause Unreal warnings/errors.
		static constexpr double MaxRadius = 1.0e8;
		if (Lidar->Range.Max > MaxRadius)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Lidar %s has a Max Range of %f, but the maximum supported Range is %f. Using "
					 "%f."),
				*Lidar->GetName(), Lidar->Range.Max.GetValue(), MaxRadius, MaxRadius);
		}

		const float Radius = std::min(Lidar->Range.Max.GetValue(), MaxRadius);
		if (!FMath::IsNearlyEqual(Sphere->GetUnscaledSphereRadius(), Radius))
		{
			Sphere->SetSphereRadius(Radius, /*bUpdateOverlaps*/ false);
		}

		const FVector CurrentLocation = Sphere->GetComponentLocation();
		const FVector TargetLocation = Lidar->GetComponentLocation();

		Sphere->SetWorldLocation(TargetLocation); // Updates overlaps if moved.

		// Update overlaps when location hasn't changed.
		if (CurrentLocation.Equals(TargetLocation, SMALL_NUMBER))
			Sphere->UpdateOverlaps();
	}

	template <typename InMapType>
	void UpdateTrackedMeshes(InMapType& MeshToInstance)
	{
		// Update tracked static meshes and remove any invalid ones.
		for (auto It = MeshToInstance.CreateIterator(); It; ++It)
		{
			if (!IsValid(It->Key.Get()))
			{
				It.RemoveCurrent();
				continue;
			}

			const FTransform& CompTransform = It->Key->GetComponentTransform();
			if (CompTransform.Equals(It->Value.InstanceData.Transform))
				continue;

			It->Value.InstanceData.SetTransform(CompTransform);
		}
	}

	FRtLambertianOpaqueMaterialBarrier* GetLambertianOpaqueMaterialBarrierFrom(
		USceneComponent& Component)
	{
		auto Data =
			Component.GetAssetUserDataOfClass(UAGX_SurfaceMaterialAssetUserData::StaticClass());
		if (Data == nullptr)
			return nullptr;

		auto SurfaceMaterialData = Cast<UAGX_SurfaceMaterialAssetUserData>(Data);
		if (SurfaceMaterialData == nullptr)
			return nullptr;

		auto LambertianOpaqueMaterial =
			Cast<UAGX_LidarLambertianOpaqueMaterial>(SurfaceMaterialData->LidarSurfaceMaterial);
		if (LambertianOpaqueMaterial == nullptr)
			return nullptr;

		return LambertianOpaqueMaterial->GetNative();
	}

	FRtLambertianOpaqueMaterialBarrier* GetLambertianOpaqueMaterialBarrierFrom(
		AAGX_Terrain& Terrain)
	{
		auto LambertianOpaqueMaterial =
			Cast<UAGX_LidarLambertianOpaqueMaterial>(Terrain.LidarSurfaceMaterial);
		if (LambertianOpaqueMaterial == nullptr)
			return nullptr;

		return LambertianOpaqueMaterial->GetNative();
	}

	TOptional<FAGX_RtShapeInstanceData> CreateShapeInstanceData(
		const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices, USceneComponent& Mesh,
		FSensorEnvironmentBarrier& SEBarrier)
	{
		FAGX_RtShapeInstanceData ShapeInstance;
		if (!ShapeInstance.Shape.AllocateNative(Vertices, Indices))
			return {};

		ShapeInstance.InstanceData.Instance.AllocateNative(ShapeInstance.Shape, SEBarrier);
		ShapeInstance.InstanceData.SetTransform(Mesh.GetComponentTransform());
		ShapeInstance.InstanceData.Instance.SetLidarSurfaceMaterialOrDefault(
			GetLambertianOpaqueMaterialBarrierFrom(Mesh));
		return ShapeInstance;
	}

	TArray<ALandscape*> GetLandscapeActors(UWorld* World)
	{
		TArray<ALandscape*> Landscapes;
		if (World == nullptr)
			return Landscapes;

		for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
		{
			if (ALandscape* Landscape = Cast<ALandscape>(*ActorIt))
				Landscapes.Add(Landscape);
		}

		return Landscapes;
	}
}

AAGX_SensorEnvironment::AAGX_SensorEnvironment()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<UAGX_SensorEnvironmentSpriteComponent>(
		USceneComponent::GetDefaultSceneRootVariableName());
}

void AAGX_SensorEnvironment::SetMagneticField(const FVector& Field)
{
	MagneticField = Field;
	if (HasNative())
		NativeBarrier.SetMagneticField(Field);
}

FVector AAGX_SensorEnvironment::GetMagneticField() const
{
	if (HasNative())
		return NativeBarrier.GetMagneticField();

	return MagneticField;
}

bool AAGX_SensorEnvironment::AddLidar(UAGX_LidarSensorComponent* Lidar)
{
	if (Lidar == nullptr)
		return false;

	for (const auto& L : LidarSensors)
	{
		if (L.GetLidarComponent() == Lidar)
			return false;
	}

	FAGX_LidarSensorReference LidarRef;
	LidarRef.SetComponent(Lidar);
	LidarSensors.Add(LidarRef);

	if (HasNative())
		RegisterLidar(LidarRef);

	return true;
}

bool AAGX_SensorEnvironment::AddIMU(UAGX_IMUSensorComponent* IMU)
{
	if (IMU == nullptr)
		return false;

	for (const auto& L : IMUSensors)
	{
		if (L.GetIMUComponent() == IMU)
			return false;
	}

	FAGX_IMUSensorReference IMURef;
	IMURef.SetComponent(IMU);
	IMUSensors.Add(IMURef);

	if (HasNative())
		RegisterIMU(IMURef);

	return true;
}

bool AAGX_SensorEnvironment::AddMesh(UStaticMeshComponent* Mesh, int32 InLod)
{
	if (!HasNative())
	{
		InitializeNative();
		if (!HasNative())
			return false;
	}

	TArray<FVector> OutVerts;
	TArray<FTriIndices> OutInds;
	const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;

	if (!AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVerts, OutInds, Lod))
		return false;

	if (!AddMesh(Mesh, OutVerts, OutInds))
		return false;

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment '%s' added Static Mesh Component '%s' in '%s'."),
			*GetName(), *Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return true;
}

bool AAGX_SensorEnvironment::AddAGXMesh(UAGX_SimpleMeshComponent* Mesh)
{
	if (!HasNative())
	{
		InitializeNative();
		if (!HasNative())
			return false;
	}

	TArray<FVector> OutVerts;
	TArray<FTriIndices> OutInds;
	if (!AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVerts, OutInds))
		return false;

	if (!AddMesh(Mesh, OutVerts, OutInds))
		return false;

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment '%s' added AGX Shape '%s' in '%s'."), *GetName(),
			*Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMesh(UInstancedStaticMeshComponent* Mesh, int32 InLod)
{
	if (Mesh == nullptr)
		return false;

	if (!HasNative())
	{
		InitializeNative();
		if (!HasNative())
			return false;
	}

	if (!TrackedInstancedMeshes.Contains(Mesh))
	{
		TArray<FVector> OutVertices;
		TArray<FTriIndices> OutIndices;
		const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;
		if (!AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVertices, OutIndices, Lod))
			return false;

		if (!AddInstancedMesh(Mesh, OutVertices, OutIndices))
			return false;
	}

	const int32 InstanceCnt = Mesh->GetInstanceCount();
	bool AllOk = true;
	for (int32 i = 0; i < InstanceCnt; i++)
	{
		AllOk &= AddInstancedMeshInstance_Internal(Mesh, i);
	}

	if (!AllOk)
		return false;

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment '%s' added Instaced Static Mesh '%s' in '%s'."),
			*GetName(), *Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMeshInstance(
	UInstancedStaticMeshComponent* Mesh, int32 Index, int32 InLod)
{
	if (Mesh == nullptr || !Mesh->IsValidInstance(Index))
		return false;

	if (!HasNative())
	{
		InitializeNative();
		if (!HasNative())
			return false;
	}

	if (!TrackedInstancedMeshes.Contains(Mesh))
	{
		TArray<FVector> OutVertices;
		TArray<FTriIndices> OutIndices;
		const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;
		if (!AGX_SensorEnvironment_helpers::GetVerticesIndices(Mesh, OutVertices, OutIndices, Lod))
			return false;

		if (!AddInstancedMesh(Mesh, OutVertices, OutIndices))
			return false;
	}

	const bool Res = AddInstancedMeshInstance_Internal(Mesh, Index);
	if (Res && DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment '%s' added AGX Shape '%s' in '%s'."), *GetName(),
			*Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return Res;
}

bool AAGX_SensorEnvironment::AddMesh(
	UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices)
{
	using namespace AGX_SensorEnvironment_helpers;
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedMeshes.Contains(Mesh))
		return false;

	auto ShapeInstance = CreateShapeInstanceData(Vertices, Indices, *Mesh, NativeBarrier);
	if (!ShapeInstance.IsSet())
		return false;

	TrackedMeshes.Add(Mesh, std::move(ShapeInstance.GetValue()));
	return true;
}

bool AAGX_SensorEnvironment::AddMesh(
	UAGX_SimpleMeshComponent* Mesh, const TArray<FVector>& Vertices,
	const TArray<FTriIndices>& Indices)
{
	using namespace AGX_SensorEnvironment_helpers;
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedAGXMeshes.Contains(Mesh))
		return false;

	auto ShapeInstance = CreateShapeInstanceData(Vertices, Indices, *Mesh, NativeBarrier);
	if (!ShapeInstance.IsSet())
		return false;

	TrackedAGXMeshes.Add(Mesh, std::move(ShapeInstance.GetValue()));
	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMesh(
	UInstancedStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
	const TArray<FTriIndices>& Indices)
{
	AGX_CHECK(HasNative());
	if (Mesh == nullptr || Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedInstancedMeshes.Contains(Mesh))
		return false;

	FAGX_RtInstancedShapeInstanceData InstancedShapeInstance;
	if (!InstancedShapeInstance.Shape.AllocateNative(Vertices, Indices))
		return false;

	TrackedInstancedMeshes.Add(Mesh, std::move(InstancedShapeInstance));
	return true;
}

bool AAGX_SensorEnvironment::AddInstancedMeshInstance_Internal(
	UInstancedStaticMeshComponent* Mesh, int32 Index)
{
	using namespace AGX_SensorEnvironment_helpers;
	AGX_CHECK(HasNative());
	AGX_CHECK(Mesh != nullptr);
	AGX_CHECK(Mesh->IsValidInstance(Index));

	FAGX_RtInstancedShapeInstanceData* InstancedShapeInstance = TrackedInstancedMeshes.Find(Mesh);

	// This function should only be called for known Instanced Static Mesh Components.
	AGX_CHECK(InstancedShapeInstance != nullptr);
	if (InstancedShapeInstance == nullptr)
		return false;

	if (InstancedShapeInstance->InstancesData.Contains(Index))
		return false; // We already track this instance.

	FAGX_RtInstanceData& InstanceData =
		InstancedShapeInstance->InstancesData.Add(Index, FAGX_RtInstanceData());
	InstanceData.Instance.AllocateNative(InstancedShapeInstance->Shape, NativeBarrier);
	AGX_CHECK(InstanceData.Instance.HasNative());
	FTransform InstanceTrans;
	Mesh->GetInstanceTransform(Index, InstanceTrans, true);
	InstanceData.SetTransform(InstanceTrans);
	InstanceData.Instance.SetLidarSurfaceMaterialOrDefault(
		GetLambertianOpaqueMaterialBarrierFrom(*Mesh));
	return true;
}

bool AAGX_SensorEnvironment::AddTerrain(AAGX_Terrain* Terrain)
{
	using namespace AGX_SensorEnvironment_helpers;
	if (Terrain == nullptr)
		return false;

	if (!HasNative())
	{
		InitializeNative();
		if (!HasNative())
			return false;
	}

	if (Terrain->bEnableTerrainPaging)
	{
		FTerrainPagerBarrier* PagerBarrier = Terrain->GetOrCreateNativeTerrainPager();
		if (PagerBarrier == nullptr)
			return false;

		if (!NativeBarrier.Add(*PagerBarrier))
			return false;

		NativeBarrier.SetLidarSurfaceMaterialOrDefault(
			*PagerBarrier, GetLambertianOpaqueMaterialBarrierFrom(*Terrain));
	}
	else
	{
		FTerrainBarrier* TerrainBarrier = Terrain->GetOrCreateNative();
		if (TerrainBarrier == nullptr)
			return false;

		if (!NativeBarrier.Add(*TerrainBarrier))
			return false;

		NativeBarrier.SetLidarSurfaceMaterialOrDefault(
			*TerrainBarrier, GetLambertianOpaqueMaterialBarrierFrom(*Terrain));
	}

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment '%s' added Terrain '%s'."), *GetName(),
			*Terrain->GetName());
	}

	return true;
}

bool AAGX_SensorEnvironment::AddMovableTerrain(UAGX_MovableTerrainComponent* Terrain)
{
	using namespace AGX_SensorEnvironment_helpers;
	if (Terrain == nullptr)
		return false;

	if (!HasNative())
	{
		InitializeNative();
		if (!HasNative())
			return false;
	}

	FTerrainBarrier* TerrainBarrier = Terrain->GetOrCreateNative();
	if (TerrainBarrier == nullptr)
		return false;

	if (!NativeBarrier.Add(*TerrainBarrier))
		return false;

	NativeBarrier.SetLidarSurfaceMaterialOrDefault(
		*TerrainBarrier, GetLambertianOpaqueMaterialBarrierFrom(*Terrain));

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment '%s' added Movable Terrain '%s'."), *GetName(),
			*Terrain->GetName());
	}

	return true;
}

bool AAGX_SensorEnvironment::AddWire(UAGX_WireComponent* Wire)
{
	using namespace AGX_SensorEnvironment_helpers;
	if (Wire == nullptr)
		return false;

	if (!HasNative())
	{
		InitializeNative();
		if (!HasNative())
			return false;
	}

	FWireBarrier* Barrier = Wire->GetOrCreateNative();
	if (Barrier == nullptr)
		return false;

	if (!NativeBarrier.Add(*Barrier))
		return false;

	NativeBarrier.SetLidarSurfaceMaterialOrDefault(
		*Barrier, GetLambertianOpaqueMaterialBarrierFrom(*Wire));

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment '%s' added Wire '%s'."), *GetName(),
			*Wire->GetName());
	}

	return true;
}

bool AAGX_SensorEnvironment::RemoveLidar(UAGX_LidarSensorComponent* Lidar)
{
	bool DidRemove = false;

	if (Lidar == nullptr)
		return DidRemove;

	for (int32 i = LidarSensors.Num() - 1; i >= 0; --i)
	{
		if (LidarSensors[i].GetLidarComponent() == Lidar)
		{
			LidarSensors.RemoveAt(i);
			DidRemove = true;
		}
	}

	if (!HasNative() || !Lidar->HasNative())
		return DidRemove;

	DidRemove |= NativeBarrier.Remove(*Lidar->GetNativeAsLidar());

	for (auto It = TrackedLidars.CreateIterator(); It; ++It)
	{
		if (It.Key().GetLidarComponent() == Lidar)
		{
			It.RemoveCurrent();
			DidRemove = true;
			break;
		}
	}

	return DidRemove;
}

bool AAGX_SensorEnvironment::RemoveIMU(UAGX_IMUSensorComponent* IMU)
{
	bool DidRemove = false;

	if (IMU == nullptr)
		return DidRemove;

	for (int32 i = IMUSensors.Num() - 1; i >= 0; --i)
	{
		if (IMUSensors[i].GetIMUComponent() == IMU)
		{
			IMUSensors.RemoveAt(i);
			DidRemove = true;
		}
	}

	if (!HasNative() || !IMU->HasNative())
		return DidRemove;

	DidRemove |= NativeBarrier.Remove(*IMU->GetNativeAsIMU());
	for (auto It = TrackedIMUs.CreateIterator(); It; ++It)
	{
		if (It->GetIMUComponent() == IMU)
		{
			It.RemoveCurrent();
			DidRemove = true;
			break;
		}
	}

	return DidRemove;
}

bool AAGX_SensorEnvironment::RemoveMesh(UStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr)
		return false;

	return TrackedMeshes.Remove(Mesh) > 0;
}

bool AAGX_SensorEnvironment::RemoveInstancedMesh(UInstancedStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr || !TrackedInstancedMeshes.Contains(Mesh))
		return false;

	TrackedInstancedMeshes.Remove(Mesh);
	return true;
}

bool AAGX_SensorEnvironment::RemoveInstancedMeshInstance(
	UInstancedStaticMeshComponent* Mesh, int32 Index)
{
	if (Mesh == nullptr)
		return false;

	auto InstancedMeshData = TrackedInstancedMeshes.Find(Mesh);
	if (InstancedMeshData == nullptr)
		return false;

	if (!InstancedMeshData->InstancesData.Contains(Index))
		return false;

	InstancedMeshData->InstancesData.Remove(Index);
	return true;
}

bool AAGX_SensorEnvironment::RemoveTerrain(AAGX_Terrain* Terrain)
{
	if (!HasNative() || Terrain == nullptr || !Terrain->HasNative())
		return false;

	if (Terrain->bEnableTerrainPaging)
		return NativeBarrier.Remove(*Terrain->GetNativeTerrainPager());
	else
		return NativeBarrier.Remove(*Terrain->GetOrCreateNative());
}

bool AAGX_SensorEnvironment::RemoveMovableTerrain(UAGX_MovableTerrainComponent* Terrain)
{
	if (!HasNative() || Terrain == nullptr || !Terrain->HasNative())
		return false;

	return NativeBarrier.Remove(*Terrain->GetOrCreateNative());
}

bool AAGX_SensorEnvironment::RemoveWire(UAGX_WireComponent* Wire)
{
	if (!HasNative() || Wire == nullptr || !Wire->HasNative())
		return false;

	return NativeBarrier.Remove(*Wire->GetNative());
}

bool AAGX_SensorEnvironment::HasNative() const
{
	return NativeBarrier.HasNative();
}

FSensorEnvironmentBarrier* AAGX_SensorEnvironment::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FSensorEnvironmentBarrier* AAGX_SensorEnvironment::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

#if WITH_EDITOR
bool AAGX_SensorEnvironment::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, LidarSensors),
			GET_MEMBER_NAME_CHECKED(ThisClass, IMUSensors),
			GET_MEMBER_NAME_CHECKED(ThisClass, bAutoAddObjects),
			GET_MEMBER_NAME_CHECKED(ThisClass, AmbientMaterial)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
}
#endif

void AAGX_SensorEnvironment::Tick(float DeltaSeconds)
{
	if (!HasNative())
		return;

	UpdateTrackedLidars();
	UpdateTrackedIMUs();
	UpdateTrackedMeshes();

	if (UpdateAddedInstancedMeshesTransforms)
		UpdateTrackedInstancedMeshes();

	UpdateTrackedAGXMeshes();
	TickTrackedLidars();
	TickTrackedIMUs();
}

void AAGX_SensorEnvironment::BeginPlay()
{
	Super::BeginPlay();
	const bool RaytraceRTXSupported = FSensorEnvironmentBarrier::IsRaytraceSupported();
	if (LidarSensors.Num() > 0 && !RaytraceRTXSupported)
	{
		const FString Message =
			"Lidar raytracing (RTX) not supported on this computer, the Lidar Sensors will not "
			"work. To enable Lidar raytracing (RTX) support, use an RTX "
			"Graphical Processing Unit (GPU) with updated driver.";
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail, 8.f);
	}

	if (!HasNative())
		InitializeNative();

	if (RaytraceRTXSupported)
	{
		UpdateAmbientMaterial();
		RegisterLidars();
	}

	RegisterIMUs();

	if (bAutoAddObjects)
	{
		// Add Terrains.
		for (TActorIterator<AActor> ActorIt(GetWorld()); ActorIt; ++ActorIt)
		{
			if (auto SceneRoot = ActorIt->GetRootComponent())
			{
				for (auto MovableTerrain :
					 FAGX_ObjectUtilities::GetChildrenOfType<UAGX_MovableTerrainComponent>(
						 *SceneRoot, /*recursive*/ true))
				{
					AddMovableTerrain(MovableTerrain);
				}
			}

			if (AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(*ActorIt))
			{
				AddTerrain(Terrain);
			}
		}
	}
}

void AAGX_SensorEnvironment::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	TrackedIMUs.Empty();
	TrackedLidars.Empty();
	TrackedMeshes.Empty();
	TrackedInstancedMeshes.Empty();
	TrackedAGXMeshes.Empty();

	if (AmbientMaterial != nullptr && AmbientMaterial->HasNative())
		AmbientMaterial->ReleaseNative();

	if (HasNative())
		NativeBarrier.ReleaseNative();
}

void AAGX_SensorEnvironment::InitializeNative()
{
	AGX_CHECK(!HasNative());
	if (HasNative())
		return;

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this);
	if (Sim == nullptr || !Sim->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_SensorEnvironment '%s' was unable to get a UAGX_Simulation with Native "
				 "in InitializeNative. Correct behavior of the SensorEnvironment cannot be "
				 "guaranteed."),
			*GetName());
		return;
	}

	// Make sure correct Raytrace device is set.
	if (FSensorEnvironmentBarrier::IsRaytraceSupported())
	{
		if (Sim->RaytraceDeviceIndex != FSensorEnvironmentBarrier::GetCurrentRayraceDevice())
		{
			if (!FSensorEnvironmentBarrier::SetCurrentRaytraceDevice(Sim->RaytraceDeviceIndex))
			{
				const FString Message = FString::Printf(
					TEXT("Tried to set Raytrace device id %d, but the selection failed. Please "
						 "review "
						 "the AGX Lidar category in the plugin settings."),
					Sim->RaytraceDeviceIndex);
				FAGX_NotificationUtilities::ShowNotification(
					Message, SNotificationItem::CS_Fail, 8.f);
			}
		}

		// Set positions integrated in PRE so that they are "seen" in the Lidar output in the same
		// step.
		// This is the same procedure as used in AGX Dynamics tutorials and examples using Lidar.
		Sim->SetPreIntegratePositions(true);
	}

	NativeBarrier.AllocateNative(*Sim->GetNative());
	if (!NativeBarrier.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AGX_SensorEnvironment '%s' was unable to create a Native AGX Dynamics "
				 "agxSensor::Environment. The Output Log may contain more information."),
			*GetName());
	}

	NativeBarrier.SetMagneticField(MagneticField);

	// In case the Level has no other AGX types in it.
	Sim->EnsureStepperCreated();
}

void AAGX_SensorEnvironment::RegisterLidars()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
		return;

	for (FAGX_LidarSensorReference& LidarRef : LidarSensors)
	{
		RegisterLidar(LidarRef);
	}
}

bool AAGX_SensorEnvironment::RegisterLidar(FAGX_LidarSensorReference& LidarRef)
{
	if (TrackedLidars.Contains(LidarRef))
		return false;

	UAGX_LidarSensorComponent* Lidar = LidarRef.GetLidarComponent();
	if (Lidar == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SensorEnvironment::RegisterLidar tried to get Lidar Sensor Component given Lidar "
				 "Sensor Reference '%s' but the returned Component was nullptr."),
			*LidarRef.Name.ToString());
		return false;
	}

	auto SensorBarrier = Lidar->GetOrCreateNative();
	if (SensorBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SensorEnvironment::RegisterLidar tried to get Native for Lidar Sensor Component "
				 "'%s' in '%s' but got nullptr."),
			*Lidar->GetName(), *GetLabelSafe(Lidar->GetOwner()));
		return false;
	}

	FLidarBarrier* Barrier = Lidar->GetNativeAsLidar();
	NativeBarrier.Add(*Barrier);

	// Associate each Lidar with a USphereComponent used to detect objects in the world to
	// give to AGX Dynamics during Play.
	USphereComponent* CollSph = nullptr;
	if (bAutoAddObjects)
	{
		CollSph = NewObject<USphereComponent>(this);
		CollSph->OnComponentBeginOverlap.AddDynamic(
			this, &AAGX_SensorEnvironment::OnLidarBeginOverlapComponent);
		CollSph->OnComponentEndOverlap.AddDynamic(
			this, &AAGX_SensorEnvironment::OnLidarEndOverlapComponent);

		// Ensure we don't miss overlap events by setting radius zero now. All collision
		// Collision spheres are updated in Step(), and the overlap events will be triggered
		// for any object within that radius.
		CollSph->SetSphereRadius(0.f, false);

		// = true yields bugs of mutliple begin/end overlaps. See internal issue 957.
		CollSph->bTraceComplexOnMove = false;

		// Ignore Landscapes, these will otherwise be terrible for performance.
		for (auto Landscape : AGX_SensorEnvironment_helpers::GetLandscapeActors(GetWorld()))
		{
			CollSph->IgnoreActorWhenMoving(Landscape, true);
		}

		CollSph->RegisterComponent();
	}

	TrackedLidars.Add(LidarRef, CollSph);
	return true;
}

void AAGX_SensorEnvironment::RegisterIMUs()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
		return;

	for (FAGX_IMUSensorReference& IMURef : IMUSensors)
		RegisterIMU(IMURef);
}

bool AAGX_SensorEnvironment::RegisterIMU(FAGX_IMUSensorReference& IMURef)
{
	if (TrackedIMUs.Contains(IMURef))
		return false;

	UAGX_IMUSensorComponent* IMU = IMURef.GetIMUComponent();
	if (IMU == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SensorEnvironment::RegisterIMU tried to get IMU Sensor Component given IMU "
				 "Sensor Reference '%s' but the returned Component was nullptr."),
			*IMURef.Name.ToString());
		return false;
	}

	auto SensorBarrier = IMU->GetOrCreateNative();
	if (SensorBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SensorEnvironment::RegisterIMU tried to get Native for IMU Sensor Component "
				 "'%s' in '%s' but got nullptr."),
			*IMU->GetName(), *GetLabelSafe(IMU->GetOwner()));
		return false;
	}

	FIMUBarrier* Barrier = IMU->GetNativeAsIMU(); 
	NativeBarrier.Add(*Barrier);
	TrackedIMUs.Add(IMURef);
	return true;
}

void AAGX_SensorEnvironment::UpdateTrackedLidars()
{
	// Update Collision Spheres and remove any destroyed Lidars.
	// Notice that overlap events will likely be triggered when updating the collision spheres radii
	// and transform.
	for (auto It = TrackedLidars.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key.GetLidarComponent()))
		{
			It.RemoveCurrent();
			continue;
		}

		if (bAutoAddObjects)
			AGX_SensorEnvironment_helpers::UpdateCollisionSphere(
				It->Key.GetLidarComponent(), It->Value.Get());
	}
}

void AAGX_SensorEnvironment::UpdateTrackedIMUs()
{
	for (auto It = TrackedIMUs.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->GetIMUComponent()))
		{
			It.RemoveCurrent();
		}
	}
}

void AAGX_SensorEnvironment::UpdateTrackedMeshes()
{
	AGX_SensorEnvironment_helpers::UpdateTrackedMeshes(TrackedMeshes);
}

void AAGX_SensorEnvironment::UpdateTrackedInstancedMeshes()
{
	for (auto It = TrackedInstancedMeshes.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key.Get()))
		{
			It.RemoveCurrent();
			continue;
		}

		// Instance.
		for (auto Ite = It->Value.InstancesData.CreateIterator(); Ite; ++Ite)
		{
			if (!It->Key->IsValidInstance(Ite->Key))
			{
				Ite.RemoveCurrent();
				continue;
			}

			FTransform InstanceTransform;
			It->Key->GetInstanceTransform(Ite->Key, InstanceTransform, true);
			if (InstanceTransform.Equals(Ite->Value.Transform))
				continue;

			Ite->Value.SetTransform(InstanceTransform);
		}
	}
}

void AAGX_SensorEnvironment::UpdateTrackedAGXMeshes()
{
	AGX_SensorEnvironment_helpers::UpdateTrackedMeshes(TrackedAGXMeshes);
}

void AAGX_SensorEnvironment::UpdateAmbientMaterial()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
		return;

	if (AmbientMaterial == nullptr)
	{
		NativeBarrier.SetAmbientMaterial(nullptr);
		return;
	}

	UWorld* World = GetWorld();
	UAGX_LidarAmbientMaterial* Instance = AmbientMaterial->GetOrCreateInstance(World);
	check(Instance);

	// Swap asset to instance as we are now in-game.
	if (AmbientMaterial != Instance)
	{
		AmbientMaterial = Instance;
	}

	NativeBarrier.SetAmbientMaterial(AmbientMaterial->GetNative());
}

void AAGX_SensorEnvironment::TickTrackedLidars() const
{
	for (auto It = TrackedLidars.CreateConstIterator(); It; ++It)
	{
		if (auto Lidar = It->Key.GetLidarComponent())
			Lidar->UpdateNativeTransform();
	}
}

void AAGX_SensorEnvironment::TickTrackedIMUs() const
{
	for (auto IMURef : TrackedIMUs)
	{
		if (auto IMU = IMURef.GetIMUComponent())
			IMU->UpdateTransformFromNative();
	}
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapComponent(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherComp))
		return;

	if (auto SceneComponent = Cast<USceneComponent>(OtherComp))
	{
		if (bIgnoreInvisibleObjects && !SceneComponent->ShouldRender())
			return;
	}

	auto InstancedMesh = Cast<UInstancedStaticMeshComponent>(OtherComp);
	if (InstancedMesh != nullptr)
	{
		OnLidarBeginOverlapInstancedStaticMeshComponent(*InstancedMesh, OtherBodyIndex);
		return;
	}

	auto Mesh = Cast<UStaticMeshComponent>(OtherComp);
	if (Mesh != nullptr)
	{
		OnLidarBeginOverlapStaticMeshComponent(*Mesh);
		return;
	}

	auto SimpleMesh = Cast<UAGX_SimpleMeshComponent>(OtherComp);
	if (SimpleMesh != nullptr)
	{
		OnLidarBeginOverlapAGXMeshComponent(*SimpleMesh);
		return;
	}
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapStaticMeshComponent(UStaticMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		AddMesh(&Mesh, DefaultLODIndex);
	else
		ShapeInstanceData->InstanceData.RefCount++;
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapInstancedStaticMeshComponent(
	UInstancedStaticMeshComponent& Mesh, int32 Index)
{
	auto InstancedMeshData = TrackedInstancedMeshes.Find(&Mesh);
	if (InstancedMeshData == nullptr)
	{
		AddInstancedMeshInstance(&Mesh, Index, DefaultLODIndex);
		return;
	}

	auto InstanceData = InstancedMeshData->InstancesData.Find(Index);
	if (InstanceData == nullptr)
		AddInstancedMeshInstance(&Mesh, Index, DefaultLODIndex);
	else
		InstanceData->RefCount++;
}

void AAGX_SensorEnvironment::OnLidarBeginOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedAGXMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		AddAGXMesh(&Mesh);
	else
		ShapeInstanceData->InstanceData.RefCount++;
}

void AAGX_SensorEnvironment::OnLidarEndOverlapComponent(
	UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex)
{
	auto InstancedMesh = Cast<UInstancedStaticMeshComponent>(OtherComp);
	if (InstancedMesh != nullptr)
	{
		OnLidarEndOverlapInstancedStaticMeshComponent(*InstancedMesh, OtherBodyIndex);
		return;
	}

	UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(OtherComp);
	if (Mesh != nullptr)
	{
		OnLidarEndOverlapStaticMeshComponent(*Mesh);
		return;
	}

	auto SimpleMesh = Cast<UAGX_SimpleMeshComponent>(OtherComp);
	if (SimpleMesh != nullptr)
	{
		OnLidarEndOverlapAGXMeshComponent(*SimpleMesh);
		return;
	}
}

void AAGX_SensorEnvironment::OnLidarEndOverlapStaticMeshComponent(UStaticMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		return;

	AGX_CHECK(ShapeInstanceData->InstanceData.RefCount > 0);
	ShapeInstanceData->InstanceData.RefCount--;
	if (ShapeInstanceData->InstanceData.RefCount == 0)
		TrackedMeshes.Remove(&Mesh);
}

void AAGX_SensorEnvironment::OnLidarEndOverlapInstancedStaticMeshComponent(
	UInstancedStaticMeshComponent& Mesh, int32 Index)
{
	if (!Mesh.IsValidInstance(Index))
		return;

	auto InstancedShape = TrackedInstancedMeshes.Find(&Mesh);
	if (InstancedShape == nullptr)
		return;

	auto InstanceData = InstancedShape->InstancesData.Find(Index);
	if (InstanceData == nullptr)
		return;

	AGX_CHECK(InstanceData->RefCount > 0);
	InstanceData->RefCount--;
	if (InstanceData->RefCount == 0)
		InstancedShape->InstancesData.Remove(Index);

	// Finally, we should remove the Instanced Static Mesh Component completely if no instances are
	// tracked.
	if (InstancedShape->InstancesData.Num() == 0)
		RemoveInstancedMesh(&Mesh);
}

void AAGX_SensorEnvironment::OnLidarEndOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedAGXMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		return;

	AGX_CHECK(ShapeInstanceData->InstanceData.RefCount > 0);
	ShapeInstanceData->InstanceData.RefCount--;
	if (ShapeInstanceData->InstanceData.RefCount == 0)
		TrackedAGXMeshes.Remove(&Mesh);
}

#if WITH_EDITOR
void AAGX_SensorEnvironment::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void AAGX_SensorEnvironment::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER(MagneticField);
}

void AAGX_SensorEnvironment::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}
#endif // WITH_EDITOR
