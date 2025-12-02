// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyReference.h"
#include "AgxEdMode/AGX_AgxEdModeSubMode.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AGX_AgxEdModeConstraints.generated.h"

/**
 * Preset for setting which actor's transform should be used as attachment frames.
 */
UENUM()
enum class EAGX_ConstraintCreationFrameSource
{
	/** Both Attachment Frames share the Constraint Component's transform. */
	ConstraintTransform UMETA(DisplayName = "Constraint Component Transform (recommended)"),

	/** Both Attachment Frames share the transform of an auto-created Constraint Frame Actor. */
	OneSharedFrameActor UMETA(DisplayName = "Shared Frame Actor"),

	/** Each Attachment Frame uses the transform of a uniquely created Constraint Frame Actor. */
	TwoFrameActors UMETA(DisplayName = "Individual Frame Actors"),

	/** Both Attachment Frames share the transform of the first Rigid Body Actor. */
	RigidBodyActor1 UMETA(DisplayName = "Rigid Body Actor 1"),

	/** Both Attachment Frames share the transform of the second Rigid Body Actor. */
	RigidBodyActor2 UMETA(DisplayName = "Rigid Body Actor 2"),

	/** Each Attachment Frame uses the transform of its Rigid Body Actor owner, with configurable
	   offsets. */
	LocalOnly UMETA(DisplayName = "Local Transforms Only")
};

/**
 * Preset for setting which actor the constraint should be attached to in the scene hierarchy.
 */
UENUM()
enum class EAGX_ConstraintActorParent
{
	/** Constraint Actor is attached to the first Rigid Body Actor. */
	RigidBodyActor1,

	/** Constraint Actor is attached to the first Rigid Body Actor. */
	RigidBodyActor2,

	/** Constraint Actor is not attached to any actor. */
	None,
};

/**
 * Sub-mode for AgxEdMode. Used to create and manage constraints.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeConstraints : public UAGX_AgxEdModeSubMode
{
	GENERATED_BODY()

public:
	static UAGX_AgxEdModeConstraints* GetInstance();

public:
	virtual FText GetDisplayName() const override;
	virtual FText GetTooltip() const override;
	virtual FSlateIcon GetIcon() const override;

public: // Constraint Creator
	UPROPERTY()
	UClass* ConstraintType;

	/// \todo These BodyReferences cannot be the regular FAGX_RigidBodyReference kind because of:
	///
	///    Warning: Illegal TEXT reference to a private object in external package
	///        (BP_Blueprint_C
	///        /Game/Levels/OriginalLevel.OriginalLevel:PersistentLevel.BP_Blueprint_2)
	///    from referencer
	///        (AGX_AgxEdModeConstraints /Script/AGXUnrealEditor.Default__AGX_AgxEdModeConstraints).
	///    Import failed...
	///
	/// What I believe this says is that we cannot have a reference from a Class Default Object to
	/// an instance that lives in a level. It can be done with TSoftObjectPtr, so we should create
	/// a BodyReference variant that uses TSoftObjectPtr instead of regular UPROPERTY
	/// pointers.
	///
	///
	/// I'm not sure what the above comment means anymore. What exactly does work currently?
	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	FAGX_RigidBodyReference RigidBody1;

	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	FAGX_RigidBodyReference RigidBody2;

	/**
	 * Which actor in the scene hierarchy should the Constraint Actor be attached to?
	 *
	 * Attaching the constraint to an actor can be convienient if the actor is moved around
	 * a lot while editing, and the constraint should follow.
	 *
	 * This can be changed afterwards.
	 */
	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	EAGX_ConstraintActorParent ConstraintParent;

	/**
	 * Which object's transform should be used as constraint attachment frame
	 * for each Rigid Body?
	 *
	 * Note that this is just an initial setting, for convenience.
	 * Attachment Frame Actors and local offsets can be edited in detail afterwards
	 * from the constraint's Details Window.
	 */
	UPROPERTY(Transient, EditAnywhere, Category = "Constraint Creator")
	EAGX_ConstraintCreationFrameSource AttachmentFrameSource;

	/** Creates a new constraint using the current property values. */
	class AAGX_ConstraintActor* CreateConstraint() const;

public: // Constraint Browser
};
