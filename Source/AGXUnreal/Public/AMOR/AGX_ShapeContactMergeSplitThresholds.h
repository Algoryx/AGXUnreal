// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AMOR/AGX_MergeSplitThresholdsBase.h"
#include "AMOR/ShapeContactMergeSplitThresholdsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeContactMergeSplitThresholds.generated.h"

/**
 * Defines the thresholds used by AMOR (merge split) for Rigid Bodies and Shapes, affecting under
 * which conditions they will merge and split.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ShapeContactMergeSplitThresholds : public UAGX_MergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	/**
	 * Maximum impact speed (along a contact normal) a merged object can resist
	 * without being split [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxImpactSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxImpactSpeed(double InMaxImpactSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	double GetMaxImpactSpeed() const;

	/**
	 * Maximum speed along a contact normal for a contact to be considered resting [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRelativeNormalSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRelativeNormalSpeed(double InMaxRelativeNormalSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	double GetMaxRelativeNormalSpeed() const;

	/**
	 * Maximum (sliding) speed along a contact tangent for a contact to be considered resting
	 * [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRelativeTangentSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRelativeTangentSpeed(double InMaxRelativeTangentSpeed);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	double GetMaxRelativeTangentSpeed() const;

	/**
	 * Maximum (rolling) speed for a contact to be considered resting [cm/s].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real MaxRollingSpeed {0.01};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaxRollingSpeed(double InMaxRollingSpeed);

	double GetMaxRollingSpeed() const;

	/**
	 * Adhesive force in the normal directions preventing the object to split (if > 0) when the
	 * object is subject to external interactions (e.g., constraints) [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real NormalAdhesion {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetNormalAdhesion(double InNormalAdhesion);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	double GetNormalAdhesion() const;

	/**
	 * Adhesive force in the tangential directions preventing the object to split (if > 0) when the
	 * object is subject to external interactions (e.g., constraints) [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	FAGX_Real TangentialAdhesion {0.0};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetTangentialAdhesion(double InTangentialAdhesion);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	double GetTangentialAdhesion() const;

	/**
	 * Check split given external forces for all objects merged (i.e., rb->getForce() the sum of
	 * rb->addForce(), including the gravity force).
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	bool bMaySplitInGravityField {false};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetMaySplitInGravityField(bool bInMaySplitInGravityField);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	bool GetMaySplitInGravityField() const;

	/**
	 * True to split when Shape Contact state is agxCollide::GeometryContact::IMPACT_STATE, i.e.,
	 * the first time the objects collide.
	 */
	UPROPERTY(EditAnywhere, Category = "Shape Contact Merge Split Thresholds")
	bool bSplitOnLogicalImpact {false};

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	void SetSplitOnLogicalImpact(bool bInSplitOnLogicalImpact);

	UFUNCTION(BlueprintCallable, Category = "Shape Contact Merge Split Thresholds")
	bool GetSplitOnLogicalImpact() const;

	void CreateNative();
	bool HasNative() const;
	FShapeContactMergeSplitThresholdsBarrier* GetOrCreateNative(UWorld* PlayingWorld);
	FShapeContactMergeSplitThresholdsBarrier* GetNative();
	const FShapeContactMergeSplitThresholdsBarrier* GetNative() const;

	static UAGX_ShapeContactMergeSplitThresholds* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholds& Source);

	UAGX_ShapeContactMergeSplitThresholds* GetOrCreateInstance(UWorld* PlayingWorld);
	UAGX_ShapeContactMergeSplitThresholds* GetInstance();

	bool IsInstance() const;

	void CopyFrom(const FMergeSplitThresholdsBarrier& Barrier);

	/**
	 * Assigns the property values of this class to the passed barrier.
	 */
	void CopyTo(FShapeContactMergeSplitThresholdsBarrier& Barrier);

public: // Deprecated function.
	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetMaxImpactSpeed instead of SetMaxImpactSpeed_BP"))
	void SetMaxImpactSpeed_BP(float InMaxImpactSpeed);

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetMaxImpactSpeed instead of GetMaxImpactSpeed_BP"))
	float GetMaxImpactSpeed_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMaxRelativeNormalSpeed instead of SetMaxRelativeNormalSpeed_BP"))
	void SetMaxRelativeNormalSpeed_BP(float InMaxRelativeNormalSpeed);

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMaxRelativeNormalSpeed instead of GetMaxRelativeNormalSpeed_BP"))
	float GetMaxRelativeNormalSpeed_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMaxRelativeTangentSpeed instead of SetMaxRelativeTangentSpeed_BP"))
	void SetMaxRelativeTangentSpeed_BP(float InMaxRelativeTangentSpeed);

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMaxRelativeTangentSpeed instead of GetMaxRelativeTangentSpeed_BP"))
	float GetMaxRelativeTangentSpeed_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetMaxRollingSpeed instead of SetMaxRollingSpeed_BP"))
	void SetMaxRollingSpeed_BP(float InMaxRollingSpeed);

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetMaxRollingSpeed instead of GetMaxRollingSpeed_BP"))
	float GetMaxRollingSpeed_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetNormalAdhesion instead of SetNormalAdhesion_BP"))
	void SetNormalAdhesion_BP(float InNormalAdhesion);

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetNormalAdhesion instead of GetNormalAdhesion_BP"))
	float GetNormalAdhesion_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetTangentialAdhesion instead of SetTangentialAdhesion_BP"))
	void SetTangentialAdhesion_BP(float InTangentialAdhesion);

	UFUNCTION(
		BlueprintCallable, Category = "Shape Contact Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetTangentialAdhesion instead of GetTangentialAdhesion_BP"))
	float GetTangentialAdhesion_BP() const;

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void CopyFrom(const UAGX_ShapeContactMergeSplitThresholds& Source);
	void SetNativeProperties();

	TWeakObjectPtr<UAGX_ShapeContactMergeSplitThresholds> Asset;
	TWeakObjectPtr<UAGX_ShapeContactMergeSplitThresholds> Instance;
	FShapeContactMergeSplitThresholdsBarrier NativeBarrier;
};
