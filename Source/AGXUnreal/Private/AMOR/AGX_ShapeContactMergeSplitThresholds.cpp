// Copyright 2025, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

void UAGX_ShapeContactMergeSplitThresholds::SetMaxImpactSpeed_BP(float InMaxImpactSpeed)
{
	SetMaxImpactSpeed(static_cast<double>(InMaxImpactSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxImpactSpeed(double InMaxImpactSpeed)
{
	AGX_ASSET_SETTER_IMPL_VALUE(MaxImpactSpeed, InMaxImpactSpeed, SetMaxImpactSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxImpactSpeed_BP() const
{
	return static_cast<float>(GetMaxImpactSpeed());
}

double UAGX_ShapeContactMergeSplitThresholds::GetMaxImpactSpeed() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxImpactSpeed, GetMaxImpactSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeNormalSpeed_BP(
	float InMaxRelativeNormalSpeed)
{
	SetMaxRelativeNormalSpeed(static_cast<double>(InMaxRelativeNormalSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeNormalSpeed(
	double InMaxRelativeNormalSpeed)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaxRelativeNormalSpeed, InMaxRelativeNormalSpeed, SetMaxRelativeNormalSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeNormalSpeed_BP() const
{
	return static_cast<float>(GetMaxRelativeNormalSpeed());
}

double UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeNormalSpeed() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxRelativeNormalSpeed, GetMaxRelativeNormalSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeTangentSpeed_BP(
	float InMaxRelativeTangentSpeed)
{
	SetMaxRelativeTangentSpeed(static_cast<double>(InMaxRelativeTangentSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRelativeTangentSpeed(
	double InMaxRelativeTangentSpeed)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MaxRelativeTangentSpeed, InMaxRelativeTangentSpeed, SetMaxRelativeTangentSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeTangentSpeed_BP() const
{
	return static_cast<float>(GetMaxRelativeTangentSpeed());
}

double UAGX_ShapeContactMergeSplitThresholds::GetMaxRelativeTangentSpeed() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxRelativeTangentSpeed, GetMaxRelativeTangentSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRollingSpeed_BP(float InMaxRollingSpeed)
{
	SetMaxRollingSpeed(static_cast<double>(InMaxRollingSpeed));
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaxRollingSpeed(double InMaxRollingSpeed)
{
	AGX_ASSET_SETTER_IMPL_VALUE(MaxRollingSpeed, InMaxRollingSpeed, SetMaxRollingSpeed);
}

float UAGX_ShapeContactMergeSplitThresholds::GetMaxRollingSpeed_BP() const
{
	return static_cast<float>(GetMaxRollingSpeed());
}

double UAGX_ShapeContactMergeSplitThresholds::GetMaxRollingSpeed() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(MaxRollingSpeed, GetMaxRollingSpeed);
}

void UAGX_ShapeContactMergeSplitThresholds::SetNormalAdhesion_BP(float InNormalAdhesion)
{
	SetNormalAdhesion(static_cast<double>(InNormalAdhesion));
}

void UAGX_ShapeContactMergeSplitThresholds::SetNormalAdhesion(double InNormalAdhesion)
{
	AGX_ASSET_SETTER_IMPL_VALUE(NormalAdhesion, InNormalAdhesion, SetNormalAdhesion);
}

float UAGX_ShapeContactMergeSplitThresholds::GetNormalAdhesion_BP() const
{
	return static_cast<float>(GetNormalAdhesion());
}

double UAGX_ShapeContactMergeSplitThresholds::GetNormalAdhesion() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(NormalAdhesion, GetNormalAdhesion);
}

void UAGX_ShapeContactMergeSplitThresholds::SetTangentialAdhesion_BP(float InTangentialAdhesion)
{
	SetTangentialAdhesion(static_cast<double>(InTangentialAdhesion));
}

void UAGX_ShapeContactMergeSplitThresholds::SetTangentialAdhesion(double InTangentialAdhesion)
{
	AGX_ASSET_SETTER_IMPL_VALUE(TangentialAdhesion, InTangentialAdhesion, SetTangentialAdhesion);
}

float UAGX_ShapeContactMergeSplitThresholds::GetTangentialAdhesion_BP() const
{
	return static_cast<float>(GetTangentialAdhesion());
}

double UAGX_ShapeContactMergeSplitThresholds::GetTangentialAdhesion() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(TangentialAdhesion, GetTangentialAdhesion);
}

void UAGX_ShapeContactMergeSplitThresholds::SetMaySplitInGravityField(
	bool bInMaySplitInGravityField)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		bMaySplitInGravityField, bInMaySplitInGravityField, SetMaySplitInGravityField);
}

bool UAGX_ShapeContactMergeSplitThresholds::GetMaySplitInGravityField() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bMaySplitInGravityField, GetMaySplitInGravityField);
}

void UAGX_ShapeContactMergeSplitThresholds::SetSplitOnLogicalImpact(bool bInSplitOnLogicalImpact)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		bSplitOnLogicalImpact, bInSplitOnLogicalImpact, SetSplitOnLogicalImpact);
}

bool UAGX_ShapeContactMergeSplitThresholds::GetSplitOnLogicalImpact() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bSplitOnLogicalImpact, GetSplitOnLogicalImpact);
}

#if WITH_EDITOR
void UAGX_ShapeContactMergeSplitThresholds::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_ShapeContactMergeSplitThresholds::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_ShapeContactMergeSplitThresholds::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxImpactSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxImpactSpeed, SetMaxImpactSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxRelativeNormalSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxRelativeNormalSpeed, SetMaxRelativeNormalSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxRelativeTangentSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxRelativeTangentSpeed, SetMaxRelativeTangentSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, MaxRollingSpeed),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(MaxRollingSpeed, SetMaxRollingSpeed) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, NormalAdhesion),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(NormalAdhesion, SetNormalAdhesion) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, TangentialAdhesion),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(TangentialAdhesion, SetTangentialAdhesion) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, bMaySplitInGravityField),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(bMaySplitInGravityField, SetMaySplitInGravityField) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_ShapeContactMergeSplitThresholds, bSplitOnLogicalImpact),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(bSplitOnLogicalImpact, SetSplitOnLogicalImpact) });
}
#endif

