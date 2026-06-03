// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_SensorEnvironmentSubsystem.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "AGX_Simulation.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Sensors/AGX_IMUSensorComponent.h"
#include "Sensors/AGX_LidarAmbientMaterial.h"
#include "Sensors/AGX_LidarLambertianOpaqueMaterial.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_LidarSurfaceMaterial.h"
#include "Sensors/AGX_SurfaceMaterialAssetUserData.h"
#include "Shapes/AGX_SimpleMeshComponent.h"
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
#include "GameFramework/Actor.h"
#include "Landscape.h"

// Standard library includes.
#include <algorithm>

namespace AGX_SensorEnvironmentSubsystem_helpers
{
	FRtLambertianOpaqueMaterialBarrier* GetLambertianOpaqueMaterialBarrierFrom(
		UAGX_LidarSurfaceMaterial* SurfaceMaterial, UWorld* World)
	{
		if (SurfaceMaterial == nullptr)
			return nullptr;

		UAGX_LidarSurfaceMaterial* SurfaceMaterialInstance =
			SurfaceMaterial->GetOrCreateInstance(World);
		if (SurfaceMaterialInstance == nullptr)
			return nullptr;

		auto LambertianOpaqueMaterial =
			Cast<UAGX_LidarLambertianOpaqueMaterial>(SurfaceMaterialInstance);
		if (LambertianOpaqueMaterial == nullptr)
			return nullptr;

		return LambertianOpaqueMaterial->GetOrCreateNative();
	}

	UAGX_LidarSurfaceMaterial* GetSurfaceMaterialFrom(const FSoftObjectPath& Path)
	{
		if (!Path.IsAsset())
			return nullptr;

		return LoadObject<UAGX_LidarSurfaceMaterial>(
			GetTransientPackage(), *Path.GetAssetPathString());
	}

	FRtLambertianOpaqueMaterialBarrier* GetDefaultLambertianOpaqueMaterialBarrier(
		const UAGX_SensorEnvironmentSubsystem& SensorEnvironment)
	{
		UAGX_LidarSurfaceMaterial* DefaultSurfaceMaterial =
			GetSurfaceMaterialFrom(SensorEnvironment.DefaultLidarSurfaceMaterial);
		if (DefaultSurfaceMaterial == nullptr)
			return nullptr;

		return GetLambertianOpaqueMaterialBarrierFrom(
			DefaultSurfaceMaterial, SensorEnvironment.GetWorld());
	}

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

		return GetLambertianOpaqueMaterialBarrierFrom(
			SurfaceMaterialData->LidarSurfaceMaterial.Get(), Component.GetWorld());
	}

	FRtLambertianOpaqueMaterialBarrier* GetLambertianOpaqueMaterialBarrierFrom(
		AAGX_Terrain& Terrain)
	{
		return GetLambertianOpaqueMaterialBarrierFrom(
			Terrain.LidarSurfaceMaterial, Terrain.GetWorld());
	}

	FRtLambertianOpaqueMaterialBarrier* GetLambertianOpaqueMaterialBarrierFromOrDefault(
		USceneComponent& Component, FRtLambertianOpaqueMaterialBarrier* DefaultMaterial)
	{
		FRtLambertianOpaqueMaterialBarrier* Material =
			GetLambertianOpaqueMaterialBarrierFrom(Component);
		return Material != nullptr ? Material : DefaultMaterial;
	}

	FRtLambertianOpaqueMaterialBarrier* GetLambertianOpaqueMaterialBarrierFromOrDefault(
		AAGX_Terrain& Terrain, FRtLambertianOpaqueMaterialBarrier* DefaultMaterial)
	{
		FRtLambertianOpaqueMaterialBarrier* Material =
			GetLambertianOpaqueMaterialBarrierFrom(Terrain);
		return Material != nullptr ? Material : DefaultMaterial;
	}

	TOptional<FAGX_RtShapeInstanceData> CreateShapeInstanceData(
		const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices, USceneComponent& Mesh,
		FSensorEnvironmentBarrier& SEBarrier, FRtLambertianOpaqueMaterialBarrier* DefaultMaterial)
	{
		FAGX_RtShapeInstanceData ShapeInstance;
		if (!ShapeInstance.Shape.AllocateNative(Vertices, Indices))
			return {};

		ShapeInstance.InstanceData.Instance.AllocateNative(ShapeInstance.Shape, SEBarrier);
		ShapeInstance.InstanceData.SetTransform(Mesh.GetComponentTransform());
		ShapeInstance.InstanceData.Instance.SetLidarSurfaceMaterial(
			GetLambertianOpaqueMaterialBarrierFromOrDefault(Mesh, DefaultMaterial));
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

	UAGX_LidarAmbientMaterial* GetAmbientMaterialFrom(const FSoftObjectPath& Path)
	{
		if (!Path.IsAsset())
			return nullptr;

		return LoadObject<UAGX_LidarAmbientMaterial>(
			GetTransientPackage(), *Path.GetAssetPathString());
	}

}

