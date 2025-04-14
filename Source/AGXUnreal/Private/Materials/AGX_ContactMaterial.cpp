// Copyright 2024, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterial.h"

// AX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "EngineUtils.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "UAGX_ContactMaterial"

void UAGX_ContactMaterial::SetContactSolver(EAGX_ContactSolver InContactSolver)
{
	// This block of code should be equivalent to AGX_ASSET_SETTER_IMPL_VALUE. We can't use that
	// macro directly because function names don't match between UAGX_ContactMaterial and
	// FContactMaterialBarrier. Uses 'ContactSolver' in UAGX_ContactMaterial and FrictionSolveType
	// in FContactMaterialBarrier.
	if (IsInstance())
	{
		ContactSolver = InContactSolver;
		if (HasNative())
		{
			NativeBarrier.SetFrictionSolveType(InContactSolver);
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetContactSolver(InContactSolver);
			return;
		}
		ContactSolver = InContactSolver;
	}
}

EAGX_ContactSolver UAGX_ContactMaterial::GetContactSolver() const
{
	// This block of code should be equivalent to AGX_ASSET_GETTER_IMPL_VALUE. We can't use that
	// macro directly because function names don't match between UAGX_ContactMaterial and
	// FContactMaterialBarrier. Uses 'ContactSolver' in UAGX_ContactMaterial and FrictionSolveType
	// in FContactMaterialBarrier.
	if (Instance != nullptr)
	{
		return Instance->GetContactSolver();
	}
	if (HasNative())
	{
		return NativeBarrier.GetFrictionSolveType();
	}
	return ContactSolver;
}

void UAGX_ContactMaterial::SetContactReductionMode(EAGX_ContactReductionMode InContactReductionMode)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		ContactReduction.Mode, InContactReductionMode, SetContactReductionMode);
}

EAGX_ContactReductionMode UAGX_ContactMaterial::GetContactReductionMode() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ContactReduction.Mode, GetContactReductionMode);
}

void UAGX_ContactMaterial::SetContactReductionLevel(
	EAGX_ContactReductionLevel InContactReductionLevel)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		ContactReduction.ContactReductionLevel, InContactReductionLevel, SetContactReductionLevel);
}

EAGX_ContactReductionLevel UAGX_ContactMaterial::GetContactReductionLevel() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ContactReduction.ContactReductionLevel, GetContactReductionLevel);
}

void UAGX_ContactMaterial::SetUseContactAreaApproach(bool bInUseContactAreaApproach)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MechanicsApproach.bUseContactAreaApproach, bInUseContactAreaApproach,
		SetUseContactAreaApproach);
}

bool UAGX_ContactMaterial::GetUseContactAreaApproach()
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		MechanicsApproach.bUseContactAreaApproach, GetUseContactAreaApproach);
}

void UAGX_ContactMaterial::SetMinElasticRestLength(double InMinLength)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MechanicsApproach.MinElasticRestLength, InMinLength, SetMinElasticRestLength);
}

double UAGX_ContactMaterial::GetMinElasticRestLength() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MechanicsApproach.MinElasticRestLength, GetMinElasticRestLength);
}

void UAGX_ContactMaterial::SetMinElasticRestLength_BP(float InMinLength)
{
	SetMinElasticRestLength(static_cast<double>(InMinLength));
}

float UAGX_ContactMaterial::GetMinElasticRestLength_BP() const
{
	return static_cast<float>(GetMinElasticRestLength());
}

void UAGX_ContactMaterial::SetMaxElasticRestLength(double InMaxLength)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MechanicsApproach.MaxElasticRestLength, InMaxLength, SetMaxElasticRestLength);
}

double UAGX_ContactMaterial::GetMaxElasticRestLength() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MechanicsApproach.MaxElasticRestLength, GetMaxElasticRestLength);
}

void UAGX_ContactMaterial::SetMaxElasticRestLength_BP(float InMaxLength)
{
	SetMaxElasticRestLength(static_cast<double>(InMaxLength));
}

float UAGX_ContactMaterial::GetMaxElasticRestLength_BP() const
{
	return static_cast<float>(GetMaxElasticRestLength());
}

void UAGX_ContactMaterial::SetFrictionModel(EAGX_FrictionModel InFrictionModel)
{
	AGX_ASSET_SETTER_IMPL_VALUE(FrictionModel, InFrictionModel, SetFrictionModel);
	if (!HasNative())
	{
		return;
	}
	if (FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
	{
		NativeBarrier.SetNormalForceMagnitude(NormalForceMagnitude);
		NativeBarrier.SetEnableScaleNormalForceWithDepth(bScaleNormalForceWithDepth);
	}
	if (IsOrientedFrictionModel())
	{
		NativeBarrier.SetPrimaryDirection(PrimaryDirection);
	}
}

EAGX_FrictionModel UAGX_ContactMaterial::GetFrictionModel() const
{
	if (Instance != nullptr)
	{
		return Instance->GetFrictionModel();
	}
	if (HasNative())
	{
		return NativeBarrier.GetFrictionModel();
	}
	return FrictionModel;
}

void UAGX_ContactMaterial::SetNormalForceMagnitude(double InNormalForceMagnitude)
{
	if (IsInstance())
	{
		NormalForceMagnitude = InNormalForceMagnitude;
		// Only some friction models have a normal force magnitude in AGX Dynamics.
		// Ignore the rest.
		if (HasNative() &&
			FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
		{
			NativeBarrier.SetNormalForceMagnitude(InNormalForceMagnitude);
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetNormalForceMagnitude(InNormalForceMagnitude);
			return;
		}
		NormalForceMagnitude = InNormalForceMagnitude;
	}
}

double UAGX_ContactMaterial::GetNormalForceMagnitude() const
{
	if (Instance != nullptr)
	{
		return Instance->GetNormalForceMagnitude();
	}
	if (HasNative() && FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
	{
		double ForceMagnitude {0.0};
		const bool bGotMagnitude = NativeBarrier.GetNormalForceMagnitude(ForceMagnitude);
		if (!bGotMagnitude)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not get normal friction force magnitude from Contact Material '%s' "
					 "because the native AGX Dynamics friction model either doesn't exist or "
					 "doesn't support constant normal force."),
				*GetName());
		}
		return ForceMagnitude;
	}
	return NormalForceMagnitude;
}

