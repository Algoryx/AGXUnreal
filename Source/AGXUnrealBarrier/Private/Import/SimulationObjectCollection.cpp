// Copyright 2024, Algoryx Simulation AB.

#include "Import/SimulationObjectCollection.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "AGXBarrierFactories.h"
#include "Constraints/AnyConstraintBarrier.h"
#include "Constraints/BallJointBarrier.h"
#include "Constraints/CylindricalJointBarrier.h"
#include "Constraints/DistanceJointBarrier.h"
#include "Constraints/HingeBarrier.h"
#include "Constraints/LockJointBarrier.h"
#include "Constraints/PrismaticBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Tires/TwoBodyTireBarrier.h"
#include "Vehicle/TrackBarrier.h"

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agx/Constraint.h>
#include <agx/Prismatic.h>
#include <agx/BallJoint.h>
#include <agx/CylindricalJoint.h>
#include <agx/DistanceJoint.h>
#include <agx/LockJoint.h>
#include <EndAGXIncludes.h>

FSimulationObjectCollection::~FSimulationObjectCollection()
{
}

TArray<FRigidBodyBarrier>& FSimulationObjectCollection::GetRigidBodies()
{
	return RigidBodies;
}

const TArray<FRigidBodyBarrier>& FSimulationObjectCollection::GetRigidBodies() const
{
	return RigidBodies;
}

namespace SimulationObjectCollection_helpers
{
	template <typename T>
	void AppendShapes(const TArray<T>& Shapes, TArray<FAnyShapeBarrier>& OutShapes)
	{
		OutShapes.Reserve(OutShapes.Num() + Shapes.Num());
		for (auto& Shape : Shapes)
		{
			// This unpacking/repackaging seems overly complicated. Why can't we just copy the
			// Barrier into the TArray?
			agxCollide::Shape* ShapeAGX = FAGX_AgxDynamicsObjectsAccess::GetShapeFrom(&Shape);
			OutShapes.Add(AGXBarrierFactories::CreateAnyShapeBarrier(ShapeAGX));
		}
	}
}

TArray<FAnyShapeBarrier> FSimulationObjectCollection::CollectAllShapes() const
{
	using namespace SimulationObjectCollection_helpers;
	TArray<FAnyShapeBarrier> AllShapes = CollectAllPrimitiveShapes();

	AppendShapes(TrimeshShapes, AllShapes);
	for (const FRigidBodyBarrier& Body : RigidBodies)
		AppendShapes(Body.GetTrimeshShapes(), AllShapes);

	return AllShapes;
}

TArray<FAnyShapeBarrier> FSimulationObjectCollection::CollectAllPrimitiveShapes() const
{
	using namespace SimulationObjectCollection_helpers;
	TArray<FAnyShapeBarrier> PrimitiveShapes;

	AppendShapes(SphereShapes, PrimitiveShapes);
	AppendShapes(BoxShapes, PrimitiveShapes);
	AppendShapes(CylinderShapes, PrimitiveShapes);
	AppendShapes(CapsuleShapes, PrimitiveShapes);
	for (const FRigidBodyBarrier& Body : RigidBodies)
	{
		AppendShapes(Body.GetSphereShapes(), PrimitiveShapes);
		AppendShapes(Body.GetBoxShapes(), PrimitiveShapes);
		AppendShapes(Body.GetCylinderShapes(), PrimitiveShapes);
		AppendShapes(Body.GetCapsuleShapes(), PrimitiveShapes);
	}
	return PrimitiveShapes;
}

TArray<FSphereShapeBarrier>& FSimulationObjectCollection::GetSphereShapes()
{
	return SphereShapes;
}

const TArray<FSphereShapeBarrier>& FSimulationObjectCollection::GetSphereShapes() const
{
	return SphereShapes;
}

TArray<FBoxShapeBarrier>& FSimulationObjectCollection::GetBoxShapes()
{
	return BoxShapes;
}

const TArray<FBoxShapeBarrier>& FSimulationObjectCollection::GetBoxShapes() const
{
	return BoxShapes;
}

TArray<FCylinderShapeBarrier>& FSimulationObjectCollection::GetCylinderShapes()
{
	return CylinderShapes;
}

const TArray<FCylinderShapeBarrier>& FSimulationObjectCollection::GetCylinderShapes() const
{
	return CylinderShapes;
}

TArray<FCapsuleShapeBarrier>& FSimulationObjectCollection::GetCapsuleShapes()
{
	return CapsuleShapes;
}

const TArray<FCapsuleShapeBarrier>& FSimulationObjectCollection::GetCapsuleShapes() const
{
	return CapsuleShapes;
}

TArray<FTrimeshShapeBarrier>& FSimulationObjectCollection::GetTrimeshShapes()
{
	return TrimeshShapes;
}

const TArray<FTrimeshShapeBarrier>& FSimulationObjectCollection::GetTrimeshShapes() const
{
	return TrimeshShapes;
}