void UAGX_SensorEnvironmentSubsystem::SetMagneticField(const FVector& Field)
{
	MagneticField = Field;
	if (HasNative())
		NativeBarrier.SetMagneticField(Field);
}

FVector UAGX_SensorEnvironmentSubsystem::GetMagneticField() const
{
	if (HasNative())
		return NativeBarrier.GetMagneticField();

	return MagneticField;
}

bool UAGX_SensorEnvironmentSubsystem::AddLidar(UAGX_LidarSensorComponent* Lidar)
{
	if (Lidar == nullptr)
		return false;

	if (TrackedLidars.Contains(Lidar))
		return false;

	if (!FSensorEnvironmentBarrier::IsRaytraceSupported())
	{
		const FString Message =
			"Lidar raytracing (RTX) not supported on this computer, the Lidar Sensor will not "
			"work. To enable Lidar raytracing (RTX) support, use an RTX "
			"Graphical Processing Unit (GPU) with updated driver.";
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail, 8.f);
		return false;
	}

	if (!Lidar->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SensorEnvironmentSubsystem::AddLidar was called for Lidar Sensor Component "
				 "'%s' in '%s' but it does not have a Native. The Sensor Environment will not "
				 "create sensor Natives."),
			*Lidar->GetName(), *GetLabelSafe(Lidar->GetOwner()));
		return false;
	}

	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	FLidarBarrier* Barrier = Lidar->GetNativeAsLidar();
	if (Barrier == nullptr)
		return false;

	if (!NativeBarrier.Add(*Barrier))
		return false;

	// Associate each Lidar with a USphereComponent used to detect objects in the world to
	// give to AGX Dynamics during Play.
	USphereComponent* CollSph = nullptr;
	if (bAutoAddObjects)
	{
		AActor* LidarOwner = Lidar->GetOwner();
		if (LidarOwner == nullptr)
			return false;

		CollSph = NewObject<USphereComponent>(LidarOwner);
		CollSph->OnComponentBeginOverlap.AddDynamic(
			this, &UAGX_SensorEnvironmentSubsystem::OnLidarBeginOverlapComponent);
		CollSph->OnComponentEndOverlap.AddDynamic(
			this, &UAGX_SensorEnvironmentSubsystem::OnLidarEndOverlapComponent);

		// Ensure we don't miss overlap events by setting radius zero now. All collision
		// Collision spheres are updated in Tick(), and the overlap events will be triggered
		// for any object within that radius.
		CollSph->SetSphereRadius(0.f, false);

		// = true yields bugs of mutliple begin/end overlaps. See internal issue 957.
		CollSph->bTraceComplexOnMove = false;

		// Ignore Landscapes, these will otherwise be terrible for performance.
		for (auto Landscape :
			 AGX_SensorEnvironmentSubsystem_helpers::GetLandscapeActors(GetWorld()))
		{
			CollSph->IgnoreActorWhenMoving(Landscape, true);
		}

		if (USceneComponent* RootComponent = LidarOwner->GetRootComponent())
			CollSph->AttachToComponent(
				RootComponent, FAttachmentTransformRules::KeepWorldTransform);

		LidarOwner->AddInstanceComponent(CollSph);
		CollSph->RegisterComponent();
	}

	TrackedLidars.Add(Lidar, CollSph);
	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddIMU(UAGX_IMUSensorComponent* IMU)
{
	if (IMU == nullptr)
		return false;

	if (TrackedIMUs.Contains(IMU))
		return false;

	if (!IMU->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("SensorEnvironmentSubsystem::AddIMU was called for IMU Sensor Component "
				 "'%s' in '%s' but it does not have a Native. The Sensor Environment will not "
				 "create sensor Natives."),
			*IMU->GetName(), *GetLabelSafe(IMU->GetOwner()));
		return false;
	}

	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	FIMUBarrier* Barrier = IMU->GetNativeAsIMU();
	if (Barrier == nullptr)
		return false;

	if (!NativeBarrier.Add(*Barrier))
		return false;

	TrackedIMUs.Add(IMU);
	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddMesh(UStaticMeshComponent* Mesh, int32 InLod)
{
	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	TArray<FVector> OutVerts;
	TArray<FTriIndices> OutInds;
	const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;

	if (!AGX_SensorEnvironmentSubsystem_helpers::GetVerticesIndices(Mesh, OutVerts, OutInds, Lod))
		return false;

	if (!AddMesh(Mesh, OutVerts, OutInds))
		return false;

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log,
			TEXT("Sensor Environment Subsystem added Static Mesh Component '%s' in '%s'."),
			*Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddAGXMesh(UAGX_SimpleMeshComponent* Mesh)
{
	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	TArray<FVector> OutVerts;
	TArray<FTriIndices> OutInds;
	if (!AGX_SensorEnvironmentSubsystem_helpers::GetVerticesIndices(Mesh, OutVerts, OutInds))
		return false;

	if (!AddMesh(Mesh, OutVerts, OutInds))
		return false;

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment Subsystem added AGX Shape '%s' in '%s'."),
			*Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddInstancedMesh(
	UInstancedStaticMeshComponent* Mesh, int32 InLod)
{
	if (Mesh == nullptr)
		return false;

	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	if (!TrackedInstancedMeshes.Contains(Mesh))
	{
		TArray<FVector> OutVertices;
		TArray<FTriIndices> OutIndices;
		const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;
		if (!AGX_SensorEnvironmentSubsystem_helpers::GetVerticesIndices(
				Mesh, OutVertices, OutIndices, Lod))
		{
			return false;
		}

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
			LogAGX, Log,
			TEXT("Sensor Environment Subsystem added Instaced Static Mesh '%s' in '%s'."),
			*Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddInstancedMeshInstance(
	UInstancedStaticMeshComponent* Mesh, int32 Index, int32 InLod)
{
	if (Mesh == nullptr || !Mesh->IsValidInstance(Index))
		return false;

	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	if (!TrackedInstancedMeshes.Contains(Mesh))
	{
		TArray<FVector> OutVertices;
		TArray<FTriIndices> OutIndices;
		const int32 Lod = InLod < 0 ? DefaultLODIndex : InLod;
		if (!AGX_SensorEnvironmentSubsystem_helpers::GetVerticesIndices(
				Mesh, OutVertices, OutIndices, Lod))
		{
			return false;
		}

		if (!AddInstancedMesh(Mesh, OutVertices, OutIndices))
			return false;
	}

	const bool Res = AddInstancedMeshInstance_Internal(Mesh, Index);
	if (Res && DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment Subsystem added AGX Shape '%s' in '%s'."),
			*Mesh->GetName(), *GetLabelSafe(Mesh->GetOwner()));
	}

	return Res;
}

bool UAGX_SensorEnvironmentSubsystem::AddTerrain(AAGX_Terrain* Terrain)
{
	using namespace AGX_SensorEnvironmentSubsystem_helpers;
	if (Terrain == nullptr)
		return false;

	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	if (Terrain->bEnableTerrainPaging)
	{
		FTerrainPagerBarrier* PagerBarrier = Terrain->GetOrCreateNativeTerrainPager();
		if (PagerBarrier == nullptr)
			return false;

		if (!NativeBarrier.Add(*PagerBarrier))
			return false;

		FRtLambertianOpaqueMaterialBarrier* DefaultMaterial =
			GetDefaultLambertianOpaqueMaterialBarrier(*this);
		NativeBarrier.SetLidarSurfaceMaterial(
			*PagerBarrier,
			GetLambertianOpaqueMaterialBarrierFromOrDefault(*Terrain, DefaultMaterial));
	}
	else
	{
		FTerrainBarrier* TerrainBarrier = Terrain->GetOrCreateNative();
		if (TerrainBarrier == nullptr)
			return false;

		if (!NativeBarrier.Add(*TerrainBarrier))
			return false;

		FRtLambertianOpaqueMaterialBarrier* DefaultMaterial =
			GetDefaultLambertianOpaqueMaterialBarrier(*this);
		NativeBarrier.SetLidarSurfaceMaterial(
			*TerrainBarrier,
			GetLambertianOpaqueMaterialBarrierFromOrDefault(*Terrain, DefaultMaterial));
	}

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment Subsystem added Terrain '%s'."),
			*Terrain->GetName());
	}

	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddMovableTerrain(UAGX_MovableTerrainComponent* Terrain)
{
	using namespace AGX_SensorEnvironmentSubsystem_helpers;
	if (Terrain == nullptr)
		return false;

	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	FTerrainBarrier* TerrainBarrier = Terrain->GetOrCreateNative();
	if (TerrainBarrier == nullptr)
		return false;

	if (!NativeBarrier.Add(*TerrainBarrier))
		return false;

	FRtLambertianOpaqueMaterialBarrier* DefaultMaterial =
		GetDefaultLambertianOpaqueMaterialBarrier(*this);
	NativeBarrier.SetLidarSurfaceMaterial(
		*TerrainBarrier,
		GetLambertianOpaqueMaterialBarrierFromOrDefault(*Terrain, DefaultMaterial));

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment Subsystem added Movable Terrain '%s'."),
			*Terrain->GetName());
	}

	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddWire(UAGX_WireComponent* Wire)
{
	using namespace AGX_SensorEnvironmentSubsystem_helpers;
	if (Wire == nullptr)
		return false;

	EnsureNativeInitialized();
	if (!HasNative())
		return false;

	FWireBarrier* Barrier = Wire->GetOrCreateNative();
	if (Barrier == nullptr)
		return false;

	if (!NativeBarrier.Add(*Barrier))
		return false;

	FRtLambertianOpaqueMaterialBarrier* DefaultMaterial =
		GetDefaultLambertianOpaqueMaterialBarrier(*this);
	NativeBarrier.SetLidarSurfaceMaterial(
		*Barrier, GetLambertianOpaqueMaterialBarrierFromOrDefault(*Wire, DefaultMaterial));

	if (DebugLogOnAdd)
	{
		UE_LOG(
			LogAGX, Log, TEXT("Sensor Environment Subsystem added Wire '%s'."), *Wire->GetName());
	}

	return true;
}

bool UAGX_SensorEnvironmentSubsystem::RemoveLidar(UAGX_LidarSensorComponent* Lidar)
{
	bool DidRemove = false;

	if (Lidar == nullptr)
		return DidRemove;

	if (TObjectPtr<USphereComponent>* Sphere = TrackedLidars.Find(Lidar))
	{
		if (*Sphere != nullptr)
		{
			(*Sphere)->DestroyComponent();
		}

		TrackedLidars.Remove(Lidar);
		DidRemove = true;
	}

	if (!HasNative() || !Lidar->HasNative())
		return DidRemove;

	DidRemove |= NativeBarrier.Remove(*Lidar->GetNativeAsLidar());
	return DidRemove;
}

bool UAGX_SensorEnvironmentSubsystem::RemoveIMU(UAGX_IMUSensorComponent* IMU)
{
	bool DidRemove = false;

	if (IMU == nullptr)
		return DidRemove;

	DidRemove |= TrackedIMUs.Remove(IMU) > 0;

	if (!HasNative() || !IMU->HasNative())
		return DidRemove;

	DidRemove |= NativeBarrier.Remove(*IMU->GetNativeAsIMU());
	return DidRemove;
}

bool UAGX_SensorEnvironmentSubsystem::RemoveMesh(UStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr)
		return false;

	return TrackedMeshes.Remove(Mesh) > 0;
}

