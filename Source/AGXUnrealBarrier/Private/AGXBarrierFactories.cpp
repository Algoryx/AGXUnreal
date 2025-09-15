// Copyright 2025, Algoryx Simulation AB.

#include "AGXBarrierFactories.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/Contacts/ShapeContactEntity.h"
#include "BarrierOnly/Vehicle/TrackRef.h"
#include "Contacts/ContactPointEntity.h"
#include "Terrain/TerrainBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agx/Hinge.h>
#include <agx/Prismatic.h>
#include <agx/BallJoint.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/LockJoint.h>
#include <agx/SingleControllerConstraint1DOF.h>
#include <agxCollide/Contacts.h>
#include <agxCollide/Sphere.h>
#include <agxCollide/Box.h>
#include <agxCollide/Trimesh.h>
#include <agxModel/TwoBodyTire.h>
#include <agxSDK/Simulation.h>
#include <agxSensor/RaytraceAmbientMaterial.h>
#include <agxTerrain/Shovel.h>
#include <agxTerrain/TerrainMaterial.h>
#include <agxWire/Wire.h>
#include <agxWire/Node.h>
#include <agxWire/WireWinchController.h>
#include <EndAGXIncludes.h>

#include <memory>

FRigidBodyBarrier AGXBarrierFactories::CreateRigidBodyBarrier(agx::RigidBody* Body)
{
	return {std::make_unique<FRigidBodyRef>(Body)};
}

FSimulationBarrier AGXBarrierFactories::CreateSimulationBarrier(agxSDK::Simulation* Simulation)
{
	return FSimulationBarrier(std::make_unique<FSimulationRef>(Simulation));
}

FAnyShapeBarrier AGXBarrierFactories::CreateAnyShapeBarrier(agxCollide::Shape* Shape)
{
	return {std::make_unique<FGeometryAndShapeRef>(Shape->getGeometry(), Shape)};
}

FSphereShapeBarrier AGXBarrierFactories::CreateSphereShapeBarrier(agxCollide::Sphere* Sphere)
{
	return {std::make_unique<FGeometryAndShapeRef>(Sphere->getGeometry(), Sphere)};
}

FBoxShapeBarrier AGXBarrierFactories::CreateBoxShapeBarrier(agxCollide::Box* Box)
{
	return {std::make_unique<FGeometryAndShapeRef>(Box->getGeometry(), Box)};
}

FCylinderShapeBarrier AGXBarrierFactories::CreateCylinderShapeBarrier(
	agxCollide::Cylinder* Cylinder)
{
	return {std::make_unique<FGeometryAndShapeRef>(Cylinder->getGeometry(), Cylinder)};
}

FCapsuleShapeBarrier AGXBarrierFactories::CreateCapsuleShapeBarrier(agxCollide::Capsule* Capsule)
{
	return {std::make_unique<FGeometryAndShapeRef>(Capsule->getGeometry(), Capsule)};
}

FTrimeshShapeBarrier AGXBarrierFactories::CreateTrimeshShapeBarrier(agxCollide::Trimesh* Trimesh)
{
	return {std::make_unique<FGeometryAndShapeRef>(Trimesh->getGeometry(), Trimesh)};
}

FAnyConstraintBarrier AGXBarrierFactories::CreateAnyConstraintBarrier(agx::Constraint* Constraint)
{
	return {std::make_unique<FConstraintRef>(Constraint)};
}

FHingeBarrier AGXBarrierFactories::CreateHingeBarrier(agx::Hinge* Hinge)
{
	return {std::make_unique<FConstraintRef>(Hinge)};
}

FPrismaticBarrier AGXBarrierFactories::CreatePrismaticBarrier(agx::Prismatic* Prismatic)
{
	return {std::make_unique<FConstraintRef>(Prismatic)};
}

FBallJointBarrier AGXBarrierFactories::CreateBallJointBarrier(agx::BallJoint* BallJoint)
{
	return {std::make_unique<FConstraintRef>(BallJoint)};
}

