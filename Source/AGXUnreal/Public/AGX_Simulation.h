// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_SimulationEnums.h"
#include "Contacts/AGX_ShapeContact.h"
#include "Contacts/AGX_ContactEnums.h"
#include "Contacts/ShapeContactBarrier.h"
#include "SimulationBarrier.h"

// Unreal Engine includes.
#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Framework/Commands/InputChord.h"
#include "Misc/EngineVersionComparison.h"
#include "Subsystems/GameInstanceSubsystem.h"

#include "AGX_Simulation.generated.h"

class AAGX_Stepper;
class AAGX_Terrain;
class UAGX_ConstraintComponent;
class UAGX_ContactMaterial;
class UAGX_RigidBodyComponent;
class UAGX_ShapeMaterial;
class UAGX_StaticMeshComponent;
class UAGX_ShapeComponent;
class UAGX_TireComponent;
class UAGX_WireComponent;

class AActor;
class UActorComponent;
class UWorld;
class FShapeBarrier;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPreStepForward, double, Time);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPostStepForward, double, Time);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPreStepForwardInternal, double /*Time*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostStepForwardInternal, double /*Time*/);

// The Keep Contact Policy parameter emulates a return value.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnImpact, double, Time, const FAGX_ShapeContact&, ShapeContact,
	const FAGX_KeepContactPolicyHandle&, KeepContactPolicy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnContact, double, Time, const FAGX_ShapeContact&, ShapeContact,
	const FAGX_KeepContactPolicyHandle&, KeepContactPolicy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
	FOnSeparation, double, TimeStamp, UAGX_ShapeComponent*, FirstShape, UAGX_ShapeComponent*,
	SecondShape);