bool UAGX_SensorEnvironmentSubsystem::RemoveInstancedMesh(UInstancedStaticMeshComponent* Mesh)
{
	if (Mesh == nullptr || !TrackedInstancedMeshes.Contains(Mesh))
		return false;

	TrackedInstancedMeshes.Remove(Mesh);
	return true;
}

bool UAGX_SensorEnvironmentSubsystem::RemoveInstancedMeshInstance(
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

bool UAGX_SensorEnvironmentSubsystem::RemoveTerrain(AAGX_Terrain* Terrain)
{
	if (!HasNative() || Terrain == nullptr || !Terrain->HasNative())
		return false;

	if (Terrain->bEnableTerrainPaging)
		return NativeBarrier.Remove(*Terrain->GetNativeTerrainPager());
	else
		return NativeBarrier.Remove(*Terrain->GetOrCreateNative());
}

bool UAGX_SensorEnvironmentSubsystem::RemoveMovableTerrain(UAGX_MovableTerrainComponent* Terrain)
{
	if (!HasNative() || Terrain == nullptr || !Terrain->HasNative())
		return false;

	return NativeBarrier.Remove(*Terrain->GetOrCreateNative());
}

bool UAGX_SensorEnvironmentSubsystem::RemoveWire(UAGX_WireComponent* Wire)
{
	if (!HasNative() || Wire == nullptr || !Wire->HasNative())
		return false;

	return NativeBarrier.Remove(*Wire->GetNative());
}

bool UAGX_SensorEnvironmentSubsystem::HasNative() const
{
	return NativeBarrier.HasNative();
}

void UAGX_SensorEnvironmentSubsystem::EnsureNativeInitialized()
{
	if (HasNative())
		return;

	InitializeNative();
}

FSensorEnvironmentBarrier* UAGX_SensorEnvironmentSubsystem::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FSensorEnvironmentBarrier* UAGX_SensorEnvironmentSubsystem::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

UAGX_SensorEnvironmentSubsystem* UAGX_SensorEnvironmentSubsystem::GetFrom(
	const UActorComponent* Component)
{
	if (!Component)
		return nullptr;

	return GetFrom(Component->GetOwner());
}

UAGX_SensorEnvironmentSubsystem* UAGX_SensorEnvironmentSubsystem::GetFrom(const AActor* Actor)
{
	if (!Actor)
		return nullptr;

	UGameInstance* GameInstance = Actor->GetGameInstance();
	if (!GameInstance)
		return nullptr;

	return GetFrom(GameInstance);
}

UAGX_SensorEnvironmentSubsystem* UAGX_SensorEnvironmentSubsystem::GetFrom(const UWorld* World)
{
	if (!World)
		return nullptr;

	if (World->IsGameWorld())
		return GetFrom(World->GetGameInstance());

	return nullptr;
}

UAGX_SensorEnvironmentSubsystem* UAGX_SensorEnvironmentSubsystem::GetFrom(
	const UGameInstance* GameInstance)
{
	if (!GameInstance)
		return nullptr;

	return GameInstance->GetSubsystem<UAGX_SensorEnvironmentSubsystem>();
}

void UAGX_SensorEnvironmentSubsystem::Deinitialize()
{
	for (auto& TrackedLidar : TrackedLidars)
	{
		if (TrackedLidar.Value != nullptr)
			TrackedLidar.Value->DestroyComponent();
	}

	TrackedIMUs.Empty();
	TrackedLidars.Empty();
	TrackedMeshes.Empty();
	TrackedInstancedMeshes.Empty();
	TrackedAGXMeshes.Empty();

	if (AmbientMaterialInstance != nullptr && AmbientMaterialInstance->HasNative())
		AmbientMaterialInstance->ReleaseNative();

	AmbientMaterialInstance = nullptr;

	if (UAGX_LidarSurfaceMaterial* SurfaceMaterial =
			AGX_SensorEnvironmentSubsystem_helpers::GetSurfaceMaterialFrom(
				DefaultLidarSurfaceMaterial))
	{
		if (SurfaceMaterial->HasNative())
			SurfaceMaterial->ReleaseNative();
	}

	if (HasNative())
		NativeBarrier.ReleaseNative();

	Super::Deinitialize();
}

void UAGX_SensorEnvironmentSubsystem::Tick(float DeltaTime)
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

TStatId UAGX_SensorEnvironmentSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAGX_SensorEnvironmentSubsystem, STATGROUP_Tickables);
}