void UAGX_ContactMaterial::SetNormalForceMagnitude_BP(float InNormalForceMagnitude)
{
	SetNormalForceMagnitude(static_cast<double>(InNormalForceMagnitude));
}

float UAGX_ContactMaterial::GetNormalForceMagnitude_BP() const
{
	return static_cast<float>(GetNormalForceMagnitude());
}

void UAGX_ContactMaterial::SetScaleNormalForceWithDepth(bool bEnabled)
{
	if (IsInstance())
	{
		bScaleNormalForceWithDepth = bEnabled;
		// Only some friction models support scaling the normal force with depth.
		if (HasNative() &&
			FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
		{
			NativeBarrier.SetEnableScaleNormalForceWithDepth(bEnabled);
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetScaleNormalForceWithDepth(bEnabled);
			return;
		}
		bScaleNormalForceWithDepth = bEnabled;
	}
}

bool UAGX_ContactMaterial::GetScaleNormalForceWithDepth() const
{
	if (Instance != nullptr)
	{
		return Instance->GetScaleNormalForceWithDepth();
	}
	if (HasNative() && FrictionModel == EAGX_FrictionModel::OrientedConstantNormalForceBoxFriction)
	{
		bool bScaleWithDepth {false};
		const bool bGotScaleWithDepth =
			NativeBarrier.GetEnableScaleNormalForceWithDepth(bScaleWithDepth);
		if (!bGotScaleWithDepth)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not get scale with depth flag from Contact Material '%s' "
					 "because the native AGX Dynamics friction model either doesn't exist or "
					 "doesn't support constant normal force."),
				*GetName());
		}
		return bScaleWithDepth;
	}
	return bScaleNormalForceWithDepth;
}

void UAGX_ContactMaterial::SetSurfaceFrictionEnabled(bool bInEnabled)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bEnableSurfaceFriction, bInEnabled, SetSurfaceFrictionEnabled);
}

bool UAGX_ContactMaterial::GetSurfaceFrictionEnabled()
{
	AGX_ASSET_GETTER_IMPL_VALUE(bEnableSurfaceFriction, GetSurfaceFrictionEnabled);
}

void UAGX_ContactMaterial::SetFrictionCoefficient(double InFrictionCoefficient)
{
	if (IsInstance())
	{
		FrictionCoefficient = InFrictionCoefficient;
		if (HasNative())
		{
			if (bUseSecondaryFrictionCoefficient)
			{
				NativeBarrier.SetPrimaryFrictionCoefficient(InFrictionCoefficient);
				NativeBarrier.SetSecondaryFrictionCoefficient(SecondaryFrictionCoefficient);
			}
			else
			{
				NativeBarrier.SetFrictionCoefficient(InFrictionCoefficient);
			}
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetFrictionCoefficient(InFrictionCoefficient);
			return;
		}
		FrictionCoefficient = InFrictionCoefficient;
	}
}

double UAGX_ContactMaterial::GetFrictionCoefficient() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(FrictionCoefficient, GetFrictionCoefficient);
}

void UAGX_ContactMaterial::SetFrictionCoefficient_BP(float InFrictionCoefficient)
{
	SetFrictionCoefficient(static_cast<double>(InFrictionCoefficient));
}

float UAGX_ContactMaterial::GetFrictionCoefficient_BP()
{
	return static_cast<float>(GetFrictionCoefficient());
}

void UAGX_ContactMaterial::SetSecondaryFrictionCoefficient(double InFrictionCoefficient)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		SecondaryFrictionCoefficient, InFrictionCoefficient, SetSecondaryFrictionCoefficient)
}

double UAGX_ContactMaterial::GetSecondaryFrictionCoefficient() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SecondaryFrictionCoefficient, GetSecondaryFrictionCoefficient);
}

void UAGX_ContactMaterial::SetSecondaryFrictionCoefficient_BP(float InFrictionCoefficient)
{
	SetSecondaryFrictionCoefficient(static_cast<double>(InFrictionCoefficient));
}

float UAGX_ContactMaterial::GetSecondaryFrictionCoefficient_BP() const
{
	return static_cast<float>(GetSecondaryFrictionCoefficient());
}

