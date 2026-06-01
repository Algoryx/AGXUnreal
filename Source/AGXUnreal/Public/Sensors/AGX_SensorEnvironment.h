// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Sensors/AGX_IMUSensorReference.h"
#include "Sensors/AGX_LidarSensorReference.h"
#include "Sensors/AGX_ShapeInstanceData.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_SensorEnvironment.generated.h"

class AAGX_Terrain;
class UAGX_LidarAmbientMaterial;
class UAGX_MovableTerrainComponent;
class UAGX_SimpleMeshComponent;
class UAGX_WireComponent;
class UInstancedStaticMeshComponent;
class USphereComponent;
class UStaticMeshComponent;

UCLASS(ClassGroup = "AGX_Sensor", Blueprintable, Category = "AGX")
class AGXUNREAL_API AAGX_SensorEnvironment : public AActor
{
	GENERATED_BODY()

public:
	AAGX_SensorEnvironment();

	/**
	 * Array of all Lidar Sensor Components that should be active in the simulation.
	 * Any Lidar Sensor Components that should be active has to be added by the user to this Array.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Sensor Environment")
	TArray<FAGX_LidarSensorReference> LidarSensors;

	/**
	 * Array of all IMU Sensor Components that should be active in the simulation.
	 * Any IMU Sensor Components that should be active has to be added by the user to this Array.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Sensor Environment")
	TArray<FAGX_IMUSensorReference> IMUSensors;

	/**
	 * Objects in the Level that gets within the range of any added Lidar will automatically be
	 * added to this Environment if they can be detected.
	 * Objects will be detected if they have a Static Mesh Component using "Generate Overlap Events"
	 * and it has a Static Mesh with "Simple Collision" active.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (ExposeOnSpawn))
	bool bAutoAddObjects {true};

	/**
	 * If set to true, any Component (e.g. Static Mesh) that has Visibility invisible will be
	 * ignored when using Auto Add Objects. If set to false, the Component will be added regardless
	 * of visibility.
	 * If manually adding objects to the Sensor Environment using any of the Add... functions, this
	 * property will not have an affect, and the object will always be added.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment",
		Meta = (EditCondition = "bAutoAddObjects"))
	bool bIgnoreInvisibleObjects {true};

	/**
	 * Default LOD index used when reading Meshes that are added to this Environment.
	 * If set to a negative value, the highest valid LOD index is used (lowest resolution).
	 * If this LOD index does not exist for a Mesh that is added, the closest valid (and lower) LOD
	 * index is selected.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	int32 DefaultLODIndex {-1};

	/**
	 * The Ambient material used by the Sensor Environment.
	 * This is used to simulate atmospheric effects on the Lidar laser rays, such as rain or fog.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	UAGX_LidarAmbientMaterial* AmbientMaterial {nullptr};

	/**
	 * Whether or not the transform of added Instanced Static Meshes should be updated each Tick.
	 * Updating Instanced Static Mesh transforms comes with some perfomance cost, especially if
	 * a large number of instances are present.
	 * As an optimization, this can be disabled by setting this property to false. Note that any
	 * transformation change of an Instanced Static Mesh Instance during Play will not be reflected
	 * in the Lidar simulation.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Sensor Environment", AdvancedDisplay)
	bool UpdateAddedInstancedMeshesTransforms {true};

	/**
	 * The (uniform) Magnetic Field of this Sensor Environment in Tesla [T].
	 * Only used with IMU Sensors that uses a Magnetometer (see AGX IMU Sensor Component).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Sensor Environment")
	FVector MagneticField {0.0, 44.754e-6, 0.0};

	/**
	 * Set the (uniform) Magnetic Field of this Sensor Environment in Tesla [T].
	 * Only used with IMU Sensors that uses a Magnetometer (see AGX IMU Sensor Component).
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	void SetMagneticField(const FVector& Field);

	/**
	 * Get the (uniform) Magnetic Field of this Sensor Environment in Tesla [T].
	 * Only used with IMU Sensors that uses a Magnetometer (see AGX IMU Sensor Component).
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	FVector GetMagneticField() const;

	/**
	 * For debugging purposes. If set to true, a message is logged in the Output Console each time
	 * an object is succesfully added to this Sensor Environment.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Sensor Environment", AdvancedDisplay)
	bool DebugLogOnAdd {false};

	/**
	 * Manually add a Lidar Sensor Component. This can also be done from the Details Panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddLidar(UAGX_LidarSensorComponent* Lidar);

	/**
	 * Manually add an IMU Sensor Component. This can also be done from the Details Panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddIMU(UAGX_IMUSensorComponent* IMU);

	/**
	 * Manually add a Static Mesh Component so that it can be detected by sensors handled by this
	 * Sensor Environment. (Optional) LOD determines the LOD index used when reading the given Mesh.
	 * If left to -1, the DefaultLODIndex is used. See property DefaultLODIndex.
	 *
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddMesh(UStaticMeshComponent* Mesh, int32 LOD = -1);

	/**
	 * Manually add an AGX Simple Mesh Component so that it can be detected by sensors handled by
	 * this Sensor Environment. Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddAGXMesh(UAGX_SimpleMeshComponent* Mesh);

	/**
	 * Manually add all instances of an Instanced Static Mesh Component so that they can be detected
	 * by sensors handled by this Sensor Environment.
	 *
	 * Instances created after calling this function will not be added to the Sensor Environment.
	 *
	 * (Optional) LOD determines the LOD index used when reading the given Mesh. If left to -1,
	 * the DefaultLODIndex is used. See property DefaultLODIndex.
	 *
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMesh(UInstancedStaticMeshComponent* Mesh, int32 LOD = -1);

	/**
	 * Manually add a single instance of an Instanced Static Mesh Component so that it can be
	 * detected by sensors handled by this Sensor Environment. The Index corresponds to the Mesh
	 * Instance to add. (Optional) LOD determines the LOD index used when reading the given Mesh. If
	 * left to -1, the DefaultLODIndex is used. See property DefaultLODIndex.
	 *
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddInstancedMeshInstance(UInstancedStaticMeshComponent* Mesh, int32 Index, int32 LOD = -1);

	/**
	 * Manually add a Terrain so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddTerrain(AAGX_Terrain* Terrain);

	/**
	 * Manually add a Movable Terrain Componentso that it can be detected by sensors handled by this
	 * Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddMovableTerrain(UAGX_MovableTerrainComponent* Terrain);

	/**
	 * Manually add a Wire so that it can be detected by sensors handled by this Sensor
	 * Environment.
	 * When using this function instead of letting the Sensor Environment 'Auto Add' the Wire, the
	 * underlying AGX Dynamics Wire will be used for raycast hits instead of the visual
	 * representation seen in the Viewport.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool AddWire(UAGX_WireComponent* Wire);

	/**
	 * Manually remove a Lidar Sensor Component from this Sensor Environment.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveLidar(UAGX_LidarSensorComponent* Lidar);

	/**
	 * Manually remove an IMU Sensor Component from this Sensor Environment.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveIMU(UAGX_IMUSensorComponent* IMU);

	/**
	 * Manually remove a Static Mesh Component from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveMesh(UStaticMeshComponent* Mesh);

	/**
	 * Manually remove an Instanced Static Mesh Component and all its instances from this Sensor
	 * Environment. Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveInstancedMesh(UInstancedStaticMeshComponent* Mesh);

	/**
	 * Manually remove a single Instanced Static Mesh Instace from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveInstancedMeshInstance(UInstancedStaticMeshComponent* Mesh, int32 Index);

	/**
	 * Manually remove a Terrain from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveTerrain(AAGX_Terrain* Terrain);

	/**
	 * Manually remove a Movable Terrain Component from this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveMovableTerrain(UAGX_MovableTerrainComponent* Terrain);

	/**
	 * Manually remove a Wire from this Sensor Environment.
	 * Should only be called for Wires manually added to this Sensor Environment.
	 * Only valid to call during Play.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Sensor Environment")
	bool RemoveWire(UAGX_WireComponent* Wire);

	bool HasNative() const;
	FSensorEnvironmentBarrier* GetNative();
	const FSensorEnvironmentBarrier* GetNative() const;

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	// ~End UObject interface.

	// ~Begin AActor interface.
	virtual void Tick(float DeltaSeconds) override;
	// ~End AActor interface.

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeNative();
	void RegisterLidars();
	bool RegisterLidar(FAGX_LidarSensorReference& LidarRef);
	void RegisterIMUs();
	bool RegisterIMU(FAGX_IMUSensorReference& IMURef);
	void UpdateTrackedLidars();
	void UpdateTrackedIMUs();
	void UpdateTrackedMeshes();
	void UpdateTrackedInstancedMeshes();
	void UpdateTrackedAGXMeshes();
	void UpdateAmbientMaterial();
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

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	void InitPropertyDispatcher();
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif // WITH_EDITOR

private:
	TMap<FAGX_LidarSensorReference, TObjectPtr<USphereComponent>> TrackedLidars;
	TMap<TWeakObjectPtr<UStaticMeshComponent>, FAGX_RtShapeInstanceData> TrackedMeshes;
	TMap<TWeakObjectPtr<UInstancedStaticMeshComponent>, FAGX_RtInstancedShapeInstanceData>
		TrackedInstancedMeshes;
	TMap<TWeakObjectPtr<UAGX_SimpleMeshComponent>, FAGX_RtShapeInstanceData> TrackedAGXMeshes;
	TSet<FAGX_IMUSensorReference> TrackedIMUs;

	FSensorEnvironmentBarrier NativeBarrier;
};
