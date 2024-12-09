// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_LidarSurfaceMaterial.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_LidarSurfaceMaterialComponent.generated.h"

class UAGX_LidarSurfaceMaterial;

/**
 * Helper Component that assigns lidar Surface Materials to other Components in the Level at
 * Begin Play.
 */
UCLASS(
	ClassGroup = "AGX_Sensor", Category = "AGX",
	Meta = (BlueprintSpawnableComponent, DisplayName = "AGX Lidar Surface Material Component"))
class AGXUNREAL_API UAGX_LidarSurfaceMaterialComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_LidarSurfaceMaterialComponent();

public:
	/**
	 * Determines what Component(s) to assign the Lidar Surface Material to.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", Meta = (ExposeOnSpawn))
	EAGX_LidarSurfaceMaterialAssignmentSelection Selection {
		EAGX_LidarSurfaceMaterialAssignmentSelection::Parent};

	/**
	 * The Lidar Surface Material assigned to Static Mesh Components and AGX Shapes.
	 * Determines the interaction of lidar laser rays with the objects that have this Material
	 * assigned.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	UAGX_LidarSurfaceMaterial* LidarSurfaceMaterial {nullptr};

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

private:
	void UpdateMaterial();
	void AssignMaterial();
	void AssignMaterial(USceneComponent* Component);
};
