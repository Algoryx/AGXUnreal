// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Math/Vector.h"
#include "Math/TwoVectors.h"
#include "Terrain/AGX_ShovelEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FRigidBodyBarrier;

struct FShovelRef;

/**
 * A Shovel describe the interaction between a rigid body and a terrain.
 *
 * Shovels are the only objects that are able to dynamically deform a terrain
 * and create particles from displaced soil.
 *
 * There is not corresponding UAGX_Shovel class. Instead, shovels are configured
 * by adding a TopEdge, a CuttingEdge, and a CuttingDirection to any Actor with
 * a RigidBody and registering that Actor in the Shovels Array of the
 * AGX_Terrain. ShovelBarriers are created when needed by the AGX_Terrain. There
 * are, however, an FAGX_Shovel class. This is an internal class used only by
 * AGX_Terrain.
 */
class AGXUNREALBARRIER_API FShovelBarrier
{
public:
	FShovelBarrier();
	FShovelBarrier(std::unique_ptr<FShovelRef> Native);
	FShovelBarrier(FShovelBarrier&& Other);
	~FShovelBarrier();

	void SetTopEdge(const FTwoVectors& TopEdge);
	FTwoVectors GetTopEdge() const;

	void SetCuttingEdge(const FTwoVectors& CuttingEdge);
	FTwoVectors GetCuttingEdge() const;

	void SetCuttingDirection(const FVector& CuttingDirection);
	FVector GetCuttingDirection() const;

	void SetToothLength(double ToothLength);
	double GetToothLength() const;

	void SetToothMinimumRadius(double MinimumToothRadius);
	double GetToothMinimumRadius() const;

	void SetToothMaximumRadius(double MaximumToothRadius);
	double GetToothMaximumRadius() const;

	void SetNumberOfTeeth(int32 NumberOfTeeth);
	int32 GetNumberOfTeeth() const;

	void SetNoMergeExtensionDistance(double NoMergeExtensionDistance);
	double GetNoMergeExtensionDistance() const;

	void SetMinimumSubmergedContactLengthFraction(double MinimumSubmergedContactLengthFraction);
	double GetMinimumSubmergedContactLengthFraction() const;

	void SetVerticalBladeSoilMergeDistance(double VerticalBladeSoilMergeDistance);
	double GetVerticalBladeSoilMergeDistance() const;

	void SetSecondarySeparationDeadloadLimit(double SecondarySeparationDeadloadLimit);
	double GetSecondarySeparationDeadloadLimit() const;

	void SetPenetrationDepthThreshold(double PenetrationDepthThreshold);
	double GetPenetrationDepthThreshold() const;

	void SetPenetrationForceScaling(double PenetrationForceScaling);
	double GetPenetrationForceScaling() const;

	void SetEnableParticleFreeDeformers(bool Enable);
	bool GetEnableParticleFreeDeformers() const;

	void SetAlwaysRemoveShovelContacts(bool Enable);
	bool GetAlwaysRemoveShovelContacts() const;

	void SetMaximumPenetrationForce(double MaximumPenetrationForce);
	double GetMaximumPenetrationForce() const;

	void SetContactRegionThreshold(double ContactRegionThreshold);
	double GetContactRegionThreshold() const;

	void SetContactRegionVerticalLimit(double ContactRegionVerticalLimit);
	double GetContactRegionVerticalLimit() const;

	void SetEnableInnerShapeCreateDynamicMass(bool Enable);
	bool GetEnableInnerShapeCreateDynamicMass() const;

	void SetEnableParticleForceFeedback(bool Enable);
	bool GetEnableParticleForceFeedback() const;

	void SetEnable(bool Enable);
	bool GetEnable() const;

	void SetParticleInclusionMultiplier(double Multiplier);
	double GetParticleInclusionMultiplier() const;

	void SetExcavationSettingsEnabled(EAGX_ExcavationMode Mode, bool Enable);
	bool GetExcavationSettingsEnabled(EAGX_ExcavationMode Mode) const;

	void SetExcavationSettingsEnableCreateDynamicMass(EAGX_ExcavationMode Mode, bool Enable);
	bool GetExcavationSettingsEnableCreateDynamicMass(EAGX_ExcavationMode Mode) const;

	void SetExcavationSettingsEnableForceFeedback(EAGX_ExcavationMode Mode, bool Enable);
	bool GetExcavationSettingsEnableForceFeedback(EAGX_ExcavationMode Mode) const;

	double GetInnerContactArea() const;

	FRigidBodyBarrier GetRigidBody() const;

	FGuid GetGuid() const;

	bool HasNative() const;
	void AllocateNative(
		FRigidBodyBarrier& Body, const FTwoVectors& TopEdge, const FTwoVectors& CuttingEdge,
		const FVector& CuttingDirection);
	FShovelRef* GetNative();
	const FShovelRef* GetNative() const;
	uint64 GetNativeAddress() const;
	void SetNativeAddress(uint64 Address);
	void ReleaseNative();

	/**
	 * Increment the reference count of the AGX Dynamics object. This should always be paired with
	 * a call to DecrementRefCount, and the count should only be artificially incremented for a
	 * very well specified duration.
	 *
	 * One use-case is during a Blueprint Reconstruction, when the Unreal Engine objects are
	 * destroyed and then recreated. During this time the AGX Dynamics objects are retained and
	 * handed between the old and the new Unreal Engine objects through a Component Instance Data.
	 * This Component Instance Data instance is considered the owner of the AGX Dynamics object
	 * during this transition period and the reference count is therefore increment during its
	 * lifetime. We're lending out ownership of the AGX Dynamics object to the Component Instance
	 * Data instance for the duration of the Blueprint Reconstruction.
	 *
	 * These functions can be const even though they have observable side effects because the
	 * reference count is not a salient part of the AGX Dynamics objects, and they are thread-safe.
	 */
	void IncrementRefCount() const;
	void DecrementRefCount() const;

	// Aliases required for the live update macros to work.
	void SetbAlwaysRemoveShovelContacts(bool InbAlwaysRemoveShovelContacts);
	void SetbEnableParticleFreeDeformers(bool InbEnableParticleFreeDeformers);
	void SetbEnableInnerShapeCreateDynamicMass(bool InbEnableInnerShapeCreateDynamicMass);
	void SetbEnableParticleForceFeedback(bool InbEnableParticleForceFeedback);

	friend class FAGX_Importer;

private:
	FShovelBarrier(const FShovelBarrier&) = delete;
	void operator=(const FShovelBarrier&) = delete;

private:
	std::unique_ptr<FShovelRef> NativeRef;
};
