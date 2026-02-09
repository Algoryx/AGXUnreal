// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackEnums.h"

// Unreal Engine includes.
#include "Containers/UnrealString.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// Standard library includes.
#include <memory>

struct FRigidBodyBarrier;
class FShapeMaterialBarrier;
class FTrackPropertiesBarrier;
class FTrackWheelBarrier;
struct FTrackRef;

/**
 * Barrier between UAGX_TrackComponent and agxVehicle::Track. UAGX_TrackComponent holds an instance
 * of TrackBarrier and hidden behind the TrackBarrier is a agx::TrackRef. This allows
 * UAGX_TrackComponent to interact with agxVehicle::Track without including agxVehicle/Track.h.
 *
 * This class handles all translation between Unreal Engine types and AGX Dynamics types, such as
 * back and forth between FVector and agx::Vec3.
 */
class AGXUNREALBARRIER_API FTrackBarrier
{
public:
	struct FTrackWheelDescription
	{
		uint8 Model;
		double Radius;
		FTransform RigidBodyTransform;
		FVector RelativePosition;
		FQuat RelativeRotation;
	};

	typedef std::tuple<FVector, FRotator> FVectorAndRotator;
	typedef std::tuple<FVector, FRotator, FVector> FVectorRotatorRadii;
	typedef std::tuple<FVector, FQuat, float> FVectorQuatRadius;

	FTrackBarrier();
	FTrackBarrier(std::unique_ptr<FTrackRef> Native);
	FTrackBarrier(FTrackBarrier&& Other);
	~FTrackBarrier();

	void AddTrackWheel(
		uint8 Model, double Radius, const FRigidBodyBarrier& RigidBody,
		const FVector& RelativePosition, const FQuat& RelativeRotation, bool bSplitSegments,
		bool bMoveNodesToRotationPlane, bool bMoveNodesToWheel);

	void SetName(const FString& Name);
	FString GetName() const;

	void ClearMaterial();
	void SetMaterial(const FShapeMaterialBarrier& Material);
	FShapeMaterialBarrier GetMaterial() const;

	void ClearProperties();
	void SetProperties(const FTrackPropertiesBarrier& Properties);
	FTrackPropertiesBarrier GetProperties() const;

	void AddCollisionGroup(const FName& GroupName);
	void AddCollisionGroups(const TArray<FName>& GroupNames);
	TArray<FName> GetCollisionGroups() const;

	TArray<FTrackWheelBarrier> GetWheels() const;

	int32 GetNumNodes() const;

	double GetWidth() const;

	double GetThickness() const;

	double GetInitialDistanceTension() const;

	FRigidBodyBarrier GetNodeBody(int index) const;

	void GetNodeSizes(TArray<FVector>& OutNodeSizes) const;

	FGuid GetGuid() const;

	/*
	 * Returns the size of a single node, where X is Thickness, Y is Width, and Z is Length.
	 * Returns a zero vector if the index is out of range (@see GetNumNodes).
	 */
	FVector GetNodeSize(uint64 index = 0) const;

	/**
	 * Get the transform of the center point of all track nodes. Scale is set to LocalScale and
	 * location is offset by LocalOffset * Rotation. Used for track rendering while playing.
	 */
	void GetNodeTransforms(
		TArray<FTransform>& OutTransforms, const FVector& LocalScale, const FVector& LocalOffset,
		const FQuat& LocalRotation) const;

	/**
	 * Get debug data for all nodes. Used for track debug visualization while playing.
	 */
	void GetDebugData(
		TArray<FVectorAndRotator>* BodyTransforms, TArray<FVectorAndRotator>* HingeTransforms,
		TArray<FVector>* MassCenters, TArray<FVectorRotatorRadii>* CollisionBoxes,
		TArray<FLinearColor>* BodyColors, TArray<FVectorQuatRadius>* WheelTransforms,
		TArray<FLinearColor>* WheelColors) const;

	/**
	 * Get node transforms preview before creating the actual track. Use to render a preview
	 * of the track while not playing.
	 */
	static void GetPreviewData(
		TArray<FTransform>& OutNodeTransforms, TArray<FVector>& OutNodeHalfExtents, uint64 NumNodes,
		double Width, double Thickness, double InitialTensionDistance,
		const TArray<FTrackWheelDescription>& Wheels);

	// Internal Merge Properties.

	void InternalMergeProperties_SetEnableMerge(bool bEnable);
	bool InternalMergeProperties_GetEnableMerge() const;

	void InternalMergeProperties_SetNumNodesPerMergeSegment(uint32 NumNodesPerMergeSegment);
	uint32 InternalMergeProperties_GetNumNodesPerMergeSegment() const;

	void InternalMergeProperties_SetContactReduction(EAGX_MergedTrackNodeContactReduction);
	EAGX_MergedTrackNodeContactReduction InternalMergeProperties_GetContactReduction() const;

	void InternalMergeProperties_SetEnableLockToReachMergeCondition(bool bEnable);
	bool InternalMergeProperties_GetEnableLockToReachMergeCondition() const;

	void InternalMergeProperties_SetLockToReachMergeConditionCompliance(double Compliance);
	double InternalMergeProperties_GetLockToReachMergeConditionCompliance() const;

	void InternalMergeProperties_SetLockToReachMergeConditionSpookDamping(double Damping);
	double InternalMergeProperties_GetLockToReachMergeConditionSpookDamping() const;

	void InternalMergeProperties_SetMaxAngleMergeCondition(double MaxAngleToMerge);
	double InternalMergeProperties_GetMaxAngleMergeCondition() const;

	// Native Handling.

	bool HasNative() const;
	FTrackRef* GetNative();
	const FTrackRef* GetNative() const;

	void AllocateNative(
		int32 NumberOfNodes, float Width, float Thickness, float InitialDistanceTension);
	void ReleaseNative();

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing
	/// AGX Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);

private:
	FTrackBarrier(const FTrackBarrier&) = delete;
	void operator=(const FTrackBarrier&) = delete;

private:
	std::unique_ptr<FTrackRef> NativeRef;
};