bool UAGX_SensorEnvironmentSubsystem::IsTickable() const
{
	const UWorld* World = GetWorld();
	return !IsTemplate() && World != nullptr && World->IsGameWorld();
}

UWorld* UAGX_SensorEnvironmentSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UAGX_SensorEnvironmentSubsystem::InitializeNative()
{
	AGX_CHECK(!HasNative());
	if (HasNative())
		return;

	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(GetGameInstance());
	if (Sim == nullptr || !Sim->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("AGX_SensorEnvironmentSubsystem was unable to get a UAGX_Simulation with Native "
				 "in InitializeNative. Correct behavior of the SensorEnvironment cannot be "
				 "guaranteed."));
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
			TEXT("AGX_SensorEnvironmentSubsystem was unable to create a Native AGX Dynamics "
				 "agxSensor::Environment. The Output Log may contain more information."));
		return;
	}

	NativeBarrier.SetMagneticField(MagneticField);

	if (FSensorEnvironmentBarrier::IsRaytraceSupported())
		UpdateAmbientMaterial();

	if (bAutoAddObjects)
		AddAutoDetectedTerrains();

	// In case the Level has no other AGX types in it.
	Sim->EnsureStepperCreated();
}

void UAGX_SensorEnvironmentSubsystem::UpdateTrackedLidars()
{
	// Update Collision Spheres and remove any destroyed Lidars.
	// Notice that overlap events will likely be triggered when updating the collision spheres radii
	// and transform.
	for (auto It = TrackedLidars.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Key.Get()))
		{
			if (It->Value != nullptr)
				It->Value->DestroyComponent();

			It.RemoveCurrent();
			continue;
		}

		if (bAutoAddObjects)
			AGX_SensorEnvironmentSubsystem_helpers::UpdateCollisionSphere(
				It->Key.Get(), It->Value.Get());
	}
}

