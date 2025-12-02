// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

//
// Compliance translational.
//

void UAGX_TrackProperties::SetHingeComplianceTranslational(double X, double Y, double Z)
{
	SetHingeComplianceTranslationalX(X);
	SetHingeComplianceTranslationalY(Y);
	SetHingeComplianceTranslationalZ(Z);
}

void UAGX_TrackProperties::SetHingeComplianceTranslationalX(double X)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeComplianceTranslational_X, X, SetHingeComplianceTranslationalX);
}

void UAGX_TrackProperties::SetHingeComplianceTranslationalY(double Y)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeComplianceTranslational_Y, Y, SetHingeComplianceTranslationalY);
}

void UAGX_TrackProperties::SetHingeComplianceTranslationalZ(double Z)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeComplianceTranslational_Z, Z, SetHingeComplianceTranslationalZ);
}

void UAGX_TrackProperties::SetHingeComplianceTranslational_BP(float X, float Y, float Z)
{
	SetHingeComplianceTranslational(
		static_cast<double>(X), static_cast<double>(Y), static_cast<double>(Z));
}

void UAGX_TrackProperties::GetHingeComplianceTranslational(double& X, double& Y, double& Z) const
{
	X = GetHingeComplianceTranslationalX();
	Y = GetHingeComplianceTranslationalY();
	Z = GetHingeComplianceTranslationalZ();
}

double UAGX_TrackProperties::GetHingeComplianceTranslationalX() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeComplianceTranslational_X, GetHingeComplianceTranslationalX);
}

double UAGX_TrackProperties::GetHingeComplianceTranslationalY() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeComplianceTranslational_Y, GetHingeComplianceTranslationalY);
}

double UAGX_TrackProperties::GetHingeComplianceTranslationalZ() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeComplianceTranslational_Z, GetHingeComplianceTranslationalZ);
}

void UAGX_TrackProperties::GetHingeComplianceTranslational_BP(float& X, float& Y, float& Z)
{
	X = GetHingeComplianceTranslationalX();
	Y = GetHingeComplianceTranslationalY();
	Z = GetHingeComplianceTranslationalZ();
}

//
// Compliance rotational.
//

void UAGX_TrackProperties::SetHingeComplianceRotational(double ComplianceX, double ComplianceY)
{
	SetHingeComplianceRotationalX(ComplianceX);
	SetHingeComplianceRotationalY(ComplianceY);
}

void UAGX_TrackProperties::SetHingeComplianceRotationalX(double Compliance)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeComplianceRotational_X, Compliance, SetHingeComplianceRotationalX);
}

void UAGX_TrackProperties::SetHingeComplianceRotationalY(double Compliance)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeComplianceRotational_Y, Compliance, SetHingeComplianceRotationalY);
}

void UAGX_TrackProperties::SetHingeComplianceRotational_BP(float X, float Y)
{
	SetHingeComplianceRotational(static_cast<double>(X), static_cast<double>(Y));
}

void UAGX_TrackProperties::GetHingeComplianceRotational(double& X, double& Y) const
{
	X = GetHingeComplianceRotationalX();
	Y = GetHingeComplianceRotationalY();
}

void UAGX_TrackProperties::GetHingeSpookDampingRotational(double& X, double& Y) const
{
	X = GetHingeSpookDampingRotationalX();
	Y = GetHingeSpookDampingRotationalY();
}

double UAGX_TrackProperties::GetHingeComplianceRotationalX() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeComplianceRotational_X, GetHingeComplianceRotationalX);
}

double UAGX_TrackProperties::GetHingeComplianceRotationalY() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeComplianceRotational_Y, GetHingeComplianceRotationalY);
}

void UAGX_TrackProperties::GetHingeComplianceRotational_BP(float& X, float& Y)
{
	X = GetHingeComplianceRotationalX();
	Y = GetHingeComplianceRotationalY();
}

//
// Damping translational.
//

void UAGX_TrackProperties::SetHingeSpookDampingTranslational(
	double DampingX, double DampingY, double DampingZ)
{
	SetHingeSpookDampingTranslationalX(DampingX);
	SetHingeSpookDampingTranslationalY(DampingY);
	SetHingeSpookDampingTranslationalZ(DampingZ);
}

