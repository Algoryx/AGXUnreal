// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_WireRenderIterator.h"
#include "AMOR/AGX_WireMergeSplitProperties.h"
#include "Wire/AGX_WireEnums.h"
#include "Wire/AGX_WireRoutingNode.h"
#include "Wire/AGX_WireWinch.h"
#include "Wire/WireBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "Engine/EngineTypes.h"
#include "UObject/UObjectGlobals.h"

#include "AGX_WireComponent.generated.h"

class UAGX_ShapeMaterial;
class UAGX_WireWinchComponent;
class UInstancedStaticMeshComponent;
class UMaterialInterface;

/**
 * A Wire is a lumped element structure with dynamic resolution, the wire will adapt the resolution,
 * i.e., lumped element segment lengths, so that no unwanted vibrations will occur. The Wire
 * simulation state is initialized from a set of routing nodes that the user places but during
 * runtime simulation nodes will be created and removed as necessary so the routing nodes cannot be
 * used to inspect the current wire path. Instead use the render iterator to iterate over the wire,
 * which will give access to FAGX_WireNode instances, which wrap the underlying AGX Dynamics wire
 * simulation nodes.
 */
UCLASS(ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_WireComponent : public USceneComponent, public IAGX_NativeOwner
{
public:
	GENERATED_BODY()

public:
	UAGX_WireComponent();

	/*
	 * Wire settings.
	 */

	/**
	 * The radius of the wire [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
		Meta = (ClampMin = "0", UIMin = "0"))
	float Radius = 1.5f;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetRadius(float InRadius);

	/**
	 * The shortest a lumped segment is allowed to become [cm].
	 *
	 * Sets an upper bound on the number of simulation nodes a certain length of
	 * wire can consist of.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
		Meta = (ClampMin = "0", UIMin = "0"))
	float MinSegmentLength = 50.0f;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetMinSegmentLength(float InMinSegmentLength);

	/**
	 * Velocity damping value of the wire [kg/s].
	 *
	 * This damping will be applied to all bodies that make up the lumped element model of the
	 * simulated wire.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
		Meta = (ClampMin = "0", UIMin = "0"))
	float LinearVelocityDamping = 0.0f;

	// Not sure what this is, or if we should expose it.
	///**
	// * Value that indicates how likely it is that mass nodes appears along the wire. Higher value
	// * means more likely.
	// */
	// UPROPERTY(
	//	EditAnywhere, BlueprintReadWrite, Category = "AGX Wire",
	//	Meta = (ClampMin = "0", UIMin = "0"))
	// float ScaleConstant = 0.35f;

	/**
	 * Defines physical properties of the wire.
	 *
	 * This determines things such as the density of the wire and how it behaves when in contact
	 * with Shapes in the world.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Wire")
	UAGX_ShapeMaterial* ShapeMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);

	/**
	 * Toggle to enable or disable collision generation against this Wire.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Wire")
	bool bCanCollide {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetCanCollide(bool CanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool GetCanCollide() const;

	/**
	 * List of collision groups that this Wire Component is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Wire")
	TArray<FName> CollisionGroups;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void RemoveCollisionGroupIfExists(FName GroupName);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Wire")
	UMaterialInterface* RenderMaterial {nullptr};

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SetRenderMaterial(UMaterialInterface* Material);

	/*
	 * Begin winch.
	 */

	/// \todo Should this really be BlueprintReadWire?
	/// It should if the variable is for initialization only, and becomes inactive after Begin Play.
	/// It should not if changes to the variable should result in winch configuration changes during
	/// Play.
	///
	/// Should there be a difference between Details Panel changes and Blueprint Visual Script
	/// changes? I don't think so.
	///
	/// Connecting/disconnecting from a winch, or switching between winches, is a rather large
	/// operation so perhaps it should have a dedicated function.
	///
	/// The same applies for the end winch as well.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire Begin Winch")
	EWireWinchOwnerType BeginWinchType = EWireWinchOwnerType::None;

/// \todo Ties in with the \todo above. Should we have this function or not? Should it be allowed
/// during Play?
#if 0
	UFUNCTION(BlueprintCallable)
	void SetBeginWinchType(EWireWinchOwnerType Type);
#endif

	/**
	 * If BeginWinchType is set to Wire during Begin Play then the settings in this Wire Winch is
	 * used to configure an AGX Dynamics Wire Winch at the begin side of the wire, and modifications
	 * to this Wire Winch, such as enabling or disabling the motor, will be reflected in the
	 * simulation.
	 *
	 * To edit this property from a Blueprint Visual Script, get a handle to it by calling Get Owned
	 * Begin Winch.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Wire Begin Winch",
		Meta = (EditConditionHides, EditCondition = "BeginWinchType == EWireWinchOwnerType::Wire"))
	FAGX_WireWinch OwnedBeginWinch;

	UFUNCTION(
		BlueprintPure, Category = "AGX Wire Begin Winch",
		Meta = (DisplayName = "Get Owned Begin Winch"))
	FAGX_WireWinchRef GetOwnedBeginWinch_BP();

	/**
	 * @return True if the begin side of the wire is attached to the owned winch.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool HasOwnedBeginWinch() const;

	/// @todo The engine example, ULiveLinkComponentController, uses EditInstanceOnly here.
	/// Determine if that is a requirement or not. Possibly related to Blueprint Editor
	/// weirdness. FComponentReference is not supported by Blueprint, so we must provide some
	/// other way to set the target from a Blueprint Visual Script.
	/// We would like to use the UseComponentPicker Meta Specifier as well, but that crashes the
	/// editor. See internal issue 466.
	UPROPERTY(
		EditAnywhere, Category = "AGX Wire Begin Winch",
		Meta =
			(AllowedClasses = "/Script/AGXUnreal.AGX_WireWinchComponent", DisallowedClasses = "",
			 AllowAnyActor, EditConditionHides,
			 EditCondition = "BeginWinchType == EWireWinchOwnerType::WireWinch"))
	FComponentReference BeginWinchComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX AMOR")
	FAGX_WireMergeSplitProperties MergeSplitProperties;

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	void CreateMergeSplitProperties();

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool HasBeginWinchComponent() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	void SetBeginWinchComponent(UAGX_WireWinchComponent* Winch);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	UAGX_WireWinchComponent* GetBeginWinchComponent();

	const UAGX_WireWinchComponent* GetBeginWinchComponent() const;

	/**
	 * Get a pointer to the FAGX_WireWinch object owned by the Winch Component pointed to by
	 * BeginWinchComponent. Will return nullptr if BeginWinchComponent is unset or set to a Winch
	 * Component that doesn't exist.
	 *
	 * This is the Wire Winch that will be used at the begin side of the wire if BeginWinchType is
	 * set to Wire Winch.
	 *
	 * Note that this will point into a Component and that Components may be destroyed and/or
	 * recreated at any time. Don't store this pointer.
	 *
	 * @return The Wire Winch owned by the associated Wire Winch Component.
	 */
	FAGX_WireWinch* GetBeginWinchComponentWinch();
	const FAGX_WireWinch* GetBeginWinchComponentWinch() const;

	UFUNCTION(
		BlueprintPure, Category = "AGX Wire Begin Winch",
		Meta = (DisplayName = "Get Begin Winch Component Winch"))
	FAGX_WireWinchRef GetBeginWinchComponentWinch_BP();

	/// \todo Winch borrowing is just an idea, it might not work out.
	/**
	 * This is the Wire Winch that will be used when Begin Winch Type is set to Other. The code or
	 * Visual Script that sets Begin Winch Type to Other is fully responsible for this pointer and
	 * the FAGX_WireWinch that it points to.
	 */
	FAGX_WireWinch* BorrowedBeginWinch;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	void SetBorrowedBeginWinch(FAGX_WireWinchRef Winch);

	/**
	 * Determine if this wire has any type of winch at the begin side. The winch can be eiter
	 * owned by the wire, i.e., OwnedBeginWinch, owned by a Winch Component referenced through
	 * BeginWinchComponent, or a borrowed winch pointed to by BorrowedBeginWinch.
	 *
	 * For this to return true BeginWinchType must be set to something other than None and the
	 * Property corresponding to the Winch Type, as described in the previous paragraph, must be
	 * set.
	 *
	 * @return True if this wire has any type winch at the begin side.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool HasBeginWinch() const;

	/**
	 * Get the Wire Winch object that the begin side of this wire is attached to.
	 *
	 * Should only be called when HasBeginWinch returns true.
	 *
	 * This can be either the owned winch, a winch in a Wire Winch Component, or a borrowed begin
	 * winch, depending on the value of BeginWinchType, or nullptr if there is no winch at the begin
	 * side.
	 *
	 * @return The attached Wire Winch, or nullptr.
	 */
	FAGX_WireWinch* GetBeginWinch();
	const FAGX_WireWinch* GetBeginWinch() const;

	/**
	 * Get a reference to the Wire Winch object that the begin side of this wire is attached to.
	 *
	 * Should only be called when HasBeginWinch returns true.
	 *
	 * This can be either the owned winch, a winch in a Wire Winch Component, or a borrowed begin
	 * winch, depending on the value of BeginWinchType, ar an invalid winch reference if there is
	 * no winch at the begin side.
	 *
	 * @return The attached begin Wire Winch, or an invalid Wire Winch if there is no winch at the
	 * begin side.
	 */
	UFUNCTION(
		BlueprintPure, Category = "AGX Wire Begin Winch", Meta = (DisplayName = "Get Begin Winch"))
	FAGX_WireWinchRef GetBeginWinch_BP();

	/**
	 * Attach the begin side of the wire to the owned begin winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool AttachOwnedBeginWinch();

	/**
	 * Attach the begin side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	bool AttachBeginWinch(UAGX_WireWinchComponent* Winch);

	/**
	 * Attach the begin side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	bool AttachBeginWinch(FAGX_WireWinchRef Winch);

	/**
	 * Attach the begin side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool AttachBeginWinchToComponent(UAGX_WireWinchComponent* Winch);

	/**
	 * Attach the begin side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool AttachBeginWinchToOther(FAGX_WireWinchRef Winch);

	/**
	 * Detach the begin side of the wire from the winch it is currently attached to.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the detachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool DetachBeginWinch();

	/*
	 * End winch.
	 */

	/// \todo See \todo comment on BeginWinchType.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire End Winch")
	EWireWinchOwnerType EndWinchType = EWireWinchOwnerType::None;

	/// \todo Ties in with the \todo above. Should we have this function or not? Should it be
	/// allowed during Play?
#if 0
	UFUNCTION(BlueprintCallable)
	void SetEndWinchType(EWireWinchOwnerType Type);
#endif

	/**
	 * If EndWinchType is set to Wire during Begin Play then the settings in this Wire Winch is
	 * used to configure an AGX Dynamics Wire Winch at the end side of the wire, and modifications
	 * to this Wire Winch, such as enabling or disabling the motor, will be reflected in the
	 * simulation.
	 *
	 * To edit this property from a Blueprint Visual Script, get a handle to it by calling Get Owned
	 * End Winch.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Wire End Winch",
		Meta = (EditConditionHides, EditCondition = "EndWinchType == EWireWinchOwnerType::Wire"))
	FAGX_WireWinch OwnedEndWinch;

	UFUNCTION(
		BlueprintPure, Category = "AGX Wire End Winch",
		Meta = (DisplayName = "Get Owned End Winch"))
	FAGX_WireWinchRef GetOwnedEndWinch_BP();

	/**
	 * @return True if the end side of the wire is attached to the owned winch.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	bool HasOwnedEndWinch() const;

	/// @todo The engine example, ULiveLinkComponentController, uses EditInstanceOnly here.
	/// Determine if that is a requirement or not. Possibly related to Blueprint Editor
	/// weirdness. FComponentReference is not supported by Blueprint, so we must provide some
	/// other way to set the target from a Blueprint Visual Script.
	/// We would like to use the UseComponentPicker Meta Specifier as well, but that crashes the
	/// editor. See internal issue 466.
	UPROPERTY(
		EditAnywhere, Category = "AGX Wire End Winch",
		Meta =
			(AllowedClasses = "/Script/AGXUnreal.AGX_WireWinchComponent", DisallowedClasses = "",
			 AllowAnyActor, EditConditionHides,
			 EditCondition = "EndWinchType == EWireWinchOwnerType::WireWinch"))
	FComponentReference EndWinchComponent;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	bool HasEndWinchComponent() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	void SetEndWinchComponent(UAGX_WireWinchComponent* Winch);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	UAGX_WireWinchComponent* GetEndWinchComponent();

	const UAGX_WireWinchComponent* GetEndWinchComponent() const;

	/**
	 * Get a pointer to the FAGX_WireWinch object owned by the Winch Component pointed to by
	 * EndWinchComponent. Will return nullptr if EndWinchComponent is unset or set to a Winch
	 * Component that doesn't exist.
	 *
	 * This is the Wire Winch that will be used at the end side of the wire if EndWinchType is
	 * set to Wire Winch.
	 *
	 * Note that this will point into a Component and that Components may be destroyed and/or
	 * recreated at any time. Don't store this pointer.
	 *
	 * @return The Wire Winch owned by the associated Wire Winch Component.
	 */
	FAGX_WireWinch* GetEndWinchComponentWinch();
	const FAGX_WireWinch* GetEndWinchComponentWinch() const;

	UFUNCTION(
		BlueprintPure, Category = "AGX Wire End Winch",
		Meta = (DisplayName = "Get End Winch Component Winch"))
	FAGX_WireWinchRef GetEndWinchComponentWinch_BP();

	/// \todo Winch borrowing is just an idea, it might not work out.
	/**
	 * This is the Wire Winch that will be used when End Winch Type is set to Other. The code or
	 * Visual Script that sets End Winch Type to Other is fully responsible for this pointer and
	 * the FAGX_WireWinch that it points to.
	 */
	FAGX_WireWinch* BorrowedEndWinch;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	void SetBorrowedEndWinch(FAGX_WireWinchRef Winch);

	/**
	 * Determine if this wire has any type of winch at the End side. The winch can be eiter
	 * owned by the wire, i.e., OwnedEndWinch, owned by a Winch Component referenced through
	 * EndWinchComponent, or a borrowed winch pointed to by BorrowedEndWinch.
	 *
	 * For this to return true EndWinchType must be set to something other than None and the
	 * Property corresponding to the Winch Type, as described in the previous paragraph, must be
	 * set.
	 *
	 * @return True if this wire has any type winch at the End side.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	bool HasEndWinch() const;

	/**
	 * Get the Wire Winch object that the End side of this wire is attached to.
	 *
	 * Should only be called when HasEndWinch returns true.
	 *
	 * The returned winch can either be the owned winch, a winch in a Wire Winch Component, or a
	 * borrowed winch, depending on the value of EndWinchType.
	 *
	 * @return The attached Wire Winch, or nullptr.
	 */
	FAGX_WireWinch* GetEndWinch();
	const FAGX_WireWinch* GetEndWinch() const;

	UFUNCTION(
		BlueprintPure, Category = "AGX Wire End Winch", Meta = (DisplayName = "Get End Winch"))
	FAGX_WireWinchRef GetEndWinch_BP();

	/**
	 * Attach the End side of the wire to the owned end winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	bool AttachOwnedEndWinch();

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	bool AttachEndWinch(UAGX_WireWinchComponent* Winch);

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	bool AttachEndWinch(FAGX_WireWinchRef Winch);

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	bool AttachEndWinchToComponent(UAGX_WireWinchComponent* Winch);

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	bool AttachEndWinchToOther(FAGX_WireWinchRef Winch);

	/**
	 * Detach the end side of the wire from the winch it is currently attached to.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the detachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire End Winch")
	bool DetachEndWinch();

	/*
	 * Side-agnostic winch.
	 */

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	void SetWinchType(EWireWinchOwnerType Type, EWireSide Side);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	EWireWinchOwnerType GetWinchType(EWireSide Side) const;

	/// @return A pointer to the owned winch at the given side, regardless of whether that winch is
	/// active, i.e. owner set to owned, or not.
	FAGX_WireWinch* GetOwnedWinch(EWireSide Side);

	/// @return A pointer to the owned Wire Winch at the given side, regardless of whether that
	/// winch is active, i.e. owner set to owned, or not.
	const FAGX_WireWinch* GetOwnedWinch(EWireSide Side) const;

	/// @return The Wire owned winch, the one used when Winch Owner Type is set to Wire, on the give
	/// side.
	UFUNCTION(BlueprintPure, Category = "AGX Wire Winch", Meta = (DisplayName = "Get Owned Winch"))
	FAGX_WireWinchRef GetOwnedWinch_BP(EWireSide Side);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Begin Winch")
	bool SetBorrowedWinch(FAGX_WireWinchRef Winch, EWireSide Side);

	/// @return True if a Wire Winch exists at the given side with the owner type that has been
	/// configured for that side.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool HasWinch(EWireSide Side) const;

	/// @return The Wire Winch at the given side of the owner type configured for that side.
	FAGX_WireWinch* GetWinch(EWireSide Side);

	/// @return The Wire Winch at the given side of the owner type configured for that side.
	const FAGX_WireWinch* GetWinch(EWireSide Side) const;

	/// @return The Wire Winch at the given side of the owner type configured for that side.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch", Meta = (DisplayName = "Get Winch"))
	FAGX_WireWinchRef GetWinch_BP(EWireSide Side);

	/**
	 * Attach the End side of the wire to the owned end winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool AttachOwnedWinch(EWireSide Side);

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	bool AttachWinch(UAGX_WireWinchComponent* Winch, EWireSide Side);

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	bool AttachWinch(FAGX_WireWinchRef Winch, EWireSide Side);

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool AttachWinchToComponent(UAGX_WireWinchComponent* Winch, EWireSide Side);

	/**
	 * Attach the end side of the wire to the given Wire Winch.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the attachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool AttachWinchToOther(FAGX_WireWinchRef Winch, EWireSide Side);

	/**
	 * Detach the end side of the wire from the winch it is currently attached to.
	 *
	 * This can be done during runtime.
	 *
	 * @return True if the detachment was successful, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool DetachWinch(EWireSide Side);

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool SetWinchOwnerType(EWireSide Side, EWireWinchOwnerType Type);

	/// @return The owner type configured for the given side.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	EWireWinchOwnerType GetWinchOwnerType(EWireSide Side) const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Winch")
	bool SetWinchComponent(UAGX_WireWinchComponent* Winch, EWireSide Side);

	/// @return The Wire Winch Component set for the given side, if there is one, regardless of
	/// whether that winch is active, i.e. owner is set to Wire Winch,or not.
	UFUNCTION(BlueprintPure, Category = "AGX Wire Winch")
	UAGX_WireWinchComponent* GetWinchComponent(EWireSide Side);

	/// @return The Wire Winch Component set for the given side, if there is one, regardless of
	/// whether that winch is active, i.e. owner is set to Wire Winch, or not.
	const UAGX_WireWinchComponent* GetWinchComponent(EWireSide Side) const;

	/// @return A pointer to the Component Reference used to identity the Wire Winch Component at
	/// the given side.
	FComponentReference* GetWinchComponentReference(EWireSide Side);

	/// @return A pointer to the borrowed winch at the given side, regardless of whether that winch
	/// is active, i.e. owner is set to Other, or not.
	FAGX_WireWinch* GetBorrowedWinch(EWireSide Side);
	const FAGX_WireWinch* GetBorrowedWinch(EWireSide Side) const;

	/*
	 * Routing.
	 */

	/**
	 * An array of nodes that are used to initialize the wire.
	 *
	 * At BeginPlay these nodes are used to create simulation nodes and after that the route nodes
	 * aren't used anymore. Use the render iterator to track the motion of the wire over time.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire Route")
	TArray<FWireRoutingNode> RouteNodes;

	/**
	 * Create a new default-constructed routing node at the end of the wire.
	 */
	FWireRoutingNode& AddNode();

	/**
	 * Create a new default-constructed routing node at the end of the wire.
	 */
	FWireRoutingNode& AddNode(int32& OutIndex);

	/**
	 * Create a new default-constructed routing node at the end of the wire.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route", Meta = (DisplayName = "Add Node"))
	UPARAM(Ref) FWireRoutingNode& CreateNode(int32& OutIndex);
	// Extra Blueprint Callable function with different name because Blueprint does not support
	// function overloading.

	/**
	 * Add a new route node to the wire.
	 *
	 * This should be called before BeginPlay since route nodes are only used during initialization.
	 *
	 * @param InNode The node to add.
	 */
	FWireRoutingNode& AddNode(const FWireRoutingNode& InNode);

	/**
	 * Add a new route node to the wire.
	 *
	 * This should be called before BeginPlay since route nodes are only used during initialization.
	 *
	 * @param InNode The node to add.
	 * @param OutIndex The index in the Route Nodes array at which the new node was placed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	UPARAM(Ref) FWireRoutingNode& AddNode(const FWireRoutingNode& InNode, int32& OutIndex);

	/**
	 * Add a new route node at the end of the wire at the given location relative to the Wire
	 * Component.
	 *
	 * @param InLocation Location of the new route node relative to the Wire Component.
	 */
	FWireRoutingNode& AddNodeAtLocation(FVector InLocation);

	/**
	 * Add a new route node at the end of the wire at the given location relative to the Wire
	 * Component.
	 *
	 * @param InLocation The location of the node, relative to the Wire Component.
	 * @param OutIndex Index in Route Nodes where the new node is stored.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	UPARAM(Ref) FWireRoutingNode& AddNodeAtLocation(FVector InLocation, int32& OutIndex);

	/**
	 * Add a default-constructed route node at the designated index in the route array, pushing all
	 * subsequent nodes one index.
	 *
	 * The index must be either within the current Route Nodes array, or one-past end.
	 *
	 * @param InNode The route node to add.
	 * @param InIndex The place in the route node array to add the node at.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	UPARAM(Ref) FWireRoutingNode& AddNodeAtIndex(const FWireRoutingNode& InNode, int32 InIndex);

	/**
	 * Add a default-constructed route node, placed at the given local location, at the designated
	 * index in the route array, pushing all subsequent nodes one index.
	 *
	 * The index must be either within the current Route Nodes array, or one-past end.
	 *
	 * @param InLocation The location of the new node relative to the Wire Component.
	 * @param InIndex The place in the route node array to add the new node at.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	UPARAM(Ref)
	FWireRoutingNode& AddNodeAtLocationAtIndex(FVector InLocation, int32 InIndex);

	/**
	 * Overwrite the node at the given index with the new node.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	void SetNode(int32 InIndex, UPARAM(Ref) const FWireRoutingNode InNode);

	/**
	 * Remove the route node at the given index.
	 * @param InIndex The index of the node to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	void RemoveNode(int32 InIndex);

	/**
	 * Set the local location of a routing node.
	 *
	 * The local location is relative to the parent set on the node's Frame. If no parent has been
	 * set then the parent is assumed to be the Wire Component.
	 *
	 * @param InIndex The index of the node to modify.
	 * @param InLocation The new local location of the node
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	void SetNodeLocalLocation(int32 InIndex, FVector InLocation);

	/**
	 * Set the location of the node at the given index. The location is relative to the Wire
	 * Component. If the routing node has a parent then a local location relative to that parent is
	 * computed that places the routing node at the requested location relative to the Wire
	 * Component.
	 *
	 * Use SetNodeLocalLocation to position the node relative to its Frame parent.
	 *
	 * @param InIndex The index of the node to remove.
	 * @param InLocation The new local location for the node.
	 * @see SetNodeLocalLocation
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Route")
	void SetNodeLocation(int32 InIndex, FVector InLocation);

	/*
	 * State inspection.
	 */

	/**
	 * Determine if the given node is a lumped node or not. Lump nodes only exist during simulation
	 * so true can only be returned if an AGX Dynamics native has been created for this wire.
	 */
	bool IsLumpedNode(const FAGX_WireNode& Node);

	/**
	 * A wire is initialized when the AGX Dynamics object has been created and added to the AGX
	 * Dynamics simulation, which happens in BeginPlay. At this point that routing nodes become
	 * inactive and the render iterator should be used to inspect the simulation nodes.
	 *
	 * The initialization may fail, which will produce a wire for which HasNative is true but
	 * IsInitialized is false.
	 *
	 * @return True if the wire has been initialized.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool IsInitialized() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	double GetRestLength() const;


	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	double GetMass() const;


	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	double GetTension() const;


	/// @return True if this wire has at least one renderable simulation node.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool HasRenderNodes() const;

	/// @return True if there are no renderable simulation nodes.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	bool GetRenderListEmpty() const;

	/// @return An iterator pointing to the first renderable FAGX_WireNode simulation node.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	FAGX_WireRenderIterator GetRenderBeginIterator() const;

	/// @return An iterator pointing one-past-end of the renderable simulation nodes.
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	FAGX_WireRenderIterator GetRenderEndIterator() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	TArray<FVector> GetRenderNodeLocations() const;

#if WITH_EDITOR
	// Callback functions related to route node parents.
	void OnRouteNodeParentMoved(
		USceneComponent* Component, EUpdateTransformFlags UpdateTransformFlags,
		ETeleportType Teleport);
	void OnRouteNodeParentReplaced(const FCoreUObjectDelegates::FReplacementObjectMap& OldToNew);
#endif

	/**
	 * Mark visuals for this Wire Component dirty. The Visuals will be updated to match to the
	 * current wire state.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void MarkVisualsDirty();

#if WITH_EDITOR
	/**
	 * Find all route node parents and setup a Transform Updated callback on each so that the wire
	 * rendering is updated automatically when the parent is moved in the editor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire")
	void SynchronizeParentMovedCallbacks();

	/**
	 * In case a wire's in-editor mesh rendering doesn't match the route nodes, i.e. the
	 * lines-and-points rendering, then click this button to reset the render state and render
	 * update callbacks.
	 */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "AGX Wire Rendering")
	void SynchronizeRendering();
#endif

	/*
	 * Copy configuration from the given Barrier.
	 * Only the basic properties, such as Radius and MinSegmentLength, are copied. More complicated
	 * properties, such as winch setup and route nodes, must be handled elsewhere. During AGX
	 * Dynamics archive import those are handled by Sim Objects Importer Helper.
	 */
	void CopyFrom(const FWireBarrier& Barrier);

	//~ Begin IAGX_NativeOwner interface.
	/**
	 * @return True if a native AGX Dynamics representation has been created for this Wire
	 * Component.
	 */
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeOwner interface.

	/**
	 * Return the Barrier object for this Wire Component, creating it if necessary. Should only be
	 * called at or after BeginPlay has begun for the current level.
	 *
	 * @return The Barrier object for this Wire Component.
	 */
	FWireBarrier* GetOrCreateNative();

	/// @return The Barrier object for this Wire Component, or nullptr if there is none.
	FWireBarrier* GetNative();

	/// @return The Barrier object for this Wire Component, or nullptr if there is none.
	const FWireBarrier* GetNative() const;

	//~ Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
#if WITH_EDITOR
	using UActorComponent::PreEditChange;
	virtual void PreEditChange(FEditPropertyChain& PropertyAboutToChange) override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

	//~ Begin ActorComponent interface.
	virtual void BeginPlay() override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End ActorComponent interface.

	//~ Begin Scene Component interface.
#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
#endif
	//~ End Scene Component interface.

private: // Deprecated functions.

	UFUNCTION(
		BlueprintCallable, Category = "AGX Wire",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetRestLength instead of GetRestLength_BP"))
	float GetRestLength_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Wire",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use GetMass instead of GetMass_BP"))
	float GetMass_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Wire",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use GetTension instead of GetTension_BP"))
	float GetTension_BP() const;