void UAGX_ContactMaterial::SetUseSecondaryFrictionCoefficient(
	bool bInUseSecondaryFrictionCoefficient)
{
	if (IsInstance())
	{
		bUseSecondaryFrictionCoefficient = bInUseSecondaryFrictionCoefficient;
		if (HasNative())
		{
			if (bUseSecondaryFrictionCoefficient)
			{
				NativeBarrier.SetPrimaryFrictionCoefficient(FrictionCoefficient);
				NativeBarrier.SetSecondaryFrictionCoefficient(SecondaryFrictionCoefficient);
			}
			else
			{
				NativeBarrier.SetFrictionCoefficient(FrictionCoefficient);
			}
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetUseSecondaryFrictionCoefficient(bInUseSecondaryFrictionCoefficient);
			return;
		}
		bUseSecondaryFrictionCoefficient = bInUseSecondaryFrictionCoefficient;
	}
}

bool UAGX_ContactMaterial::GetUseSecondaryFrictionCoefficient() const
{
	if (Instance != nullptr)
	{
		return Instance->GetUseSecondaryFrictionCoefficient();
	}
	else
	{
		return bUseSecondaryFrictionCoefficient;
	}
}

void UAGX_ContactMaterial::SetSurfaceViscosity(double InSurfaceViscosity)
{
	if (IsInstance())
	{
		SurfaceViscosity = InSurfaceViscosity;
		if (HasNative())
		{
			if (bUseSecondarySurfaceViscosity)
			{
				NativeBarrier.SetPrimarySurfaceViscosity(InSurfaceViscosity);
				NativeBarrier.SetSecondarySurfaceViscosity(SecondarySurfaceViscosity);
			}
			else
			{
				NativeBarrier.SetSurfaceViscosity(InSurfaceViscosity);
			}
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetSurfaceViscosity(InSurfaceViscosity);
			return;
		}
		SurfaceViscosity = InSurfaceViscosity;
	}
}

double UAGX_ContactMaterial::GetSurfaceViscosity() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SurfaceViscosity, GetSurfaceViscosity);
}

void UAGX_ContactMaterial::SetSurfaceViscosity_BP(float InSurfaceViscosity)
{
	SetSurfaceViscosity(static_cast<double>(InSurfaceViscosity));
}

float UAGX_ContactMaterial::GetSurfaceViscosity_BP() const
{
	return static_cast<float>(GetSurfaceViscosity());
}

void UAGX_ContactMaterial::SetSecondarySurfaceViscosity(double InSurfaceViscosity)
{
	if (IsInstance())
	{
		SecondarySurfaceViscosity = InSurfaceViscosity;
		if (HasNative() && bUseSecondarySurfaceViscosity)
		{
			NativeBarrier.SetSecondarySurfaceViscosity(InSurfaceViscosity);
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetSecondarySurfaceViscosity(InSurfaceViscosity);
			return;
		}
		SecondarySurfaceViscosity = InSurfaceViscosity;
	}
}

double UAGX_ContactMaterial::GetSecondarySurfaceViscosity() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SecondarySurfaceViscosity, GetSecondarySurfaceViscosity);
}

void UAGX_ContactMaterial::SetSecondarySurfaceViscosity_BP(float InSurfaceViscosity)
{
	SetSecondarySurfaceViscosity(static_cast<double>(InSurfaceViscosity));
}

float UAGX_ContactMaterial::GetSecondarySurfaceViscosity_BP() const
{
	return static_cast<float>(GetSecondarySurfaceViscosity());
}

void UAGX_ContactMaterial::SetUseSecondarySurfaceViscosity(bool bInUseSecondarySurfaceViscosity)
{
	if (IsInstance())
	{
		bUseSecondarySurfaceViscosity = bInUseSecondarySurfaceViscosity;
		if (HasNative())
		{
			if (bUseSecondarySurfaceViscosity)
			{
				NativeBarrier.SetPrimarySurfaceViscosity(SurfaceViscosity);
				NativeBarrier.SetSecondarySurfaceViscosity(SecondarySurfaceViscosity);
			}
			else
			{
				NativeBarrier.SetSurfaceViscosity(SurfaceViscosity);
			}
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetUseSecondarySurfaceViscosity(bInUseSecondarySurfaceViscosity);
			return;
		}
		bUseSecondarySurfaceViscosity = bInUseSecondarySurfaceViscosity;
	}
}

bool UAGX_ContactMaterial::GetUseSecondarySurfaceViscosity() const
{
	if (Instance != nullptr)
	{
		return Instance->GetUseSecondarySurfaceViscosity();
	}
	else
	{
		return bUseSecondarySurfaceViscosity;
	}
}

void UAGX_ContactMaterial::SetPrimaryDirection(FVector InPrimaryDirection)
{
	if (IsInstance())
	{
		PrimaryDirection = InPrimaryDirection;
		if (HasNative() && IsOrientedFrictionModel())
		{
			NativeBarrier.SetPrimaryDirection(InPrimaryDirection);
		}
	}
	else
	{
		if (Instance != nullptr)
		{
			Instance->SetPrimaryDirection(InPrimaryDirection);
			return;
		}
		PrimaryDirection = InPrimaryDirection;
	}
}

FVector UAGX_ContactMaterial::GetPrimaryDirection() const
{
	if (Instance != nullptr)
	{
		return Instance->GetPrimaryDirection();
	}
	if (HasNative() && IsOrientedFrictionModel())
	{
		FVector Direction {0.0};
		if (!NativeBarrier.GetPrimaryDirection(Direction))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Could not get primary direction from Contact Material '%s' because the "
					 "native AGX Dynamics friction model either doesn't exist or doesn't support "
					 "directional friction."),
				*GetName());
		}
		return Direction;
	}
	return PrimaryDirection;
}

