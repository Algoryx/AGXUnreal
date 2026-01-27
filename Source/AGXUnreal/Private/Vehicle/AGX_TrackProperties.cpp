// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_CustomVersion.h"
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Vehicle/AGX_TrackComponent.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

void UAGX_TrackProperties::SetBendingStiffnessLateral(double Stiffness)
{
	AGX_ASSET_SETTER_IMPL_VALUE(BendingStiffnessLateral, Stiffness, SetBendingStiffnessLateral);
}

double UAGX_TrackProperties::GetBendingStiffnessLateral() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(BendingStiffnessLateral, GetBendingStiffnessLateral);
}

void UAGX_TrackProperties::SetBendingAttenuationLateral(double Attenuation)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		BendingAttenuationLateral, Attenuation, SetBendingAttenuationLateral);
}

double UAGX_TrackProperties::GetBendingAttenuationLateral() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(BendingAttenuationLateral, GetBendingAttenuationLateral);
}

void UAGX_TrackProperties::SetBendingStiffnessVertical(double Stiffness)
{
	AGX_ASSET_SETTER_IMPL_VALUE(BendingStiffnessVertical, Stiffness, SetBendingStiffnessVertical);
}

double UAGX_TrackProperties::GetBendingStiffnessVertical() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(BendingStiffnessVertical, GetBendingStiffnessVertical);
}

void UAGX_TrackProperties::SetBendingAttenuationVertical(double Attenuation)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		BendingAttenuationVertical, Attenuation, SetBendingAttenuationVertical);
}

double UAGX_TrackProperties::GetBendingAttenuationVertical() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(BendingAttenuationVertical, GetBendingAttenuationVertical);
}

void UAGX_TrackProperties::SetShearStiffnessLateral(double Stiffness)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearStiffnessLateral, Stiffness, SetShearStiffnessLateral);
}

double UAGX_TrackProperties::GetShearStiffnessLateral() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearStiffnessLateral, GetShearStiffnessLateral);
}

void UAGX_TrackProperties::SetShearAttenuationLateral(double Attenuation)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearAttenuationLateral, Attenuation, SetShearAttenuationLateral);
}

double UAGX_TrackProperties::GetShearAttenuationLateral() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearAttenuationLateral, GetShearAttenuationLateral);
}

void UAGX_TrackProperties::SetShearStiffnessVertical(double Stiffness)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearStiffnessVertical, Stiffness, SetShearStiffnessVertical);
}

double UAGX_TrackProperties::GetShearStiffnessVertical() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearStiffnessVertical, GetShearStiffnessVertical);
}

void UAGX_TrackProperties::SetShearAttenuationVertical(double Attenuation)
{
	AGX_ASSET_SETTER_IMPL_VALUE(ShearAttenuationVertical, Attenuation, SetShearAttenuationVertical);
}

double UAGX_TrackProperties::GetShearAttenuationVertical() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(ShearAttenuationVertical, GetShearAttenuationVertical);
}

void UAGX_TrackProperties::SetTensileStiffness(double Stiffness)
{
	AGX_ASSET_SETTER_IMPL_VALUE(TensileStiffness, Stiffness, SetTensileStiffness);
}

double UAGX_TrackProperties::GetTensileStiffness() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(TensileStiffness, GetTensileStiffness);
}

void UAGX_TrackProperties::SetTensileAttenuation(double Attenuation)
{
	AGX_ASSET_SETTER_IMPL_VALUE(TensileAttenuation, Attenuation, SetTensileAttenuation);
}

double UAGX_TrackProperties::GetTensileAttenuation() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(TensileAttenuation, GetTensileAttenuation);
}

void UAGX_TrackProperties::SetTorsionalStiffness(double Stiffness)
{
	AGX_ASSET_SETTER_IMPL_VALUE(TorsionalStiffness, Stiffness, SetTorsionalStiffness);
}

double UAGX_TrackProperties::GetTorsionalStiffness() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(TorsionalStiffness, GetTorsionalStiffness);
}

void UAGX_TrackProperties::SetTorsionalAttenuation(double Attenuation)
{
	AGX_ASSET_SETTER_IMPL_VALUE(TorsionalAttenuation, Attenuation, SetTorsionalAttenuation);
}