protected:
	// ~Begin UActorComponent interface.
	virtual void OnRegister() override;
	virtual void DestroyComponent(bool bPromoteChildren) override;
	// ~End UActorComponent interface.

private:

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	void CreateNative();
	void CreateVisuals();
	bool UpdateNativeMaterial();

#if WITH_EDITOR
	bool DoesPropertyAffectVisuals(const FName& MemberPropertyName) const;
#endif

	TArray<FVector> GetNodesForRendering() const;
	bool ShouldRenderSelf() const;
	void UpdateVisuals();
	void RenderSelf(const TArray<FVector>& Points);
	void SetVisualsInstanceCount(int32 Num);

private:
	FWireBarrier NativeBarrier;
	TObjectPtr<UInstancedStaticMeshComponent> VisualCylinders;
	TObjectPtr<UInstancedStaticMeshComponent> VisualSpheres;

	/**
	 * Keep track which node frame parents we have registered a callback with. Note that a single
	 * entry here may correspond to multiple routing nodes. Must use a raw-pointer key to a
	 * weak-pointer values since TMap require that keys don't change, which a weak-pointer may do
	 * during garbage collection. We keep the weak pointer because the parent may be destroyed at
	 * any time and we need to be able to detect that.
	 */
	struct FParentDelegate
	{
		TWeakObjectPtr<USceneComponent> Parent;
		FDelegateHandle DelegateHandle;
	};
	TMap<USceneComponent*, FParentDelegate> DelegateHandles;

#if WITH_EDITOR
	/// Handle to the delegate registered with the engine Map Changed event to update visuals
	/// after load.
	FDelegateHandle MapLoadDelegateHandle;

	/// Handle to the delegate registered with the engine Objects Replaced event. Used to update
	/// Transform Updated callbacks on Scene Components that are a parent of a routing node.
	FDelegateHandle ObjectsReplacedDelegateHandle;
#endif
};
