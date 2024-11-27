// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"
#include "AMOR/WireMergeSplitThresholdsBarrier.h"
#include "Utilities/AGX_Statistics.h"
#include "Contacts/ShapeContactBarrier.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"

// Standard library includes.
#include <memory>

struct FSimulationRef;

class FRigidBodyBarrier;
class FConstraintBarrier;
class FContactMaterialBarrier;
class FShapeBarrier;
class FShapeMaterialBarrier;
class FTerrainBarrier;
class FTerrainPagerBarrier;
class FTireBarrier;
class FWireBarrier;

class AGXUNREALBARRIER_API FSimulationBarrier
{
public:
	FSimulationBarrier();
	~FSimulationBarrier();

	bool Add(FConstraintBarrier& Constraint);
	bool Add(FContactMaterialBarrier& ContactMaterial);

	/**
	 * Note that Shapes that are child of the passed Rigid Body are NOT added to the simulation
	 * when calling this function. All Shapes, whether child to a Rigid Body or a free Shape, are
	 * responsible for adding themselves to the Simulation. This makes it easier to handle e.g.
	 * changes in the Component attachment hierarchy during Play.
	 */
	bool Add(FRigidBodyBarrier& Body);
	bool Add(FShapeBarrier& Shape);
	bool Add(FShapeMaterialBarrier& Material);
	bool Add(FTerrainBarrier& Terrain);
	bool Add(FTerrainPagerBarrier& Terrain);
	bool Add(FTireBarrier& Tire);
	bool Add(FWireBarrier& Wire);

	bool Remove(FConstraintBarrier& Constraint);
	bool Remove(FContactMaterialBarrier& ContactMaterial);

	/**
	 * Note that agx::Simulation::remove(agx::RigidBody*, bool) is called with RemoveGeometries =
	 * false.
	 */
	bool Remove(FRigidBodyBarrier& Body);
	bool Remove(FShapeBarrier& Shape);
	bool Remove(FShapeMaterialBarrier& Material);
	bool Remove(FTerrainBarrier& Terrain);
	bool Remove(FTerrainPagerBarrier& Terrain);
	bool Remove(FTireBarrier& Tire);
	bool Remove(FWireBarrier& Wire);

	void SetEnableCollisionGroupPair(const FName& Group1, const FName& Group2, bool CanCollide);

	static void SetEnableCollision(FRigidBodyBarrier& Body1, FRigidBodyBarrier& Body2, bool Enable);

	bool WriteAGXArchive(const FString& Filename) const;

	void EnableRemoteDebugging(int16 Port);

	void SetEnableAMOR(bool bEnable);
	bool GetEnableAMOR();

	void SetTimeStep(double TimeStep);
	double GetTimeStep() const;

	static void SetNumThreads(uint32 NumThreads);
	static uint32 GetNumThreads();

	void SetEnableContactWarmstarting(bool bEnable);
	bool GetEnableContactWarmstarting() const;

	void SetNumPpgsIterations(int32 NumIterations);
	int32 GetNumPpgsIterations() const;

	void SetUniformGravity(const FVector& Gravity);
	FVector GetUniformGravity() const;

	void SetPointGravity(const FVector& Origin, float Magnitude);
	FVector GetPointGravity(float& OutMagnitude) const;

	/**
	 * Returns all Shape Contacts associated with a Shape.
	 */
	TArray<FShapeContactBarrier> GetShapeContacts(const FShapeBarrier& Shape) const;

	/**
	 * Returns all Shape Contacts in the current Simulation.
	 */
	TArray<FShapeContactBarrier> GetShapeContacts() const;

	/**
	 * Perform one simulation step, moving the time stamp forward by one time step duration.
	 */
	void Step();

	/**
	 * The returned value is usually the amount of time that has been simulated, but SetTimeStamp
	 * may invalidate this assumption.
	 * @return The current simulation time stamp.
	 */
	double GetTimeStamp() const;

	/**
	 * Set the current simulation time stamp. Does not simulate to that time, just moves the clock
	 * hands.
	 * @param TimeStamp The new time stamp.
	 */
	void SetTimeStamp(double TimeStamp);

	void SetPreIntegratePositions(bool Enable);
	bool GetPreIntegratePositions() const;

	/// \todo Statistics isn't a per-simulation thing in AGX Dynamics, so having statistics
	/// management here is a bit misleading.
	void SetStatisticsEnabled(bool bEnable);
	FAGX_Statistics GetStatistics();

	bool HasNative() const;
	void AllocateNative();
	FSimulationRef* GetNative();
	const FSimulationRef* GetNative() const;
	void ReleaseNative();

	/**
	 * Functions for creating a Merge Split Thresholds Barrier which can be modified to
	 * change the underlying global AGX Dynamics thresholds.
	 */
	FShapeContactMergeSplitThresholdsBarrier GetGlobalShapeContactTresholds() const;
	FConstraintMergeSplitThresholdsBarrier GetGlobalConstraintTresholds() const;
	FWireMergeSplitThresholdsBarrier GetGlobalWireTresholds() const;

private:
	FSimulationBarrier(const FSimulationBarrier&) = delete;
	void operator=(const FSimulationBarrier&) = delete;

private:
	std::unique_ptr<FSimulationRef> NativeRef;
};