bool UAGX_ContactMaterial::IsConstantNormalForceFrictionModel() const
{
	return ::IsConstantNormalForceFrictionModel(FrictionModel);
}

bool UAGX_ContactMaterial::IsOrientedFrictionModel() const
{
	return ::IsOrientedFrictionModel(FrictionModel);
}

void UAGX_ContactMaterial::SetRestitution(double InRestitution)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Restitution, InRestitution, SetRestitution);
}

double UAGX_ContactMaterial::GetRestitution() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Restitution, GetRestitution);
}

void UAGX_ContactMaterial::SetRestitution_BP(float InRestitution)
{
	SetRestitution(static_cast<double>(InRestitution));
}

float UAGX_ContactMaterial::GetRestitution_BP() const
{
	return static_cast<float>(GetRestitution());
}

void UAGX_ContactMaterial::SetYoungsModulus(double InYoungsModulus)
{
	AGX_ASSET_SETTER_IMPL_VALUE(YoungsModulus, InYoungsModulus, SetYoungsModulus);
}

double UAGX_ContactMaterial::GetYoungsModulus() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(YoungsModulus, GetYoungsModulus);
}

void UAGX_ContactMaterial::SetYoungsModulus_BP(float InYoungsModulus)
{
	return SetYoungsModulus(static_cast<double>(InYoungsModulus));
}

float UAGX_ContactMaterial::GetYoungsModulus_BP() const
{
	return static_cast<float>(GetYoungsModulus());
}

void UAGX_ContactMaterial::SetSpookDamping(double InSpookDamping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SpookDamping, InSpookDamping, SetSpookDamping);
}

double UAGX_ContactMaterial::GetSpookDamping() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SpookDamping, GetSpookDamping);
}

void UAGX_ContactMaterial::SetSpookDamping_BP(float InSpookDamping)
{
	SetSpookDamping(static_cast<double>(InSpookDamping));
}

float UAGX_ContactMaterial::GetSpookDamping_BP() const
{
	return static_cast<float>(GetSpookDamping());
}

void UAGX_ContactMaterial::SetAdhesiveForce(double InAdhesiveForce)
{
	AGX_ASSET_SETTER_IMPL_VALUE(AdhesiveForce, InAdhesiveForce, SetAdhesiveForce);
}

double UAGX_ContactMaterial::GetAdhesiveForce() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AdhesiveForce, GetAdhesiveForce);
}

void UAGX_ContactMaterial::SetAdhesiveForce_BP(float InAdhesiveForce)
{
	SetAdhesiveForce(static_cast<double>(InAdhesiveForce));
}

float UAGX_ContactMaterial::GetAdhesiveForce_BP() const
{
	return static_cast<float>(GetAdhesiveForce());
}

void UAGX_ContactMaterial::SetAdhesiveOverlap(double InAdhesiveOverlap)
{
	AGX_ASSET_SETTER_IMPL_VALUE(AdhesiveOverlap, InAdhesiveOverlap, SetAdhesiveOverlap)
}

double UAGX_ContactMaterial::GetAdhesiveOverlap() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AdhesiveOverlap, GetAdhesiveOverlap);
}

void UAGX_ContactMaterial::SetAdhesiveOverlap_BP(float InAdhesiveOverlap)
{
	SetAdhesiveOverlap(static_cast<double>(InAdhesiveOverlap));
}

float UAGX_ContactMaterial::GetAdhesiveOverlap_BP() const
{
	return static_cast<float>(GetAdhesiveOverlap());
}

void UAGX_ContactMaterial::CommitToAsset()
{
	if (IsInstance())
	{
		if (HasNative())
		{
#if WITH_EDITOR
			Asset->Modify();
#endif
			Asset->CopyFrom(*GetNative(), nullptr);
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(*Asset);
#endif
		}
	}
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
}

void UAGX_ContactMaterial::CopyFrom(const UAGX_ContactMaterial* Source)
{
	if (Source == nullptr)
	{
		return;
	}

	/// \todo Is there a way to make this in a more implicit way? Easy to forget these when
	/// adding properties. Loop over PropertyHandle, perhaps?

#ifdef COPY_PROPERTY
#error "COPY_PROPERTY already defined, pick a different name."
#endif
#define COPY_PROPERTY(Name)  \
	{                        \
		Name = Source->Name; \
	}

	COPY_PROPERTY(Material1);
	COPY_PROPERTY(Material2);

	COPY_PROPERTY(ContactSolver);
	COPY_PROPERTY(ContactReduction);
	COPY_PROPERTY(MechanicsApproach);

	COPY_PROPERTY(FrictionModel);
	COPY_PROPERTY(NormalForceMagnitude);
	COPY_PROPERTY(bScaleNormalForceWithDepth);
	COPY_PROPERTY(bEnableSurfaceFriction);
	COPY_PROPERTY(FrictionCoefficient);
	COPY_PROPERTY(SecondaryFrictionCoefficient);
	COPY_PROPERTY(bUseSecondaryFrictionCoefficient);
	COPY_PROPERTY(SurfaceViscosity);
	COPY_PROPERTY(SecondarySurfaceViscosity);
	COPY_PROPERTY(bUseSecondarySurfaceViscosity);
	COPY_PROPERTY(PrimaryDirection);
	COPY_PROPERTY(OrientedFrictionReferenceFrameActor);
	COPY_PROPERTY(OrientedFrictionReferenceFrameComponent);

	COPY_PROPERTY(Restitution);
	COPY_PROPERTY(YoungsModulus);
	COPY_PROPERTY(SpookDamping);
	COPY_PROPERTY(AdhesiveForce);
	COPY_PROPERTY(AdhesiveOverlap);

#undef COPY_PROPERTY
}

