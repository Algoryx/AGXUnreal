// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
// For some reason, these could not be forward declared without compiler error.
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/CylinderShapeBarrier.h"
#include "Shapes/CapsuleShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "SimulationBarrier.h"
#include "Terrain/ShovelBarrier.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "Containers/Array.h"

// Standard library includes.
#include <memory>

// Constraints.
class FAnyConstraintBarrier;
class FHingeBarrier;
class FPrismaticBarrier;
class FBallJointBarrier;
class FCylindricalJointBarrier;
class FDistanceJointBarrier;
class FLockJointBarrier;
class FSingleControllerConstraint1DOFBarrier;

// Others.
class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FShapeMaterialBarrier;
class FSimulationBarrier;
class FTwoBodyTireBarrier;
class FTrackBarrier;

struct AGXUNREALBARRIER_API FObserverFrameData
{
	FString Name;
	FGuid BodyGuid;
	FGuid ObserverGuid;
	FTransform Transform;
};

struct AGXUNREALBARRIER_API FSimulationObjectCollection
{
public:
	FSimulationObjectCollection() = default;
	~FSimulationObjectCollection();

	TArray<FRigidBodyBarrier>& GetRigidBodies();
	const TArray<FRigidBodyBarrier>& GetRigidBodies() const;

	TArray<FAnyShapeBarrier> CollectAllShapes() const;
	TArray<FAnyShapeBarrier> CollectAllPrimitiveShapes() const;

	TArray<FSphereShapeBarrier>& GetSphereShapes();
	const TArray<FSphereShapeBarrier>& GetSphereShapes() const;

	TArray<FBoxShapeBarrier>& GetBoxShapes();
	const TArray<FBoxShapeBarrier>& GetBoxShapes() const;

	TArray<FCylinderShapeBarrier>& GetCylinderShapes();
	const TArray<FCylinderShapeBarrier>& GetCylinderShapes() const;

	TArray<FCapsuleShapeBarrier>& GetCapsuleShapes();
	const TArray<FCapsuleShapeBarrier>& GetCapsuleShapes() const;

	TArray<FTrimeshShapeBarrier>& GetTrimeshShapes();
	const TArray<FTrimeshShapeBarrier>& GetTrimeshShapes() const;

	TArray<FAnyConstraintBarrier> CollectAllConstraints() const;

	TArray<FHingeBarrier>& GetHingeConstraints();
	const TArray<FHingeBarrier>& GetHingeConstraints() const;

	TArray<FPrismaticBarrier>& GetPrismaticConstraints();
	const TArray<FPrismaticBarrier>& GetPrismaticConstraints() const;

	TArray<FBallJointBarrier>& GetBallConstraints();
	const TArray<FBallJointBarrier>& GetBallConstraints() const;

	TArray<FCylindricalJointBarrier>& GetCylindricalConstraints();
	const TArray<FCylindricalJointBarrier>& GetCylindricalConstraints() const;

	TArray<FDistanceJointBarrier>& GetDistanceConstraints();
	const TArray<FDistanceJointBarrier>& GetDistanceConstraints() const;

	TArray<FLockJointBarrier>& GetLockConstraints();
	const TArray<FLockJointBarrier>& GetLockConstraints() const;

	TArray<FSingleControllerConstraint1DOFBarrier>& GetSingleControllerConstraint1DOFs();

	const TArray<FSingleControllerConstraint1DOFBarrier>& GetSingleControllerConstraint1DOFs()
		const;

	TArray<FContactMaterialBarrier>& GetContactMaterials();
	const TArray<FContactMaterialBarrier>& GetContactMaterials() const;

	TArray<std::pair<FString, FString>>& GetDisabledCollisionGroups();
	const TArray<std::pair<FString, FString>>& GetDisabledCollisionGroups() const;

	TArray<FObserverFrameData>& GetObserverFrames();
	const TArray<FObserverFrameData>& GetObserverFrames() const;

	TArray<FShapeMaterialBarrier>& GetShapeMaterials();
	const TArray<FShapeMaterialBarrier>& GetShapeMaterials() const;

	TArray<FTwoBodyTireBarrier>& GetTwoBodyTires();
	const TArray<FTwoBodyTireBarrier>& GetTwoBodyTires() const;

	TArray<FWireBarrier>& GetWires();
	const TArray<FWireBarrier>& GetWires() const;

	TArray<FShovelBarrier>& GetShovels();
	const TArray<FShovelBarrier>& GetShovels() const;

	TArray<FTrackBarrier>& GetTracks();
	const TArray<FTrackBarrier>& GetTracks() const;

	std::shared_ptr<FSimulationBarrier>& GetSimulation();
	const std::shared_ptr<FSimulationBarrier>& GetSimulation() const;

	TArray<FPLX_Input>& GetPLXInputs();
	const TArray<FPLX_Input>& GetPLXInputs() const;

	TArray<FPLX_Output>& GetPLXOutputs();
	const TArray<FPLX_Output>& GetPLXOutputs() const;

private:
	FSimulationObjectCollection(const FSimulationObjectCollection&) = delete;
	void operator=(const FSimulationObjectCollection&) = delete;

	// The Simulation from which all other Simulation Objects collected from.
	std::shared_ptr<FSimulationBarrier> Simulation;

	// These are all Shapes, even those owned by a RigidBody.
	TArray<FSphereShapeBarrier> SphereShapes;
	TArray<FBoxShapeBarrier> BoxShapes;
	TArray<FCylinderShapeBarrier> CylinderShapes;
	TArray<FCapsuleShapeBarrier> CapsuleShapes;
	TArray<FTrimeshShapeBarrier> TrimeshShapes;

	// These are "free" Constraints only, i.e. not owned by e.g. Two Body Tire or similar.
	TArray<FHingeBarrier> HingeConstraints;
	TArray<FPrismaticBarrier> PrismaticConstraints;
	TArray<FBallJointBarrier> BallConstraints;
	TArray<FCylindricalJointBarrier> CylindricalConstraints;
	TArray<FDistanceJointBarrier> DistanceConstraints;
	TArray<FLockJointBarrier> LockConstraints;
	TArray<FSingleControllerConstraint1DOFBarrier> SingleControllerConstraint1DOFs;

	TArray<FRigidBodyBarrier> RigidBodies;
	TArray<FContactMaterialBarrier> ContactMaterials;
	TArray<std::pair<FString, FString>> DisabledCollisionGroups;
	TArray<FObserverFrameData> ObserverFrames;
	TArray<FShapeMaterialBarrier> ShapeMaterials;
	TArray<FTwoBodyTireBarrier> TwoBodyTires;
	TArray<FWireBarrier> Wires;
	TArray<FShovelBarrier> Shovels;
	TArray<FTrackBarrier> Tracks;

	TArray<FPLX_Input> PLXInputs;
	TArray<FPLX_Output> PLXOutputs;
};
