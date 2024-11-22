// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Utilities/AGX_StringUtilities.h"

/// \todo Determine which of these are really needed.
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/AGX_ConstraintFrameActor.h"
#include "Constraints/ConstraintBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "CoreGlobals.h"
#include "UObject/UObjectGlobals.h"

namespace AGX_ConstraintComponent_helpers
{
	void SetLocalScope(UAGX_ConstraintComponent& Constraint)
	{
		AActor* Owner = FAGX_ObjectUtilities::GetRootParentActor(Constraint);
		Constraint.BodyAttachment1.RigidBody.LocalScope = Owner;
		Constraint.BodyAttachment1.FrameDefiningComponent.LocalScope = Owner;
		Constraint.BodyAttachment2.RigidBody.LocalScope = Owner;
		Constraint.BodyAttachment2.FrameDefiningComponent.LocalScope = Owner;
	}
}

EDofFlag ConvertDofsArrayToBitmask(const TArray<EDofFlag>& LockedDofsOrdered)
{
	uint8 bitmask(0);
	for (EDofFlag flag : LockedDofsOrdered)
	{
		bitmask |= (uint8) flag;
	}
	return (EDofFlag) bitmask;
}

TMap<EGenericDofIndex, int32> BuildNativeDofIndexMap(const TArray<EDofFlag>& LockedDofsOrdered)
{
	TMap<EGenericDofIndex, int32> DofIndexMap;
	for (int32 NativeIndex = 0; NativeIndex < LockedDofsOrdered.Num(); ++NativeIndex)
	{
		switch (LockedDofsOrdered[NativeIndex])
		{
			case EDofFlag::DofFlagTranslational1:
				DofIndexMap.Add(EGenericDofIndex::Translational1, NativeIndex);
				break;
			case EDofFlag::DofFlagTranslational2:
				DofIndexMap.Add(EGenericDofIndex::Translational2, NativeIndex);
				break;
			case EDofFlag::DofFlagTranslational3:
				DofIndexMap.Add(EGenericDofIndex::Translational3, NativeIndex);
				break;
			case EDofFlag::DofFlagRotational1:
				DofIndexMap.Add(EGenericDofIndex::Rotational1, NativeIndex);
				break;
			case EDofFlag::DofFlagRotational2:
				DofIndexMap.Add(EGenericDofIndex::Rotational2, NativeIndex);
				break;
			case EDofFlag::DofFlagRotational3:
				DofIndexMap.Add(EGenericDofIndex::Rotational3, NativeIndex);
				break;
			default:
				checkNoEntry();
		}
	}

	// General mappings, -1 is the value of DOF::ALL_DOF in all AGX Dynamics constraints.
	DofIndexMap.Add(EGenericDofIndex::AllDof, -1);

	return DofIndexMap;
}

