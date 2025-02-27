// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "Vehicle/AGX_TrackWheel.h"
#include "Vehicle/TrackBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_TrackComponent.generated.h"

class UAGX_ShapeMaterial;
class UAGX_TrackProperties;
class UAGX_TrackInternalMergeProperties;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class UStaticMesh;
struct FAGX_ImportContext;

/**
 * Object holding track node transforms and sizes generated before the actual simulation
 * has started, in order to visualize a preview of the track.
 */
class AGXUNREAL_API FAGX_TrackPreviewData
{
public:
	TArray<FTransform> NodeTransforms;
	TArray<FVector> NodeHalfExtents;
};

/**
 * Experimental
 * This feature is experimental and may change in the next release. We do not guarantee backwards
 * compatibility.
 *
 *
 * Given a set of wheels, automatically generates a continuous track with a given number of shoes,
 * also called Track Nodes. Each generated shoe become a separately simulated Rigid Body within
 * AGX Dynamics, connected together using constraints.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, LOD, Physics, Replication))
class AGXUNREAL_API UAGX_TrackComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_TrackComponent();

	/**
	 * Whether the AGX Dynamics Tracks should be created or not. Cannot be changed while playing.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bEnabled = true;

	/**
	 * Number of nodes in the track.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	int NumberOfNodes = 20;

	/**
	 * Width of the track nodes [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	float Width = 50.0f;

	/**
	 * Thickness of the track nodes [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	float Thickness = 15.0f;

	/**
	 * Value (distance) of how much shorter each node should be which causes tension
	 * in the system of tracks and wheels [cm].
	 *
	 * Since contacts and other factors are included it's not possible to know the exact
	 * tension after the system has been created.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	float InitialDistanceTension = 0.01f;

	/**
	 * Define the bulk and surface properties of each generated shoe.
	 *
	 * It is recommended to also create a Contact Material between the selected Shape Material and
	 * the Shape Materials set on the Shapes that the Track will drive on.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	UAGX_ShapeMaterial* ShapeMaterial;

	/**
	 * Additional properties defining the setup and behavior of the Track.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	UAGX_TrackProperties* TrackProperties;

	/**
	 * Properties controlling how the Rigid Bodies created for the Track shoes may be merged with
	 * each other.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	UAGX_TrackInternalMergeProperties* InternalMergeProperties;

	/**
	 * List of collision groups that the track nodes are part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	TArray<FName> CollisionGroups;

	/**
	 * The mass of the each track node Rigid Body [kg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track", Meta = (EditCondition = "!bAutoGenerateMass"))
	float NodeMass = 1.0f;

	/**
	 * Whether the track node mass should be computed automatically from the collision shape
	 * volume and material density.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bAutoGenerateMass = true;

	/**
	 * Center of mass offset [cm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track",
		Meta = (EditCondition = "!bAutoGenerateCenterOfMassOffset"))
	FVector NodeCenterOfMassOffset = FVector::ZeroVector;

	/**
	 * Whether the center of mass offset should be computed automatically.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bAutoGenerateCenterOfMassOffset = true;

	/**
	 * The three-component diagonal of the inertia tensor [kgm^2].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Track",
		Meta = (EditCondition = "!bAutoGeneratePrincipalInertia"))
	FVector NodePrincipalInertia = FVector::OneVector;

	/**
	 * Whether the principal inertia should be computed automatically.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track")
	bool bAutoGeneratePrincipalInertia = true;

	/**
	 * An array of track wheels that are used to route the track and determine interaction
	 * characteristics between the wheel geometry and the track nodes.
	 *
	 * At BeginPlay these nodes are used to initialize the track and after that the wheel objects
	 * aren't used anymore.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Wheels")
	TArray<FAGX_TrackWheel> Wheels;

	/**
	 * Whether to show debug visualization when the track is selected in the Editor (must be ejected
	 * from Pawn if playing).
	 *
	 * -- Track Nodes --
	 *
	 *  Rigid Body Frame:     Red, green, blue XYZ Axes (based at body position)
	 *  Collision Box:        Black Wire Box
	 *  Center Of Mass:       Purple Point [Play Only]
	 *  Hinge Rotation Axis:  White Arrow (based at hinge position) [Play Only]
	 *
	 *
	 * -- Wheels --
	 *
	 *  Radius And Frame:      Green Cylinder
	 *  Rotation Axis:         Green Arrow
	 *
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Debug Visual")
	bool bShowEditorDebugGraphics = true;

	/**
	 * Whether the debug graphics should colorize the collision boxes based on merged states
	 * (black means no merge).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Debug Visual")
	bool bColorizeMergedBodies = false;

	/**
	 * Whether this component should try to update the Track Preview (debug rendering and actual
	 * track rendering) automatically whenever a property changes.
	 *
	 * For manual update, click the 'Update Preview' from the Track Component's Details Panel,
	 * or the 'Update Visuals' button from the Track Renderer's Detail Panel.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Track Debug Visual")
	bool bAutoUpdateTrackPreview = true;

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	/**
	 * Event that broadcasts whenever the outside-of-play Track Preview Data has changed.
	 * Typically useful for a track rendering component that outside of Play want to update its
	 * track preview rendering only when something has changed.
	 */
	DECLARE_EVENT_OneParam(FLayerViewModel, FTrackPreviewNeedsUpdateEvent, UAGX_TrackComponent*)
		FTrackPreviewNeedsUpdateEvent& GetTrackPreviewNeedsUpdateEvent()
	{
		return TrackPreviewNeedsUpdateEvent;
	}

	void CopyFrom(const FTrackBarrier& Barrier, FAGX_ImportContext* Context);

	/**
	 * Get the number of nodes in this track.
	 *
	 * This is the same as the number of simulated Rigid Bodies during Play, and the number of
	 * preview nodes in the preview data during editing.
	 *
	 * During editing, i.e. not Play, this number is read from the Track Preview, if one is
	 * available. If no Track Preview is available then the NumberOfNodes property is returned.
	 * During Play, including Play In Editor, this will return the number of nodes created and
	 * simulated by AGX Dynamics.
	 *
	 * @return The number track nodes.
	 */
	int32 GetNumNodes() const;

	/**
	 * Get the transform of the center point of all track nodes.
	 *
	 * Scale is set to LocalScale and location is offset by LocalOffset * Rotation.
	 *
	 * During editing the node transforms are taken from the preview data. If no preview data is
	 * available then OutTransforms is emptied. During Play, including Play-In-Editor, the node
	 * transforms are read from the AGX Dynamics Track instance. Used for track rendering while
	 * playing.
	 *
	 * @param OutTransforms Array filled with one transform per node.
	 * @param LocalScale All transforms' Scale is set to this value.
	 * @param LocalOffset All transforms' Location is offset by this vector, rotated by each
	 * transforms' rotation.
	 */
	void GetNodeTransforms(
		TArray<FTransform>& OutTransforms, const FVector& LocalScale, const FVector& LocalOffset,
		const FQuat& LocalRotation) const;

	/**
	 * Get the sizes of all track nodes.
	 *
	 * Only valid to call during simulation after an AGX Dynamics Track has been created.
	 *
	 * This is the full size, not the half-extent, of each node.
	 */
	void GetNodeSizes(TArray<FVector>& OutNodeSizes) const;

	/**
	 * Get the size of a track node.
	 *
	 * If Index is out of bounds then the zero vector is returned.
	 *
	 * Only valid to call during simulation after an AGX Dynamics Track has been created.
	 *
	 * @param Index The index of the node to to get.
	 * @return The size of the node, or the zero vector if index is out of bounds.
	 * @see GetNumNodes
	 */
	FVector GetNodeSize(int32 Index) const;

	/**
	 * Returns a preview of the track node transforms and sizes. Should only be used when not
	 * playing.
	 * @param bForceUpdate Update preview data regardless of dirty flag.
	 */
	FAGX_TrackPreviewData* GetTrackPreview(bool bForceUpdate = false) const;

	///////////////////

	UFUNCTION(BlueprintCallable, Category = "AGX Track Visual")
	UInstancedStaticMeshComponent* GetVisualMeshes();

	/**
	 * The Static Mesh used to render each shoe the Track.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Visual")
	UStaticMesh* RenderMesh;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Visual")
	void SetRenderMesh(UStaticMesh* Mesh);

	/**
	 * The render material to apply to the visual Mesh.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Track Visual",
		Meta = (EditCondition = "RenderMesh != nullptr", FullyExpand = "true"))
	TArray<TObjectPtr<UMaterialInterface>> RenderMaterials;

	UFUNCTION(BlueprintCallable, Category = "AGX Track Visual")
	void SetRenderMaterial(int32 ElementIndex, UMaterialInterface* Material);

	/**
	 * Whether to automatically compute the Scale and Offset necessary to fit the visual Static
	 * Mesh's local bounds (defined by Local Mesh Bounds Min/Max) to the physical track node box.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Track Visual")
	bool bAutoScaleAndOffset {true};

	/**
	 * Local Rotation to apply to the visual Static Mesh before synchronizing its position and
	 * rotation with a track node [deg].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Track Visual")
	FRotator Rotation;

	/**
	 * Local Scale to apply to the visual Static Mesh before synchronizing its position and rotation
	 * with a track node. Scale is relative to the original mesh size, not to the track node.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Track Visual",
		Meta = (EditCondition = "!bAutoScaleAndOffset"))
	FVector Scale {FVector::OneVector};

	/**
	 * Local Translation to apply to the visual Static Mesh before synchronizing its position and
	 * rotation with a track node [cm].
	 *
	 * Applied after Scale.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Track Visual",
		Meta = (EditCondition = "!bAutoScaleAndOffset"))
	FVector Offset {FVector::ZeroVector};

	/**
	 * The max-point of the axis-aligned local box volume which should be fitted to the
	 * physical track node box when  auto-computing mesh scale and offset.
	 *
	 * Set this to the maximum local coordinate that the physical track node should cover.
	 * If it is desired that visual parts (e.g. teeth or the part overlapping the neighboring shoe)
	 * appear outside of the physical track node, then set this value to not cover those parts,
	 * i.e. a bit smaller than the local bounding box (spanning all vertices).
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Track Visual",
		Meta = (EditCondition = "bAutoScaleAndOffset"))
	FVector LocalMeshBoundsMax {FVector::OneVector * 50.0f};

	/**
	 * The min-point of the axis-aligned local box volume which should be fitted to the
	 * physical track node box when auto-computing mesh scale and offset.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Track Visual",
		Meta = (EditCondition = "bAutoScaleAndOffset"))
	FVector LocalMeshBoundsMin {-FVector::OneVector * 50.0f};

public:
	/**
	 * Call whenever a property etc that affects the track preview data has changed.
	 * Will set bUpdateIfNecessary to true, and broadcast the TrackPreviewNeedsUpdateEvent
	 * to inform external classes that if GetTrackPreview() is called again it will
	 * generate a new track preview based on recent property changes.
	 */
	void RaiseTrackPreviewNeedsUpdate(bool bDoNotBroadcastIfAlreadyRaised = true);

	/// Get the native AGX Dynamics representation of this track. Create it if necessary.
	FTrackBarrier* GetOrCreateNative();

	/// Return the native AGX Dynamics representation of this track. May return nullptr.
	FTrackBarrier* GetNative();

	const FTrackBarrier* GetNative() const;

	// ~Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeOwner interface.

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostDuplicate(bool bDuplicateForPIE) override;
	virtual void PostLoad() override;
	// ~End UObject interface.

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	void ApplyComponentInstanceData(
		const FActorComponentInstanceData* Data, ECacheApplyPhase CacheApplyPhase);
	virtual void OnRegister() override;
	virtual void DestroyComponent(bool bPromoteChildren) override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent Interface

	// ~Begin USceneComponent interface.
