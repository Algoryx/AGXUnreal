// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_ContactMaterialEnums.h"
#include "Materials/AGX_ContactMaterialMechanicsApproach.h"
#include "Materials/AGX_ContactMaterialReductionMode.h"
#include "Materials/ContactMaterialBarrier.h"

// Unreal Engine includes.
#include "UObject/Object.h"

#include "AGX_ContactMaterial.generated.h"

class UAGX_ContactMaterialRegistrarComponent;
class UAGX_ShapeMaterial;
struct FAGX_ImportContext;

/**
 * Defines material properties for contacts between Shapes with specific Shape Materials. This
 * will override many of their individual material properties (does for example not override ones
 * affecting mass, such as density).
 *
 * Contact Materials are created by the user in-Editor by creating an UAGX_ContactMaterial asset.
 * In-Editor they are treated as assets and can be referenced by a Contact Material Registrar
 * Component.
 *
 * When game begins playing, one UAGX_ContactMaterial instance will be created for each
 * UAGX_ContactMaterial asset that is referenced by a Contact Material Registrar Component. The
 * UAGX_ContactMaterial instance will create the actual native AGX Contact Material and add it to
 * the simulation. The in-game Contact Material Registrar Component that referenced the
 * UAGX_ContactMaterial asset will swap its reference to the in-game created
 * UAGX_ContactMaterial instance instead. This means that ultimately only
 * UAGX_ContactMaterial instances will be referenced in-game. When play stops the in-Editor state
 * will be returned automatically by the Unreal Editor itself.
 *
 * Note that this means that any UAGX_ContactMaterial assets that are not referenced a Contact
 * Material Registrar Component will be inactive.
 *
 * Note also that it is not allowed to replace the Materials properties after instance has been
 * created.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_ContactMaterial : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * First material.
	 */
	UPROPERTY(EditAnywhere, Category = "Materials")
	UAGX_ShapeMaterial* Material1 {nullptr};

	/**
	 * Second material. Can be same as first material.
	 */
	UPROPERTY(EditAnywhere, Category = "Materials")
	UAGX_ShapeMaterial* Material2 {nullptr};

	/**
	 * Solvers to use to calculate the normal and friction equations when objects with this contact
	 * material collide.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	EAGX_ContactSolver ContactSolver {EAGX_ContactSolver::Split};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetContactSolver(EAGX_ContactSolver InContactSolver);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	EAGX_ContactSolver GetContactSolver() const;

	/**
	 * Whether contact reduction should be enabled and to what extent.
	 *
	 * By using contact reduction, the number of contact points later submitted to the solver as
	 * contact constraint can be heavily reduced, hence improving performance.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	FAGX_ContactMaterialReductionMode ContactReduction;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetContactReductionMode(EAGX_ContactReductionMode InContactReductionMode);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	EAGX_ContactReductionMode GetContactReductionMode() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetContactReductionLevel(EAGX_ContactReductionLevel InContactReductionLevel);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	EAGX_ContactReductionLevel GetContactReductionLevel() const;

	/**
	 * AGX use by default a contact point based method for calculating the corresponding response
	 * between two overlapping geometries.
	 *
	 * There is also a different method available which is named area contact approach. This method
	 * will try to calculate the spanning area between the overlapping contact point. This can
	 * result in a better approximation of the actual overlapping volume and the stiffness in the
	 * response (contact constraint).
	 *
	 * In general, this will lead to slightly less stiff, more realistic contacts and therefore the
	 * Young’s modulus value usually has to be increased to get a similar simulation results as
	 * running the simulation without the contact area approach.
	 */
	UPROPERTY(EditAnywhere, Category = "Contacts Processing")
	FAGX_ContactMaterialMechanicsApproach MechanicsApproach;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetUseContactAreaApproach(bool bInUseContactAreaApproach);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool GetUseContactAreaApproach();

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetMinElasticRestLength(double InMinLength);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetMinElasticRestLength() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetMaxElasticRestLength(double InMaxLength);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetMaxElasticRestLength() const;

	/**
	 * The friction model used when two objects with this contact material collides.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction")
	EAGX_FrictionModel FrictionModel {EAGX_FrictionModel::IterativeProjectedConeFriction};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetFrictionModel(EAGX_FrictionModel InFrictionModel);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	EAGX_FrictionModel GetFrictionModel() const;

	/**
	 * Constant normal force used by the friction model 'Constant Normal Force Box Friction' [N].
	 *
	 * This should be set to an estimation of the force, in Newtons, by which the two colliding
	 * objects are being pushed together. If the main contributor to this force is gravity then
	 * this value should be set to the mass of the upper object and any additional load it is
	 * carrying times the gravitational acceleration in m/s^2.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(EditCondition =
				 "FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"))
	FAGX_Real NormalForceMagnitude {100.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetNormalForceMagnitude(double InNormalForceMagnitude);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetNormalForceMagnitude() const;

	/**
	 * Whether the 'Normal Force Magnitude' should be scaled by contact point depth.
	 * Only used by friction model 'Constant Normal Force Box Friction'.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(EditCondition =
				 "FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"))
	bool bScaleNormalForceWithDepth {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetScaleNormalForceWithDepth(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool GetScaleNormalForceWithDepth() const;

	/**
	 * Whether surface friction should be calculated in the solver for this Contact Material.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction")
	bool bEnableSurfaceFriction {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetSurfaceFrictionEnabled(bool bInEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool GetSurfaceFrictionEnabled();

	/**
	 * Friction in all directions if 'Secondary Friction Coefficient' is disabled, else only in the
	 * primary direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real FrictionCoefficient {0.25 / (2 * 0.3)};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetFrictionCoefficient(double InFrictionCoefficient);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetFrictionCoefficient() const;

	/**
	 * Friction in the secondary direction, if enabled.
	 *
	 * Only used by Oriented Friction Models.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(ClampMin = "0.0", UIMin = "0.0",
			 // We would like to include a check for oriented friction model here, but Unreal
			 // Engine 4.26 doesn't support that in combination with InlineEditConditionToggle on
			 // bUseSecondaryFrictionCoefficient.
			 EditCondition = "bUseSecondaryFrictionCoefficient"))
	FAGX_Real SecondaryFrictionCoefficient {0.25 / (2 * 0.3)};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetSecondaryFrictionCoefficient(double InFrictionCoefficient);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetSecondaryFrictionCoefficient() const;

	/**
	 * Whether it should be possible to define friction coefficient per each of the two
	 * perpendicular surface direction.
	 *
	 * If enabled, 'Friction Coefficient' represents the primary direction and 'Secondary Friction
	 * Coefficient' represents the secondary direction.
	 *
	 * If disable, 'Friction Coefficient' represents all directions and 'Secondary Friction
	 * Coefficient' is not used.
	 *
	 * Note that secondary direction friction coefficient is only used by Oriented Friction Models.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (InlineEditConditionToggle))
	bool bUseSecondaryFrictionCoefficient {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetUseSecondaryFrictionCoefficient(bool bInUseSecondaryFrictionCoefficient);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool GetUseSecondaryFrictionCoefficient() const;

	/**
	 * Surface viscosity, telling how 'wet' the friction is between the colliding materials.
	 *
	 * Represents all surface directions if 'Secondary Surface Viscosity' is disable, else only in
	 * the primary direction.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SurfaceViscosity {5.0e-9};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetSurfaceViscosity(double InSurfaceViscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetSurfaceViscosity() const;

	/**
	 * Surface viscosity in the secondary direction, if enabled.
	 *
	 * Only used by Oriented Friction Models.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
			(ClampMin = "0.0", UIMin = "0.0",
			 // We would like to include a check for oriented friction model here, but Unreal
			 // Engine 4.26 doesn't support that in combination with InlineEditConditionToggle on
			 // bUseSecondarySurfaceViscosity.
			 EditCondition = "bUseSecondarySurfaceViscosity"))
	FAGX_Real SecondarySurfaceViscosity {5.0e-9};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetSecondarySurfaceViscosity(double InSecondarySurfaceViscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetSecondarySurfaceViscosity() const;

	/**
	 * Whether it should be possible to define surface viscosity per each of the two perpendicular
	 * surface direction.
	 *
	 * If enabled, 'Surface Viscosity' represents the primary direction and 'Secondary Surface
	 * Viscosity' represents the secondary direction.
	 *
	 * If disable, 'Surface Viscosity' represents all directions and 'Secondary Surface Viscosity'
	 * is not used.
	 *
	 * Note that secondary direction surface viscosity is only used by Oriented Friction Models.
	 */
	UPROPERTY(EditAnywhere, Category = "Friction", Meta = (InlineEditConditionToggle))
	bool bUseSecondarySurfaceViscosity {false};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetUseSecondarySurfaceViscosity(bool bInUseSecondarySurfaceViscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool GetUseSecondarySurfaceViscosity() const;

	// clang-format off
	// Unreal Header Tool doesn't support line breaks in EditCondition.
	/**
	 * Primary friction/viscosity direction relative to Reference Frame.
	 * Secondary direction will be perpendicular to primary direction.
	 *
	 * Only used by Oriented Friction Models.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
		(EditCondition =
			"FrictionModel == EAGX_FrictionModel::OrientedBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedScaledBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedIterativeProjectedConeFriction || FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"
		))
	FVector PrimaryDirection{FVector::ForwardVector};
	// clang-format on

	/**
	 * Set the primary, or forward, direction of this Contact Material.
	 *
	 * For an oriented friction model this is the direction that uses the regular friction and
	 * surface viscosity parameters. The secondary direction, a vector in the friction plane and
	 * perpendicular to the primary direction, uses the secondary friction and surface viscosity,
	 * if either or both of them are enabled.
	 *
	 * The primary direction is only used by the oriented friction models.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetPrimaryDirection(FVector InPrimaryDirection);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	FVector GetPrimaryDirection() const;

	// clang-format off
	// Unreal Header Tool doesn't support line breaks in EditCondition.
	/**
	 * The name of the actor that contains the component to use as reference frame for a
	 * Oriented Friction Model (component specified by the property
	 * OrientedFrictionReferenceFrameComponent).
	 *
	 * If this name is left empty, the reference frame component is supposed to exist
	 * in the same actor as the ContactMaterialRegistrarComponent that owns this contact material.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
		(EditCondition =
			"FrictionModel == EAGX_FrictionModel::OrientedBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedScaledBoxFriction  || FrictionModel == EAGX_FrictionModel::OrientedIterativeProjectedConeFriction || FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"
		))
	FName OrientedFrictionReferenceFrameActor{NAME_None};
	// clang-format on

	// clang-format off
	// Unreal Header Tool doesn't support line breaks in EditCondition.
	/**
	 * The component whose transform should be used as Reference Frame for a Oriented Friction
	 * Model.
	 *
	 * The component must be a Rigid Body Component.
	 *
	 * The component must exist in the actor specified by the property
	 * OrientedFrictionReferenceFrameActor, or if the actor is not specified the component is
	 * supposed to exist in the same actor as the ContactMaterialRegistrarComponent that owns this
	 * contact material.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Friction",
		Meta =
		(EditCondition =
			"FrictionModel == EAGX_FrictionModel::OrientedBoxFriction || FrictionModel == EAGX_FrictionModel::OrientedScaledBoxFriction || FrictionModel == EAGX_FrictionModel::OrientedIterativeProjectedConeFriction || FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction"
		))
	FName OrientedFrictionReferenceFrameComponent{NAME_None};
	// clang-format on

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool IsConstantNormalForceFrictionModel() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	bool IsOrientedFrictionModel() const;
	/**
	 * Material restitution, i.e. how "bouncy" the normal collisions are.
	 *
	 * A value of 1.0 means that the body does not lose energy during normal-collisions.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real Restitution {0.5};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetRestitution(double InRestitution);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetRestitution() const;

	/**
	 * Young's modulus of the contact material. Same as spring coefficient k [Pa].
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulus {2.0 / 5.0e-9};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetYoungsModulus(double InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetYoungsModulus() const;

	/**
	 * Spook Damping which represents the time the contact constraint has to fulfill its violation
	 * [s].
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SpookDamping {4.5 / 60.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetSpookDamping(double InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetSpookDamping() const;

	/**
	 * The attractive force between two colliding objects [N].
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AdhesiveForce {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetAdhesiveForce(double InAdhesiveForce);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetAdhesiveForce() const;

	/**
	 * Allowed overlap from surface for resting contact [cm].
	 *
	 * At lower overlap, the adhesion force will take effect.
	 * At this overlap, no adhesive force is applied.
	 * At higher overlap, the (usual) contact force is applied.
	 */
	UPROPERTY(EditAnywhere, Category = "General", Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AdhesiveOverlap {0.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void SetAdhesiveOverlap(double InAdhesiveOverlap);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	double GetAdhesiveOverlap() const;

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material")
	void CommitToAsset();

public:
	UAGX_ContactMaterial() = default;
	virtual ~UAGX_ContactMaterial() = default;

	void CopyFrom(const UAGX_ContactMaterial* Source);
	void CopyFrom(const FContactMaterialBarrier& Source, FAGX_ImportContext* Context);

	/**
	 * Create the Play instance for the given Source Contact Material, which should be an asset.
	 * The AGX Dynamics Native will be created immediately.
	 */
	static UAGX_ContactMaterial* CreateInstanceFromAsset(
		const UAGX_ContactMaterialRegistrarComponent& Registrar, UAGX_ContactMaterial* Source);

	/**
	 * Get the instance, i.e. Play version, of this Contact Material.
	 *
	 * For an asset Contact Material GetInstance will return nullptr if we are not currently in Play
	 * or if an instance has not been created with GetOrCreateInstance yet.
	 *
	 * For an instance GetInstance will always return itself.
	 */
	UAGX_ContactMaterial* GetInstance();

	/**
	 * If the World Registrar is part of an in-game World and this Contact Material is an
	 * asset that don't yet have an associated Contact Material instance, then a new
	 * UAGX_ContactMaterial instance that represents the asset for the lifetime of the GameInstance
	 * is created and returned. If an instance has already been created for the asset then that
	 * instance is returned. If called on an instance the the instance itself is returned. Returns
	 * nullptr if the world that the given Registrar is part of isn't a game world.
	 */
	UAGX_ContactMaterial* GetOrCreateInstance(
		const UAGX_ContactMaterialRegistrarComponent& Registrar);

	/**
	 * If this Contact Material is an instance created from an asset, then the UAGX_ContactMaterial
	 * asset it was created from is returned, if it still exists. If called on an asset then it
	 * returns itself.
	 */
	UAGX_ContactMaterial* GetAsset();

	/**
	 * Return true if this UAGX_ContactMaterial is an instance in a game world created from an asset
	 * Return false	if this UAGX_ContactMaterial is an asset.
	 */
	bool IsInstance() const;

	bool HasNative() const;
	FContactMaterialBarrier* GetNative();
	const FContactMaterialBarrier* GetNative() const;
	FContactMaterialBarrier* GetOrCreateNative(
		const UAGX_ContactMaterialRegistrarComponent& Registrar);

	void UpdateNativeProperties(const UAGX_ContactMaterialRegistrarComponent& Registrar);

	// ~Begin UObject interface.
	virtual void Serialize(FArchive& Archive) override;
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

public: // Deprecated functions.
	// clang-format off

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMinElasticRestLength instead of SetMinElasticRestLength_BP"))
	void SetMinElasticRestLength_BP(float InMinLength);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMinElasticRestLength instead of GetMinElasticRestLength_BP"))
	float GetMinElasticRestLength_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMaxElasticRestLength instead of SetMaxElasticRestLength_BP"))
	void SetMaxElasticRestLength_BP(float InMaxLength);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMaxElasticRestLength instead of GetMaxElasticRestLength_BP"))
	float GetMaxElasticRestLength_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetNormalForceMagnitude instead of SetNormalForceMagnitude_BP"))
	void SetNormalForceMagnitude_BP(float InNormalForceMagnitude);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetNormalForceMagnitude instead of GetNormalForceMagnitude_BP"))
	float GetNormalForceMagnitude_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetFrictionCoefficient instead of SetFrictionCoefficient_BP"))
	void SetFrictionCoefficient_BP(float InFrictionCoefficient);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetFrictionCoefficient instead of GetFrictionCoefficient_BP"))
	float GetFrictionCoefficient_BP();

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage =
				"Use SetSecondaryFrictionCoefficient instead of SetSecondaryFrictionCoefficient_BP"))
	void SetSecondaryFrictionCoefficient_BP(float InFrictionCoefficient);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta = (
			DeprecatedFunction,
			DeprecationMessage =
				"Use GetSecondaryFrictionCoefficient instead of GetSecondaryFrictionCoefficient_BP"))
	float GetSecondaryFrictionCoefficient_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetSurfaceViscosity instead of SetSurfaceViscosity_BP"))
	void SetSurfaceViscosity_BP(float InSurfaceViscosity);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetSurfaceViscosity instead of GetSurfaceViscosity_BP"))
	float GetSurfaceViscosity_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetSecondarySurfaceViscosity instead of SetSecondarySurfaceViscosity_BP"))
	void SetSecondarySurfaceViscosity_BP(float InSecondarySurfaceViscosity);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetSecondarySurfaceViscosity instead of GetSecondarySurfaceViscosity_BP"))
	float GetSecondarySurfaceViscosity_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetRestitution instead of SetRestitution_BP"))
	void SetRestitution_BP(float InRestitution);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetRestitution instead of GetRestitution_BP"))
	float GetRestitution_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetYoungsModulus instead of SetYoungsModulus_BP"))
	void SetYoungsModulus_BP(float InYoungsModulus);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetYoungsModulus instead of GetYoungsModulus_BP"))
	float GetYoungsModulus_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetSpookDamping instead of SetSpookDamping_BP"))
	void SetSpookDamping_BP(float InSpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetSpookDamping instead of GetSpookDamping_BP"))
	float GetSpookDamping_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetAdhesiveForce instead of SetAdhesiveForce_BP"))
	void SetAdhesiveForce_BP(float InAdhesiveForce);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetAdhesiveForce instead of GetAdhesiveForce_BP"))
	float GetAdhesiveForce_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetAdhesiveOverlap instead of SetAdhesiveOverlap_BP"))
	void SetAdhesiveOverlap_BP(float InAdhesiveOverlap);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Contact Material",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetAdhesiveOverlap instead of GetAdhesiveOverlap_BP"))
	float GetAdhesiveOverlap_BP() const;

	// clang-format on

private:
#if WITH_EDITOR
	virtual void InitPropertyDispatcher();
#endif

	void CreateNative(const UAGX_ContactMaterialRegistrarComponent& Registrar);

private:
	TWeakObjectPtr<UAGX_ContactMaterial> Asset;
	TWeakObjectPtr<UAGX_ContactMaterial> Instance;
	FContactMaterialBarrier NativeBarrier;
};