void UAGX_TrackProperties::SetHingeSpookDampingTranslationalX(double Damping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeSpookDampingTranslational_X, Damping, SetHingeSpookDampingTranslationalX);
}

void UAGX_TrackProperties::SetHingeSpookDampingTranslationalY(double Damping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeSpookDampingTranslational_Y, Damping, SetHingeSpookDampingTranslationalY);
}

void UAGX_TrackProperties::SetHingeSpookDampingTranslationalZ(double Damping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeSpookDampingTranslational_Z, Damping, SetHingeSpookDampingTranslationalZ);
}

void UAGX_TrackProperties::SetHingeSpookDampingTranslational_BP(
	float DampingX, float DampingY, float DampingZ)
{
	// clang-format off
	SetHingeSpookDampingTranslational(
		static_cast<double>(DampingX),
		static_cast<double>(DampingY),
		static_cast<double>(DampingZ));
	// clang-format on
}

void UAGX_TrackProperties::GetHingeSpookDampingTranslational(double& DampingX, double& DampingY, double& DampingZ) const
{
	DampingX = GetHingeSpookDampingTranslationalX();
	DampingY = GetHingeSpookDampingTranslationalY();
	DampingZ = GetHingeSpookDampingTranslationalZ();
}

double UAGX_TrackProperties::GetHingeSpookDampingTranslationalX() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		HingeSpookDampingTranslational_X, GetHingeSpookDampingTranslationalX);
}

double UAGX_TrackProperties::GetHingeSpookDampingTranslationalY() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		HingeSpookDampingTranslational_Y, GetHingeSpookDampingTranslationalY);
}

double UAGX_TrackProperties::GetHingeSpookDampingTranslationalZ() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		HingeSpookDampingTranslational_Z, GetHingeSpookDampingTranslationalZ);
}

void UAGX_TrackProperties::GetHingeSpookDampingTranslational_BP(
	float& DampingX, float& DampingY, float& DampingZ)
{
	DampingX = GetHingeSpookDampingTranslationalX();
	DampingY = GetHingeSpookDampingTranslationalY();
	DampingZ = GetHingeSpookDampingTranslationalZ();
}

//
// Damping rotational.
//

void UAGX_TrackProperties::SetHingeSpookDampingRotational(double DampingX, double DampingY)
{
	SetHingeSpookDampingRotationalX(DampingX);
	SetHingeSpookDampingRotationalY(DampingY);
}

void UAGX_TrackProperties::SetHingeSpookDampingRotationalX(double Damping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeSpookDampingRotational_X, Damping, SetHingeSpookDampingRotationalX);
}

void UAGX_TrackProperties::SetHingeSpookDampingRotationalY(double Damping)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		HingeSpookDampingRotational_Y, Damping, SetHingeSpookDampingRotationalY);
}

void UAGX_TrackProperties::SetHingeSpookDampingRotational_BP(float X, float Y)
{
	SetHingeSpookDampingRotational(static_cast<double>(X), static_cast<double>(Y));
}

double UAGX_TrackProperties::GetHingeSpookDampingRotationalX() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeSpookDampingRotational_X, GetHingeSpookDampingRotationalX);
}

double UAGX_TrackProperties::GetHingeSpookDampingRotationalY() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeSpookDampingRotational_Y, GetHingeSpookDampingRotationalY);
}

void UAGX_TrackProperties::GetHingeSpookDampingRotational_BP(float& X, float& Y)
{
	X = GetHingeSpookDampingRotationalX();
	Y = GetHingeSpookDampingRotationalY();
}

//
// Hinge range.
//

void UAGX_TrackProperties::SetHingeRangeEnabled(bool bEnable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bEnableHingeRange, bEnable, SetHingeRangeEnabled);
}

bool UAGX_TrackProperties::GetHingeRangeEnabled() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(bEnableHingeRange, GetHingeRangeEnabled);
}

void UAGX_TrackProperties::SetHingeRange(FAGX_RealInterval InHingeRange)
{
	AGX_ASSET_SETTER_IMPL_VALUE(HingeRange, InHingeRange, SetHingeRange);
}

void UAGX_TrackProperties::SetHingeRange(double Min, double Max)
{
	const FAGX_RealInterval Range(Min, Max);
	AGX_ASSET_SETTER_IMPL_VALUE(HingeRange, Range, SetHingeRange);
}

