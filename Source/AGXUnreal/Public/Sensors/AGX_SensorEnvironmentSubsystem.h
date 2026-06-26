// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_ShapeInstanceData.h"
#include "Sensors/SensorEnvironmentBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Tickable.h"

#include "AGX_SensorEnvironmentSubsystem.generated.h"

class AAGX_Terrain;
class AActor;
class UAGX_IMUSensorComponent;
class UAGX_LidarAmbientMaterial;
class UAGX_LidarSensorComponent;
class UAGX_LidarSurfaceMaterial;
class UAGX_MovableTerrainComponent;
class UAGX_SimpleMeshComponent;
class UAGX_WireComponent;
class UActorComponent;
class UGameInstance;
class UInstancedStaticMeshComponent;
class UPrimitiveComponent;
class USphereComponent;
class UStaticMeshComponent;
class UWorld;

UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", Config = Engine, DefaultConfig)
class AGXUNREAL_API UAGX_SensorEnvironmentSubsystem : public UGameInstanceSubsystem,
													  public FTickableGameObject
{
	GENERATED_BODY()

public:
	/**
	 * Objects in the Level that gets within the range of any added Lidar will automatically be
	 * added to this Environment if they can be detected.
	 * Objects will be detected if they have a Static Mesh Component using "Generate Overlap Events"
	 * and it has a Static Mesh with "Simple Collision" active.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	bool bAutoAddObjects {true};

	/**
	 * If set to true, any Component (e.g. Static Mesh) that has Visibility invisible will be
	 * ignored when using Auto Add Objects. If set to false, the Component will be added regardless
	 * of visibility.
	 * If manually adding objects to the Sensor Environment using any of the Add... functions, this
	 * property will not have an affect, and the object will always be added.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (EditCondition = "bAutoAddObjects"))
	bool bIgnoreInvisibleObjects {true};

	/**
	 * Default LOD index used when reading Meshes that are added to this Environment.
	 * If set to a negative value, the highest valid LOD index is used (lowest resolution).
	 * If this LOD index does not exist for a Mesh that is added, the closest valid (and lower) LOD
	 * index is selected.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	int32 DefaultLODIndex {-1};

	/**
	 * The Ambient material used by the Sensor Environment.
	 * This is used to simulate atmospheric effects on the Lidar laser rays, such as rain or fog.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (AllowedClasses = "/Script/AGXUnreal.AGX_LidarAmbientMaterial"))
	FSoftObjectPath AmbientMaterial;

	/**
	 * Default Lidar Surface Material assigned to all objects added to a Sensor Environment
	 * when the object has no explicitly assigned Lidar Surface Material.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (AllowedClasses = "/Script/AGXUnreal.AGX_LidarSurfaceMaterial"))
	FSoftObjectPath DefaultLidarSurfaceMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool SetAmbientMaterial(UAGX_LidarAmbientMaterial* InAmbientMaterial);

	/**
	 * Set positions integrated in PRE so that they are "seen" in the Lidar output in the
	 * same step.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Sensor Environment", AdvancedDisplay)
	bool bSetPreIntegratePosition {true};

	/**
	 * Whether or not the transform of added Instanced Static Meshes should be updated each
	 * Tick. Updating Instanced Static Mesh transforms comes with some perfomance cost,
	 * especially if a large number of instances are present.
	 * As an optimization, this can be disabled by setting this property to false. Note that any
	 * transformation change of an Instanced Static Mesh Instance during Play will not be reflected
	 * in the Lidar simulation.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadWrite, Category = "AGX Sensor Environment",
		AdvancedDisplay)
	bool UpdateAddedInstancedMeshesTransforms {true};

	/**
	 * The (uniform) Magnetic Field of this Sensor Environment in Tesla [T].
	 * Only used with IMU Sensors that uses a Magnetometer (see AGX IMU Sensor Component).
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	FVector MagneticField {0.0, 44.754e-6, 0.0};

	/**
	 * For debugging purposes. If set to true, a message is logged in the Output Console each time
	 * an object is succesfully added to this Sensor Environment.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadWrite, Category = "AGX Sensor Environment",
		AdvancedDisplay)
	bool DebugLogOnAdd {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	void SetMagneticField(const FVector& Field);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	FVector GetMagneticField() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddLidar(UAGX_LidarSensorComponent* Lidar);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddIMU(UAGX_IMUSensorComponent* IMU);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddMesh(UStaticMeshComponent* Mesh, int32 LOD = -1);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddAGXMesh(UAGX_SimpleMeshComponent* Mesh);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMesh(UInstancedStaticMeshComponent* Mesh, int32 LOD = -1);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMeshInstance(UInstancedStaticMeshComponent* Mesh, int32 Index, int32 LOD = -1);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddTerrain(AAGX_Terrain* Terrain);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddMovableTerrain(UAGX_MovableTerrainComponent* Terrain);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddWire(UAGX_WireComponent* Wire);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveLidar(UAGX_LidarSensorComponent* Lidar);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveIMU(UAGX_IMUSensorComponent* IMU);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveMesh(UStaticMeshComponent* Mesh);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveInstancedMesh(UInstancedStaticMeshComponent* Mesh);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveInstancedMeshInstance(UInstancedStaticMeshComponent* Mesh, int32 Index);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveTerrain(AAGX_Terrain* Terrain);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveMovableTerrain(UAGX_MovableTerrainComponent* Terrain);

	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveWire(UAGX_WireComponent* Wire);

	bool HasNative() const;
	void EnsureNativeInitialized();
	FSensorEnvironmentBarrier* GetNative();
	const FSensorEnvironmentBarrier* GetNative() const;

	static UAGX_SensorEnvironmentSubsystem* GetFrom(const UActorComponent* Component);
	static UAGX_SensorEnvironmentSubsystem* GetFrom(const AActor* Actor);
	static UAGX_SensorEnvironmentSubsystem* GetFrom(const UWorld* World);
	static UAGX_SensorEnvironmentSubsystem* GetFrom(const UGameInstance* GameInstance);

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	// ~End UObject interface.

	// ~Begin USubsystem interface.
	virtual void Deinitialize() override;
	// ~End USubsystem interface.

	// ~Begin FTickableGameObject interface.
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	// ~End FTickableGameObject interface.

private:
	void InitializeNative();
	void UpdateTrackedLidars();
	void UpdateTrackedIMUs();
	void UpdateTrackedMeshes();
	void UpdateTrackedInstancedMeshes();
	void UpdateTrackedAGXMeshes();
	bool UpdateAmbientMaterial();
	void TickTrackedLidars() const;
	void TickTrackedIMUs() const;

	bool AddMesh(
		UStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	bool AddMesh(
		UAGX_SimpleMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	bool AddInstancedMesh(
		UInstancedStaticMeshComponent* Mesh, const TArray<FVector>& Vertices,
		const TArray<FTriIndices>& Indices);

	bool AddInstancedMeshInstance_Internal(UInstancedStaticMeshComponent* Mesh, int32 Index);
	bool AddAutoDetectedTerrains();

	UFUNCTION()
	void OnLidarBeginOverlapComponent(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnLidarEndOverlapComponent(
		UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void OnLidarBeginOverlapStaticMeshComponent(UStaticMeshComponent& Mesh);
	void OnLidarBeginOverlapInstancedStaticMeshComponent(
		UInstancedStaticMeshComponent& Mesh, int32 Index);
	void OnLidarBeginOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh);

	void OnLidarEndOverlapStaticMeshComponent(UStaticMeshComponent& Mesh);
	void OnLidarEndOverlapInstancedStaticMeshComponent(
		UInstancedStaticMeshComponent& Mesh, int32 Index);
	void OnLidarEndOverlapAGXMeshComponent(UAGX_SimpleMeshComponent& Mesh);

private:
	TMap<TWeakObjectPtr<UAGX_LidarSensorComponent>, TObjectPtr<USphereComponent>> TrackedLidars;
	TMap<TWeakObjectPtr<UStaticMeshComponent>, FAGX_RtShapeInstanceData> TrackedMeshes;
	TMap<TWeakObjectPtr<UInstancedStaticMeshComponent>, FAGX_RtInstancedShapeInstanceData>
		TrackedInstancedMeshes;
	TMap<TWeakObjectPtr<UAGX_SimpleMeshComponent>, FAGX_RtShapeInstanceData> TrackedAGXMeshes;
	TSet<TWeakObjectPtr<UAGX_IMUSensorComponent>> TrackedIMUs;

	TObjectPtr<UAGX_LidarAmbientMaterial> AmbientMaterialInstance {nullptr};

	FSensorEnvironmentBarrier NativeBarrier;
};