double UAGX_TrackProperties::GetTorsionalAttenuation() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(TorsionalAttenuation, GetTorsionalAttenuation);
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

	BendingStiffnessLateral = Source->BendingStiffnessLateral;
	BendingAttenuationLateral = Source->BendingAttenuationLateral;
	BendingStiffnessVertical = Source->BendingStiffnessVertical;
	BendingAttenuationVertical = Source->BendingAttenuationVertical;
	ShearStiffnessLateral = Source->ShearStiffnessLateral;
	ShearAttenuationLateral = Source->ShearAttenuationLateral;
	ShearStiffnessVertical = Source->ShearStiffnessVertical;
	ShearAttenuationVertical = Source->ShearAttenuationVertical;
	TensileStiffness = Source->TensileStiffness;
	TensileAttenuation = Source->TensileAttenuation;
	TorsionalStiffness = Source->TorsionalStiffness;
	TorsionalAttenuation = Source->TorsionalAttenuation;

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
	BendingStiffnessLateral = Source.GetBendingStiffnessLateral();
	BendingAttenuationLateral = Source.GetBendingAttenuationLateral();
	BendingStiffnessVertical = Source.GetBendingStiffnessVertical();
	BendingAttenuationVertical = Source.GetBendingAttenuationVertical();
	ShearStiffnessLateral = Source.GetShearStiffnessLateral();
	ShearAttenuationLateral = Source.GetShearAttenuationLateral();
	ShearStiffnessVertical = Source.GetShearStiffnessVertical();
	ShearAttenuationVertical = Source.GetShearAttenuationVertical();
	TensileStiffness = Source.GetTensileStiffness();
	TensileAttenuation = Source.GetTensileAttenuation();
	TorsionalStiffness = Source.GetTorsionalStiffness();
	TorsionalAttenuation = Source.GetTorsionalAttenuation();

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

	NativeBarrier.SetBendingStiffnessLateral(BendingStiffnessLateral);
	NativeBarrier.SetBendingAttenuationLateral(BendingAttenuationLateral);
	NativeBarrier.SetBendingStiffnessVertical(BendingStiffnessVertical);
	NativeBarrier.SetBendingAttenuationVertical(BendingAttenuationVertical);
	NativeBarrier.SetShearStiffnessLateral(ShearStiffnessLateral);
	NativeBarrier.SetShearAttenuationLateral(ShearAttenuationLateral);
	NativeBarrier.SetShearStiffnessVertical(ShearStiffnessVertical);
	NativeBarrier.SetShearAttenuationVertical(ShearAttenuationVertical);
	NativeBarrier.SetTensileStiffness(TensileStiffness);
	NativeBarrier.SetTensileAttenuation(TensileAttenuation);
	NativeBarrier.SetTorsionalStiffness(TorsionalStiffness);
	NativeBarrier.SetTorsionalAttenuation(TorsionalAttenuation);

	// Hinge parameters.
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

void UAGX_TrackProperties::SerializeInternal(const UAGX_TrackComponent& Track, FArchive& Archive)
{
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);

	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::TerrainPropertiesUsesStiffnessAttenuation))
	{
		// TODO
	}
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
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				NodesToWheelsMergeThreshold, SetNodesToWheelsMergeThreshold)
		});
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(ThisClass, NodesToWheelsSplitThreshold),
		[](ThisClass* This)
		{
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

	AGX_COMPONENT_DEFAULT_DISPATCHER(BendingStiffnessLateral);
	AGX_COMPONENT_DEFAULT_DISPATCHER(BendingAttenuationLateral);
	AGX_COMPONENT_DEFAULT_DISPATCHER(BendingStiffnessVertical);
	AGX_COMPONENT_DEFAULT_DISPATCHER(BendingAttenuationVertical);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearStiffnessLateral);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearAttenuationLateral);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearStiffnessVertical);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShearAttenuationVertical);
	AGX_COMPONENT_DEFAULT_DISPATCHER(TensileStiffness);
	AGX_COMPONENT_DEFAULT_DISPATCHER(TensileAttenuation);
	AGX_COMPONENT_DEFAULT_DISPATCHER(TorsionalStiffness);
	AGX_COMPONENT_DEFAULT_DISPATCHER(TorsionalAttenuation);
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
