// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ConstraintEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintPropertyPerDof.generated.h"

/**
 * A struct for a property that has one double component per DOF (Degree of Freedom).
 * Order indexes of DOFs below should match the order in the enum EGenericDofIndex.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintDoublePropertyPerDof
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_1_IsEditable"))
	FAGX_Real Translational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_2_IsEditable"))
	FAGX_Real Translational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_3_IsEditable"))
	FAGX_Real Translational_3;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_1_IsEditable"))
	FAGX_Real Rotational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_2_IsEditable"))
	FAGX_Real Rotational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_3_IsEditable"))
	FAGX_Real Rotational_3;

	FAGX_ConstraintDoublePropertyPerDof(
		double DefaultValue = 0.0, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		: Translational_1(DefaultValue)
		, Translational_2(DefaultValue)
		, Translational_3(DefaultValue)
		, Rotational_1(DefaultValue)
		, Rotational_2(DefaultValue)
		, Rotational_3(DefaultValue)
		, Translational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational1)
		, Translational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational2)
		, Translational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational3)
		, Rotational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational1)
		, Rotational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational2)
		, Rotational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational3)
	{
	}

	double operator[](int32 Index) const
	{
		switch (Index)
		{
			case 0:
				return Translational_1;
			case 1:
				return Translational_2;
			case 2:
				return Translational_3;
			case 3:
				return Rotational_1;
			case 4:
				return Rotational_2;
			case 5:
				return Rotational_3;
			default:
				checkNoEntry();
				return Translational_1; // Must return something, so arbitrarily pick a member.
		}
	}

	double operator[](EGenericDofIndex Index) const
	{
		return operator[](static_cast<int32>(Index));
	}

	double& operator[](int32 Index)
	{
		switch (Index)
		{
			case 0:
				return Translational_1;
			case 1:
				return Translational_2;
			case 2:
				return Translational_3;
			case 3:
				return Rotational_1;
			case 4:
				return Rotational_2;
			case 5:
				return Rotational_3;
			default:
				checkNoEntry();
				return Translational_1; // Must return something, so arbitrarily pick a member.
		}
	}

	double& operator[](EGenericDofIndex Index)
	{
		return operator[](static_cast<int32>(Index));
	}

	bool operator==(const FAGX_ConstraintDoublePropertyPerDof& Other) const
	{
		return Translational_1 == Other.Translational_1 &&
			   Translational_2 == Other.Translational_2 &&
			   Translational_3 == Other.Translational_3 && Rotational_1 == Other.Rotational_1 &&
			   Rotational_2 == Other.Rotational_2 && Rotational_3 == Other.Rotational_3;
	}

	void Set(EGenericDofIndex Index, double Value)
	{
		if (Index == EGenericDofIndex::AllDof)
		{
			SetAll(Value);
		}
		else
		{
			this->operator[](Index) = Value;
		}
	}

	void SetAll(double Value)
	{
		if (Translational_1_IsEditable)
		{
			Translational_1 = Value;
		}
		if (Translational_2_IsEditable)
		{
			Translational_2 = Value;
		}
		if (Translational_3_IsEditable)
		{
			Translational_3 = Value;
		}

		if (Rotational_1_IsEditable)
		{
			Rotational_1 = Value;
		}
		if (Rotational_2_IsEditable)
		{
			Rotational_2 = Value;
		}
		if (Rotational_3_IsEditable)
		{
			Rotational_3 = Value;
		}
	}

private:
	UPROPERTY(Transient)
	bool Translational_1_IsEditable;

	UPROPERTY(Transient)
	bool Translational_2_IsEditable;

	UPROPERTY(Transient)
	bool Translational_3_IsEditable;

	UPROPERTY(Transient)
	bool Rotational_1_IsEditable;

	UPROPERTY(Transient)
	bool Rotational_2_IsEditable;

	UPROPERTY(Transient)
	bool Rotational_3_IsEditable;
};

/**
 * A struct for a property that has one double range component per DOF (Degree Of Freedom).
 * Order indexes of DOFs below should match the order in the enum EGenericDofIndex.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangePropertyPerDof
{
	GENERATED_BODY()

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_1_IsEditable"))
	FAGX_RealInterval Translational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_2_IsEditable"))
	FAGX_RealInterval Translational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Translational_3_IsEditable"))
	FAGX_RealInterval Translational_3;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_1_IsEditable"))
	FAGX_RealInterval Rotational_1;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_2_IsEditable"))
	FAGX_RealInterval Rotational_2;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Property Per Dof",
		Meta = (EditCondition = "Rotational_3_IsEditable"))
	FAGX_RealInterval Rotational_3;

	FAGX_ConstraintRangePropertyPerDof(
		double DefaultMinValue = 0.0, double DefaultMaxValue = 0.0,
		EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		: Translational_1(DefaultMinValue, DefaultMaxValue)
		, Translational_2(DefaultMinValue, DefaultMaxValue)
		, Translational_3(DefaultMinValue, DefaultMaxValue)
		, Rotational_1(DefaultMinValue, DefaultMaxValue)
		, Rotational_2(DefaultMinValue, DefaultMaxValue)
		, Rotational_3(DefaultMinValue, DefaultMaxValue)
		, Translational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational1)
		, Translational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational2)
		, Translational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagTranslational3)
		, Rotational_1_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational1)
		, Rotational_2_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational2)
		, Rotational_3_IsEditable((uint8) EditableDofs & (uint8) EDofFlag::DofFlagRotational3)
	{
	}

	FAGX_ConstraintRangePropertyPerDof(
		FAGX_RealInterval DefaultInterval, EDofFlag EditableDofs = EDofFlag::DOF_FLAG_ALL)
		: FAGX_ConstraintRangePropertyPerDof(DefaultInterval.Min, DefaultInterval.Max, EditableDofs)
	{
	}

	FAGX_RealInterval& operator[](int32 Index)
	{
		switch (Index)
		{
			case 0:
				return Translational_1;
			case 1:
				return Translational_2;
			case 2:
				return Translational_3;
			case 3:
				return Rotational_1;
			case 4:
				return Rotational_2;
			case 5:
				return Rotational_3;
			default:
				checkNoEntry();
				return Translational_1; // Must return something, so arbitrarily pick a member.
		}
	}

	FAGX_RealInterval& operator[](EGenericDofIndex Index)
	{
		return operator[](static_cast<int32>(Index));
	};

	FAGX_RealInterval operator[](int32 Index) const
	{
		switch (Index)
		{
			case 0:
				return Translational_1;
			case 1:
				return Translational_2;
			case 2:
				return Translational_3;
			case 3:
				return Rotational_1;
			case 4:
				return Rotational_2;
			case 5:
				return Rotational_3;
			default:
				checkNoEntry();
				return Translational_1; // Must return something, so arbitrarily pick a member.
		}
	}

	FAGX_RealInterval operator[](EGenericDofIndex Index) const
	{
		return operator[](static_cast<int32>(Index));
	};

	bool operator==(const FAGX_ConstraintRangePropertyPerDof& Other) const
	{
		return Translational_1 == Other.Translational_1 &&
			   Translational_2 == Other.Translational_2 &&
			   Translational_3 == Other.Translational_3 && Rotational_1 == Other.Rotational_1 &&
			   Rotational_2 == Other.Rotational_2 && Rotational_3 == Other.Rotational_3;
	}

	void Set(EGenericDofIndex Index, FAGX_RealInterval Interval)
	{
		if (Index == EGenericDofIndex::AllDof)
		{
			SetAll(Interval);
		}
		else
		{
			this->operator[](Index) = Interval;
		}
	}

	void SetAll(FAGX_RealInterval Interval)
	{
		if (Translational_1_IsEditable)
		{
			Translational_1 = Interval;
		}
		if (Translational_2_IsEditable)
		{
			Translational_2 = Interval;
		}
		if (Translational_3_IsEditable)
		{
			Translational_3 = Interval;
		}

		if (Rotational_1_IsEditable)
		{
			Rotational_1 = Interval;
		}
		if (Rotational_2_IsEditable)
		{
			Rotational_2 = Interval;
		}
		if (Rotational_3_IsEditable)
		{
			Rotational_3 = Interval;
		}
	}

private:
	UPROPERTY(Transient)
	bool Translational_1_IsEditable;

	UPROPERTY(Transient)
	bool Translational_2_IsEditable;

	UPROPERTY(Transient)
	bool Translational_3_IsEditable;

	UPROPERTY(Transient)
	bool Rotational_1_IsEditable;

	UPROPERTY(Transient)
	bool Rotational_2_IsEditable;

	UPROPERTY(Transient)
	bool Rotational_3_IsEditable;
};
