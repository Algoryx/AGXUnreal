// Copyright 2024, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

#if WITH_EDITOR
void UAGX_ShapeMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ShapeMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShapeMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	// Surface properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, bFrictionEnabled),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Surface.bFrictionEnabled, SetFrictionEnabled) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Roughness),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Surface.Roughness, SetRoughness) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, Viscosity),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Surface.Viscosity, SetSurfaceViscosity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveForce),
		[](ThisClass* This)
		{
			if (This->IsInstance())
			{
				This->Asset->Surface.AdhesiveForce = This->Surface.AdhesiveForce;
			}
			This->SetAdhesion(This->Surface.AdhesiveForce, This->Surface.AdhesiveOverlap);
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Surface),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialSurfaceProperties, AdhesiveOverlap),
		[](ThisClass* This)
		{
			if (This->IsInstance())
			{
				This->Asset->Surface.AdhesiveOverlap = This->Surface.AdhesiveOverlap;
			}
			This->SetAdhesion(This->Surface.AdhesiveForce, This->Surface.AdhesiveOverlap);
		});

	// Bulk properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, Density),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Bulk.Density, SetDensity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, Viscosity),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Bulk.Viscosity, SetBulkViscosity) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, SpookDamping),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Bulk.SpookDamping, SetSpookDamping) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, MinElasticRestLength),
		[](ThisClass* This)
		{
			if (This->IsInstance())
			{
				This->Asset->Bulk.MinElasticRestLength = This->Bulk.MinElasticRestLength;
			}
			This->SetMinMaxElasticRestLength(
				This->Bulk.MinElasticRestLength, This->Bulk.MaxElasticRestLength);
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Bulk),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialBulkProperties, MaxElasticRestLength),
		[](ThisClass* This)
		{
			if (This->IsInstance())
			{
				This->Asset->Bulk.MaxElasticRestLength = This->Bulk.MaxElasticRestLength;
			}
			This->SetMinMaxElasticRestLength(
				This->Bulk.MinElasticRestLength, This->Bulk.MaxElasticRestLength);
		});

	// Wire properties.
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, YoungsModulusStretch),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Wire.YoungsModulusStretch, SetYoungsModulusStretch) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, YoungsModulusBend),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Wire.YoungsModulusBend, SetYoungsModulusBend) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, SpookDampingStretch),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Wire.SpookDampingStretch, SetSpookDampingStretch) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeMaterial, Wire),
		GET_MEMBER_NAME_CHECKED(FAGX_ShapeMaterialWireProperties, SpookDampingBend),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(Wire.SpookDampingBend, SetSpookDampingBend) });
}
#endif

// Surface properties.
void UAGX_ShapeMaterial::SetFrictionEnabled(bool Enabled)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Surface.bFrictionEnabled, Enabled, SetFrictionEnabled);
}

bool UAGX_ShapeMaterial::GetFrictionEnabled() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Surface.bFrictionEnabled, GetFrictionEnabled);
}

void UAGX_ShapeMaterial::SetRoughness(double InRoughness)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Surface.Roughness, InRoughness, SetRoughness);
}

double UAGX_ShapeMaterial::GetRoughness() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Surface.Roughness, GetRoughness);
}

void UAGX_ShapeMaterial::SetSurfaceViscosity(double Viscosity)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Surface.Viscosity, Viscosity, SetSurfaceViscosity);
}

double UAGX_ShapeMaterial::GetSurfaceViscosity() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Surface.Viscosity, GetSurfaceViscosity);
}

void UAGX_ShapeMaterial::SetAdhesion(double AdhesiveForce, double AdhesiveOverlap)
{
	if (IsInstance())
	{
		Surface.AdhesiveForce = AdhesiveForce;
		Surface.AdhesiveOverlap = AdhesiveOverlap;
		if (HasNative())
		{
			NativeBarrier.SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);
		}
	}
	else // IsAsset
	{
		if (Instance != nullptr)
		{
			Instance->SetAdhesion(AdhesiveForce, AdhesiveOverlap);
			return;
		}
		Surface.AdhesiveForce = AdhesiveForce;
		Surface.AdhesiveOverlap = AdhesiveOverlap;
	}
}

double UAGX_ShapeMaterial::GetAdhesiveForce() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Surface.AdhesiveForce, GetAdhesiveForce);
}

double UAGX_ShapeMaterial::GetAdhesiveOverlap() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Surface.AdhesiveOverlap, GetAdhesiveOverlap);
}

// Bulk properties.