UAGX_ConstraintComponent::UAGX_ConstraintComponent()
	: BodyAttachment1(this)
	, BodyAttachment2(this)
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAGX_ConstraintComponent::UAGX_ConstraintComponent(const TArray<EDofFlag>& LockedDofsOrdered)
	: BodyAttachment1(this)
	, BodyAttachment2(this)
	, bEnable(true)
	, SolveType(EAGX_SolveType::StDirect)
	, Compliance(
		  ConstraintConstants::DefaultCompliance(), ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, SpookDamping(
		  ConstraintConstants::DefaultSpookDamping(), ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, ForceRange(
		  ConstraintConstants::DefaultForceRange(), ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, LockedDofsBitmask(ConvertDofsArrayToBitmask(LockedDofsOrdered))
	, LockedDofs(LockedDofsOrdered)
	, NativeDofIndexMap(BuildNativeDofIndexMap(LockedDofsOrdered))
	, Elasticity_DEPRECATED(
		  ConstraintConstants::DefaultElasticity(), ConvertDofsArrayToBitmask(LockedDofsOrdered))
{
}

void UAGX_ConstraintComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// This code is run after the constructor and after InitProperties, where property values are
	// copied from the Class Default Object, but before deserialization in cases where this object
	// is created from another, such as at the start of a Play-in-Editor session or when loading
	// a map in a cooked build (I hope).
	AGX_ConstraintComponent_helpers::SetLocalScope(*this);

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

namespace AGX_ConstraintComponent_helpers
{
	bool SetBody(
		FAGX_ConstraintBodyAttachment& Attachment, UAGX_RigidBodyComponent* Body,
		UAGX_ConstraintComponent& Outer)
	{
		if (Outer.HasNative())
		{
			// Too late to set body, the AGX Dynamics constraint has already been created.
			UE_LOG(
				LogAGX, Warning,
				TEXT(
					"SetBody called on Constraint Component '%s', in Actor '%s', which has already "
					"initialized the AGX Dynamics constraint. Cannot move initialized constraints "
					"between bodies."),
				*Outer.GetName(), *GetLabelSafe(Outer.GetOwner()));
			return false;
		}

		Attachment.RigidBody.SetComponent(Body);
		return true;
	}

	void SetLocalLocation(
		FAGX_ConstraintBodyAttachment& Attachment, const FVector& LocalLocation,
		FConstraintBarrier& Barrier, int32 BodyIndex)
	{
		Attachment.LocalFrameLocation = LocalLocation;
		if (Barrier.HasNative())
		{
			/// \todo Need to compute what the location is relative to the constraint body for the
			/// cases where the FrameDefiningSource is anything other than
			/// EAGX_FrameDefiningSource::RigidBody.
			Barrier.SetLocalLocation(BodyIndex, LocalLocation);
		}
	}
}

bool UAGX_ConstraintComponent::SetBody1(UAGX_RigidBodyComponent* Body)
{
	if (Body == nullptr)
	{
		/// \todo Consider removing this print. The body may be set to nullptr before being set to
		/// the final body later. As long as it's set correctly before Begin Play it's all good.
		/// This commonly happens when a Blueprint has a Body member that is assigned to the
		/// Constraint in the Construction Script. While in the Blueprint Editor the Body will be
		/// nullptr.
		UE_LOG(
			LogAGX, Error,
			TEXT("Nullptr passed to SetBody1 on Constraint Component '%s' in Actor '%s'. The first "
				 "body must always be a valid body."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	return AGX_ConstraintComponent_helpers::SetBody(BodyAttachment1, Body, *this);
}

bool UAGX_ConstraintComponent::SetBody2(UAGX_RigidBodyComponent* Body)
{
	return AGX_ConstraintComponent_helpers::SetBody(BodyAttachment2, Body, *this);
}

void UAGX_ConstraintComponent::SetConstraintAttachmentLocation1(FVector LocalLocation)
{
	AGX_ConstraintComponent_helpers::SetLocalLocation(
		BodyAttachment1, LocalLocation, *NativeBarrier, 0);
}

void UAGX_ConstraintComponent::SetConstraintAttachmentLocation2(FVector LocalLocation)
{
	AGX_ConstraintComponent_helpers::SetLocalLocation(
		BodyAttachment2, LocalLocation, *NativeBarrier, 1);
}

void UAGX_ConstraintComponent::SetEnable(bool InEnabled)
{
	if (HasNative())
	{
		NativeBarrier->SetEnable(InEnabled);
	}
	bEnable = InEnabled;
}

bool UAGX_ConstraintComponent::GetEnable() const
{
	if (HasNative())
	{
		return NativeBarrier->GetEnable();
	}
	else
	{
		return bEnable;
	}
}

void UAGX_ConstraintComponent::SetEnableSelfCollision(bool InEnabled)
{
	bSelfCollision = InEnabled;

	if (!HasNative())
		return;

	UAGX_RigidBodyComponent* Body1 = BodyAttachment1.GetRigidBody();
	UAGX_RigidBodyComponent* Body2 = BodyAttachment2.GetRigidBody();
	if (Body1 == nullptr || Body2 == nullptr || !Body1->HasNative() || !Body2->HasNative())
		return;

	UAGX_Simulation::SetEnableCollision(*Body1, *Body2, InEnabled);
}

bool UAGX_ConstraintComponent::GetEnableSelfCollision() const
{
	return bSelfCollision;
}

namespace
{
	template <typename T>
	void SetOnBarrier(
		UAGX_ConstraintComponent& Component, EGenericDofIndex Index, const TCHAR* FunctionName,
		const T& Callback)
	{
		if (!Component.HasNative())
		{
			return;
		}

		int32 NativeDof;
		if (Component.ToNativeDof(Index, NativeDof))
		{
			Callback(NativeDof);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Invalid degree of freedom for constraint type %s passed to %s on "
					 "'%s' in '%s'."),
				*Component.GetClass()->GetName(), FunctionName, *Component.GetName(),
				(Component.GetOwner() != nullptr ? *Component.GetOwner()->GetName()
												 : TEXT("(null)")));
		}
	}

	template <typename T, typename Return>
	Return GetFromBarrier(
		const UAGX_ConstraintComponent& Component, EGenericDofIndex Index,
		const TCHAR* FunctionName, const Return& Fallback, const T& Callback)
	{
		if (!Component.HasNative())
		{
			return Fallback;
		}

		int32 NativeDof;
		if (Component.ToNativeDof(Index, NativeDof))
		{
			return Callback(NativeDof);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Invalid degree of freedom for constraint type %s passed to %s on "
					 "'%s' in '%s'."),
				*Component.GetClass()->GetName(), FunctionName, *Component.GetName(),
				(Component.GetOwner() != nullptr ? *Component.GetOwner()->GetName()
												 : TEXT("(null)")));
			return Fallback;
		}
	}
}

void UAGX_ConstraintComponent::SetComplianceFloat(EGenericDofIndex Index, float InCompliance)
{
	SetCompliance(Index, static_cast<double>(InCompliance));
}

void UAGX_ConstraintComponent::SetCompliance(EGenericDofIndex Index, double InCompliance)
{
	SetOnBarrier(
		*this, Index, TEXT("SetCompliance"), [this, InCompliance](int32 NativeDof)
		{ NativeBarrier->SetCompliance(InCompliance, NativeDof); });
	Compliance.Set(Index, InCompliance);
}

float UAGX_ConstraintComponent::GetComplianceFloat(EGenericDofIndex Index) const
{
	return static_cast<float>(GetCompliance(Index));
}

double UAGX_ConstraintComponent::GetCompliance(EGenericDofIndex Index) const
{
	return GetFromBarrier(
		*this, Index, TEXT("GetCompliance"), Compliance[Index],
		[this](int32 NativeDof) { return NativeBarrier->GetCompliance(NativeDof); });
}

void UAGX_ConstraintComponent::SetElasticityFloat(EGenericDofIndex Index, float InElasticity)
{
	SetElasticity(Index, static_cast<double>(InElasticity));
}

void UAGX_ConstraintComponent::SetElasticity(EGenericDofIndex Index, double InElasticity)
{
	SetCompliance(Index, 1.0 / InElasticity);
}

float UAGX_ConstraintComponent::GetElasticityFloat(EGenericDofIndex Index) const
{
	return static_cast<float>(GetElasticity(Index));
}

double UAGX_ConstraintComponent::GetElasticity(EGenericDofIndex Index) const
{
	return 1.0 / GetCompliance(Index);
}

void UAGX_ConstraintComponent::SetSpookDampingFloat(EGenericDofIndex Index, float InSpookDamping)
{
	SetSpookDamping(Index, static_cast<double>(InSpookDamping));
}

void UAGX_ConstraintComponent::SetSpookDamping(EGenericDofIndex Index, double InSpookDamping)
{
	SetOnBarrier(
		*this, Index, TEXT("SetSpookDamping"), [this, InSpookDamping](int32 NativeDof)
		{ NativeBarrier->SetSpookDamping(InSpookDamping, NativeDof); });
	SpookDamping.Set(Index, InSpookDamping);
}

float UAGX_ConstraintComponent::GetSpookDampingFloat(EGenericDofIndex Index) const
{
	return static_cast<float>(GetSpookDamping(Index));
}

double UAGX_ConstraintComponent::GetSpookDamping(EGenericDofIndex Index) const
{
	return GetFromBarrier(
		*this, Index, TEXT("GetSpookDamping"), SpookDamping[Index],
		[this](int32 NativeDof) { return NativeBarrier->GetSpookDamping(NativeDof); });
}

void UAGX_ConstraintComponent::SetForceRange(EGenericDofIndex Index, double Min, double Max)
{
	SetForceRange(Index, {Min, Max});
}

void UAGX_ConstraintComponent::SetForceRangeFloat(
	EGenericDofIndex Index, float RangeMin, float RangeMax)
{
	SetForceRange(
		Index, FAGX_RealInterval(static_cast<double>(RangeMin), static_cast<double>(RangeMax)));
}

void UAGX_ConstraintComponent::SetForceRange(
	EGenericDofIndex Index, const FAGX_RealInterval& InForceRange)
{
	SetOnBarrier(
		*this, Index, TEXT("SetForceRange"), [this, InForceRange](int32 NativeDof)
		{ NativeBarrier->SetForceRange(InForceRange.Min, InForceRange.Max, NativeDof); });
	ForceRange.Set(Index, InForceRange);
}

double UAGX_ConstraintComponent::GetForceRangeMin(EGenericDofIndex Index) const
{
	return GetForceRange(Index).Min;
}

float UAGX_ConstraintComponent::GetForceRangeMinFloat(EGenericDofIndex Index) const
{
	return static_cast<float>(GetForceRangeMin(Index));
}

double UAGX_ConstraintComponent::GetForceRangeMax(EGenericDofIndex Index) const
{
	return GetForceRange(Index).Max;
}

float UAGX_ConstraintComponent::GetForceRangeMaxFloat(EGenericDofIndex Index) const
{
	return static_cast<float>(GetForceRangeMax(Index));
}

void UAGX_ConstraintComponent::GetForceRange(EGenericDofIndex Index, double& Min, double& Max) const
{
	FAGX_RealInterval Range = GetForceRange(Index);
	Min = Range.Min;
	Max = Range.Max;
}


FAGX_RealInterval UAGX_ConstraintComponent::GetForceRange(EGenericDofIndex Index) const
{
	return GetFromBarrier(
		*this, Index, TEXT("GetForceRange"), ForceRange[Index],
		[this](int32 NativeDof) { return NativeBarrier->GetForceRange(NativeDof); });
}

void UAGX_ConstraintComponent::GetForceRangeFloat(
	EGenericDofIndex Index, float& Min, float& Max) const
{
	FAGX_RealInterval Range = GetForceRange(Index);
	Min = Range.Min;
	Max = Range.Max;
}

void UAGX_ConstraintComponent::SetComputeForces(bool bInComputeForces)
{
	if (HasNative())
	{
		NativeBarrier->SetEnableComputeForces(bInComputeForces);
	}

	bComputeForces = bInComputeForces;
}

bool UAGX_ConstraintComponent::GetComputeForces() const
{
	if (HasNative())
	{
		return NativeBarrier->GetEnableComputeForces();
	}
	else
	{
		return bComputeForces;
	}
}

void UAGX_ConstraintComponent::SetEnableComputeForces(bool bInEnable)
{
	SetComputeForces(bInEnable);
}

bool UAGX_ConstraintComponent::GetEnableComputeForces() const
{
	return GetComputeForces();
}

bool UAGX_ConstraintComponent::GetValid() const
{
	if (!HasNative())
	{
		return false;
	}

	return NativeBarrier->GetValid();
}

bool UAGX_ConstraintComponent::GetLastForceIndex(
	int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm) const
{
	if (!HasNative())
	{
		return false;
	}

	return NativeBarrier->GetLastForce(BodyIndex, OutForce, OutTorque, bForceAtCm);
}

bool UAGX_ConstraintComponent::GetLastForceBody(
	const UAGX_RigidBodyComponent* Body, FVector& OutForce, FVector& OutTorque,
	bool bForceAtCm) const
{
	if (Body == nullptr || !HasNative())
		return false;

	return NativeBarrier->GetLastForce(Body->GetNative(), OutForce, OutTorque, bForceAtCm);
}

bool UAGX_ConstraintComponent::GetLastLocalForceIndex(
	int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm) const
{
	if (!HasNative())
	{
		return false;
	}

	return NativeBarrier->GetLastLocalForce(BodyIndex, OutForce, OutTorque, bForceAtCm);
}

bool UAGX_ConstraintComponent::GetLastLocalForceBody(
	const UAGX_RigidBodyComponent* Body, FVector& OutForce, FVector& OutTorque,
	bool bForceAtCm) const
{
	if (!HasNative())
	{
		return false;
	}

	return NativeBarrier->GetLastLocalForce(Body->GetNative(), OutForce, OutTorque, bForceAtCm);
}

void UAGX_ConstraintComponent::CopyFrom(
	const FConstraintBarrier& Barrier, bool ForceOverwriteInstances)
{
	AGX_COPY_PROPERTY_FROM(ImportGuid, Barrier.GetGuid(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(bEnable, Barrier.GetEnable(), *this, ForceOverwriteInstances)
	EAGX_SolveType SolveTypeBarrier = static_cast<EAGX_SolveType>(Barrier.GetSolveType());
	AGX_COPY_PROPERTY_FROM(SolveType, SolveTypeBarrier, *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(
		bComputeForces, Barrier.GetEnableComputeForces(), *this, ForceOverwriteInstances);

	const static TArray<EGenericDofIndex> Dofs {
		EGenericDofIndex::Translational1, EGenericDofIndex::Translational2,
		EGenericDofIndex::Translational3, EGenericDofIndex::Rotational1,
		EGenericDofIndex::Rotational2,	  EGenericDofIndex::Rotational3};

	// Manually update archetype instances for properties that the AGX_COPY_PROPERTY_FROM macro
	// cannot handle.
	const FMergeSplitPropertiesBarrier Msp =
		FMergeSplitPropertiesBarrier::CreateFrom(*const_cast<FConstraintBarrier*>(&Barrier));
	if (FAGX_ObjectUtilities::IsTemplateComponent(*this))
	{
		for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*this))
		{
			// Compliance, Damping and Force Range.
			const bool ComplianceInSync = Instance->Compliance == Compliance;
			const bool SpookDampingInSync = Instance->SpookDamping == SpookDamping;
			const bool ForceRangeInSync = Instance->ForceRange == ForceRange;
			for (const auto& Dof : Dofs)
			{
				if (const int32* NativeDofPtr = NativeDofIndexMap.Find(Dof))
				{
					const int32 NativeDof = *NativeDofPtr;
					if (ForceOverwriteInstances || ComplianceInSync)
						Instance->Compliance[Dof] = Barrier.GetCompliance(NativeDof);

					if (ForceOverwriteInstances || SpookDampingInSync)
						Instance->SpookDamping[Dof] = Barrier.GetSpookDamping(NativeDof);

					if (ForceOverwriteInstances || ForceRangeInSync)
						Instance->ForceRange[Dof] = Barrier.GetForceRange(NativeDof);
				}
			}

			// Merge Split Properties.
			if (Msp.HasNative())
			{
				if (ForceOverwriteInstances ||
					Instance->MergeSplitProperties == MergeSplitProperties)
				{
					Instance->MergeSplitProperties.CopyFrom(Msp);
				}
			}
		}
	}

	// Finally, update this component for properties that the AGX_COPY_PROPERTY_FROM macro
	// cannot handle.
	for (const auto& Dof : Dofs)
	{
		if (const int32* NativeDofPtr = NativeDofIndexMap.Find(Dof))
		{
			const int32 NativeDof = *NativeDofPtr;
			Compliance[Dof] = Barrier.GetCompliance(NativeDof);
			SpookDamping[Dof] = Barrier.GetSpookDamping(NativeDof);
			ForceRange[Dof] = Barrier.GetForceRange(NativeDof);
		}
	}

	if (Msp.HasNative())
	{
		MergeSplitProperties.CopyFrom(Msp);
	}
}

void UAGX_ConstraintComponent::SetSolveType(EAGX_SolveType InSolveType)
{
	if (HasNative())
	{
		NativeBarrier->SetSolveType(InSolveType);
	}
	SolveType = InSolveType;
}

EAGX_SolveType UAGX_ConstraintComponent::GetSolveType() const
{
	if (HasNative())
	{
		/// \todo How should we do error checking here?
		return static_cast<EAGX_SolveType>(NativeBarrier->GetSolveType());
	}
	else
	{
		return SolveType;
	}
}

bool UAGX_ConstraintComponent::HasNative() const
{
	return NativeBarrier->HasNative();
}

uint64 UAGX_ConstraintComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier->GetNativeAddress());
}

void UAGX_ConstraintComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier->SetNativeAddress(static_cast<uintptr_t>(NativeAddress));

	if (HasNative())
	{
		MergeSplitProperties.BindBarrierToOwner(*GetNative());
	}
}

