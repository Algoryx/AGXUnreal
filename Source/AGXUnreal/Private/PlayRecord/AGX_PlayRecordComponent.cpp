// Copyright 2025, Algoryx Simulation AB.

#include "PlayRecord/AGX_PlayRecordComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_Constraint2DofComponent.h"
#include "PlayRecord/AGX_PlayRecord.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Standard library includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"

#include <limits>

UAGX_PlayRecordComponent::UAGX_PlayRecordComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_PlayRecordComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentIndex = 0;
}

void UAGX_PlayRecordComponent::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

#if WITH_EDITOR
	if (PlayRecord != nullptr)
	{
		FAGX_ObjectUtilities::MarkAssetDirty(*PlayRecord);
	}
#endif
}

namespace AGX_PlayRecordComponent_helpers
{
	void AddDoubles(const double* Data, int32 Count, FAGX_PlayRecordState& OutState)
	{
		for (int32 I = 0; I < Count; ++I)
		{
			OutState.Values.Add(Data[I]);
		}
	}

	void GetDoubles(
		double* OutData, int32 Count, const FAGX_PlayRecordState& State, int32& OutIndex)
	{
		for (int32 I = 0; I < Count; ++I)
		{
			OutData[I] = State.Values[OutIndex++];
		}
	}

	void AddVector(const FVector& Data, FAGX_PlayRecordState& OutState)
	{
		for (int32 I = 0; I < 3; ++I)
		{
			OutState.Values.Add(Data[I]);
		}
	}

	FVector GetVector(const FAGX_PlayRecordState& State, int32& OutIndex)
	{
		FVector Result;
		for (int32 I = 0; I < 3; ++I)
		{
			Result[I] = State.Values[OutIndex++];
		}
		return Result;
	}

	void AddQuat(const FQuat& Data, FAGX_PlayRecordState& OutState)
	{
		OutState.Values.Add(Data.X);
		OutState.Values.Add(Data.Y);
		OutState.Values.Add(Data.Z);
		OutState.Values.Add(Data.W);
	}

	FQuat GetQuat(const FAGX_PlayRecordState& State, int32& OutIndex)
	{
		FQuat Result;
		Result.X = State.Values[OutIndex++];
		Result.Y = State.Values[OutIndex++];
		Result.Z = State.Values[OutIndex++];
		Result.W = State.Values[OutIndex++];
		return Result;
	}

	void RecordAngle(const UAGX_ConstraintComponent& Constraint, FAGX_PlayRecordState& OutState)
	{
		if (const UAGX_Constraint1DofComponent* Constraint1Dof =
				Cast<const UAGX_Constraint1DofComponent>(&Constraint))
		{
			OutState.Values.Add(Constraint1Dof->GetAngle());
			return;
		}

		if (const UAGX_Constraint2DofComponent* Constraint2Dof =
				Cast<const UAGX_Constraint2DofComponent>(&Constraint))
		{
			OutState.Values.Add(Constraint2Dof->GetAngle(EAGX_Constraint2DOFFreeDOF::FIRST));
			OutState.Values.Add(Constraint2Dof->GetAngle(EAGX_Constraint2DOFFreeDOF::SECOND));
			return;
		}

		UE_LOG(
			LogAGX, Warning, TEXT("RecordAngle was called with unsupported Constraint '%s'."),
			*Constraint.GetName());
	}

	void RecordRigidBody(const UAGX_RigidBodyComponent& RigidBody, FAGX_PlayRecordState& OutState)
	{
		const FVector Position = RigidBody.GetPosition();
		AddVector(Position, OutState);
		const FQuat Rotation = RigidBody.GetRotation();
		AddQuat(Rotation, OutState);
	}

	void PlayBackRigidBody(
		UAGX_RigidBodyComponent& RigidBody, const FAGX_PlayRecordState& State, double TimeStep,
		int32& OutIndex)
	{
		const FVector Position = GetVector(State, OutIndex);
		const FQuat Rotation = GetQuat(State, OutIndex);
		RigidBody.MoveTo(Position, Rotation, TimeStep);
	}