namespace AGX_ContactMaterial_helpers
{
	void SetupMaterials(
		const FContactMaterialBarrier& Barrier, UAGX_ContactMaterial& OutCm,
		FAGX_ImportContext& Context)
	{
		FShapeMaterialBarrier MBarrier1 = Barrier.GetMaterial1();
		FShapeMaterialBarrier MBarrier2 = Barrier.GetMaterial2();
		if (!MBarrier1.HasNative() || !MBarrier2.HasNative())
			return;

		const FString Name1 = MBarrier1.GetName();
		const FString Name2 = MBarrier2.GetName();

		const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			OutCm.GetOuter(),
			FString::Printf(TEXT("CM_%s_%s"), *MBarrier1.GetName(), *MBarrier2.GetName()),
			UAGX_ContactMaterial::StaticClass());
		OutCm.Rename(*Name);

		UAGX_ShapeMaterial* Material1 =
			FAGX_ImportRuntimeUtilities::GetOrCreateShapeMaterial(MBarrier1, &Context);
		UAGX_ShapeMaterial* Material2 =
			FAGX_ImportRuntimeUtilities::GetOrCreateShapeMaterial(MBarrier2, &Context);
		AGX_CHECK(Material1 != nullptr && Material2 != nullptr);

		OutCm.Material1 = Material1;
		OutCm.Material2 = Material2;
	}
}

void UAGX_ContactMaterial::CopyFrom(
	const FContactMaterialBarrier& Source, FAGX_ImportContext* Context)
{
	if (!Source.HasNative())
	{
		return;
	}

#define COPY_PROPERTY(Name) Name = Source.Get##Name();

	ContactSolver = Source.GetFrictionSolveType();

	ContactReduction = FAGX_ContactMaterialReductionMode();
	ContactReduction.Mode = Source.GetContactReductionMode();
	ContactReduction.ContactReductionLevel = Source.GetContactReductionLevel();

	MechanicsApproach = FAGX_ContactMaterialMechanicsApproach();
	MechanicsApproach.bUseContactAreaApproach = Source.GetUseContactAreaApproach();
	MechanicsApproach.MinElasticRestLength = Source.GetMinElasticRestLength();
	MechanicsApproach.MaxElasticRestLength = Source.GetMaxElasticRestLength();

	COPY_PROPERTY(FrictionModel);
	Source.GetNormalForceMagnitude(NormalForceMagnitude);
	Source.GetEnableScaleNormalForceWithDepth(bScaleNormalForceWithDepth);
	bEnableSurfaceFriction = Source.GetSurfaceFrictionEnabled();
	COPY_PROPERTY(FrictionCoefficient);
	COPY_PROPERTY(SecondaryFrictionCoefficient);
	bUseSecondaryFrictionCoefficient = FrictionCoefficient != SecondaryFrictionCoefficient;
	COPY_PROPERTY(SurfaceViscosity);
	COPY_PROPERTY(SecondarySurfaceViscosity);
	bUseSecondarySurfaceViscosity = SurfaceViscosity != SecondarySurfaceViscosity;
	Source.GetPrimaryDirection(PrimaryDirection);

	OrientedFrictionReferenceFrameActor = FName();
	OrientedFrictionReferenceFrameComponent =
		FName(Source.GetOrientedFrictionModelReferenceFrameBodyName());

	COPY_PROPERTY(Restitution);
	COPY_PROPERTY(YoungsModulus);
	COPY_PROPERTY(SpookDamping);
	COPY_PROPERTY(AdhesiveForce);
	COPY_PROPERTY(AdhesiveOverlap);

	ImportGuid = Source.GetGuid();

#undef COPY_PROPERTY

	if (Context != nullptr)
		AGX_ContactMaterial_helpers::SetupMaterials(Source, *this, *Context);
}