#if WITH_EDITOR
	virtual void OnUpdateTransform(
		EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
#endif

private:
#if WITH_EDITOR
	// Fill in a bunch of callbacks in PropertyDispatcher so we don't have to manually check each
	// and every Property in PostEditChangeProperty and PostEditChangeChainProperty.
	void InitPropertyDispatcher();
#endif

	// Find and set OwningActor of RigidBodyReferences and SceneComponentReferences which
	// in case this component is part of a Blueprint Actor.
	void SetComponentReferencesLocalScope();

	// Create the native AGX Dynamics object.
	void CreateNative();

	// Set ShapeMaterial assignment on native. Create native ShapeMaterial if not yet created.
	void UpdateNativeMaterial();

	// Set TrackProperties assignment on native. Create native TrackProperties if not yet created.
	void WriteTrackPropertiesToNative();

	// Write UAGX_TrackInternalMergeProperties properties to native.
	void WriteInternalMergePropertiesToNative();

	// Write mass, center of mass, inertia tensor, and auto-generation flags to native bodies.
	void WriteMassPropertiesToNative();

	/**
	 * Should be called whenever properties (excluding transform and shapes) need to be pushed
	 * onto the native in runtime. Writes all properties to native, except for those only
	 * used during initialization.
	 */
	void UpdateNativeProperties();

private:
	void CreateVisuals();
	void UpdateVisuals();
	bool ShouldRenderSelf() const;
	void SetVisualsInstanceCount(int32 Num);
	bool ComputeNodeTransforms(TArray<FTransform>& OutTransforms);
	bool ComputeVisualScaleAndOffset(
		FVector& OutVisualScale, FVector& OutVisualOffset, const FVector& PhysicsNodeSize) const;
	void WriteRenderMaterialsToVisualMesh();

#if WITH_EDITOR
	void WriteRenderMaterialsToVisualMeshWithCheck();
	void EnsureValidRenderMaterials();
#endif

private:
	// The AGX Dynamics object only exists while simulating.
	// Initialized in BeginPlay and released in EndPlay.
	FTrackBarrier NativeBarrier;

	TObjectPtr<UInstancedStaticMeshComponent> VisualMeshes;
	TArray<FTransform> NodeTransformsCache;
	TArray<FTransform> NodeTransformsCachePrev;

	mutable bool MayAttemptTrackPreview = false;

	mutable TSharedPtr<FAGX_TrackPreviewData> TrackPreview = nullptr;
	mutable bool bTrackPreviewNeedsUpdate = true;

	FTrackPreviewNeedsUpdateEvent TrackPreviewNeedsUpdateEvent;
};

/**
 * This struct's only purpose is to inform UAGX_TrackComponent when a Blueprint Reconstruction is
 * complete, i.e. when properties have been deserialized and instance data applied.
 *
 * It inherits FAGX_NativeOwnerInstanceData because UAGX_TrackComponent is a native owner.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TrackComponentInstanceData : public FAGX_NativeOwnerInstanceData
{
	GENERATED_BODY()

	FAGX_TrackComponentInstanceData() = default;
	FAGX_TrackComponentInstanceData(
		const IAGX_NativeOwner* NativeOwner, const USceneComponent* SourceComponent,
		TFunction<IAGX_NativeOwner*(UActorComponent*)> InDowncaster);

	virtual ~FAGX_TrackComponentInstanceData() override = default;

	virtual void ApplyToComponent(
		UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override;
};