FConstraintBarrier* UAGX_ConstraintComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Constraint '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}

	return GetNative();
}

FConstraintBarrier* UAGX_ConstraintComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeBarrier.Get();
}

const FConstraintBarrier* UAGX_ConstraintComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeBarrier.Get();
}

bool UAGX_ConstraintComponent::AreFramesInViolatedState(float Tolerance, FString* OutMessage) const
{
	auto WriteMessage = [OutMessage](EDofFlag Dof, float Error)
	{
		if (OutMessage == nullptr)
		{
			return;
		}

		UEnum* DofEnum = FindObject<UEnum>(nullptr, TEXT("EDofFlag"));
		FString DofString = DofEnum->GetValueAsString(Dof);
		*OutMessage += FString::Printf(TEXT("%s has violation %f."), *DofString, Error);
	};

	FVector Location1 = BodyAttachment1.GetGlobalFrameLocation();
	FQuat Rotation1 = BodyAttachment1.GetGlobalFrameRotation();

	FVector Location2 = BodyAttachment2.GetGlobalFrameLocation();
	FQuat Rotation2 = BodyAttachment2.GetGlobalFrameRotation();

	FVector Location2InLocal1 = Rotation1.Inverse().RotateVector(Location2 - Location1);
	FQuat Rotation2InLocal1 = Rotation1.Inverse() * Rotation2;

	if (IsDofLocked(EDofFlag::DofFlagTranslational1))
	{
		if (FMath::Abs(Location2InLocal1.X) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagTranslational1, FMath::Abs(Location2InLocal1.X));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagTranslational2))
	{
		if (FMath::Abs(Location2InLocal1.Y) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagTranslational2, FMath::Abs(Location2InLocal1.Y));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagTranslational3))
	{
		if (FMath::Abs(Location2InLocal1.Z) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagTranslational3, FMath::Abs(Location2InLocal1.Z));
			return true;
		}
	}

	/// \todo Checks below might not be correct for ALL scenarios. What if there is for example a 90
	/// degrees rotation around one axis and then a rotation around another?

	if (IsDofLocked(EDofFlag::DofFlagRotational1))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisY().Z) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagRotational1, FMath::Abs(Rotation2InLocal1.GetAxisY().Z));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagRotational2))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisX().Z) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagRotational2, FMath::Abs(Rotation2InLocal1.GetAxisX().Z));
			return true;
		}
	}

	if (IsDofLocked(EDofFlag::DofFlagRotational3))
	{
		if (FMath::Abs(Rotation2InLocal1.GetAxisX().Y) > Tolerance)
		{
			WriteMessage(EDofFlag::DofFlagRotational3, FMath::Abs(Rotation2InLocal1.GetAxisX().Y));
			return true;
		}
	}
	return false;
}