	void PrepareForPlayback(UAGX_ConstraintComponent& Constraint)
	{
		static constexpr auto INF = std::numeric_limits<double>::infinity();
		if (UAGX_Constraint1DofComponent* Constraint1Dof =
				Cast<UAGX_Constraint1DofComponent>(&Constraint))
		{
			Constraint1Dof->LockController.SetEnable(true);
			Constraint1Dof->ElectricMotorController.SetEnable(false);
			Constraint1Dof->FrictionController.SetEnable(false);
			Constraint1Dof->TargetSpeedController.SetEnable(false);
			Constraint1Dof->LockController.SetForceRange(FAGX_RealInterval(-INF, INF));
			Constraint1Dof->LockController.SetSpookDamping(0.0333); // Default value.
			Constraint1Dof->LockController.SetCompliance(1e-08); // Default value.
			return;
		}

		if (UAGX_Constraint2DofComponent* Constraint2Dof =
				Cast<UAGX_Constraint2DofComponent>(&Constraint))
		{
			Constraint2Dof->LockController1.SetEnable(true);
			Constraint2Dof->LockController2.SetEnable(true);
			Constraint2Dof->ElectricMotorController1.SetEnable(false);
			Constraint2Dof->ElectricMotorController2.SetEnable(false);
			Constraint2Dof->FrictionController1.SetEnable(false);
			Constraint2Dof->FrictionController2.SetEnable(false);
			Constraint2Dof->TargetSpeedController1.SetEnable(false);
			Constraint2Dof->TargetSpeedController2.SetEnable(false);
			Constraint2Dof->LockController1.SetForceRange(FAGX_RealInterval(-INF, INF));
			Constraint2Dof->LockController2.SetForceRange(FAGX_RealInterval(-INF, INF));
			Constraint2Dof->LockController1.SetSpookDamping(0.0333); // Default value.
			Constraint2Dof->LockController2.SetSpookDamping(0.0333); // Default value.
			Constraint2Dof->LockController1.SetCompliance(1e-08); // Default value.
			Constraint2Dof->LockController2.SetCompliance(1e-08); // Default value.
			return;
		}

		UE_LOG(
			LogAGX, Warning,
			TEXT("PrepareForPlayback was called with unsupported Constraint '%s'."),
			*Constraint.GetName());
	}

	void PrepareForPlayback(
		UAGX_RigidBodyComponent& Body,
		TMap<UAGX_RigidBodyComponent*, EAGX_MotionControl>& OriginalMotionControl)
	{
		OriginalMotionControl.Add(&Body, Body.GetMotionControl());
		Body.SetMotionControl(EAGX_MotionControl::MC_KINEMATICS);
	}

	void FinalizePlayback(
		UAGX_RigidBodyComponent& Body,
		TMap<UAGX_RigidBodyComponent*, EAGX_MotionControl>& OriginalMotionControl)
	{
		EAGX_MotionControl* MotionControl = OriginalMotionControl.Find(&Body);
		if (MotionControl == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Finalize Playback called on Rigid Body for which Prepare For Playback was "
					 "not called."));
			return;
		}

		Body.SetMotionControl(*MotionControl);
	}
}

void UAGX_PlayRecordComponent::RecordConstraintPositions(
	const TArray<UAGX_ConstraintComponent*>& Constraints)
{
	if (PlayRecord == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("RecordConstraintPositions was called on '%s' but the given Play Record Asset "
				 "is not set."),
			*GetName());
		return;
	}

	if (CurrentIndex == 0)
	{
		PlayRecord->States.Empty(InitialStatesAllocationSize);
	}

	const auto LastIndex = PlayRecord->States.Add(FAGX_PlayRecordState());
	AGX_CHECK(CurrentIndex == LastIndex);

	FAGX_PlayRecordState& State = PlayRecord->States.Last();
	for (const auto& Constraint : Constraints)
	{
		if (Constraint == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Constraint in RecordConstraintPositions. Constraint "
					 "position recording may not give the wanted result."),
				*GetName());
			continue;
		}

		AGX_PlayRecordComponent_helpers::RecordAngle(*Constraint, State);
	}

	CurrentIndex++;
}

void UAGX_PlayRecordComponent::RecordRigidBodyPositions(
	const TArray<UAGX_RigidBodyComponent*>& RigidBodies)
{
	using namespace AGX_PlayRecordComponent_helpers;

	if (PlayRecord == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("RecordRigidBodyPositions was called on '%s' but the given Play Record Asset is "
				 "not set."),
			*GetName());
		return;
	}

	if (CurrentIndex == 0)
	{
		PlayRecord->States.Empty(InitialStatesAllocationSize);
	}

	const auto LastIndex = PlayRecord->States.Add(FAGX_PlayRecordState());
	AGX_CHECK(CurrentIndex == LastIndex);

	FAGX_PlayRecordState& State = PlayRecord->States.Last();
	for (UAGX_RigidBodyComponent* Body : RigidBodies)
	{
		if (Body == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Rigid Body in RecordRigidBodyPositions. Rigid Body "
					 "position recording may not give the wanted result."),
				*GetName());
			continue;
		}

		RecordRigidBody(*Body, State);
	}

	CurrentIndex++;
}

