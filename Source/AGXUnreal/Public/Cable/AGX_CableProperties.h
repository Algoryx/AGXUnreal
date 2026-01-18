// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Cable/CablePropertiesBarrier.h"
#include "UObject/Object.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Cable/CablePropertiesBarrier.h"

#include "AGX_CableProperties.generated.h"

class UWorld;

struct FAGX_ImportContext;

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
	 * Spook Damping when bending the Cable [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double SpookDampingBend {0.0333};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetSpookDampingBend(double SpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetSpookDampingBend() const;

	/**
	 * Spook Damping when twisting the Cable [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double SpookDampingTwist {0.0333};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetSpookDampingTwist(double SpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetSpookDampingTwist() const;

	/**
	 * Spook Damping when stretching the Cable [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	double SpookDampingStretch {0.0333};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetSpookDampingStretch(double SpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	double GetSpookDampingStretch() const;

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


	UAGX_CableProperties* GetOrCreateInstance(UWorld* PlayingWorld);
	bool IsInstance() const;
	UAGX_CableProperties* CreateInstanceFromAsset(
		const UWorld* PlayingWorld, UAGX_CableProperties* Source);
	UAGX_CableProperties* GetInstance();
	UAGX_CableProperties* GetAsset();

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void CommitToAsset();

	void CopyFrom(const FCablePropertiesBarrier& Source, FAGX_ImportContext* Context);
	void CopyFrom(const UAGX_CableProperties* Source);

	bool HasNative() const;
	FCablePropertiesBarrier* GetNative();
	const FCablePropertiesBarrier* GetNative() const;
	FCablePropertiesBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	
private:
#if WITH_EDITOR
	virtual void InitPropertyDispatcher();
#endif

	void CreateNative();

private:
	/// The persistent asset that this runtime instance was created from. Nullptr for assets.
	TWeakObjectPtr<UAGX_CableProperties> Asset {nullptr};

	/// The runtime instance that was created from a persistent asset. Nullptr for instances.
	TWeakObjectPtr<UAGX_CableProperties> Instance {nullptr};

	FCablePropertiesBarrier NativeBarrier;
};