UAGX_ShapeContactMergeSplitThresholds* UAGX_ShapeContactMergeSplitThresholds::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_ShapeContactMergeSplitThresholds* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = UAGX_ShapeContactMergeSplitThresholds::CreateFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

UAGX_ShapeContactMergeSplitThresholds* UAGX_ShapeContactMergeSplitThresholds::GetInstance()
{
	if (IsInstance())
	{
		return this;
	}

	return Instance.Get();
}

void UAGX_ShapeContactMergeSplitThresholds::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_ShapeContactMergeSplitThresholds "
					 "'%s' who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called "
					 "prior "
					 "to calling this function."),
				*GetName());
			return;
		}

		Instance->CreateNative();
		return;
	}

	AGX_CHECK(IsInstance());
	NativeBarrier.AllocateNative();
	AGX_CHECK(HasNative());

	SetNativeProperties();
}

FShapeContactMergeSplitThresholdsBarrier* UAGX_ShapeContactMergeSplitThresholds::GetOrCreateNative(
	UWorld* PlayingWorld)
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_ShapeContactMergeSplitThresholds '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative(PlayingWorld);
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative();
	}

	return &NativeBarrier;
}

FShapeContactMergeSplitThresholdsBarrier* UAGX_ShapeContactMergeSplitThresholds::GetNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			return nullptr;
		}

		return Instance->GetNative();
	}

	AGX_CHECK(IsInstance());
	return HasNative() ? &NativeBarrier : nullptr;
}

