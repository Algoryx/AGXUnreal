// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"


void UAGX_TerrainWheelMaterial::SetSinkageExponentParameterA(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SinkageExponentParameterA, Value, SetSinkageExponentParameterA);
}

double UAGX_TerrainWheelMaterial::GetSinkageExponentParameterA() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SinkageExponentParameterA, GetSinkageExponentParameterA);
}

void UAGX_TerrainWheelMaterial::SetSinkageExponentParameterB(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(SinkageExponentParameterB, Value, SetSinkageExponentParameterB);
}

double UAGX_TerrainWheelMaterial::GetSinkageExponentParameterB() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SinkageExponentParameterB, GetSinkageExponentParameterB);
}

void UAGX_TerrainWheelMaterial::SetCohesion(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Cohesion, Value, SetCohesion);
}

double UAGX_TerrainWheelMaterial::GetCohesion() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Cohesion, GetCohesion);
}

void UAGX_TerrainWheelMaterial::SetAngleOfInternalFriction(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(AngleOfInternalFriction, Value, SetAngleOfInternalFriction);
}

double UAGX_TerrainWheelMaterial::GetAngleOfInternalFriction() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AngleOfInternalFriction, GetAngleOfInternalFriction);
}

void UAGX_TerrainWheelMaterial::SetShearModulusXParameterA(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearModulusXParameterA, Value, SetShearModulusXParameterA);
}

double UAGX_TerrainWheelMaterial::GetShearModulusXParameterA() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearModulusXParameterA, GetShearModulusXParameterA);
}

void UAGX_TerrainWheelMaterial::SetShearModulusXParameterB(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearModulusXParameterB, Value, SetShearModulusXParameterB);
}

double UAGX_TerrainWheelMaterial::GetShearModulusXParameterB() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearModulusXParameterB, GetShearModulusXParameterB);
}

void UAGX_TerrainWheelMaterial::SetShearModulusYParameterA(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearModulusYParameterA, Value, SetShearModulusYParameterA);
}

double UAGX_TerrainWheelMaterial::GetShearModulusYParameterA() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearModulusYParameterA, GetShearModulusYParameterA);
}

void UAGX_TerrainWheelMaterial::SetShearModulusYParameterB(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearModulusYParameterB, Value, SetShearModulusYParameterB);
}

double UAGX_TerrainWheelMaterial::GetShearModulusYParameterB() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearModulusYParameterB, GetShearModulusYParameterB);
}

void UAGX_TerrainWheelMaterial::SetCohesiveModulusBekker(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(CohesiveModulusBekker, Value, SetCohesiveModulusBekker);
}

double UAGX_TerrainWheelMaterial::GetCohesiveModulusBekker() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(CohesiveModulusBekker, GetCohesiveModulusBekker);
}

void UAGX_TerrainWheelMaterial::SetFrictionalModulusBekker(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(FrictionalModulusBekker, Value, SetFrictionalModulusBekker);
}

double UAGX_TerrainWheelMaterial::GetFrictionalModulusBekker() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(FrictionalModulusBekker, GetFrictionalModulusBekker);
}

void UAGX_TerrainWheelMaterial::SetCohesiveModulusReece(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(CohesiveModulusReece, Value, SetCohesiveModulusReece);
}

double UAGX_TerrainWheelMaterial::GetCohesiveModulusReece() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(CohesiveModulusReece, GetCohesiveModulusReece);
}

void UAGX_TerrainWheelMaterial::SetFrictionalModulusReece(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(FrictionalModulusReece, Value, SetFrictionalModulusReece);
}

double UAGX_TerrainWheelMaterial::GetFrictionalModulusReece() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(FrictionalModulusReece, GetFrictionalModulusReece);
}

void UAGX_TerrainWheelMaterial::SetMassDensity(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(MassDensity, Value, SetMassDensity);
}

double UAGX_TerrainWheelMaterial::GetMassDensity() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MassDensity, GetMassDensity);
}

void UAGX_TerrainWheelMaterial::SetMaximumNormalStressAngleParameterA(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaximumNormalStressAngleParameterA, Value, SetMaximumNormalStressAngleParameterA);
}

double UAGX_TerrainWheelMaterial::GetMaximumNormalStressAngleParameterA() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		MaximumNormalStressAngleParameterA, GetMaximumNormalStressAngleParameterA);
}

