// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Misc/EngineVersionComparison.h"

#include "AGX_ContactMaterialRegistrarComponent.generated.h"

class FContactMaterialBarrier;
class UAGX_ContactMaterial;
struct FAGX_ImportContext;

/**
 * Defines which AGX Contact Materials should be used by the owning level.
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", AutoExpandCategories = ("AGX Contact Material Registrar"),
	Meta = (BlueprintSpawnableComponent), HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_ContactMaterialRegistrarComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_ContactMaterialRegistrarComponent();

	/**
	 * User defined AGX Contact Materials to use in this level.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Contact Material Registrar")
	TArray<UAGX_ContactMaterial*> ContactMaterials;

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material Registrar")
	void RemoveContactMaterial(UAGX_ContactMaterial* ContactMaterial);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material Registrar")
	void AddContactMaterial(UAGX_ContactMaterial* ContactMaterial);

	UFUNCTION(BlueprintCallable, Category = "AGX Contact Material Registrar")
	const TArray<UAGX_ContactMaterial*>& GetContactMaterials() const;

	void CopyFrom(const TArray<FContactMaterialBarrier>& Barriers, FAGX_ImportContext* Context);

	// ~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	// ~ End UActorComponent Interface

	// ~ Begin UObject Interface
#if WITH_EDITOR
	virtual bool CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
		const UProperty* InProperty
#else
		const FProperty* InProperty
#endif
	) const override;
#endif
	// ~ End UObject Interface

private:
	/*
	 * Clears the ContactMaterials array and unregisteres all instances with the Simulation if one
	 * exists.
	 */
	void ClearAll();
};