const FShapeContactMergeSplitThresholdsBarrier* UAGX_ShapeContactMergeSplitThresholds::GetNative()
	const
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			return nullptr;
		}

		return Instance->GetNative();
	}

	AGX_CHECK(IsInstance());
	return HasNative() ? &NativeBarrier : nullptr;
}

bool UAGX_ShapeContactMergeSplitThresholds::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

UAGX_ShapeContactMergeSplitThresholds* UAGX_ShapeContactMergeSplitThresholds::CreateFromAsset(
	UWorld* PlayingWorld, UAGX_ShapeContactMergeSplitThresholds& Source)
{
	AGX_CHECK(PlayingWorld);
	AGX_CHECK(PlayingWorld->IsGameWorld());
	AGX_CHECK(!Source.IsInstance());

	const FString InstanceName = Source.GetName() + "_Instance";
	auto NewInstance = NewObject<UAGX_ShapeContactMergeSplitThresholds>(
		GetTransientPackage(), UAGX_ShapeContactMergeSplitThresholds::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

bool UAGX_ShapeContactMergeSplitThresholds::IsInstance() const
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

void UAGX_ShapeContactMergeSplitThresholds::CopyFrom(const FMergeSplitThresholdsBarrier& Barrier)
{
	const auto ScMstBarrier =
		static_cast<const FShapeContactMergeSplitThresholdsBarrier*>(&Barrier);
	MaxImpactSpeed = ScMstBarrier->GetMaxImpactSpeed();
	MaxRelativeNormalSpeed = ScMstBarrier->GetMaxRelativeNormalSpeed();
	MaxRelativeTangentSpeed = ScMstBarrier->GetMaxRelativeTangentSpeed();
	MaxRollingSpeed = ScMstBarrier->GetMaxRollingSpeed();
	NormalAdhesion = ScMstBarrier->GetNormalAdhesion();
	TangentialAdhesion = ScMstBarrier->GetTangentialAdhesion();
	bMaySplitInGravityField = ScMstBarrier->GetMaySplitInGravityField();
	bSplitOnLogicalImpact = ScMstBarrier->GetSplitOnLogicalImpact();
	ImportGuid = ScMstBarrier->GetGuid();
}

void UAGX_ShapeContactMergeSplitThresholds::CopyFrom(
	const UAGX_ShapeContactMergeSplitThresholds& Source)
{
	MaxImpactSpeed = Source.MaxImpactSpeed;
	MaxRelativeNormalSpeed = Source.MaxRelativeNormalSpeed;
	MaxRelativeTangentSpeed = Source.MaxRelativeTangentSpeed;
	MaxRollingSpeed = Source.MaxRollingSpeed;
	NormalAdhesion = Source.NormalAdhesion;
	TangentialAdhesion = Source.TangentialAdhesion;
	bMaySplitInGravityField = Source.bMaySplitInGravityField;
	bSplitOnLogicalImpact = Source.bSplitOnLogicalImpact;
}

void UAGX_ShapeContactMergeSplitThresholds::CopyTo(
	FShapeContactMergeSplitThresholdsBarrier& Barrier)
{
	Barrier.SetMaxImpactSpeed(MaxImpactSpeed);
	Barrier.SetMaxRelativeNormalSpeed(MaxRelativeNormalSpeed);
	Barrier.SetMaxRelativeTangentSpeed(MaxRelativeTangentSpeed);
	Barrier.SetMaxRollingSpeed(MaxRollingSpeed);
	Barrier.SetNormalAdhesion(NormalAdhesion);
	Barrier.SetTangentialAdhesion(TangentialAdhesion);
	Barrier.SetMaySplitInGravityField(bMaySplitInGravityField);
	Barrier.SetSplitOnLogicalImpact(bSplitOnLogicalImpact);
}

void UAGX_ShapeContactMergeSplitThresholds::SetNativeProperties()
{
	if (HasNative())
	{
		CopyTo(NativeBarrier);
	}
}