void UAGX_TerrainWheelMaterial::SetMaximumNormalStressAngleParameterB(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaximumNormalStressAngleParameterB, Value, SetMaximumNormalStressAngleParameterB);
}

double UAGX_TerrainWheelMaterial::GetMaximumNormalStressAngleParameterB() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		MaximumNormalStressAngleParameterB, GetMaximumNormalStressAngleParameterB);
}

void UAGX_TerrainWheelMaterial::SetRearAngleParameterA(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(RearAngleParameterA, Value, SetRearAngleParameterA);
}

double UAGX_TerrainWheelMaterial::GetRearAngleParameterA() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(RearAngleParameterA, GetRearAngleParameterA);
}

void UAGX_TerrainWheelMaterial::SetRearAngleParameterB(double Value)
{
	AGX_ASSET_SETTER_IMPL_VALUE(RearAngleParameterB, Value, SetRearAngleParameterB);
}

double UAGX_TerrainWheelMaterial::GetRearAngleParameterB() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(RearAngleParameterB, GetRearAngleParameterB);
}


bool UAGX_TerrainWheelMaterial::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

FTerrainWheelMaterialBarrier* UAGX_TerrainWheelMaterial::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

const FTerrainWheelMaterialBarrier* UAGX_TerrainWheelMaterial::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

void UAGX_TerrainWheelMaterial::ReleaseNative()
{
	if (Instance != nullptr)
	{
		Instance->ReleaseNative();
		return;
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

void UAGX_TerrainWheelMaterial::CommitToAsset()
{
	if (IsInstance())
	{
		if (FTerrainWheelMaterialBarrier* Barrier = GetNative())
		{
#if WITH_EDITOR
			Asset->Modify();
#endif
			Asset->CopyFrom(*Barrier);
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

UAGX_TerrainWheelMaterial* UAGX_TerrainWheelMaterial::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_TerrainWheelMaterial& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_TerrainWheelMaterial* NewInstance = NewObject<UAGX_TerrainWheelMaterial>(
		GetTransientPackage(), UAGX_TerrainWheelMaterial::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_TerrainWheelMaterial* UAGX_TerrainWheelMaterial::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_TerrainWheelMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = CreateInstanceFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

FTerrainWheelMaterialBarrier* UAGX_TerrainWheelMaterial::GetOrCreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_TerrainWheelMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative();
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

void UAGX_TerrainWheelMaterial::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	AGX_CHECK(IsInstance());
	NativeBarrier.SetSinkageExponentParameterA(SinkageExponentParameterA);
	NativeBarrier.SetSinkageExponentParameterB(SinkageExponentParameterB);
	NativeBarrier.SetCohesion(Cohesion);
	NativeBarrier.SetAngleOfInternalFriction(AngleOfInternalFriction);
	NativeBarrier.SetShearModulusXParameterA(ShearModulusXParameterA);
	NativeBarrier.SetShearModulusXParameterB(ShearModulusXParameterB);
	NativeBarrier.SetShearModulusYParameterA(ShearModulusYParameterA);
	NativeBarrier.SetShearModulusYParameterB(ShearModulusYParameterB);
	NativeBarrier.SetCohesiveModulusBekker(CohesiveModulusBekker);
	NativeBarrier.SetFrictionalModulusBekker(FrictionalModulusBekker);
	NativeBarrier.SetCohesiveModulusReece(CohesiveModulusReece);
	NativeBarrier.SetFrictionalModulusReece(FrictionalModulusReece);
	NativeBarrier.SetMassDensity(MassDensity);
	NativeBarrier.SetMaximumNormalStressAngleParameterA(MaximumNormalStressAngleParameterA);
	NativeBarrier.SetMaximumNormalStressAngleParameterB(MaximumNormalStressAngleParameterB);
	NativeBarrier.SetRearAngleParameterA(RearAngleParameterA);
	NativeBarrier.SetRearAngleParameterB(RearAngleParameterB);
}

bool UAGX_TerrainWheelMaterial::IsInstance() const
{
	// This is the case for runtime imported instances.
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	// A runtime non-imported instance of this class will always have a reference to it's
	// corresponding Asset. An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;
	AGX_CHECK(bIsInstance != IsAsset());
	return bIsInstance;
}

void UAGX_TerrainWheelMaterial::CopyFrom(const FTerrainWheelMaterialBarrier& Source)
{
	if (!Source.HasNative())
		return;

	SinkageExponentParameterA = Source.GetSinkageExponentParameterA();
	SinkageExponentParameterB = Source.GetSinkageExponentParameterB();
	Cohesion = Source.GetCohesion();
	AngleOfInternalFriction = Source.GetAngleOfInternalFriction();
	ShearModulusXParameterA = Source.GetShearModulusXParameterA();
	ShearModulusXParameterB = Source.GetShearModulusXParameterB();
	ShearModulusYParameterA = Source.GetShearModulusYParameterA();
	ShearModulusYParameterB = Source.GetShearModulusYParameterB();
	CohesiveModulusBekker = Source.GetCohesiveModulusBekker();
	FrictionalModulusBekker = Source.GetFrictionalModulusBekker();
	CohesiveModulusReece = Source.GetCohesiveModulusReece();
	FrictionalModulusReece = Source.GetFrictionalModulusReece();
	MassDensity = Source.GetMassDensity();
	MaximumNormalStressAngleParameterA = Source.GetMaximumNormalStressAngleParameterA();
	MaximumNormalStressAngleParameterB = Source.GetMaximumNormalStressAngleParameterB();
	RearAngleParameterA = Source.GetRearAngleParameterA();
	RearAngleParameterB = Source.GetRearAngleParameterB();
}

void UAGX_TerrainWheelMaterial::CopyProperties(const UAGX_TerrainWheelMaterial& Source)
{
	SinkageExponentParameterA = Source.SinkageExponentParameterA;
	SinkageExponentParameterB = Source.SinkageExponentParameterB;
	Cohesion = Source.Cohesion;
	AngleOfInternalFriction = Source.AngleOfInternalFriction;
	ShearModulusXParameterA = Source.ShearModulusXParameterA;
	ShearModulusXParameterB = Source.ShearModulusXParameterB;
	ShearModulusYParameterA = Source.ShearModulusYParameterA;
	ShearModulusYParameterB = Source.ShearModulusYParameterB;
	CohesiveModulusBekker = Source.CohesiveModulusBekker;
	FrictionalModulusBekker = Source.FrictionalModulusBekker;
	CohesiveModulusReece = Source.CohesiveModulusReece;
	FrictionalModulusReece = Source.FrictionalModulusReece;
	MassDensity = Source.MassDensity;
	MaximumNormalStressAngleParameterA = Source.MaximumNormalStressAngleParameterA;
	MaximumNormalStressAngleParameterB = Source.MaximumNormalStressAngleParameterB;
	RearAngleParameterA = Source.RearAngleParameterA;
	RearAngleParameterB = Source.RearAngleParameterB;
}

void UAGX_TerrainWheelMaterial::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_TerrainWheelMaterial '%s' who's "
					 "instance is nullptr. "
					 "Ensure e.g. GetOrCreateInstance is called prior to calling this function."),
				*GetName());
			return;
		}
		return Instance->CreateNative();
	}

	AGX_CHECK(IsInstance());
	if (NativeBarrier.HasNative())
	{
		NativeBarrier.ReleaseNative();
	}

	NativeBarrier.AllocateNative();
	check(HasNative());

	UpdateNativeProperties();
}