void UAGX_TrackProperties::SetHingeRange_BP(float Min, float Max)
{
	SetHingeRange(static_cast<double>(Min), static_cast<double>(Max));
}

FAGX_RealInterval UAGX_TrackProperties::GetHingeRange() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(HingeRange, GetHingeRange);
}

void UAGX_TrackProperties::GetHingeRange(double& Min, double& Max) const
{
	FAGX_RealInterval Range = GetHingeRange();
	Min = Range.Min;
	Max = Range.Max;
}

void UAGX_TrackProperties::GetHingeRange_BP(float& Min, float& Max) const
{
	FAGX_RealInterval Range = GetHingeRange();
	Min = static_cast<float>(Range.Min);
	Max = static_cast<float>(Range.Max);
}

//
// Merge nodes to wheels.
//

void UAGX_TrackProperties::SetOnInitializeMergeNodesToWheelsEnabled(bool bEnable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		bEnableOnInitializeMergeNodesToWheels, bEnable, SetOnInitializeMergeNodesToWheelsEnabled);
}

bool UAGX_TrackProperties::GetOnInitializeMergeNodesToWheelsEnabled() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		bEnableOnInitializeMergeNodesToWheels, GetOnInitializeMergeNodesToWheelsEnabled);
}

// Transform nodes to wheels.

void UAGX_TrackProperties::SetOnInitializeTransformNodesToWheelsEnabled(bool bEnable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		bEnableOnInitializeTransformNodesToWheels, bEnable,
		SetOnInitializeTransformNodesToWheelsEnabled);
}

bool UAGX_TrackProperties::GetOnInitializeTransformNodesToWheelsEnabled() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		bEnableOnInitializeTransformNodesToWheels, GetOnInitializeTransformNodesToWheelsEnabled);
}

void UAGX_TrackProperties::SetTransformNodesToWheelsOverlap(double Overlap)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		TransformNodesToWheelsOverlap, Overlap, SetTransformNodesToWheelsOverlap);
}

void UAGX_TrackProperties::SetTransformNodesToWheelsOverlap_BP(float Overlap)
{
	SetTransformNodesToWheelsOverlap(static_cast<double>(Overlap));
}

double UAGX_TrackProperties::GetTransformNodesToWheelsOverlap() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(TransformNodesToWheelsOverlap, GetTransformNodesToWheelsOverlap);
}

float UAGX_TrackProperties::GetTransformNodesToWheelsOverlap_BP() const
{
	return static_cast<float>(GetTransformNodesToWheelsOverlap());
}

// Merge threshold.

void UAGX_TrackProperties::SetNodesToWheelsMergeThreshold(double MergeThreshold)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		NodesToWheelsMergeThreshold, MergeThreshold, SetNodesToWheelsMergeThreshold);
}

void UAGX_TrackProperties::SetNodesToWheelsMergeThreshold_BP(float MergeThreshold)
{
	SetNodesToWheelsMergeThreshold(static_cast<double>(MergeThreshold));
}

// Split threshold.

void UAGX_TrackProperties::SetNodesToWheelsSplitThreshold(double SplitThreshold)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		NodesToWheelsSplitThreshold, SplitThreshold, SetNodesToWheelsSplitThreshold);
}

void UAGX_TrackProperties::SetNodesToWheelsSplitThreshold_BP(float SplitThreshold)
{
	SetNodesToWheelsSplitThreshold(static_cast<double>(SplitThreshold));
}

// Num nodes average direction.

void UAGX_TrackProperties::SetNumNodesIncludedInAverageDirection(int32 NumIncludedNodes)
{
	if (NumIncludedNodes < 1)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"Zero or negative value passed to SetNumNodesIncludedInAverageDirection, ignored."),
			NumIncludedNodes)
		return;
	}

	AGX_ASSET_SETTER_IMPL_VALUE(
		NumNodesIncludedInAverageDirection, NumIncludedNodes,
		SetNumNodesIncludedInAverageDirection);
}

int32 UAGX_TrackProperties::GetNumNodesIncludedInAverageDirection() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		NumNodesIncludedInAverageDirection, GetNumNodesIncludedInAverageDirection);
}

// Stabilizing hinge normal force.