UAGX_ContactMaterial* UAGX_ContactMaterial::CreateInstanceFromAsset(
	const UAGX_ContactMaterialRegistrarComponent& Registrar, UAGX_ContactMaterial* Source)
{
	check(Source);
	check(!Source->IsInstance());

	UWorld* World = Registrar.GetWorld();
	check(World != nullptr);
	check(World->IsGameWorld());

	const FString InstanceName = Source->GetName() + "_Instance";

	UAGX_ContactMaterial* NewInstance = NewObject<UAGX_ContactMaterial>(
		GetTransientPackage(), UAGX_ContactMaterial::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative(Registrar);

	return NewInstance;
}

UAGX_ContactMaterial* UAGX_ContactMaterial::GetInstance()
{
	if (IsInstance())
	{
		return this;
	}
	else
	{
		return Instance.Get();
	}
}

UAGX_ContactMaterial* UAGX_ContactMaterial::GetOrCreateInstance(
	const UAGX_ContactMaterialRegistrarComponent& Registrar)
{
	if (IsInstance())
	{
		return this;
	}
	else
	{
		UAGX_ContactMaterial* InstancePtr = Instance.Get();
		if (InstancePtr == nullptr)
		{
			const UWorld* World = Registrar.GetWorld();
			if (World && World->IsGameWorld())
			{
				InstancePtr = UAGX_ContactMaterial::CreateInstanceFromAsset(Registrar, this);
				Instance = InstancePtr;
			}
		}

		return InstancePtr;
	}
}

UAGX_ContactMaterial* UAGX_ContactMaterial::GetAsset()
{
	if (IsInstance())
	{
		return Asset.Get();
	}
	else
	{
		return this;
	}
}

bool UAGX_ContactMaterial::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;

	// Internal testing the hypothesis that UObject::IsAsset is a valid inverse of this function.
	// @todo Consider removing this function and instead use UObject::IsAsset if the below check
	// has never failed for some period of time.
	AGX_CHECK(bIsInstance != IsAsset());

	return bIsInstance;
}

bool UAGX_ContactMaterial::HasNative() const
{
	if (IsInstance())
	{
		return NativeBarrier.HasNative();
	}
	else
	{
		return Instance != nullptr && Instance->HasNative();
	}
}

FContactMaterialBarrier* UAGX_ContactMaterial::GetNative()
{
	return const_cast<FContactMaterialBarrier*>(const_cast<const ThisClass*>(this)->GetNative());
}

const FContactMaterialBarrier* UAGX_ContactMaterial::GetNative() const
{
	if (IsInstance())
	{
		return NativeBarrier.HasNative() ? &NativeBarrier : nullptr;
	}
	else
	{
		return Instance != nullptr ? Instance->GetNative() : nullptr;
	}
}

FContactMaterialBarrier* UAGX_ContactMaterial::GetOrCreateNative(
	const UAGX_ContactMaterialRegistrarComponent& Registrar)
{
	if (IsInstance())
	{
		if (!HasNative())
		{
			CreateNative(Registrar);
		}
		return GetNative();
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_ContactMaterial '%s' who's instance is "
					 "nullptr. Ensure e.g. GetOrCreateInstance is called prior to calling this "
					 "function"),
				*GetName());
			return nullptr;
		}
		return Instance->GetOrCreateNative(Registrar);
	}
}

namespace AGX_ContactMaterial_helpers
{
	AActor* FindActorByName(UWorld* World, const FName& ActorName)
	{
		check(World);
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (It->GetFName().IsEqual(ActorName, ENameCase::CaseSensitive))
			{
				return *It;
			}
		}
		return nullptr;
	}

	/**
	 * Finds the Component that should be used as the oriented friction model Reference Frame,
	 * given a Component name, owning Actor name (optional), and the
	 * Contact Material Registrar Component that registers the contact material.
	 *
	 * If the actor name is empty the component will be searched for in the Actor owning the
	 * Contact Material Registrar Component.
	 *
	 * Currently the component must be an UAGX_RigidBodyComponent.
	 */
	UAGX_RigidBodyComponent* FindReferenceFrameComponent(
		const FName& ComponentName, const FName& ActorName,
		const UAGX_ContactMaterialRegistrarComponent& Registrar)
	{
		check(Registrar.GetTypedOuter<AActor>());

		if (ComponentName.IsNone())
			return nullptr;

		AActor* OwningActor = ActorName.IsNone() ? Registrar.GetTypedOuter<AActor>()
												 : FindActorByName(Registrar.GetWorld(), ActorName);

		if (OwningActor == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("FindReferenceFrame() failed to find reference frame component matching "
					 "ComponentName: '%s', ActorName: '%s', Registrar: '%s' in '%s' because the "
					 "owning actor could not be found."),
				*ComponentName.ToString(), *ActorName.ToString(), *Registrar.GetName(),
				*GetFNameSafe(Registrar.GetOwner()).ToString());
			return nullptr;
		}

		TArray<UAGX_RigidBodyComponent*> Bodies;
		OwningActor->GetComponents(Bodies);
		UAGX_RigidBodyComponent** It = Bodies.FindByPredicate(
			[ComponentName](UAGX_RigidBodyComponent* Body)
			{ return Body->GetFName().IsEqual(ComponentName, ENameCase::CaseSensitive); });
		if (It == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("FindReferenceFrame() failed to find reference frame component matching "
					 "ComponentName: '%s', ActorName: '%s', Registrar: '%s' in '%s' because the "
					 "component could not be found inside the actor."),
				*ComponentName.ToString(), *ActorName.ToString(), *Registrar.GetName(),
				*GetFNameSafe(Registrar.GetOwner()).ToString());
			return nullptr;
		}
		return *It;
	}
}