#if WITH_EDITOR
void UAGX_TerrainWheelMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_TerrainWheelMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TerrainWheelMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	AGX_COMPONENT_DEFAULT_DISPATCHER(SinkageExponentParameterA);
	AGX_COMPONENT_DEFAULT_DISPATCHER(SinkageExponentParameterB);
	AGX_COMPONENT_DEFAULT_DISPATCHER(Cohesion);
	AGX_COMPONENT_DEFAULT_DISPATCHER(AngleOfInternalFriction);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearModulusXParameterA);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearModulusXParameterB);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearModulusYParameterA);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearModulusYParameterB);
	AGX_COMPONENT_DEFAULT_DISPATCHER(CohesiveModulusBekker);
	AGX_COMPONENT_DEFAULT_DISPATCHER(FrictionalModulusBekker);
	AGX_COMPONENT_DEFAULT_DISPATCHER(CohesiveModulusReece);
	AGX_COMPONENT_DEFAULT_DISPATCHER(FrictionalModulusReece);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MassDensity);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MaximumNormalStressAngleParameterA);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MaximumNormalStressAngleParameterB);
	AGX_COMPONENT_DEFAULT_DISPATCHER(RearAngleParameterA);
	AGX_COMPONENT_DEFAULT_DISPATCHER(RearAngleParameterB);
}
#endif