EDofFlag UAGX_ConstraintComponent::GetLockedDofsBitmask() const
{
	return LockedDofsBitmask;
}

bool UAGX_ConstraintComponent::IsDofLocked(EDofFlag Dof) const
{
	return static_cast<uint8>(LockedDofsBitmask) & static_cast<uint8>(Dof);
}

bool UAGX_ConstraintComponent::IsRotational() const
{
	return !IsDofLocked(EDofFlag::DofFlagRotational1) ||
		   !IsDofLocked(EDofFlag::DofFlagRotational2) || !IsDofLocked(EDofFlag::DofFlagRotational3);
}

namespace
{
	FAGX_ConstraintBodyAttachment* SelectByName(
		const FName& Name, FAGX_ConstraintBodyAttachment* Attachment1,
		FAGX_ConstraintBodyAttachment* Attachment2)
	{
		if (Name == GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, BodyAttachment1))
		{
			return Attachment1;
		}
		else if (Name == GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, BodyAttachment2))
		{
			return Attachment2;
		}
		else
		{
			return nullptr;
		}
	}
}

#if WITH_EDITOR

void UAGX_ConstraintComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, bEnable),
		[](ThisClass* This) { This->SetEnable(This->bEnable); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, bSelfCollision),
		[](ThisClass* This) { This->SetEnableSelfCollision(This->bSelfCollision); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, SolveType),
		[](ThisClass* This) { This->SetSolveType(This->SolveType); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, Compliance),
		[](ThisClass* This) { This->UpdateNativeCompliance(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, SpookDamping),
		[](ThisClass* This) { This->UpdateNativeSpookDamping(); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, ForceRange),
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangePropertyPerDof, Translational_1),
		[](ThisClass* This) {
			This->SetForceRange(EGenericDofIndex::Translational1, This->ForceRange.Translational_1);
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, ForceRange),
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangePropertyPerDof, Translational_2),
		[](ThisClass* This) {
			This->SetForceRange(EGenericDofIndex::Translational2, This->ForceRange.Translational_2);
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, ForceRange),
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangePropertyPerDof, Translational_3),
		[](ThisClass* This) {
			This->SetForceRange(EGenericDofIndex::Translational3, This->ForceRange.Translational_3);
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, ForceRange),
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangePropertyPerDof, Rotational_1),
		[](ThisClass* This)
		{ This->SetForceRange(EGenericDofIndex::Rotational1, This->ForceRange.Rotational_1); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, ForceRange),
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangePropertyPerDof, Rotational_2),
		[](ThisClass* This)
		{ This->SetForceRange(EGenericDofIndex::Rotational2, This->ForceRange.Rotational_2); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, ForceRange),
		GET_MEMBER_NAME_CHECKED(FAGX_ConstraintRangePropertyPerDof, Rotational_3),
		[](ThisClass* This)
		{ This->SetForceRange(EGenericDofIndex::Rotational3, This->ForceRange.Rotational_3); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ConstraintComponent, bComputeForces),
		[](ThisClass* This) { This->SetComputeForces(This->bComputeForces); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, MergeSplitProperties),
		[](ThisClass* This) { This->MergeSplitProperties.OnPostEditChangeProperty(*This); });
}