void UAGX_ContactMaterial::UpdateNativeProperties(
	const UAGX_ContactMaterialRegistrarComponent& Registrar)
{
	if (!IsInstance() || !HasNative())
	{
		return;
	}

	// Friction related properties
	{
		// Setting Friction Model before Normal Force Magnitude, Scale With
		// Depth, and Solve Type because they are part of the Friction Model object.
		NativeBarrier.SetFrictionModel(FrictionModel);

		NativeBarrier.SetSurfaceFrictionEnabled(bEnableSurfaceFriction);
		if (bUseSecondaryFrictionCoefficient)
		{
			NativeBarrier.SetPrimaryFrictionCoefficient(FrictionCoefficient);
			NativeBarrier.SetSecondaryFrictionCoefficient(SecondaryFrictionCoefficient);
		}
		else
		{
			NativeBarrier.SetFrictionCoefficient(FrictionCoefficient);
		}
		if (bUseSecondarySurfaceViscosity)
		{
			NativeBarrier.SetPrimarySurfaceViscosity(SurfaceViscosity);
			NativeBarrier.SetSecondarySurfaceViscosity(SecondarySurfaceViscosity);
		}
		else
		{
			NativeBarrier.SetSurfaceViscosity(SurfaceViscosity);
		}

		// Update properties exclusive to Constant Normal Force friction models.
		if (IsConstantNormalForceFrictionModel())
		{
			NativeBarrier.SetNormalForceMagnitude(NormalForceMagnitude);
			NativeBarrier.SetEnableScaleNormalForceWithDepth(bScaleNormalForceWithDepth);
		}

		// Update properties exclusive to oriented friction models.
		if (IsOrientedFrictionModel())
		{
			NativeBarrier.SetPrimaryDirection(PrimaryDirection);

			// Set reference frame on native.
			UAGX_RigidBodyComponent* ReferenceFrameBody =
				AGX_ContactMaterial_helpers::FindReferenceFrameComponent(
					OrientedFrictionReferenceFrameComponent, OrientedFrictionReferenceFrameActor,
					Registrar);
			if (!ReferenceFrameBody)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("ContactMaterial '%s' has a oriented friction model but the component "
						 "to use as Reference Frame could not be found. Oriented friction "
						 "might not work as expected."),
					*GetName());
				NativeBarrier.SetOrientedFrictionModelReferenceFrame(nullptr);
			}
			else
			{
				FRigidBodyBarrier* ReferenceFrameBodyBarrier =
					ReferenceFrameBody->GetOrCreateNative();
				check(ReferenceFrameBodyBarrier);
				NativeBarrier.SetOrientedFrictionModelReferenceFrame(ReferenceFrameBodyBarrier);
			}
		}
	}

	// Contact Processing related properties
	{
		NativeBarrier.SetFrictionSolveType(ContactSolver);
		NativeBarrier.SetContactReductionMode(ContactReduction.Mode);
		NativeBarrier.SetContactReductionLevel(ContactReduction.ContactReductionLevel);
		NativeBarrier.SetUseContactAreaApproach(MechanicsApproach.bUseContactAreaApproach);
		NativeBarrier.SetMinMaxElasticRestLength(
			MechanicsApproach.MinElasticRestLength, MechanicsApproach.MaxElasticRestLength);
	}

	// General properties
	{
		NativeBarrier.SetRestitution(Restitution);
		NativeBarrier.SetYoungsModulus(YoungsModulus);
		NativeBarrier.SetSpookDamping(SpookDamping);
		NativeBarrier.SetAdhesion(AdhesiveForce, AdhesiveOverlap);
	}
}

void UAGX_ContactMaterial::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	ContactReduction.Serialize(Archive);
}

void UAGX_ContactMaterial::PostInitProperties()
{
	UObject::PostInitProperties();
#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR

void UAGX_ContactMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	UObject::PostEditChangeChainProperty(Event);
}

void UAGX_ContactMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

#ifdef DEFAULT_DISPATCHER
#error "Macro name DEFAULT_DISPATCHER aleady in use, chose a different name."
#endif

#define DEFAULT_DISPATCHER(Property)                  \
	Dispatcher.Add(                                   \
		GET_MEMBER_NAME_CHECKED(ThisClass, Property), \
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Property, Set##Property) });

#ifdef NESTED_DISPATCHER
#error "Macro name NESTED_DISPATCHER already in use, chose a different name."
#endif

#define NESTED_DISPATCHER(OuterProperty, OuterType, InnerProperty, Setter) \
	Dispatcher.Add(                                                        \
		GET_MEMBER_NAME_CHECKED(ThisClass, OuterProperty),                 \
		GET_MEMBER_NAME_CHECKED(OuterType, InnerProperty),                 \
		[](ThisClass* This)                                                \
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(OuterProperty.InnerProperty, Setter) });

#ifdef SETTER_DISPATCHER
#error "Macro name SETTER_DISPATCHER already in use, chose a different name."
#endif

