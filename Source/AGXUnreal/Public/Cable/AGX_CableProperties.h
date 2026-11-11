// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Cable/CablePropertiesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_CableProperties.generated.h"

class UWorld;

struct FAGX_ImportContext;
struct FCableBarrier;

/**
 * An asset used to hold properties defining the physical behaviour of a Cable.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API UAGX_CableProperties : public UObject
{
	GENERATED_BODY()

public:
	UAGX_CableProperties() = default;

	/**
	 * Attenuation when bending the Cable [steps].
	 * The number of simulation steps the constraint has to fulfill its violation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double AttenuationBend {2.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetAttenuationBend(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetAttenuationBend() const;

	/**
	 * Attenuation when twisting the Cable [steps].
	 * The number of simulation steps the constraint has to fulfill its violation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double AttenuationTwist {2.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetAttenuationTwist(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetAttenuationTwist() const;

	/**
	 * Attenuation when stretching the Cable [steps].
	 * The number of simulation steps the constraint has to fulfill its violation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double AttenuationStretch {2.0};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetAttenuationStretch(double Attenuation);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetAttenuationStretch() const;

	/**
	 * Poisson's Ratio when bending the Cable.
	 * Defines the ratio of transverse strain to axial strain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double PoissonsRatioBend {0.333};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetPoissonsRatioBend(double PoissonsRatio);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetPoissonsRatioBend() const;

	/**
	 * Poisson's Ratio when twisting the Cable.
	 * Defines the ratio of transverse strain to axial strain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double PoissonsRatioTwist {0.333};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetPoissonsRatioTwist(double PoissonsRatio);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetPoissonsRatioTwist() const;

	/**
	 * Poisson's Ratio when stretching the Cable.
	 * Defines the ratio of transverse strain to axial strain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double PoissonsRatioStretch {0.333};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetPoissonsRatioStretch(double PoissonsRatio);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetPoissonsRatioStretch() const;

	/**
	 * Young's Modulus when bending the Cable [Pa].
	 * Controls the stiffness of the Cable during bending.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	FAGX_Real YoungsModulusBend {1.0e9};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetYoungsModulusBend(double YoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetYoungsModulusBend() const;

	/**
	 * Young's Modulus when twisting the Cable [Pa].
	 * Controls the stiffness of the Cable during twisting.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	FAGX_Real YoungsModulusTwist {1.0e9};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetYoungsModulusTwist(double YoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetYoungsModulusTwist() const;

	/**
	 * Young's Modulus when stretching the Cable [Pa].
	 * Controls the stiffness of the Cable during stretching.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	FAGX_Real YoungsModulusStretch {1.0e9};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetYoungsModulusStretch(double YoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetYoungsModulusStretch() const;

	/**
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import")
	FGuid ImportGuid;

	UAGX_CableProperties* GetOrCreateInstance(UWorld* PlayingWorld);
	bool IsInstance() const;
	static UAGX_CableProperties* CreateInstanceFromAsset(
		const UWorld* PlayingWorld, UAGX_CableProperties* Source);
	UAGX_CableProperties* GetInstance();
	UAGX_CableProperties* GetAsset();

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void CommitToAsset();

	void CopyFrom(const FCableBarrier& Source, FAGX_ImportContext* Context);
	void CopyFrom(const FCablePropertiesBarrier& Source);
	void CopyFrom(const UAGX_CableProperties* Source);

	bool HasNative() const;
	FCablePropertiesBarrier* GetNative();
	const FCablePropertiesBarrier* GetNative() const;
	FCablePropertiesBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	// ~Begin UObject interface.
	virtual void Serialize(FArchive& Archive) override;
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
// ~End UObject interface.

	// Legacy Blueprint functions use the project simulation timestep to convert seconds to steps.
	UFUNCTION(
		BlueprintCallable, Category = "AGX Cable",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SetAttenuationBend instead."))
	void SetSpookDampingBend(double SpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Cable",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use GetAttenuationBend instead."))
	double GetSpookDampingBend() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Cable",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SetAttenuationTwist instead."))
	void SetSpookDampingTwist(double SpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Cable",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use GetAttenuationTwist instead."))
	double GetSpookDampingTwist() const;

	UFUNCTION(
		BlueprintCallable, Category = "AGX Cable",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use SetAttenuationStretch instead."))
	void SetSpookDampingStretch(double SpookDamping);

	UFUNCTION(
		BlueprintCallable, Category = "AGX Cable",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use GetAttenuationStretch instead."))
	double GetSpookDampingStretch() const;

private:
#if WITH_EDITOR
	virtual void InitPropertyDispatcher();
#endif

	void CreateNative();

private:
	// Unreal loads the old property names into these fields, but does not save them again.
	UPROPERTY()
	double SpookDampingBend_DEPRECATED {0.0333};

	UPROPERTY()
	double SpookDampingTwist_DEPRECATED {0.0333};

	UPROPERTY()
	double SpookDampingStretch_DEPRECATED {0.0333};

	/// The persistent asset that this runtime instance was created from. Nullptr for assets.
	TWeakObjectPtr<UAGX_CableProperties> Asset {nullptr};

	/// The runtime instance that was created from a persistent asset. Nullptr for instances.
	TWeakObjectPtr<UAGX_CableProperties> Instance {nullptr};

	FCablePropertiesBarrier NativeBarrier;
};