void UAGX_ConstraintComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	// The root property that contains the property that was changed, i.e., the member of this
	// class.
	const FName Member = GetFNameSafe(PropertyChangedEvent.MemberProperty);

	// The leaf property that was changed. May be nested in a struct.
	const FName Property = GetFNameSafe(PropertyChangedEvent.Property);

	/// @todo Consider rewriting these as Property Dispatcher callbacks, for consistency.

	FAGX_ConstraintBodyAttachment* ModifiedBodyAttachment =
		SelectByName(Member, &BodyAttachment1, &BodyAttachment2);

	if (ModifiedBodyAttachment)
	{
		/// @todo Code below needs to be triggered also when modified through code!
		/// Editor-only probably OK though, since it is just for Editor convenience.
		/// See FCoreUObjectDelegates::OnObjectPropertyChanged.
		/// Or/additional add Refresh button to AAGX_ConstraintFrameActor's Details Panel
		/// that rebuilds the constraint usage list.

		/// @todo This property is three levels deep instead of two for the FrameDefiningActor.
		/// The middle layer is an FAGX_SceneComponentReference. Here we don't know if the
		/// property change happened in the ModifiedBodyAttachment's RigidBody or its
		/// FrameDefiningComponent. Assuming we have to update.
		/// Consider doing this in PostEditChangeChainProperty instead.
		if (Property == GET_MEMBER_NAME_CHECKED(FAGX_SceneComponentReference, OwningActor))
		{
			ModifiedBodyAttachment->OnFrameDefiningComponentChanged(this);
		}
		if (Property == GET_MEMBER_NAME_CHECKED(FAGX_SceneComponentReference, Name))
		{
			ModifiedBodyAttachment->OnFrameDefiningComponentChanged(this);
		}

		// When using Local Scope instead of piggy-backing on Owning Actor we don't need to do
		// anything special here. We can let Owning Actor become None / nullptr, Component Reference
		// will do the correct things as long as Local Scope hasn't been accidentally changed.
		AGX_ConstraintComponent_helpers::SetLocalScope(*this);
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UAGX_ConstraintComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif

void UAGX_ConstraintComponent::CreateMergeSplitProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_ConstraintComponent::CreateMergeSplitProperties was called "
				 "on Constraint '%s' that does not have a Native AGX Dynamics object. Only call "
				 "this function during play."),
			*GetName());
		return;
	}

	if (!MergeSplitProperties.HasNative())
	{
		MergeSplitProperties.CreateNative(*this);
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_ConstraintComponent::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

#define TRY_SET_DOF_VALUE(SourceStruct, GenericDof, Func)         \
	{                                                             \
		int32 Dof;                                                \
		if (ToNativeDof(GenericDof, Dof) && 0 <= Dof && Dof <= 5) \
			Func(SourceStruct[(int32) GenericDof], Dof);          \
	}

#define TRY_SET_DOF_RANGE_VALUE(SourceStruct, GenericDof, Func)                                    \
	{                                                                                              \
		int32 Dof;                                                                                 \
		if (ToNativeDof(GenericDof, Dof) && 0 <= Dof && Dof <= 5)                                  \
			Func(SourceStruct[(int32) GenericDof].Min, SourceStruct[(int32) GenericDof].Max, Dof); \
	}

void UAGX_ConstraintComponent::UpdateNativeProperties()
{
	if (!HasNative())
	{
		return;
	}

	NativeBarrier->SetEnable(bEnable);
	NativeBarrier->SetSolveType(SolveType);
	SetEnableSelfCollision(bSelfCollision);

	/// @todo Could just loop NativeDofIndexMap instead!!

	TRY_SET_DOF_VALUE(Compliance, EGenericDofIndex::Translational1, NativeBarrier->SetCompliance);
	TRY_SET_DOF_VALUE(Compliance, EGenericDofIndex::Translational2, NativeBarrier->SetCompliance);
	TRY_SET_DOF_VALUE(Compliance, EGenericDofIndex::Translational3, NativeBarrier->SetCompliance);
	TRY_SET_DOF_VALUE(Compliance, EGenericDofIndex::Rotational1, NativeBarrier->SetCompliance);
	TRY_SET_DOF_VALUE(Compliance, EGenericDofIndex::Rotational2, NativeBarrier->SetCompliance);
	TRY_SET_DOF_VALUE(Compliance, EGenericDofIndex::Rotational3, NativeBarrier->SetCompliance);

	TRY_SET_DOF_VALUE(
		SpookDamping, EGenericDofIndex::Translational1, NativeBarrier->SetSpookDamping);
	TRY_SET_DOF_VALUE(
		SpookDamping, EGenericDofIndex::Translational2, NativeBarrier->SetSpookDamping);
	TRY_SET_DOF_VALUE(
		SpookDamping, EGenericDofIndex::Translational3, NativeBarrier->SetSpookDamping);
	TRY_SET_DOF_VALUE(SpookDamping, EGenericDofIndex::Rotational1, NativeBarrier->SetSpookDamping);
	TRY_SET_DOF_VALUE(SpookDamping, EGenericDofIndex::Rotational2, NativeBarrier->SetSpookDamping);
	TRY_SET_DOF_VALUE(SpookDamping, EGenericDofIndex::Rotational3, NativeBarrier->SetSpookDamping);

	TRY_SET_DOF_RANGE_VALUE(
		ForceRange, EGenericDofIndex::Translational1, NativeBarrier->SetForceRange);
	TRY_SET_DOF_RANGE_VALUE(
		ForceRange, EGenericDofIndex::Translational2, NativeBarrier->SetForceRange);
	TRY_SET_DOF_RANGE_VALUE(
		ForceRange, EGenericDofIndex::Translational3, NativeBarrier->SetForceRange);
	TRY_SET_DOF_RANGE_VALUE(
		ForceRange, EGenericDofIndex::Rotational1, NativeBarrier->SetForceRange);
	TRY_SET_DOF_RANGE_VALUE(
		ForceRange, EGenericDofIndex::Rotational2, NativeBarrier->SetForceRange);
	TRY_SET_DOF_RANGE_VALUE(
		ForceRange, EGenericDofIndex::Rotational3, NativeBarrier->SetForceRange);

	NativeBarrier->SetEnableComputeForces(bComputeForces);
}

namespace
{
	template <typename FFunction>
	void UpdateNativePerDof(
		bool bHasNative, const TMap<EGenericDofIndex, int32>& NativeDofIndexMap,
		const FFunction& Callback)
	{
		if (!bHasNative)
		{
			return;
		}

		for (auto& Dof : NativeDofIndexMap)
		{
			EGenericDofIndex GenericDof = Dof.Key;
			int32 NativeDof = Dof.Value;
			if (GenericDof == EGenericDofIndex::AllDof)
			{
				continue;
			}
			Callback(GenericDof, NativeDof);
		}
	}
}

void UAGX_ConstraintComponent::UpdateNativeCompliance()
{
	UpdateNativePerDof(
		HasNative(), NativeDofIndexMap, [this](EGenericDofIndex GenericDof, int32 NativeDof)
		{ NativeBarrier->SetCompliance(Compliance[GenericDof], NativeDof); });
}

void UAGX_ConstraintComponent::UpdateNativeSpookDamping()
{
	UpdateNativePerDof(
		HasNative(), NativeDofIndexMap, [this](EGenericDofIndex GenericDof, int32 NativeDof)
		{ NativeBarrier->SetSpookDamping(SpookDamping[GenericDof], NativeDof); });
}

#undef TRY_SET_DOF_VAlUE
#undef TRY_SET_DOF_RANGE

#if WITH_EDITOR
void UAGX_ConstraintComponent::PostLoad()
{
	Super::PostLoad();
	BodyAttachment1.OnFrameDefiningComponentChanged(this);
	BodyAttachment2.OnFrameDefiningComponentChanged(this);
}

void UAGX_ConstraintComponent::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);
	BodyAttachment1.OnFrameDefiningComponentChanged(this);
	BodyAttachment2.OnFrameDefiningComponentChanged(this);
}