/**
 * Manages an AGX simulation instance.
 *
 * When an instance of this class exists, native AGX objects relating to the
 * simulation can be created (RigidBody, Constraint, ContactMaterial, etc). When
 * this instance is destroyed, all those native objects must be destroyed.
 *
 * Lifetime is bound to a GameInstance. Therefore, each playing GameInstance
 * will have exactly one UAGX_Simulation instance.
 *
 * When not playing, the CDO (class default object) can be modified through the
 * Editor UI. From the toolbar select Settings -> Project Settings -> Plugins ->
 * AGX Dynamics. This is useful for setting simulation properties like time step
 * and contact materials. When a GameInstance is started, the properties of the
 * CDO will automatically be copied over by Unreal Engine to the GameInstance
 * specific UAGX_Simulation instance.
 *
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Config = Engine, DefaultConfig)
class AGXUNREAL_API UAGX_Simulation : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public: // Properties.
	/**
	 * Whether or not the number of AGX Dynamics worker threads should be set or be left at the
	 * AGX Dynamics default.
	 */
	UPROPERTY(
		Config, EditAnywhere, Category = "Overrides",
		Meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverrideNumThreads {true};

	/**
	 * The number of worker threads that AGX Dynamics will create and use during Play.
	 *
	 * Set to 0 to use all hardware threads.
	 *
	 * Disable this setting to let AGX Dynamics choose the number of worker threads.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Dynamics",
		Meta = (DisplayName = "Number of Threads", EditCondition = "bOverrideNumThreads"))
	int32 NumThreads = 4;

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	void SetNumThreads(int32 InNumThreads);

	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	int32 GetNumThreads() const;

	/**
	 * Step length of the integrator [s].
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Solver",
		Meta = (ClampMin = "0.001", UIMin = "0.001", ClampMax = "1.0", UIMax = "1.0"))
	double TimeStep = 1.0 / 60.0;
	// BlueprintReadOnly because will need to make UFunctions for proper Set/GetTimeStep.

	/**
	 * Contact warmstarting adds an extra step to the solver where the previous timesteps contact
	 * data is matched to the current timesteps data so the previous forces can be used by the
	 * solver to find a solution faster when direct friction is used.
	 *
	 * If using auto-generated ContactMaterials and default settings, enabling this is not
	 * recommended and will just add some overhead. But if a FrictionModel with DIRECT solvetype is
	 * used, enabling this is recommended.
	 *
	 * The state data used to match contacts for warmstarting is not serialized. This means that
	 * storing and then restoring the simulation can give slightly different trajectories compared
	 * to running the simulation non-stop.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Solver")
	bool bContactWarmstarting = false;

	/**
	 * Set to true to override the AGX Dynamics default value for the number of Parallel Projected
	 * Gauss-Seidel (PPGS) solver iterations. The number of iterations to use is set with Num PPGS
	 * Iterations.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Solver",
		Meta =
			(DisplayName = "Override PPGS Iterations", PinHiddenByDefault,
			 InlineEditConditionToggle))
	bool bOverridePPGSIterations = false;

	/**
	 * The number of solver resting iterations to use for the Parallel Projected Gauss-Seidel (PPGS)
	 * solver. This value influences the accuracy and computation cost of e.g. terrain particles.
	 *
	 * Disable this setting to let AGX Dynamics choose the number of PPGS iterations.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Solver",
		Meta =
			(ClampMin = 1, UIMin = 1, DisplayName = "Num PPGS Iterations",
			 EditCondition = "bOverridePPGSIterations"))
	int32 NumPpgsIterations = 25;

	/** Specifies the gravity model used by the simulation. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Gravity")
	TEnumAsByte<enum EAGX_GravityModel> GravityModel = EAGX_GravityModel::Uniform;

	UFUNCTION(BlueprintCallable, Category = "AGX Simulation")
	void SetGravityModel(EAGX_GravityModel InModel);

	/** Specifies the gravity vector when using Uniform Gravity Field with magnitude [cm/s^2]. */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Gravity",
		Meta = (EditCondition = "GravityModel == EAGX_GravityModel::Uniform"))
	FVector UniformGravity = FVector(0.0f, 0.0f, -980.665f);

	UFUNCTION(BlueprintCallable, Category = "AGX Simulation")
	void SetUniformGravity(FVector InGravity);

	/** Specifies the world location towards which the gravity field is directed when using Point
	 * Gravity Field [cm]. */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Gravity",
		Meta = (EditCondition = "GravityModel == EAGX_GravityModel::Point"))
	FVector PointGravityOrigin = FVector::ZeroVector;

	/** Specifies the gravity magnitude when using Point Gravity Field [cm/s^2]. */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Gravity",
		Meta = (EditCondition = "GravityModel == EAGX_GravityModel::Point"))
	float PointGravityMagnitude = -980.665f;

	UFUNCTION(BlueprintCallable, Category = "AGX Simulation")
	void SetPointGravity(FVector InOrigin, float InMagnitude);

	/**
	 * Simulation stepping mode. This controls what happens when the simulation is unable to keep
	 * up with real-time.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Simulation Stepping Mode")
	TEnumAsByte<enum EAGX_StepMode> StepMode = SmDropImmediately;

	/** Maximum time lag for the Catch up over time Capped step mode before dropping [s]. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Simulation Stepping Mode")
	double TimeLagCap = 1.0;

	/** Set to true to enable statistics gathering in AGX Dynamics. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Statistics")
	bool bEnableStatistics {true};

	/**
	 * Set to true to enable the contact event listener that triggers the On Impact and On Contact
	 * events in AGX Simulation. Enabling this is not necessary if you only use Contact Event
	 * Listener Components.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Simulation")
	bool bEnableGlobalContactEventListener {true};

	/**
	 * If enabled, whenever a Blueprint Asset from an imported OpenPLX file is deleted, the
	 * corresponding OpenPLX files located in Project/OpenPLXModels used by that Blueprint is
	 * deleted.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "OpenPLX",
		Meta = (DisplayName = "Delete OpenPLX File Copy on Blueprint Deletion"))
	bool bDeletePLXFileCopyOnBlueprintDeletion {true};

	/**
	 * Globally enable or disable AMOR (Merge Split Handler) in AGX Dynamics.
	 * Note that each RigidBody / Geometry / Wire / Constraint need to enable merge/split
	 * individually for AMOR to be used for those.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AGX AMOR")
	bool bEnableAMOR = false;

	/**
	 * Changes the default AMOR Shape Contact Merge Split Thresholds for all Rigid Bodies
	 * and Shapes that does not specify their own.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "AGX AMOR",
		Meta = (AllowedClasses = "/Script/AGXUnreal.AGX_ShapeContactMergeSplitThresholds"))
	FSoftObjectPath GlobalShapeContactMergeSplitThresholds;

	/**
	 * Changes the default AMOR Constraint Merge Split Thresholds for all Constraints
	 * that does not specify their own.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "AGX AMOR",
		Meta = (AllowedClasses = "/Script/AGXUnreal.AGX_ConstraintMergeSplitThresholds"))
	FSoftObjectPath GlobalConstraintMergeSplitThresholds;

	/**
	 * Changes the default AMOR Wire Merge Split Thresholds for all Wires
	 * that does not specify their own.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "AGX AMOR",
		Meta = (AllowedClasses = "/Script/AGXUnreal.AGX_WireMergeSplitThresholds"))
	FSoftObjectPath GlobalWireMergeSplitThresholds;

	/**
	 * Currently active GPU device index used for Lidar Raytracing (RTX).
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	int32 RaytraceDeviceIndex {0};

#if WITH_EDITORONLY_DATA

	/**
	 * Set to true to write an AGX Dynamics for Unreal archive of the initial state.
	 * The archive is written to the path set in ExportPath on the first game Tick.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Startup")
	bool bExportInitialState = false;

	/**
	 * Path to the file to which the initial AGX Dynamics state is written if ExportInitialState is
	 * set.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Startup",
		Meta = (EditCondition = "bExportInitialState"))
	FString ExportPath;
#endif

	UPROPERTY(
		Config, EditAnywhere, Category = "Overrides",
		Meta = (PinHiddenByDefault, InlineEditConditionToggle))
	bool bOverrideDynamicWireContacts {false};

	/**
	 * Set to true to force-enable dynamic wire contacts for all wire-on-shape collisions.
	 * By default dynamic wire contacts are enabled per Geometry using Wire Controller.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Startup",
		Meta = (EditCondition = "bOverrideDynamicWireContacts"))
	bool bEnableDynamicWireContacts {false};

	/**
	 * Remote debugging allows agxViewer, the default scene viewer in AGX
	 * Dynamics, to connect to the AGX_Simulation running inside Unreal Engine
	 * and render the internal simulation state using its built-in debug
	 * rendering capabilities.
	 *
	 * To connect to a running Unreal Engine instance launch agxViewer with
	 *    agxViewer -p --connect localhost:<PORT>
	 * where <PORT> is the port number configured in Project Settings > Plugins >  AGX Dynamics >
	 * Debug > RemoteDebuggingPort.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Debug")
	uint8 bRemoteDebugging : 1;

	/** Network port to use for remote debugging. */
	UPROPERTY(Config, EditAnywhere, Category = "Debug", Meta = (EditCondition = "bRemoteDebugging"))
	int16 RemoteDebuggingPort;

	/**
	 * Draws all Shape Contacts to the screen each Simulation time step.
	 * This can be helpful for quicly inspecting contact behaviours between objects in a Simulation.
	 * Note that this operation is computationally intensive, meaning it should be turned off
	 * whenever performance is important.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bDrawShapeContacts {false};

	/**
	 * Returns all Shape Contacts in the currently running Simulation.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Dynamics")
	TArray<FAGX_ShapeContact> GetShapeContacts() const;

	/**
	 * Maximum distance between the active Viewport camera and any AGX Constraint within which
	 * the AGX Constraint graphical representation is scaled such that it's size is constant as
	 * drawn on the screen [cm].
	 *
	 * When moving the camera further away than this distance the on-screen size of the constraint
	 * visualization will start to shrink along with the surrounding geometry.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Rendering",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	float ConstraintVisualizationScalingDistanceMax = 400.f;

public: // Member functions.
	UFUNCTION(BlueprintCallable, Category = "Solver")
	void SetEnableContactWarmstarting(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Solver")
	bool GetEnableContactWarmstarting() const;

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	void SetEnableAMOR(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	bool GetEnableAMOR();

	/**
	 * Whether or not to override each AGX Shape with the selected AdditionalUnrealCollision
	 * setting. If this is set to false, the setting on each AGX Shape is used instead.
	 */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Other")
	bool bOverrideAdditionalUnrealCollision {false};

	/**
	 * Keyboard shortcut to enable Grab Mode, making it possible to grab a Rigid Body in the
	 * viewport with the mouse.
	 * Note: if changed, the Unreal Editor must be restarted for the change to take effect.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Other")
	FInputChord GrabModeKeyboardShortcut {EKeys::Insert};

	/**
	 * Additional Unreal Collision Geometry to use for all AGX Shapes.
	 * Does not affect AGX Dynamics, but can be used to get support for e.g. LineTrace (Query) or
	 * use as a blocking volume against Chaos physics objects (Physics). Supported for all AGX
	 * primitive Shapes. Not supported for Trimesh Shapes.
	 * This is used for all primitive AGX Shapes if OverrideAdditionalUnrealCollision is set to
	 * true, otherwise the AdditionalUnrealCollision setting belonging to each AGX Shape is used.
	 */
	UPROPERTY(
		Config, EditAnywhere, BlueprintReadOnly, Category = "Other",
		Meta = (EditCondition = "bOverrideAdditionalUnrealCollision"))
	TEnumAsByte<enum ECollisionEnabled::Type> AdditionalUnrealCollision {
		ECollisionEnabled::QueryOnly};

	/**
	 * Set the number of solver resting iterations to use for the Parallel Projected Gauss-Seidel
	 * (PPGS) solver. This value influences the accuracy and computation cost of e.g. terrain
	 * particles.
	 * @param NumIterations The number of PPGS iterations to perform each solve.
	 */
	UFUNCTION(BlueprintCallable, Category = "Solver")
	void SetNumPpgsIterations(int32 NumIterations);

	/**
	 * \return The number of Parallel Projected Gauss-Seidel solver iterations to perform per
	 * solve.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Solver")
	int32 GetNumPpgsIterations();

	/**
	 * Get a collection of statistics from the AGX Dynamics simulation.
	 * These will likely be split into several functions in the future.
	 */
	UFUNCTION(BlueprintCallable, Category = "Statistics")
	FAGX_Statistics GetStatistics();

	/**
	 * Returns the current time within the AGX Dynamics simulation world.
	 *
	 * The returned value is usually the amount of time that has been simulated, but calls to
	 * SetTimeStamp will invalidate this.
	 *
	 * The AGX Dynamics time tries to follow the Unreal Engine world time, using the DeltaTime
	 * values passed to the Tick callbacks, but some of the Step Modes allow for the two times to
	 * diverge.
	 *
	 * @return The current simulation time stamp.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	double GetTimeStamp() const;

	/**
	 * Set the current simulation time stamp. Does not simulate to that time, just moves the clock
	 * hands.
	 * @param NewTimeStamp The new time stamp.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void SetTimeStamp(double NewTimeStamp);

	/**
	 * Delegate that is executed before each Simulation step forward.
	 * Users may bind to this delegate in order to get a callback before each Simulation step
	 * forward. This may be executed zero, one or several times per Unreal Engine Tick, depending on
	 * the Step Mode and Time Step selected in the AGX Dynamics for Unreal settings.
	 *
	 * Note: all bound callbacks to this delegate are cleared on Level Transition meaning that
	 * objects surviving a Level Transition that also are bound to this delegates must bind to it
	 * again in the new Level.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnPreStepForward PreStepForward;

	/**
	 * Delegate that is executed after each Simulation step forward.
	 * Users may bind to this delegate in order to get a callback after each Simulation step
	 * forward. This may be executed zero, one or several times per Unreal Engine Tick, depending on
	 * the Step Mode and Time Step selected in the AGX Dynamics for Unreal settings.
	 *
	 * Note: all bound callbacks to this delegate are cleared on Level Transition meaning that
	 * objects surviving a Level Transition that also are bound to this delegates must bind to it
	 * again in the new Level.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnPostStepForward PostStepForward;

	/**
	 * Event that is triggered by AGX Dynamics during Step Forward after collision detection but
	 * before solve. An impact is any overlap that is new this time step, i.e. the two Shapes were
	 * not overlapping in the previous time step.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnImpact OnImpact;

	/**
	 * Event that is triggered by AGX Dynamics during Step Forward after collision detection but
	 * before solve. A contact is any overlap that is not new this time step, i.e. the two Shapes
	 * were overlapping in the previous time step.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnContact OnContact;

	/**
	 * Event that is triggered by AGX Dynamics during Step Forward after collision detection but
	 * before solve. A separation is when two Shapes are no longer overlapping, i.e. the two Shapes
	 * were overlapping in the previous time step but not this time step.
	 */
	UPROPERTY(BlueprintAssignable, Category = "Simulation")
	FOnSeparation OnSeparation;

	void Add(UAGX_ConstraintComponent& Constraint);

	/**
	 * Note that Shapes that are child of the passed Rigid Body are NOT added to the simulation
	 * when calling this function.
	 */
	void Add(UAGX_RigidBodyComponent& Body);
	void Add(UAGX_ShapeComponent& Shape);
	void Add(UAGX_ShapeMaterial& Shape);
	void Add(UAGX_StaticMeshComponent& Body);
	void Add(AAGX_Terrain& Terrain);
	void Add(UAGX_TireComponent& Tire);
	void Add(UAGX_WireComponent& Wire);

	void Remove(UAGX_ConstraintComponent& Constraint);
	void Remove(UAGX_RigidBodyComponent& Body);
	void Remove(UAGX_ShapeComponent& Shape);
	void Remove(UAGX_ShapeMaterial& Shape);
	void Remove(UAGX_StaticMeshComponent& Body);
	void Remove(AAGX_Terrain& Terrain);
	void Remove(UAGX_TireComponent& Tire);
	void Remove(UAGX_WireComponent& Wire);

	void Register(UAGX_ContactMaterial& Material);
	void Unregister(UAGX_ContactMaterial& Material);

	void SetEnableCollisionGroupPair(const FName& Group1, const FName& Group2, bool CanCollide);

	static void SetEnableCollision(
		UAGX_RigidBodyComponent& Body1, UAGX_RigidBodyComponent& Body2, bool Enable);

	UFUNCTION(BlueprintCallable, BlueprintPure = False, Category = "Simulation")
	bool WriteAGXArchive(const FString& Filename) const;

	bool HasNative() const;

	FSimulationBarrier* GetNative();
	const FSimulationBarrier* GetNative() const;

	/**
	 * Perform a number of steps, possibly zero, in response to the elapsed Unreal Engine time
	 * according to the rules of the selected Step Mode.
	 *
	 * This member function is typically called automatically by an AAGX_Stepper instance. If you
	 * need precise control over stepping then set the Step Mode to None and call StepOnce to
	 * perform a step.
	 *
	 * @param DeltaTime The Unreal Engine time that has passed since the last frame.
	 */
	void Step(double DeltaTime);

	/**
	 * Step the AGX Dynamics simulation once. Typically used with the 'None' Step Mode to have full
	 * control over when the simulation is stepped. Does not do any delta time tracking so mixing
	 * automatic frame stepping, i.e. Step Mode != None, and StepOnce may step more than intended.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void StepOnce();

	/**
	 * Set true to integrate positions at the start of the timestep rather than at the end.
	 * Set false to integrate positions at the end of the timestep.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	bool SetPreIntegratePositions(bool Enable);

	/**
	 * Returns true if set to integrate positions at the start of the timestep.
	 * Returns false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	bool GetPreIntegratePositions() const;

	static UAGX_Simulation* GetFrom(const UActorComponent* Component);

	static UAGX_Simulation* GetFrom(const AActor* Actor);

	static UAGX_Simulation* GetFrom(const UWorld* World);

	static UAGX_Simulation* GetFrom(const UGameInstance* GameInstance);

	TArray<FShapeContactBarrier> GetShapeContacts(const FShapeBarrier& Shape) const;

#if WITH_EDITOR
	virtual bool CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
		const UProperty* InProperty
#else
		const FProperty* InProperty
#endif
	) const override;
#endif

	void CreateNative();

	/**
	 * This is called automatically once an object is added to this Simulation. For scenarios where
	 * no objects with an AGX Dynamics representation exists in the Level, but a stepping Simulation
	 * object is still required, this function can be called manually, for example in Begin Play.
	 */
	void EnsureStepperCreated();

	friend class AAGX_Stepper;