FCylindricalJointBarrier AGXBarrierFactories::CreateCylindricalJointBarrier(
	agx::CylindricalJoint* CylindricalJoint)
{
	return {std::make_unique<FConstraintRef>(CylindricalJoint)};
}

FDistanceJointBarrier AGXBarrierFactories::CreateDistanceJointBarrier(
	agx::DistanceJoint* DistanceJoint)
{
	return {std::make_unique<FConstraintRef>(DistanceJoint)};
}

FLockJointBarrier AGXBarrierFactories::CreateLockJointBarrier(agx::LockJoint* LockJoint)
{
	return {std::make_unique<FConstraintRef>(LockJoint)};
}

FSingleControllerConstraint1DOFBarrier
AGXBarrierFactories::CreateSingleControllerConstraint1DOFBarrier(
	agx::SingleControllerConstraint1DOF* Constraint)
{
	return {std::make_unique<FConstraintRef>(Constraint)};
}

FTwistRangeControllerBarrier AGXBarrierFactories::CreateTwistRangeControllerBarrier(
	agx::TwistRangeController* Controller)
{
	return {std::make_unique<FElementaryConstraintRef>(Controller)};
}

FShapeMaterialBarrier AGXBarrierFactories::CreateShapeMaterialBarrier(agx::Material* Material)
{
	return {std::make_unique<FMaterialRef>(Material)};
}

FContactMaterialBarrier AGXBarrierFactories::CreateContactMaterialBarrier(
	agx::ContactMaterial* ContactMaterial)
{
	return {std::make_unique<FContactMaterialRef>(ContactMaterial)};
}

FRtAmbientMaterialBarrier AGXBarrierFactories::CreateLidarAmbientMaterialBarrier(
	agxSensor::RtAmbientMaterial Material)
{
	FRtAmbientMaterialBarrier Barrier;
	Barrier.AllocateNative();
	return Barrier;
}

FShapeContactBarrier AGXBarrierFactories::CreateShapeContactBarrier(
	agxCollide::GeometryContact GeometryContact)
{
	return {std::make_unique<FShapeContactEntity>(GeometryContact)};
}

FContactPointBarrier AGXBarrierFactories::CreateContactPointBarrier(
	agxCollide::ContactPoint ContactPoint)
{
	return {std::make_unique<FContactPointEntity>(ContactPoint)};
}

FTwoBodyTireBarrier AGXBarrierFactories::CreateTwoBodyTireBarrier(agxModel::TwoBodyTire* Tire)
{
	return {std::make_unique<FTireRef>(Tire)};
}

FTerrainBarrier AGXBarrierFactories::CreateTerrainBarrier(agxTerrain::Terrain* Terrain)
{
	return {std::make_unique<FTerrainRef>(Terrain)};
}

FTerrainMaterialBarrier AGXBarrierFactories::CreateTerrainMaterialBarrier(
	agxTerrain::TerrainMaterial* Material)
{
	return {std::make_unique<FTerrainMaterialRef>(Material)};
}

FWireBarrier AGXBarrierFactories::CreateWireBarrier(agxWire::Wire* Wire)
{
	return {std::make_unique<FWireRef>(Wire)};
}

FWireNodeBarrier AGXBarrierFactories::CreateWireNodeBarrier(agxWire::Node* Node)
{
	return {std::make_unique<FWireNodeRef>(Node)};
}

FWireWinchBarrier AGXBarrierFactories::CreateWireWinchBarrier(agxWire::WireWinchController* Winch)
{
	return {std::make_unique<FWireWinchRef>(Winch)};
}

FShovelBarrier AGXBarrierFactories::CreateShovelBarrier(agxTerrain::Shovel* Shovel)
{
	return {std::make_unique<FShovelRef>(Shovel)};
}

FTrackBarrier AGXBarrierFactories::CreateTrackBarrier(agxVehicle::Track* Track)
{
	return {std::make_unique<FTrackRef>(Track)};
}