void UAGX_TrackProperties::SetMinStabilizingHingeNormalForce(double MinNormalForce)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		MinStabilizingHingeNormalForce, MinNormalForce, SetMinStabilizingHingeNormalForce);
}

void UAGX_TrackProperties::SetMinStabilizingHingeNormalForce_BP(float MinNormalForce)
{
	SetMinStabilizingHingeNormalForce(static_cast<double>(MinNormalForce));
}

// Stabilizing hinge friction force.

void UAGX_TrackProperties::SetStabilizingHingeFrictionParameter(double FrictionParameter)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		StabilizingHingeFrictionParameter, FrictionParameter, SetStabilizingHingeFrictionParameter);
}

void UAGX_TrackProperties::SetStabilizingHingeFrictionParameter_BP(float FrictionParameter)
{
	SetStabilizingHingeFrictionParameter(static_cast<double>(FrictionParameter));
}

void UAGX_TrackProperties::CommitToAsset()
{
	if (IsInstance())
	{
		if (HasNative())
		{
			Asset->CopyFrom(*GetNative());
		}
	}
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
}

void UAGX_TrackProperties::CopyFrom(const UAGX_TrackProperties* Source)
{
	if (Source == nullptr)
	{
		return;
	}

	HingeComplianceTranslational_X = Source->HingeComplianceTranslational_X;
	HingeComplianceTranslational_Y = Source->HingeComplianceTranslational_Y;
	HingeComplianceTranslational_Z = Source->HingeComplianceTranslational_Z;

	HingeComplianceRotational_X = Source->HingeComplianceRotational_X;
	HingeComplianceRotational_Y = Source->HingeComplianceRotational_Y;

	HingeSpookDampingTranslational_X = Source->HingeSpookDampingTranslational_X;
	HingeSpookDampingTranslational_Y = Source->HingeSpookDampingTranslational_Y;
	HingeSpookDampingTranslational_Z = Source->HingeSpookDampingTranslational_Z;

	HingeSpookDampingRotational_X = Source->HingeSpookDampingRotational_X;
	HingeSpookDampingRotational_Y = Source->HingeSpookDampingRotational_Y;

	bEnableHingeRange = Source->bEnableHingeRange;
	HingeRange = Source->HingeRange;
	bEnableOnInitializeMergeNodesToWheels = Source->bEnableOnInitializeMergeNodesToWheels;
	bEnableOnInitializeTransformNodesToWheels = Source->bEnableOnInitializeTransformNodesToWheels;
	TransformNodesToWheelsOverlap = Source->TransformNodesToWheelsOverlap;

	NodesToWheelsMergeThreshold = Source->NodesToWheelsMergeThreshold;
	NodesToWheelsSplitThreshold = Source->NodesToWheelsSplitThreshold;
	NumNodesIncludedInAverageDirection = Source->NumNodesIncludedInAverageDirection;

	MinStabilizingHingeNormalForce = Source->MinStabilizingHingeNormalForce;
	StabilizingHingeFrictionParameter = Source->StabilizingHingeFrictionParameter;
}

void UAGX_TrackProperties::CopyFrom(const FTrackPropertiesBarrier& Source)
{
	HingeComplianceTranslational_X = Source.GetHingeCompliance(0);
	HingeComplianceTranslational_Y = Source.GetHingeCompliance(1);
	HingeComplianceTranslational_Z = Source.GetHingeCompliance(2);

	HingeComplianceRotational_X = Source.GetHingeCompliance(3);
	HingeComplianceRotational_Y = Source.GetHingeCompliance(4);

	HingeSpookDampingTranslational_X = Source.GetHingeSpookDamping(0);
	HingeSpookDampingTranslational_Y = Source.GetHingeSpookDamping(1);
	HingeSpookDampingTranslational_Z = Source.GetHingeSpookDamping(2);

	HingeSpookDampingRotational_X = Source.GetHingeSpookDamping(3);
	HingeSpookDampingRotational_Y = Source.GetHingeSpookDamping(4);

	bEnableHingeRange = Source.GetHingeRangeEnabled();
	HingeRange = Source.GetHingeRangeRange();
	bEnableOnInitializeMergeNodesToWheels = Source.GetOnInitializeMergeNodesToWheelsEnabled();
	bEnableOnInitializeTransformNodesToWheels =
		Source.GetOnInitializeTransformNodesToWheelsEnabled();
	TransformNodesToWheelsOverlap = Source.GetTransformNodesToWheelsOverlap();

	NodesToWheelsMergeThreshold = Source.GetNodesToWheelsMergeThreshold();
	NodesToWheelsSplitThreshold = Source.GetNodesToWheelsSplitThreshold();
	NumNodesIncludedInAverageDirection = Source.GetNumNodesIncludedInAverageDirection();

	MinStabilizingHingeNormalForce = Source.GetMinStabilizingHingeNormalForce();
	StabilizingHingeFrictionParameter = Source.GetStabilizingHingeFrictionParameter();

	ImportGuid = Source.GetGuid();
}