private:
	// ~Begin USubsystem interface.
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End USubsystem interface.

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	// ~End UObject interface.

	void InitPropertyDispatcher();
#endif

private:
	/**
	 * Releases the AGX Native object and also clears all bound Pre/Post StepForward delegates.
	 * Objects surviving this Level Transition that also are bound to any of these delegates must
	 * bind to it again in the new Level.
	 */
	void OnLevelTransition();

	int32 StepCatchUpImmediately(double DeltaTime);
	int32 StepCatchUpOverTime(double DeltaTime);
	int32 StepCatchUpOverTimeCapped(double DeltaTime);
	int32 StepDropImmediately(double DeltaTime);

	void PreStep();
	void PostStep();

	/**
	 * Called by AGX Dynamics when two Shapes first touch, if Enable Global Contact Event Listener
	 * is true. Triggers the On Impact delegate.
	 *
	 * @param TimeStamp AGX Dynamics simulation time stamp.
	 * @param Contact Contact information between two Shapes.
	 * @return What to do with the contact.
	 */
	EAGX_KeepContactPolicy ImpactCallback(double TimeStamp, FShapeContactBarrier& Contact);

	/**
	 * Called by AGX Dynamics on time steps after two Shapes first touch, if still in contact and if
	 * Enable Global Contact Event Listener is true. Triggers the On Contact delegate.
	 *
	 * @param TimeStamp AGX Dynamics simulation time stamp.
	 * @param Contact Contact information between two Shapes.
	 * @return What to do with the contact.
	 */
	EAGX_KeepContactPolicy ContactCallback(double TimeStamp, FShapeContactBarrier& Contact);

	/**
	 * Called by AGX Dynamics when two Shapes no longer overlap, if Enable Global Contact Event
	 * Listener is true. Triggers the On Separation delegate.
	 *
	 * @param TimeStamp AGX Dynamics simulation time stamp.
	 * @param FirstShape The Shape no longer overlapping with Second Shape.
	 * @param SecondShape The Shape no longer overlapping with First Shape.
	 */
	void SeparationCallback(
		double TimeStamp, FAnyShapeBarrier& FirstShape, FAnyShapeBarrier& SecondShape);

	void EnsureValidLicense();

	void SetGravity();

	void SetGlobalNativeMergeSplitThresholds();

	void ReleaseNative();

private:
	FSimulationBarrier NativeBarrier;

	/// Time that we couldn't step because DeltaTime was not an even multiple
	/// of the AGX Dynamics step size. That fraction of a time step is carried
	/// over to the next call to Step.
	double LeftoverTime;

	// The time it took to do a frame's stepping the last frame we actually took a step.
	double LastTotalStepTime {0.0};

	TWeakObjectPtr<AAGX_Stepper> Stepper;

	// Record for keeping track of the number of times any Contact Material has been
	// registered/unregistered. Value is incremented on Register() and decremented on Unregister().
	TMap<UAGX_ContactMaterial*, int32> ContactMaterials;

	// Internal post/pre step forward delegates.
	FOnPreStepForwardInternal PreStepForwardInternal;
	FOnPostStepForwardInternal PostStepForwardInternal;

	friend class FAGX_InternalDelegateAccessor;
};