void UAGX_PlayRecordComponent::PlayBackConstraintPositions(
	const TArray<UAGX_ConstraintComponent*>& Constraints)
{
	using namespace AGX_PlayRecordComponent_helpers;

	if (PlayRecord == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("PlayBackConstraintPositions was called on '%s' but the given Play Record Asset "
				 "is not set."),
			*GetName());
		return;
	}

	if (PlayRecord->States.Num() == 0)
		return;

	if (CurrentIndex >= PlayRecord->States.Num())
		return; // We have passed the end of the recording.

	if (CurrentIndex == 0)
	{
		// At the beginning of the recording.
		for (auto& Constraint : Constraints)
		{
			if (Constraint != nullptr)
				PrepareForPlayback(*Constraint);
		}
	}

	const int32 NumValuesInState = PlayRecord->States[CurrentIndex].Values.Num();
	auto SetPosition = [NumValuesInState, this](
						   FAGX_ConstraintLockController& LockController,
						   const FString& PlayRecordName, int32 ValueIndex)
	{
		if (ValueIndex >= NumValuesInState)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' was given Play Record with too few entries to control the given "
					 "Constraint, at PlayRecord index: %d."),
				*PlayRecordName, CurrentIndex);
			return;
		}

		LockController.SetPosition(PlayRecord->States[CurrentIndex].Values[ValueIndex]);
	};

	int32 CurrenDofIndex = 0;
	for (UAGX_ConstraintComponent* Constraint : Constraints)
	{
		if (Constraint == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Constraint in PlayBackConstraintPositions. Constraint "
					 "position playback may not give the wanted result."),
				*GetName());
			continue;
		}

		if (UAGX_Constraint1DofComponent* Constraint1Dof =
				Cast<UAGX_Constraint1DofComponent>(Constraint))
		{
			SetPosition(Constraint1Dof->LockController, GetName(), CurrenDofIndex);
			CurrenDofIndex++;
		}
		else if (
			UAGX_Constraint2DofComponent* Constraint2Dof =
				Cast<UAGX_Constraint2DofComponent>(Constraint))
		{
			SetPosition(Constraint2Dof->LockController1, GetName(), CurrenDofIndex);
			CurrenDofIndex++;
			SetPosition(Constraint2Dof->LockController2, GetName(), CurrenDofIndex);
			CurrenDofIndex++;
		}
	}

	CurrentIndex++;
}

void UAGX_PlayRecordComponent::PlayBackRigidBodyPositions(
	const TArray<UAGX_RigidBodyComponent*>& RigidBodies)
{
	using namespace AGX_PlayRecordComponent_helpers;

	if (PlayRecord == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("PlayBackRigidBodyPositions was called on '%s' but the given Play Record Asset is "
				 "not set."),
			*GetName());
		return;
	}

	if (PlayRecord->States.IsEmpty())
		return;

	if (CurrentIndex >= PlayRecord->States.Num())
	{
		if (CurrentIndex == PlayRecord->States.Num())
		{
			for (UAGX_RigidBodyComponent* Body : RigidBodies)
			{
				FinalizePlayback(*Body, OriginalMotionControl);
			}
			++CurrentIndex;
		}
		return; // We have passed the end of the recording.
	}

	if (CurrentIndex == 0)
	{
		// At the beginning of the recording.
		for (UAGX_RigidBodyComponent* Body : RigidBodies)
		{
			if (Body == nullptr)
				continue;
			PrepareForPlayback(*Body, OriginalMotionControl);
		}
	}

	const FAGX_PlayRecordState& State = PlayRecord->States[CurrentIndex];
	const int32 NumValuesInState = State.Values.Num();
	constexpr int32 BytesPerBody = 3 + 4; // FVector + FQuat.
	const int32 ExpectedNumBytes = BytesPerBody * RigidBodies.Num();
	if (NumValuesInState != ExpectedNumBytes)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' found recording state at index %d with incorrect number of data points. "
				 "Expected %d (bytes per body) * %d (num bodies) = %d but found %d. Skipping "
				 "this state."),
			*GetName(), CurrentIndex, BytesPerBody, RigidBodies.Num(), ExpectedNumBytes,
			NumValuesInState);
		++CurrentIndex;
		return;
	}

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("'%s' is part of a world that does not have an AGX Simulation. Cannot play back "
				 "Rigid Body recordings."),
				 *GetName());
		return;
	}

	const double TimeStep = Simulation->TimeStep;

	int32 ValuesIndex {0};
	for (UAGX_RigidBodyComponent* Body : RigidBodies)
	{
		if (Body == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("'%s' found nullptr Rigid Body in PlayBackRigidBodyPositions. Rigid Body "
					 "position playback may not give the wanted result."),
				*GetName());
			continue;
		}

		PlayBackRigidBody(*Body, State, TimeStep, ValuesIndex);
	}

	++CurrentIndex;
}

void UAGX_PlayRecordComponent::Reset()
{
	CurrentIndex = 0;
}