UAGX_TrackProperties* UAGX_TrackProperties::CreateInstanceFromAsset(
	const UWorld* PlayingWorld, UAGX_TrackProperties* Source)
{
	check(Source);
	check(!Source->IsInstance());
	check(PlayingWorld != nullptr);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source->GetName() + "_Instance";

	UAGX_TrackProperties* NewInstance = NewObject<UAGX_TrackProperties>(
		GetTransientPackage(), UAGX_TrackProperties::StaticClass(), *InstanceName, RF_Transient);
	NewInstance->Asset = Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_TrackProperties* UAGX_TrackProperties::GetInstance()
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

UAGX_TrackProperties* UAGX_TrackProperties::GetOrCreateInstance(const UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}
	else
	{
		UAGX_TrackProperties* InstancePtr = Instance.Get();
		if (InstancePtr == nullptr && PlayingWorld && PlayingWorld->IsGameWorld())
		{
			InstancePtr = UAGX_TrackProperties::CreateInstanceFromAsset(PlayingWorld, this);
			Instance = InstancePtr;
		}

		return InstancePtr;
	}
}

UAGX_TrackProperties* UAGX_TrackProperties::GetAsset()
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

bool UAGX_TrackProperties::IsInstance() const
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

bool UAGX_TrackProperties::HasNative() const
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

FTrackPropertiesBarrier* UAGX_TrackProperties::GetNative()
{
	return const_cast<FTrackPropertiesBarrier*>(const_cast<const ThisClass*>(this)->GetNative());
}

const FTrackPropertiesBarrier* UAGX_TrackProperties::GetNative() const
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

FTrackPropertiesBarrier* UAGX_TrackProperties::GetOrCreateNative()
{
	if (IsInstance())
	{
		if (!HasNative())
		{
			CreateNative();
		}
		return GetNative();
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_TrackProperties '%s' who's instance is "
					 "nullptr. Ensure e.g. GetOrCreateInstance is called prior to calling this "
					 "function"),
				*GetName());
			return nullptr;
		}
		return Instance->GetOrCreateNative();
	}
}

void UAGX_TrackProperties::UpdateNativeProperties()
{
	if (!IsInstance() || !HasNative())
	{
		return;
	}

	// Hinge parameters.
	NativeBarrier.SetHingeComplianceTranslationalX(HingeComplianceTranslational_X);
	NativeBarrier.SetHingeComplianceTranslationalY(HingeComplianceTranslational_Y);
	NativeBarrier.SetHingeComplianceTranslationalZ(HingeComplianceTranslational_Z);
	NativeBarrier.SetHingeComplianceRotationalX(HingeComplianceRotational_X);
	NativeBarrier.SetHingeComplianceRotationalY(HingeComplianceRotational_Y);
	NativeBarrier.SetHingeSpookDampingTranslationalX(HingeSpookDampingTranslational_X);
	NativeBarrier.SetHingeSpookDampingTranslationalY(HingeSpookDampingTranslational_Y);
	NativeBarrier.SetHingeSpookDampingTranslationalZ(HingeSpookDampingTranslational_Z);
	NativeBarrier.SetHingeSpookDampingRotationalX(HingeSpookDampingRotational_X);
	NativeBarrier.SetHingeSpookDampingRotationalY(HingeSpookDampingRotational_Y);
	NativeBarrier.SetHingeRangeEnabled(bEnableHingeRange);
	NativeBarrier.SetHingeRangeRange(HingeRange);

	// On initialize parameters.
	NativeBarrier.SetOnInitializeMergeNodesToWheelsEnabled(bEnableOnInitializeMergeNodesToWheels);
	NativeBarrier.SetOnInitializeTransformNodesToWheelsEnabled(
		bEnableOnInitializeTransformNodesToWheels);
	NativeBarrier.SetTransformNodesToWheelsOverlap(TransformNodesToWheelsOverlap);

	// Merge/split parameters.
	NativeBarrier.SetNodesToWheelsMergeThreshold(NodesToWheelsMergeThreshold);
	NativeBarrier.SetNodesToWheelsSplitThreshold(NodesToWheelsSplitThreshold);
	NativeBarrier.SetNumNodesIncludedInAverageDirection(NumNodesIncludedInAverageDirection);

	// Stabilization parameters.
	NativeBarrier.SetMinStabilizingHingeNormalForce(MinStabilizingHingeNormalForce);
	NativeBarrier.SetStabilizingHingeFrictionParameter(StabilizingHingeFrictionParameter);
}