void UAGX_SensorEnvironmentSubsystem::UpdateTrackedIMUs()
{
	for (auto It = TrackedIMUs.CreateIterator(); It; ++It)
	{
		if (!IsValid(It->Get()))
			It.RemoveCurrent();
	}
}

void UAGX_SensorEnvironmentSubsystem::UpdateTrackedMeshes()
{
	AGX_SensorEnvironmentSubsystem_helpers::UpdateTrackedMeshes(TrackedMeshes);
}

void UAGX_SensorEnvironmentSubsystem::UpdateTrackedInstancedMeshes()
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

void UAGX_SensorEnvironmentSubsystem::UpdateTrackedAGXMeshes()
{
	AGX_SensorEnvironmentSubsystem_helpers::UpdateTrackedMeshes(TrackedAGXMeshes);
}

void UAGX_SensorEnvironmentSubsystem::UpdateAmbientMaterial()
{
	AGX_CHECK(HasNative());
	if (!HasNative())
		return;

	UAGX_LidarAmbientMaterial* AmbientMaterialAsset =
		AGX_SensorEnvironmentSubsystem_helpers::GetAmbientMaterialFrom(AmbientMaterial);
	if (AmbientMaterialAsset == nullptr)
	{
		NativeBarrier.SetAmbientMaterial(nullptr);
		AmbientMaterialInstance = nullptr;
		return;
	}

	UWorld* World = GetWorld();
	UAGX_LidarAmbientMaterial* Instance = AmbientMaterialAsset->GetOrCreateInstance(World);
	check(Instance);

	AmbientMaterialInstance = Instance;
	NativeBarrier.SetAmbientMaterial(AmbientMaterialInstance->GetNative());
}

