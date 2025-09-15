// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

namespace agx
{
	class BallJoint;
	class Constraint;
	class ContactMaterial;
	class CylindricalJoint;
	class DistanceJoint;
	class Hinge;
	class LockJoint;
	class SingleControllerConstraint1DOF;
	class MassProperties;
	class Material;
	class Prismatic;
	class RigidBody;
}

namespace agxCollide
{
	class ContactPoint;
	class Geometry;
	class GeometryContact;
	class RenderData;
	class Shape;
}

namespace agxModel
{
	class Tire;
	class TwoBodyTire;
}

namespace agxSDK
{
	class MergeSplitProperties;
	class MergeSplitThresholds;
	class Simulation;
}

namespace agxTerrain
{
	class Shovel;
	class Terrain;
	class TerrainMaterial;
}

namespace agxVehicle
{
	class Track;
	class TrackProperties;
	class TrackWheel;
}

namespace agxWire
{
	class Node;
	class Wire;
	class WireWinchController;
}

// Namespace agx.
class FBallJointBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FHingeBarrier;
class FLockJointBarrier;
class FSingleControllerConstraint1DOFBarrier;
class FMassPropertiesBarrier;
class FPrismaticBarrier;
class FRigidBodyBarrier;
class FShapeMaterialBarrier;

// Namespace agxCollide.
class FContactPointBarrier;
class FRenderDataBarrier;
class FShapeBarrier;
class FShapeContactBarrier;

// Namespace agxModel.
class FTireBarrier;
class FTwoBodyTireBarrier;

// Namespace agxSDK.
class FMergeSplitPropertiesBarrier;
class FMergeSplitThresholdsBarrier;
class FSimulationBarrier;

// Namespace agxTerrain.
class FShovelBarrier;
class FTerrainBarrier;
class FTerrainMaterialBarrier;

// Namespace agxVehicle.
class FTrackBarrier;
class FTrackPropertiesBarrier;
class FTrackWheelBarrier;

// Namespace agxWire
class FWireBarrier;
class FWireNodeBarrier;
class FWireWinchBarrier;

class AGXUNREALBARRIER_API FAGX_AgxDynamicsObjectsAccess
{
public:
	// Namespace agx.
	static agx::BallJoint* GetFrom(const FBallJointBarrier* Barrier);
	static agx::Constraint* GetFrom(const FConstraintBarrier* Barrier);
	static agx::ContactMaterial* GetFrom(const FContactMaterialBarrier* Barrier);
	static agx::CylindricalJoint* GetFrom(const FCylindricalJointBarrier* Barrier);
	static agx::DistanceJoint* GetFrom(const FDistanceJointBarrier* Barrier);
	static agx::Hinge* GetFrom(const FHingeBarrier* Barrier);
	static agx::LockJoint* GetFrom(const FLockJointBarrier* Barrier);
	static agx::SingleControllerConstraint1DOF* GetFrom(
		const FSingleControllerConstraint1DOFBarrier* Barrier);
	static agx::MassProperties* GetFrom(const FMassPropertiesBarrier* Barrier);
	static agx::Material* GetFrom(const FShapeMaterialBarrier* Barrier);
	static agx::Prismatic* GetFrom(const FPrismaticBarrier* Barrier);
	static agx::RigidBody* GetFrom(const FRigidBodyBarrier& Barrier);
	static agx::RigidBody* GetFrom(const FRigidBodyBarrier* Barrier);
	static agx::RigidBody* TryGetFrom(const FRigidBodyBarrier& Barrier);
	static agx::RigidBody* TryGetFrom(const FRigidBodyBarrier* Barrier);

	// Namespace agxCollide.
	static agxCollide::ContactPoint* GetFrom(const FContactPointBarrier* Barrier);
	static agxCollide::Geometry* GetGeometryFrom(const FShapeBarrier* Barrier);
	static agxCollide::GeometryContact* GetFrom(const FShapeContactBarrier* Barrier);
	static const agxCollide::RenderData* GetFrom(const FRenderDataBarrier* Barrier);
	static agxCollide::Shape* GetShapeFrom(const FShapeBarrier* Barrier);

	// Namespace agxModel.
	static agxModel::Tire* GetFrom(const FTireBarrier* Barrier);
	static agxModel::TwoBodyTire* GetFrom(const FTwoBodyTireBarrier* Barrier);

	// Namespace agxSDK.
	static agxSDK::MergeSplitProperties* GetFrom(const FMergeSplitPropertiesBarrier* Barrier);
	static agxSDK::MergeSplitThresholds* GetFrom(const FMergeSplitThresholdsBarrier* Barrier);
	static agxSDK::Simulation* GetFrom(const FSimulationBarrier* Barrier);

	// Namespace agxTerrain.
	static agxTerrain::Shovel* GetFrom(const FShovelBarrier* Barrier);
	static agxTerrain::Terrain* GetFrom(const FTerrainBarrier* Barrier);
	static agxTerrain::TerrainMaterial* GetFrom(const FTerrainMaterialBarrier* Barrier);

	// Namespace agxVehicle.
	static agxVehicle::Track* GetFrom(const FTrackBarrier* Barrier);
	static agxVehicle::TrackProperties* GetFrom(const FTrackPropertiesBarrier* Barrier);
	static agxVehicle::TrackWheel* GetFrom(const FTrackWheelBarrier* Barrier);

	// Namespace agxWire.
	static agxWire::Node* GetFrom(const FWireNodeBarrier* Barrier);
	static agxWire::Wire* GetFrom(const FWireBarrier* Barrier);
	static agxWire::WireWinchController* GetFrom(const FWireWinchBarrier* Barrier);
};