void UAGX_ShapeMaterial::SetDensity(double InDensity)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Bulk.Density, InDensity, SetDensity);
}

double UAGX_ShapeMaterial::GetDensity() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Bulk.Density, GetDensity);
}

void UAGX_ShapeMaterial::SetYoungsModulus(double InYoungsModulus)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Bulk.YoungsModulus, InYoungsModulus, SetYoungsModulus);
}

double UAGX_ShapeMaterial::GetYoungsModulus() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Bulk.YoungsModulus, GetYoungsModulus);
}

void UAGX_ShapeMaterial::SetBulkViscosity(double InBulkViscosity)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Bulk.Viscosity, InBulkViscosity, SetBulkViscosity);
}

double UAGX_ShapeMaterial::GetBulkViscosity() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Bulk.Viscosity, GetBulkViscosity);
}

void UAGX_ShapeMaterial::SetSpookDamping(double InSpookDamping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Bulk.SpookDamping, InSpookDamping, SetSpookDamping);
}

double UAGX_ShapeMaterial::GetSpookDamping() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Bulk.SpookDamping, GetSpookDamping);
}

void UAGX_ShapeMaterial::SetMinMaxElasticRestLength(double InMin, double InMax)
{
	if (IsInstance())
	{
		Bulk.MinElasticRestLength = InMin;
		Bulk.MaxElasticRestLength = InMax;
		if (HasNative())
		{
			NativeBarrier.SetMinMaxElasticRestLength(
				Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);
		}
	}
	else // IsAsset
	{
		if (Instance != nullptr)
		{
			Instance->SetMinMaxElasticRestLength(InMin, InMax);
			return;
		}
		Bulk.MinElasticRestLength = InMin;
		Bulk.MaxElasticRestLength = InMax;
	}
}

double UAGX_ShapeMaterial::GetMinElasticRestLength() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Bulk.MinElasticRestLength, GetMinElasticRestLength);
}

double UAGX_ShapeMaterial::GetMaxElasticRestLength() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Bulk.MaxElasticRestLength, GetMaxElasticRestLength);
}

// Wire properties.

double UAGX_ShapeMaterial::GetYoungsModulusStretch() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Wire.YoungsModulusStretch, GetYoungsModulusStretch);
}

void UAGX_ShapeMaterial::SetYoungsModulusStretch(double InYoungsModulus)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		Wire.YoungsModulusStretch, InYoungsModulus, SetYoungsModulusStretch);
}

double UAGX_ShapeMaterial::GetYoungsModulusBend() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Wire.YoungsModulusBend, GetYoungsModulusBend);
}

void UAGX_ShapeMaterial::SetYoungsModulusBend(double InYoungsModulus)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Wire.YoungsModulusBend, InYoungsModulus, SetYoungsModulusBend);
}

double UAGX_ShapeMaterial::GetSpookDampingStretch() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Wire.SpookDampingStretch, GetSpookDampingStretch);
}

void UAGX_ShapeMaterial::SetSpookDampingStretch(double InSpookDamping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Wire.SpookDampingStretch, InSpookDamping, SetSpookDampingStretch);
}

double UAGX_ShapeMaterial::GetSpookDampingBend() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Wire.SpookDampingBend, GetSpookDampingBend);
}

void UAGX_ShapeMaterial::SetSpookDampingBend(double InSpookDamping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Wire.SpookDampingBend, InSpookDamping, SetSpookDampingBend);
}

void UAGX_ShapeMaterial::CopyFrom(const FShapeMaterialBarrier& Source, FAGX_ImportContext* Context)
{
	// Copy shape material bulk properties.
	Bulk.Density = Source.GetDensity();
	Bulk.YoungsModulus = Source.GetYoungsModulus();
	Bulk.Viscosity = Source.GetBulkViscosity();
	Bulk.SpookDamping = Source.GetSpookDamping();
	Bulk.MinElasticRestLength = Source.GetMinElasticRestLength();
	Bulk.MaxElasticRestLength = Source.GetMaxElasticRestLength();

	// Copy shape material surface properties.
	Surface.bFrictionEnabled = Source.GetFrictionEnabled();
	Surface.Roughness = Source.GetRoughness();
	Surface.Viscosity = Source.GetSurfaceViscosity();
	Surface.AdhesiveForce = Source.GetAdhesiveForce();
	Surface.AdhesiveOverlap = Source.GetAdhesiveOverlap();

	ImportGuid = Source.GetGuid();

	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		GetOuter(), Source.GetName(), UAGX_ShapeMaterial::StaticClass());
	Rename(*Name);

	if (Context != nullptr && Context->ShapeMaterials != nullptr &&
		!Context->ShapeMaterials->Contains(ImportGuid))
	{
		Context->ShapeMaterials->Add(ImportGuid, this);
	}
}