void UAGX_SensorEnvironmentSubsystem::TickTrackedLidars() const
{
	for (auto It = TrackedLidars.CreateConstIterator(); It; ++It)
	{
		if (auto Lidar = It->Key.Get())
			Lidar->UpdateNativeTransform();
	}
}

void UAGX_SensorEnvironmentSubsystem::TickTrackedIMUs() const
{
	for (auto IMURef : TrackedIMUs)
	{
		if (auto IMU = IMURef.Get())
			IMU->UpdateTransformFromNative();
	}
}

bool UAGX_SensorEnvironmentSubsystem::AddMesh(
	UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices, const TArray<FTriIndices>& Indices)
{
	using namespace AGX_SensorEnvironmentSubsystem_helpers;
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedMeshes.Contains(Mesh))
		return false;

	FRtLambertianOpaqueMaterialBarrier* DefaultMaterial =
		GetDefaultLambertianOpaqueMaterialBarrier(*this);
	auto ShapeInstance =
		CreateShapeInstanceData(Vertices, Indices, *Mesh, NativeBarrier, DefaultMaterial);
	if (!ShapeInstance.IsSet())
		return false;

	TrackedMeshes.Add(Mesh, std::move(ShapeInstance.GetValue()));
	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddMesh(
	UAGX_SimpleMeshComponent* Mesh, const TArray<FVector>& Vertices,
	const TArray<FTriIndices>& Indices)
{
	using namespace AGX_SensorEnvironmentSubsystem_helpers;
	AGX_CHECK(HasNative());

	if (Mesh == nullptr)
		return false;

	if (Vertices.Num() <= 0 || Indices.Num() <= 0)
		return false;

	if (TrackedAGXMeshes.Contains(Mesh))
		return false;

	FRtLambertianOpaqueMaterialBarrier* DefaultMaterial =
		GetDefaultLambertianOpaqueMaterialBarrier(*this);
	auto ShapeInstance =
		CreateShapeInstanceData(Vertices, Indices, *Mesh, NativeBarrier, DefaultMaterial);
	if (!ShapeInstance.IsSet())
		return false;

	TrackedAGXMeshes.Add(Mesh, std::move(ShapeInstance.GetValue()));
	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddInstancedMesh(
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

bool UAGX_SensorEnvironmentSubsystem::AddInstancedMeshInstance_Internal(
	UInstancedStaticMeshComponent* Mesh, int32 Index)
{
	using namespace AGX_SensorEnvironmentSubsystem_helpers;
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
	FRtLambertianOpaqueMaterialBarrier* DefaultMaterial =
		GetDefaultLambertianOpaqueMaterialBarrier(*this);
	InstanceData.Instance.SetLidarSurfaceMaterial(
		GetLambertianOpaqueMaterialBarrierFromOrDefault(*Mesh, DefaultMaterial));
	return true;
}

bool UAGX_SensorEnvironmentSubsystem::AddAutoDetectedTerrains()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
		return false;

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
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

	return true;
}

void UAGX_SensorEnvironmentSubsystem::OnLidarBeginOverlapComponent(
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

void UAGX_SensorEnvironmentSubsystem::OnLidarBeginOverlapStaticMeshComponent(
	UStaticMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		AddMesh(&Mesh, DefaultLODIndex);
	else
		ShapeInstanceData->InstanceData.RefCount++;
}

void UAGX_SensorEnvironmentSubsystem::OnLidarBeginOverlapInstancedStaticMeshComponent(
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

void UAGX_SensorEnvironmentSubsystem::OnLidarBeginOverlapAGXMeshComponent(
	UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedAGXMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		AddAGXMesh(&Mesh);
	else
		ShapeInstanceData->InstanceData.RefCount++;
}

void UAGX_SensorEnvironmentSubsystem::OnLidarEndOverlapComponent(
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

void UAGX_SensorEnvironmentSubsystem::OnLidarEndOverlapStaticMeshComponent(
	UStaticMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		return;

	AGX_CHECK(ShapeInstanceData->InstanceData.RefCount > 0);
	ShapeInstanceData->InstanceData.RefCount--;
	if (ShapeInstanceData->InstanceData.RefCount == 0)
		TrackedMeshes.Remove(&Mesh);
}

void UAGX_SensorEnvironmentSubsystem::OnLidarEndOverlapInstancedStaticMeshComponent(
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

void UAGX_SensorEnvironmentSubsystem::OnLidarEndOverlapAGXMeshComponent(
	UAGX_SimpleMeshComponent& Mesh)
{
	FAGX_RtShapeInstanceData* ShapeInstanceData = TrackedAGXMeshes.Find(&Mesh);
	if (ShapeInstanceData == nullptr)
		return;

	AGX_CHECK(ShapeInstanceData->InstanceData.RefCount > 0);
	ShapeInstanceData->InstanceData.RefCount--;
	if (ShapeInstanceData->InstanceData.RefCount == 0)
		TrackedAGXMeshes.Remove(&Mesh);
}