#endif

void UAGX_ConstraintComponent::OnRegister()
{
	Super::OnRegister();

	// On Register is called after all object initialization has completed, i.e. Unreal Engine
	// will not be messing with this object anymore. It is now safe to set the Local Scope on our
	// Component References.
	AGX_ConstraintComponent_helpers::SetLocalScope(*this);
}

void UAGX_ConstraintComponent::OnUnregister()
{
#if WITH_EDITOR
	BodyAttachment1.UnregisterFromConstraintFrameComponent(this);
	BodyAttachment2.UnregisterFromConstraintFrameComponent(this);
#endif

	Super::OnUnregister();
}

void UAGX_ConstraintComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		CreateNative();
		if (HasNative())
		{
			MergeSplitProperties.OnBeginPlay(*this);
		}
	}
}

void UAGX_ConstraintComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_ConstraintComponent will inherit this one's Native, so don't wreck it.
		// It's still safe to release the native since the Simulation will hold a reference if
		// necessary.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);
		}
	}

	if (HasNative())
	{
		NativeBarrier->ReleaseNative();
	}
}

void UAGX_ConstraintComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::ConstraintsStoreComplianceInsteadOfElasticity))
	{
		for (int32 I = 0; I < NumGenericDofs; ++I)
		{
			Compliance[I] = 1.0 / Elasticity_DEPRECATED[I];
		}
	}
}

bool UAGX_ConstraintComponent::ToNativeDof(EGenericDofIndex GenericDof, int32& NativeDof) const
{
	if (const int32* NativeDofPtr = NativeDofIndexMap.Find(GenericDof))
	{
		NativeDof = *NativeDofPtr;
		return true;
	}
	else
	{
		return false;
	}
}

void UAGX_ConstraintComponent::CreateNative()
{
	/// \todo Verify that we are in-game!

	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	CreateNativeImpl();
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error, TEXT("Constraint '%s' in '%s': Unable to create constraint."),
			*GetFName().ToString(), *GetOwner()->GetName());
		return;
	}

	NativeBarrier->SetName(GetName());

	if (!GetValid())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Constraint '%s' in '%s': Created invalid constraint. See the LogAGXDynamics "
				 "categoty in the Output Log."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	UpdateNativeProperties();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Constraint '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Simulation->Add(*this);
}
