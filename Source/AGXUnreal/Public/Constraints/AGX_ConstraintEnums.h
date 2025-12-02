// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/** Specifies in what solvers the constraint will be solved. */
UENUM()
enum EAGX_SolveType
{
	/** Solved only in the DIRECT solver. */
	StDirect = 1 UMETA(DisplayName = "Direct"),

	/** Solved only in the ITERATIVE solver. */
	StIterative = (1 << 2) UMETA(DisplayName = "Iterative"),

	/** Solved both in the ITERATIVE and the DIRECT solver. */
	StDirectAndIterative = (StDirect | StIterative) UMETA(DisplayName = "Direct & Iterative")
};

/** Specifies which Actor or Component is used to define the transform of the attachment frame of
 * the Constraint's Body Attachment. */
UENUM(BlueprintType)
enum EAGX_FrameDefiningSource
{
	InvalidSource = 0 UMETA(Hidden),

	/** The transformation of the Constraint itself is used to define the transform of the
	   attachment frame. */
	Constraint = 1 UMETA(DisplayName = "Constraint"),

	/** The transformation of the Rigid Body is used to define the transform of the attachment
	   frame. */
	RigidBody = 2 UMETA(DisplayName = "RigidBody"),

	/** The transformation of some other Actor or Component is used to define the transform of the
	   attachment frame. */
	Other = 3 UMETA(DisplayName = "Other")
};

/**
 * Constraint type independent index of Degree of Freedom(DOF). Does never change
 * index order layout, even in derived constraints, contrary to the AGX's native
 * constraint-specific DOF indexes.
 */
UENUM()
enum class EGenericDofIndex : uint8
{
	/** All degrees of freedom */
	AllDof = static_cast<uint8>(-1) UMETA(DisplayName = "All"),

	/** DOF for the first translational axis */
	Translational1 = 0 UMETA(DisplayName = "Translation1"),

	/** DOF for the second translational axis */
	Translational2 = 1 UMETA(DisplayName = "Translation2"),

	/** DOF for the third translational axis */
	Translational3 = 2 UMETA(DisplayName = "Translation3"),

	/** DOF corresponding to the first rotational axis */
	Rotational1 = 3 UMETA(DisplayName = "Rotation1"),

	/** DOF corresponding to the second rotational axis */
	Rotational2 = 4 UMETA(DisplayName = "Rotation2"),

	/** DOF for rotation around Z-axis */
	Rotational3 = 5 UMETA(DisplayName = "Rotation3"),

	NumDofs = 6 UMETA(Hidden)
};

static constexpr int32 NumGenericDofs = (int32) EGenericDofIndex::NumDofs;

/**
 * Flags used to be able to identify DOFs and combine them into a bitmask.
 */
UENUM(Meta = (Bitflags))
enum class EDofFlag : uint8
{
	DOF_FLAG_ALL = 0x3F UMETA(DisplayName = "All"),
	//	DOF_FLAG_NONE = 0 UMETA(DisplayName = "None"), /// \todo Consider adding this. When would it
	// be used?
	DofFlagTranslational1 = 1 << 0 UMETA(DisplayName = "Translation1"),
	DofFlagTranslational2 = 1 << 1 UMETA(DisplayName = "Translation2"),
	DofFlagTranslational3 = 1 << 2 UMETA(DisplayName = "Translation3"),
	DofFlagRotational1 = 1 << 3 UMETA(DisplayName = "Rotation1"),
	DofFlagRotational2 = 1 << 4 UMETA(DisplayName = "Rotation2"),
	DofFlagRotational3 = 1 << 5 UMETA(DisplayName = "Rotation3"),
};
