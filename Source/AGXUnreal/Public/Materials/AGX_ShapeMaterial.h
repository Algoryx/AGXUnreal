// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "Materials/AGX_ShapeMaterialBulkProperties.h"
#include "Materials/AGX_ShapeMaterialSurfaceProperties.h"
#include "Materials/AGX_ShapeMaterialWireProperties.h"
#include "Materials/ShapeMaterialBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterial.generated.h"

struct FAGX_ImportContext;

/**
 * Defines contacts properties between Shapes as well as properties affecting the mass
 * distribution of AGX Rigid Bodies.
 *
 * Since a contact involves two Shapes, the final parameters used as input to the force
 * equations are a fusion of the two shape's AGX Materials. Though, the way the two material's
 * properties are combined might not be desirable in all scenarios. Therefore, there is another AGX
 * asset called AGX Contact Material that provides a well-defined and more detailed definition over
 * the parameters to use when two specific AGX Materials come in contact with each other.
 *
 * It is preferred to use AGX Contact Materials for superior simulation results!
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable,
	AutoExpandCategories = ("Material Properties"))
class AGXUNREAL_API UAGX_ShapeMaterial : public UObject
{
	GENERATED_BODY()

public:
	// Bulk properties.

	UPROPERTY(EditAnywhere, Category = "AGX Shape Material")
	FAGX_ShapeMaterialBulkProperties Bulk;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetDensity(double InDensity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	double GetDensity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetYoungsModulus(double InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	double GetYoungsModulus() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetBulkViscosity(double InBulkViscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	double GetBulkViscosity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetSpookDamping(double InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	double GetSpookDamping() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	void SetMinMaxElasticRestLength(double InMin, double InMax);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	double GetMinElasticRestLength() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Bulk Properties")
	double GetMaxElasticRestLength() const;

	// Surface properties.

	UPROPERTY(EditAnywhere, Category = "AGX Shape Material")
	FAGX_ShapeMaterialSurfaceProperties Surface;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	void SetFrictionEnabled(bool Enabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	bool GetFrictionEnabled() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	void SetRoughness(double Roughness);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	double GetRoughness() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	void SetSurfaceViscosity(double Viscosity);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	double GetSurfaceViscosity() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	void SetAdhesion(double AdhesiveForce, double AdhesiveOverlap);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	double GetAdhesiveForce() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Surface Properties")
	double GetAdhesiveOverlap() const;

	// Wire properties.

	UPROPERTY(EditAnywhere, Category = "AGX Shape Material")
	FAGX_ShapeMaterialWireProperties Wire;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	double GetYoungsModulusStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetYoungsModulusStretch(double InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	double GetYoungsModulusBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetYoungsModulusBend(double InYoungsModulus);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	double GetSpookDampingStretch() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetSpookDampingStretch(double InSpookDamping);

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	double GetSpookDampingBend() const;

	UFUNCTION(BlueprintCallable, Category = "AGX Material Wire Properties")
	void SetSpookDampingBend(double InSpookDamping);

	/*
	 * The import Guid of this Object. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	void CommitToAsset();

	void CopyFrom(const FShapeMaterialBarrier& Source, FAGX_ImportContext* Context);

	static UAGX_ShapeMaterial* CreateInstanceFromAsset(
		UWorld* PlayingWorld, UAGX_ShapeMaterial* Source);

	UAGX_ShapeMaterial* GetOrCreateInstance(UWorld* PlayingWorld);
	FShapeMaterialBarrier* GetOrCreateShapeMaterialNative(UWorld* PlayingWorld);

	FShapeMaterialBarrier* GetNative();
	const FShapeMaterialBarrier* GetNative() const;
	bool HasNative() const;
	void UpdateNativeProperties();

	bool IsInstance() const;

	void CopyShapeMaterialProperties(const UAGX_ShapeMaterial* Source);

private:
	void CreateNative(UWorld* PlayingWorld);

#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();

	void WriteSurfacePropertyToInstance(const FName& PropertyName);
	void WriteBulkPropertyToInstance(const FName& PropertyName);
	void WriteWirePropertyToInstance(const FName& PropertyName);
#endif

	TWeakObjectPtr<UAGX_ShapeMaterial> Asset;
	TWeakObjectPtr<UAGX_ShapeMaterial> Instance;
	FShapeMaterialBarrier NativeBarrier;
};