UAGX_ShapeMaterial* UAGX_ShapeMaterial::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_ShapeMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ShapeMaterial::CreateInstanceFromAsset(PlayingWorld, this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_ShapeMaterial* UAGX_ShapeMaterial::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_ShapeMaterial* Source)
{
	check(Source);
	check(!Source->IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source->GetName() + "_Instance";

	UAGX_ShapeMaterial* NewInstance = NewObject<UAGX_ShapeMaterial>(
		GetTransientPackage(), UAGX_ShapeMaterial::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;
	NewInstance->CopyShapeMaterialProperties(Source);
	NewInstance->CreateNative(PlayingWorld);

	return NewInstance;
}

FShapeMaterialBarrier* UAGX_ShapeMaterial::GetOrCreateShapeMaterialNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateShapeMaterialNative was called on UAGX_ShapeMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateShapeMaterialNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative(PlayingWorld);
	}
	return GetNative();
}

void UAGX_ShapeMaterial::CommitToAsset()
{
	if (IsInstance())
	{
		if (FShapeMaterialBarrier* Barrier = this->GetNative())
		{
#if WITH_EDITOR
			Asset->Modify();
#endif
			Asset->CopyFrom(*Barrier, nullptr);
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(*Asset);
#endif
		}
	}
	else if (Instance != nullptr) // IsAsset
	{
		Instance->CommitToAsset();
	}
}

void UAGX_ShapeMaterial::CreateNative(UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on a UAGX_ShapeMaterial who's instance is nullptr. "
					 "Ensure e.g. GetOrCreateInstance is called prior to calling this function."));
			return;
		}
		return Instance->CreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (NativeBarrier.HasNative())
	{
		NativeBarrier.ReleaseNative();
	}

	NativeBarrier.AllocateNative(TCHAR_TO_UTF8(*GetName()));
	check(HasNative());

	UpdateNativeProperties();

	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(PlayingWorld);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shape Material '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName());
		return;
	}

	Simulation->Add(*this);
}

FShapeMaterialBarrier* UAGX_ShapeMaterial::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

const FShapeMaterialBarrier* UAGX_ShapeMaterial::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

bool UAGX_ShapeMaterial::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

void UAGX_ShapeMaterial::UpdateNativeProperties()
{
	if (HasNative())
	{
		AGX_CHECK(IsInstance());
		NativeBarrier.SetName(TCHAR_TO_UTF8(*GetName()));

		// Bulk properties.
		NativeBarrier.SetDensity(Bulk.Density);
		NativeBarrier.SetYoungsModulus(Bulk.YoungsModulus);
		NativeBarrier.SetBulkViscosity(Bulk.Viscosity);
		NativeBarrier.SetSpookDamping(Bulk.SpookDamping);
		NativeBarrier.SetMinMaxElasticRestLength(
			Bulk.MinElasticRestLength, Bulk.MaxElasticRestLength);

		// Surface properties.
		NativeBarrier.SetFrictionEnabled(Surface.bFrictionEnabled);
		NativeBarrier.SetRoughness(Surface.Roughness);
		NativeBarrier.SetSurfaceViscosity(Surface.Viscosity);
		NativeBarrier.SetAdhesion(Surface.AdhesiveForce, Surface.AdhesiveOverlap);

		// Wire properties.
		NativeBarrier.SetYoungsModulusStretch(Wire.YoungsModulusStretch);
		NativeBarrier.SetYoungsModulusBend(Wire.YoungsModulusBend);
		NativeBarrier.SetSpookDampingStretch(Wire.SpookDampingStretch);
		NativeBarrier.SetSpookDampingBend(Wire.SpookDampingBend);
	}
}

bool UAGX_ShapeMaterial::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;

	// Internal testing the hypothesis that UObject::IsAsset is a valid inverse of this function.
	// @todo Consider removing this function and instead use UObject::IsAsset, if the below check
	// has never failed.
	AGX_CHECK(bIsInstance != IsAsset());

	return bIsInstance;
}

void UAGX_ShapeMaterial::CopyShapeMaterialProperties(const UAGX_ShapeMaterial* Source)
{
	if (Source)
	{
		Bulk = Source->Bulk;
		Surface = Source->Surface;
		Wire = Source->Wire;
	}
}