TArray<FAnyConstraintBarrier> FSimulationObjectCollection::CollectAllConstraints() const
{
	TArray<FAnyConstraintBarrier> AllConstraints;
	auto AddConstraints = [&AllConstraints](const auto& Constraints)
	{
		AllConstraints.Reserve(AllConstraints.Num() + Constraints.Num());
		for (const auto& Constraint : Constraints)
		{
			// This unpacking/repackaging seems overly complicated. Why can't we just copy the
			// Barrier into the TArray?
			agx::Constraint* ConstraintAGX = FAGX_AgxDynamicsObjectsAccess::GetFrom(&Constraint);
			AllConstraints.Add(AGXBarrierFactories::CreateAnyConstraintBarrier(ConstraintAGX));
		}
	};
	AddConstraints(HingeConstraints);
	AddConstraints(PrismaticConstraints);
	AddConstraints(BallConstraints);
	AddConstraints(CylindricalConstraints);
	AddConstraints(DistanceConstraints);
	AddConstraints(LockConstraints);
	return AllConstraints;
}

TArray<FHingeBarrier>& FSimulationObjectCollection::GetHingeConstraints()
{
	return HingeConstraints;
}

const TArray<FHingeBarrier>& FSimulationObjectCollection::GetHingeConstraints() const
{
	return HingeConstraints;
}

TArray<FPrismaticBarrier>& FSimulationObjectCollection::GetPrismaticConstraints()
{
	return PrismaticConstraints;
}

const TArray<FPrismaticBarrier>& FSimulationObjectCollection::GetPrismaticConstraints() const
{
	return PrismaticConstraints;
}

TArray<FBallJointBarrier>& FSimulationObjectCollection::GetBallConstraints()
{
	return BallConstraints;
}

const TArray<FBallJointBarrier>& FSimulationObjectCollection::GetBallConstraints() const
{
	return BallConstraints;
}

TArray<FCylindricalJointBarrier>& FSimulationObjectCollection::GetCylindricalConstraints()
{
	return CylindricalConstraints;
}

const TArray<FCylindricalJointBarrier>& FSimulationObjectCollection::GetCylindricalConstraints()
	const
{
	return CylindricalConstraints;
}

TArray<FDistanceJointBarrier>& FSimulationObjectCollection::GetDistanceConstraints()
{
	return DistanceConstraints;
}

const TArray<FDistanceJointBarrier>& FSimulationObjectCollection::GetDistanceConstraints() const
{
	return DistanceConstraints;
}

TArray<FLockJointBarrier>& FSimulationObjectCollection::GetLockConstraints()
{
	return LockConstraints;
}

const TArray<FLockJointBarrier>& FSimulationObjectCollection::GetLockConstraints() const
{
	return LockConstraints;
}

TArray<FContactMaterialBarrier>& FSimulationObjectCollection::GetContactMaterials()
{
	return ContactMaterials;
}

const TArray<FContactMaterialBarrier>& FSimulationObjectCollection::GetContactMaterials() const
{
	return ContactMaterials;
}

TArray<std::pair<FString, FString>>& FSimulationObjectCollection::GetDisabledCollisionGroups()
{
	return DisabledCollisionGroups;
}

const TArray<std::pair<FString, FString>>& FSimulationObjectCollection::GetDisabledCollisionGroups()
	const
{
	return DisabledCollisionGroups;
}

TArray<FObserverFrameData>& FSimulationObjectCollection::GetObserverFrames()
{
	return ObserverFrames;
}
const TArray<FObserverFrameData>& FSimulationObjectCollection::GetObserverFrames() const
{
	return ObserverFrames;
}

TArray<FShapeMaterialBarrier>& FSimulationObjectCollection::GetShapeMaterials()
{
	return ShapeMaterials;
}

const TArray<FShapeMaterialBarrier>& FSimulationObjectCollection::GetShapeMaterials() const
{
	return ShapeMaterials;
}

TArray<FTwoBodyTireBarrier>& FSimulationObjectCollection::GetTwoBodyTires()
{
	return TwoBodyTires;
}

const TArray<FTwoBodyTireBarrier>& FSimulationObjectCollection::GetTwoBodyTires() const
{
	return TwoBodyTires;
}

TArray<FWireBarrier>& FSimulationObjectCollection::GetWires()
{
	return Wires;
}

const TArray<FWireBarrier>& FSimulationObjectCollection::GetWires() const
{
	return Wires;
}

TArray<FShovelBarrier>& FSimulationObjectCollection::GetShovels()
{
	return Shovels;
}

const TArray<FShovelBarrier>& FSimulationObjectCollection::GetShovels() const
{
	return Shovels;
}

TArray<FTrackBarrier>& FSimulationObjectCollection::GetTracks()
{
	return Tracks;
}

const TArray<FTrackBarrier>& FSimulationObjectCollection::GetTracks() const
{
	return Tracks;
}

TArray<FPLX_Input>& FSimulationObjectCollection::GetPLXInputs()
{
	return PLXInputs;
}

const TArray<FPLX_Input>& FSimulationObjectCollection::GetPLXInputs() const
{
	return PLXInputs;
}

TArray<FPLX_Output>& FSimulationObjectCollection::GetPLXOutputs()
{
	return PLXOutputs;
}

const TArray<FPLX_Output>& FSimulationObjectCollection::GetPLXOutputs() const
{
	return PLXOutputs;
}