#define SETTER_DISPATCHER(Property, Setter)           \
	Dispatcher.Add(                                   \
		GET_MEMBER_NAME_CHECKED(ThisClass, Property), \
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Property, Setter) });

	// Dispatcher registration starts here.

	DEFAULT_DISPATCHER(ContactSolver);

	NESTED_DISPATCHER(
		ContactReduction, FAGX_ContactMaterialReductionMode, Mode, SetContactReductionMode);

	NESTED_DISPATCHER(
		ContactReduction, FAGX_ContactMaterialReductionMode, ContactReductionLevel,
		SetContactReductionLevel);

	NESTED_DISPATCHER(
		MechanicsApproach, FAGX_ContactMaterialMechanicsApproach, bUseContactAreaApproach,
		SetUseContactAreaApproach);

	NESTED_DISPATCHER(
		MechanicsApproach, FAGX_ContactMaterialMechanicsApproach, MinElasticRestLength,
		SetMinElasticRestLength);

	NESTED_DISPATCHER(
		MechanicsApproach, FAGX_ContactMaterialMechanicsApproach, MaxElasticRestLength,
		SetMaxElasticRestLength);

	DEFAULT_DISPATCHER(FrictionModel);
	DEFAULT_DISPATCHER(NormalForceMagnitude);
	SETTER_DISPATCHER(bScaleNormalForceWithDepth, SetScaleNormalForceWithDepth);
	SETTER_DISPATCHER(bEnableSurfaceFriction, SetSurfaceFrictionEnabled);
	DEFAULT_DISPATCHER(FrictionCoefficient);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, SecondaryFrictionCoefficient),
		[](ThisClass* This)
		{
			// When using InlineEditConditionToggle on a bool property for some other property, when
			// editing the bool Unreal Editor triggers the value callback instead of the bool
			// callback. So here we don't know which was changed. Updating both.
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				bUseSecondaryFrictionCoefficient, SetUseSecondaryFrictionCoefficient);
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				SecondaryFrictionCoefficient, SetSecondaryFrictionCoefficient);
		});

	// This is currently (Unreal Engine 4.25) never called because Unreal Engine calls the
	// value callback when toggling the InlineEditConditionToggle. Leaving it here in case
	// that's changed in later versions.
	SETTER_DISPATCHER(bUseSecondaryFrictionCoefficient, SetUseSecondaryFrictionCoefficient);

	DEFAULT_DISPATCHER(SurfaceViscosity);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, SecondarySurfaceViscosity),
		[](ThisClass* This)
		{
			// When using InlineEditConditionToggle on the bool Unreal Editor triggers the value
			// callback, instead of the bool callback, when toggling the bool. So we don't know
			// which it is. Setting both.
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				bUseSecondarySurfaceViscosity, SetUseSecondarySurfaceViscosity);
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				SecondarySurfaceViscosity, SetSecondarySurfaceViscosity);
		});

	// This is currently (Unreal Engine 4.25) never called because Unreal Engine calls the
	// value callback when toggling the InlineEditConditionToggle. Leaving it here in case
	// that's changed in later versions.
	SETTER_DISPATCHER(bUseSecondarySurfaceViscosity, SetUseSecondarySurfaceViscosity);

	DEFAULT_DISPATCHER(PrimaryDirection);

	// Here we would like to detect and handle changes to the Oriented Friction Reference Frame,
	// but that is currently not possible because to update the AGX Dynamics side we need to find
	// the Rigid Body Component, and to find the Rigid Body Component we need the Contact Material
	// Registrar that was used to create the Contact Material Instance. We currently don't store
	// that in the Contact Material Asset. See internal GitLab issue 707.
#if 0
	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, OrientedFrictionReferenceFrameComponent),
		[](ThisClass* This) {
			if (IsInstance())
			{
				This->Asset->OrientedFrictionReferenceFrameComponent =
					This->OrientedFrictionReferenceFrameComponent;
			}

			This->SetOrientedFrictionReferenceFrameComponent(
				This->OrientedFrictionReferenceFrameComponent, Registrar);
		});

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, OrientedFrictionReferenceFrameActor),
		[](ThisClass* This) {
			if (IsInstance())
			{
				This->Asset->OrientedFrictionReferenceFrameComponent =
					This->OrientedFrictionReferenceFrameComponent;
			}
			This->SetOrientedFrictionReferenceFrameActor(
				This->OrientedFrictionReferenceFrameActor, Registrar);
		});
#endif

	DEFAULT_DISPATCHER(Restitution);
	DEFAULT_DISPATCHER(SpookDamping);
	DEFAULT_DISPATCHER(YoungsModulus);
	DEFAULT_DISPATCHER(AdhesiveForce);
	DEFAULT_DISPATCHER(AdhesiveOverlap);

#undef DEFAULT_DISPATCHER
#undef NESTED_DISPATCHER
#undef SETTER_DISPATCHER
}

#endif

void UAGX_ContactMaterial::CreateNative(const UAGX_ContactMaterialRegistrarComponent& Registrar)
{
	if (IsInstance())
	{
		check(!HasNative());
		UWorld* World = Registrar.GetWorld();
		check(World != nullptr && World->IsGameWorld());

		Material1 = Material1 != nullptr ? Material1->GetOrCreateInstance(World) : nullptr;
		Material2 = Material2 != nullptr ? Material2->GetOrCreateInstance(World) : nullptr;

		FShapeMaterialBarrier* MaterialBarrier1 =
			Material1 != nullptr ? Material1->GetOrCreateShapeMaterialNative(World) : nullptr;
		FShapeMaterialBarrier* MaterialBarrier2 =
			Material2 != nullptr ? Material2->GetOrCreateShapeMaterialNative(World) : nullptr;

		NativeBarrier.AllocateNative(MaterialBarrier1, MaterialBarrier2);
		if (!HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("UAGX_ContactMaterial '%s' failed to create native AGX Dynamics instance. See "
					 "the AGXDynamics log channel for additional information."),
				*GetName());
			return;
		}
		UpdateNativeProperties(Registrar);
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"CreateNative was colled on an UAGX_ContactMaterial who's instance is nullptr. "
					"Ensure e.g. GetOrCreateInstance is called prior to calling this function"));
			return;
		}
		return Instance->CreateNative(Registrar);
	}
}

#undef LOCTEXT_NAMESPACE