void UAGX_TrackProperties::PostInitProperties()
{
	Super::PostInitProperties();
#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR

void UAGX_TrackProperties::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TrackProperties::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceTranslational_X),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeComplianceTranslational_X, SetHingeComplianceTranslationalX)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceTranslational_Y),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeComplianceTranslational_Y, SetHingeComplianceTranslationalY)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceTranslational_Z),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeComplianceTranslational_Z, SetHingeComplianceTranslationalZ)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceRotational_X),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeComplianceRotational_X, SetHingeComplianceRotationalX)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeComplianceRotational_Y),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeComplianceRotational_Y, SetHingeComplianceRotationalY)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeSpookDampingTranslational_X),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeSpookDampingTranslational_X, SetHingeSpookDampingTranslationalX)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeSpookDampingTranslational_Y),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeSpookDampingTranslational_Y, SetHingeSpookDampingTranslationalY)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeSpookDampingTranslational_Z),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeSpookDampingTranslational_Z, SetHingeSpookDampingTranslationalZ)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeSpookDampingRotational_X),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeSpookDampingRotational_X, SetHingeSpookDampingRotationalX)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeSpookDampingRotational_Y),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				HingeSpookDampingRotational_Y, SetHingeSpookDampingRotationalY)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bEnableHingeRange), [](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(bEnableHingeRange, SetHingeRangeEnabled) });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, HingeRange),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(HingeRange, SetHingeRange) });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bEnableOnInitializeMergeNodesToWheels),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				bEnableOnInitializeMergeNodesToWheels, SetOnInitializeMergeNodesToWheelsEnabled)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, bEnableOnInitializeTransformNodesToWheels),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				bEnableOnInitializeTransformNodesToWheels,
				SetOnInitializeTransformNodesToWheelsEnabled)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, TransformNodesToWheelsOverlap),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				TransformNodesToWheelsOverlap, SetTransformNodesToWheelsOverlap)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NodesToWheelsMergeThreshold),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				NodesToWheelsMergeThreshold, SetNodesToWheelsMergeThreshold)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NodesToWheelsSplitThreshold),
		[](ThisClass* This) {
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				NodesToWheelsSplitThreshold, SetNodesToWheelsSplitThreshold)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NumNodesIncludedInAverageDirection),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				NumNodesIncludedInAverageDirection, SetNumNodesIncludedInAverageDirection)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, MinStabilizingHingeNormalForce),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				MinStabilizingHingeNormalForce, SetMinStabilizingHingeNormalForce)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, StabilizingHingeFrictionParameter),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				StabilizingHingeFrictionParameter, SetStabilizingHingeFrictionParameter)
		});
}

#endif

void UAGX_TrackProperties::CreateNative()
{
	if (IsInstance())
	{
		check(!HasNative());
		NativeBarrier.AllocateNative();
		if (!HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("UAGX_TrackProperties '%s' failed to create native AGX Dynamics instance. See "
					 "the AGXDynamics log channel for additional information."),
				*GetName());
			return;
		}
		UpdateNativeProperties();
	}
	else
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"CreateNative was colled on an UAGX_TrackProperties who's instance is nullptr. "
					"Ensure e.g. GetOrCreateInstance is called prior to calling this function"));
			return;
		}
		return Instance->CreateNative();
	}
}
