// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "Import/AGX_ImportContext.h"
#include "Terrain/AGX_ShovelProperties.h"
#include "Terrain/AGX_TerrainEnums.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "CoreGlobals.h"
#include "UObject/Package.h"

class FRigidBodyBarrier;

#define LOCTEXT_NAMESPACE "AGX_ShovelComponent"

namespace AGX_ShovelComponent_helpers
{
	void SetLocalScope(UAGX_ShovelComponent& Shovel)
	{
		AActor* const Owner = FAGX_ObjectUtilities::GetRootParentActor(Shovel);
		Shovel.RigidBody.LocalScope = Owner;
		Shovel.TopEdge.Start.Parent.LocalScope = Owner;
		Shovel.TopEdge.End.Parent.LocalScope = Owner;
		Shovel.CuttingEdge.Start.Parent.LocalScope = Owner;
		Shovel.CuttingEdge.End.Parent.LocalScope = Owner;
		Shovel.CuttingDirection.Parent.LocalScope = Owner;
	}
}

// Sets default values for this component's properties
UAGX_ShovelComponent::UAGX_ShovelComponent()
{
	// Keep ticking off until we have a reason to turn it on.
	PrimaryComponentTick.bCanEverTick = false;

	AGX_ShovelComponent_helpers::SetLocalScope(*this);
}

void UAGX_ShovelComponent::SetEnabled(bool bInEnabled)
{
	bEnabled = bInEnabled;
	if (HasNative())
	{
		NativeBarrier.SetEnable(bEnabled);
	}
}

bool UAGX_ShovelComponent::IsEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnable();
	}
	return bEnabled;
}

void UAGX_ShovelComponent::SetShovelProperties(UAGX_ShovelProperties* Properties)
{
	if (ShovelProperties != nullptr && ShovelProperties->IsInstance())
	{
		ShovelProperties->UnregisterShovel(*this);
	}

	ShovelProperties = Properties;
	if (ShovelProperties != nullptr)
	{
		if (ShovelProperties->IsAsset())
		{
			if (GetWorld() != nullptr && GetWorld()->IsGameWorld())
			{
				ShovelProperties = ShovelProperties->GetOrCreateInstance(GetWorld());
				if (ShovelProperties != nullptr)
				{
					ShovelProperties->RegisterShovel(*this);
				}
				else
				{
					UE_LOG(
						LogAGX, Warning,
						TEXT(
							"Shovel %s in %s: Could not create Shovel Properties instance. Default "
							"shovel settings will be used."),
						*GetName(), *GetLabelSafe(GetOwner()));
				}
			}
		}
		else
		{
			ShovelProperties->RegisterShovel(*this);
		}
	}

	if (HasNative())
	{
		if (ShovelProperties != nullptr)
		{
			WritePropertiesToNative();
		}
		else
		{
			// TODO Not sure what to do here. With a nullptr Shovel Properties we want to use the
			// default settings for everything. WritePropertiesToNative achieves this by doing
			// nothing when Shovel Properties is nullptr, which works from Allocate Native since
			// at that point nothing has yet changed the AGX Dynamics instance of the shovel. Here,
			// however, we want to restore the default values. Where can we find those default
			// values? Can we new up a template agxTerrain::Shovel in the Shovel Barrier and copy
			// the values from that?
#if 1
			// Printing a log message for now, until we find a long-term solution.
			UE_LOG(
				LogAGX, Warning,
				TEXT("'None' Shovel Properties assigned to Shovel '%s' in '%s' at runtime. This is "
					 "currently not supported. The current Shovel Properties values will remain on "
					 "the AGX Dynamics representation of the shovel."),
				*GetName(), *GetLabelSafe(GetOwner()));
#else
			NativeBarrier.SetDefaultProperties();
#endif
		}
	}
}

namespace AGX_ShovelComponent_helpers
{
	template <typename SetterFunc>
	void PropagateEdgeUpdate(
		const UAGX_ShovelComponent& Shovel, const FAGX_Edge& Edge, const TCHAR* EdgeName,
		const SetterFunc& Setter)
	{
		if (!Shovel.HasNative())
			return;

		UAGX_RigidBodyComponent* Body = Shovel.RigidBody.GetRigidBody();
		if (Body == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot set %s on Shovel '%s' in '%s' because the Shovel does not have a "
					 "Rigid Body."),
				EdgeName, *Shovel.GetName(), *GetLabelSafe(Shovel.GetOwner()));
			return;
		}

		const FTwoVectors EdgeInBody = Edge.GetLocationsRelativeTo(*Body, Shovel);
		if (FMath::IsNearlyZero(FVector::Distance(EdgeInBody.v1, EdgeInBody.v2)))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Invalid %s passed to '%s' in '%s': Length must be non-zero."), EdgeName,
				*Shovel.GetName(), *GetLabelSafe(Shovel.GetOwner()));
			return;
		}

		Setter(EdgeInBody);
	}
}

void UAGX_ShovelComponent::SetTopEdge(FAGX_Edge InTopEdge)
{
	TopEdge = InTopEdge;
	AGX_ShovelComponent_helpers::PropagateEdgeUpdate(
		*this, TopEdge, TEXT("Top Edge"),
		[this](const FTwoVectors& EdgeInBody) { NativeBarrier.SetTopEdge(EdgeInBody); });
}

void UAGX_ShovelComponent::SetCuttingEdge(FAGX_Edge InCuttingEdge)
{
	CuttingEdge = InCuttingEdge;
	AGX_ShovelComponent_helpers::PropagateEdgeUpdate(
		*this, CuttingEdge, TEXT("Cutting Edge"),
		[this](const FTwoVectors& EdgeInBody) { NativeBarrier.SetCuttingEdge(EdgeInBody); });
}

void UAGX_ShovelComponent::SetCuttingDirection(FAGX_Frame InCuttingDirection)
{
	CuttingDirection = InCuttingDirection;
	if (HasNative())
	{
		if (UAGX_RigidBodyComponent* Body = RigidBody.GetRigidBody())
		{
			const FRotator CuttingRotation = CuttingDirection.GetRotationRelativeTo(*Body, *this);
			const FVector DirectionInBody = CuttingRotation.RotateVector(FVector::ForwardVector);
			NativeBarrier.SetCuttingDirection(DirectionInBody);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Cannot set Cutting Direction on Shovel '%s' in '%s' because the Shovel does "
					 "not have a Rigid Body."),
				*GetName(), *GetLabelSafe(GetOwner()));
		}
	}
}

void UAGX_ShovelComponent::FinalizeShovelEdit()
{
	if (HasNative())
	{
		SetTopEdge(TopEdge);
		SetCuttingEdge(CuttingEdge);
		SetCuttingDirection(CuttingDirection);
	}
}

FAGX_Frame* UAGX_ShovelComponent::GetFrame(EAGX_ShovelFrame Frame)
{
	switch (Frame)
	{
		case EAGX_ShovelFrame::CuttingDirection:
			return &CuttingDirection;
		case EAGX_ShovelFrame::CuttingEdgeBegin:
			return &CuttingEdge.Start;
		case EAGX_ShovelFrame::CuttingEdgeEnd:
			return &CuttingEdge.End;
		case EAGX_ShovelFrame::TopEdgeBegin:
			return &TopEdge.Start;
		case EAGX_ShovelFrame::TopEdgeEnd:
			return &TopEdge.End;
		case EAGX_ShovelFrame::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Warning,
		TEXT("Unknown Shovel Frame source, %d, passed to UAGX_ShovelComponent::GetFrame."),
		static_cast<int>(Frame));
	return nullptr;
}

namespace AGX_ShovelComponent_helpers
{
	UAGX_RigidBodyComponent* GetRigidBody(
		const FShovelBarrier& Barrier, FAGX_ImportContext* Context)
	{
		if (Context == nullptr || Context->RigidBodies == nullptr)
			return nullptr;

		auto BodyBarrier = Barrier.GetRigidBody();
		if (!BodyBarrier.HasNative())
			return nullptr;

		return Context->RigidBodies->FindRef(BodyBarrier.GetGuid());
	}

	FString CreateShovelName(UAGX_ShovelComponent& Shovel, UAGX_RigidBodyComponent* RigidBody)
	{
		// Shovels don't have names in AGX Dynamics so generate one using the body's name.
		FString Name;
		if (RigidBody == nullptr)
			Name = "Shovel_Bodyless";
		else
			Name = FString::Printf(TEXT("Shovel_%s"), *RigidBody->GetName());

		return FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			Shovel.GetOwner(), Name, UAGX_ShovelComponent::StaticClass());
	}

	UAGX_ShovelProperties* CreateShovelProperties(
		const FShovelBarrier& Barrier, FAGX_ImportContext& Context)
	{
		// Shovel properties share guid with shovel and is always unique to a shovel.
		const FGuid Guid = Barrier.GetGuid();
		AGX_CHECK(!Context.ShovelProperties->Contains(Guid));

		auto Properties = NewObject<UAGX_ShovelProperties>(Context.Outer, NAME_None, RF_Public | RF_Standalone);
		FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(*Properties, Context.SessionGuid);
		Properties->CopyFrom(Barrier, &Context);
		return Properties;
	}
}

void UAGX_ShovelComponent::CopyFrom(const FShovelBarrier& Barrier, FAGX_ImportContext* Context)
{
	using namespace AGX_ShovelComponent_helpers;

	bEnabled = Barrier.GetEnable();
	const FTwoVectors Top = Barrier.GetTopEdge();
	TopEdge.Start.LocalLocation = Top.v1;

	TopEdge.Start.LocalRotation = FRotator(ForceInitToZero);
	TopEdge.End.LocalLocation = Top.v2;

	TopEdge.End.LocalRotation = FRotator(ForceInitToZero);
	const FTwoVectors Cutting = Barrier.GetCuttingEdge();

	CuttingEdge.Start.LocalLocation = Cutting.v1;
	CuttingEdge.Start.LocalRotation = FRotator(ForceInitToZero);
	CuttingEdge.End.LocalLocation = Cutting.v2;
	CuttingEdge.End.LocalRotation = FRotator(ForceInitToZero);

	CuttingDirection.LocalLocation = 0.5 * (Cutting.v1 + Cutting.v2);
	const FRotator CuttingDirectionRotation =
		FRotationMatrix::MakeFromX(Barrier.GetCuttingDirection()).Rotator();
	CuttingDirection.LocalRotation = CuttingDirectionRotation;

	ImportGuid = Barrier.GetGuid();

	UAGX_RigidBodyComponent* Body = GetRigidBody(Barrier, Context);

	Rename(*CreateShovelName(*this, Body));

	const FName BodyName = Body != nullptr ? Body->GetFName() : NAME_None;
	RigidBody.Name = BodyName;
	TopEdge.Start.Parent.Name = BodyName;
	TopEdge.End.Parent.Name = BodyName;
	CuttingEdge.Start.Parent.Name = BodyName;
	CuttingEdge.End.Parent.Name = BodyName;
	CuttingDirection.Parent.Name = BodyName;

	if (Context == nullptr || Context->Shovels == nullptr || Context->ShovelProperties == nullptr ||
		Context->RigidBodies == nullptr)
		return; // We are done.

	AGX_CHECK(!Context->Shovels->Contains(ImportGuid));
	Context->Shovels->Add(ImportGuid, this);

	ShovelProperties = CreateShovelProperties(Barrier, *Context);
}

bool UAGX_ShovelComponent::SwapEdgeDirections()
{
	std::swap(TopEdge.Start, TopEdge.End);
	std::swap(CuttingEdge.Start, CuttingEdge.End);
	if (HasNative())
	{
		UAGX_RigidBodyComponent* BodyComponent = RigidBody.GetRigidBody();
		if (BodyComponent == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Shovel '%s' in '%s' does not have a Rigid Body. Ignoring this Shovel."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return false;
		}

		FRigidBodyBarrier* BodyBarrier = BodyComponent->GetNative();
		if (BodyBarrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT(
					"Shovel '%s' in '%s' has a Rigid Body, '%s' in '%s', that could not create its "
					"AGX Dynamics representation. Ignoring this Shovel."),
				*GetName(), *GetLabelSafe(GetOwner()), *BodyComponent->GetName(),
				*GetLabelSafe(BodyComponent->GetOwner()));
			return false;
		}

		const FTwoVectors TopEdgeInBody = TopEdge.GetLocationsRelativeTo(*BodyComponent, *this);
		const FTwoVectors CuttingEdgeInBody =
			CuttingEdge.GetLocationsRelativeTo(*BodyComponent, *this);
		NativeBarrier.SetTopEdge(TopEdgeInBody);
		NativeBarrier.SetCuttingEdge(CuttingEdgeInBody);
	}
	return true;
}

void UAGX_ShovelComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// This code is run after the constructor and after Init Properties, where property values are
	// copied from the Class Default Object, but before deserialization in cases where this object
	// is created from another, such as at the start of a Play-in-Editor session or when loading
	// a map in a cooked build (I hope).
	AGX_ShovelComponent_helpers::SetLocalScope(*this);

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_ShovelComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif

void UAGX_ShovelComponent::OnRegister()
{
	Super::OnRegister();
	AGX_ShovelComponent_helpers::SetLocalScope(*this);
}

void UAGX_ShovelComponent::BeginPlay()
{
	Super::BeginPlay();

	// We are now in a game world. Make sure we use an instance of the Shovel Properties and that
	// we are registered with that instance.
	if (ShovelProperties != nullptr)
	{
		ShovelProperties = ShovelProperties->GetOrCreateInstance(GetWorld());
		if (ShovelProperties != nullptr && ShovelProperties->IsInstance())
		{
			ShovelProperties->RegisterShovel(*this);
		}
		else
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Shovel '%s' in '%s': Could not create Shovel Properties instance. Default "
					 "shovel settings will be used."),
				*GetName(), *GetLabelSafe(GetOwner()));
		}
	}

	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		AllocateNative();
	}
}

void UAGX_ShovelComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	if (ShovelProperties != nullptr && ShovelProperties->IsInstance())
	{
		ShovelProperties->UnregisterShovel(*this);
	}
	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_ShovelComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

bool UAGX_ShovelComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_ShovelComponent::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}
	NativeBarrier.IncrementRefCount();
	return NativeBarrier.GetNativeAddress();
}

void UAGX_ShovelComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(NativeAddress);
	NativeBarrier.DecrementRefCount();
}

FShovelBarrier* UAGX_ShovelComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for Shovel '%s' in '%s' was made "
					 "but we are in the middle of a Blueprint Reconstruction and the requested "
					 "instance has not yet been restored. The instance cannot be returned, which "
					 "may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}
		AllocateNative();
	}
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("'%s' in '%s': Could not allocate AGX Dynamics Shovel in "
				 "UAGX_ShovelComponent::GetOrCreateNative, nullptr will be returned to caller."),
			*GetName(), *GetLabelSafe(GetOwner()));
	}
	return GetNative();
}

FShovelBarrier* UAGX_ShovelComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}

	return &NativeBarrier;
}

const FShovelBarrier* UAGX_ShovelComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_ShovelComponent::AllocateNative()
{
	check(!GIsReconstructingBlueprintInstances);
	check(!HasNative());

	UAGX_RigidBodyComponent* BodyComponent = RigidBody.GetRigidBody();
	if (BodyComponent == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shovel '%s' in '%s' does not have a Rigid Body. Ignoring this Shovel."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FRigidBodyBarrier* BodyBarrier = BodyComponent->GetOrCreateNative();
	if (BodyBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Shovel '%s' in '%s' has a Rigid Body, '%s' in '%s', that could not create its "
				 "AGX Dynamics representation. Ignoring this Shovel."),
			*GetName(), *GetLabelSafe(GetOwner()), *BodyComponent->GetName(),
			*GetLabelSafe(BodyComponent->GetOwner()));
		return;
	}

	const FTwoVectors TopEdgeInBody = TopEdge.GetLocationsRelativeTo(*BodyComponent, *this);
	const FTwoVectors CuttingEdgeInBody = CuttingEdge.GetLocationsRelativeTo(*BodyComponent, *this);
	const FRotator CuttingRotation = CuttingDirection.GetRotationRelativeTo(*BodyComponent, *this);
	const FVector CuttingDirectionInBody = CuttingRotation.RotateVector(FVector::ForwardVector);
	NativeBarrier.AllocateNative(
		*BodyBarrier, TopEdgeInBody, CuttingEdgeInBody, CuttingDirectionInBody);
	check(HasNative()); /// \todo Consider better error handling than 'check'.

	WritePropertiesToNative();

	// No need to add Shovel to Simulation. Shovels only needs to be added to the Terrain, and that
	// is handled by the Terrain itself.
}

void UAGX_ShovelComponent::SetbEnabled(bool bInEnable)
{
	SetEnabled(bInEnable);
}

#if WITH_EDITOR
void UAGX_ShovelComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER(bEnabled);
	AGX_COMPONENT_DEFAULT_DISPATCHER(ShovelProperties);
	AGX_COMPONENT_DEFAULT_DISPATCHER(TopEdge);
	AGX_COMPONENT_DEFAULT_DISPATCHER(CuttingEdge);
	AGX_COMPONENT_DEFAULT_DISPATCHER(CuttingDirection);
}
#endif

bool UAGX_ShovelComponent::WritePropertiesToNative()
{
	if (!HasNative())
	{
		return false;
	}

	NativeBarrier.SetEnable(bEnabled);

	if (ShovelProperties == nullptr)
	{
		// No properties means use the defaults. It is not an error.
		// todo Simply returning is not correct if we used to have a Shovel Properties but don't
		// anymore. In that case we would like to restore the default values. Read from the
		// UAGX_ShovelProperties class default object?
		return true;
	}

	NativeBarrier.SetToothLength(ShovelProperties->ToothLength);
	NativeBarrier.SetToothMinimumRadius(ShovelProperties->ToothMinimumRadius);
	NativeBarrier.SetToothMaximumRadius(ShovelProperties->ToothMaximumRadius);
	NativeBarrier.SetNumberOfTeeth(ShovelProperties->NumberOfTeeth);
	NativeBarrier.SetNoMergeExtensionDistance(ShovelProperties->NoMergeExtensionDistance);
	NativeBarrier.SetMinimumSubmergedContactLengthFraction(
		ShovelProperties->MinimumSubmergedContactLengthFraction);
	NativeBarrier.SetVerticalBladeSoilMergeDistance(
		ShovelProperties->VerticalBladeSoilMergeDistance);
	NativeBarrier.SetSecondarySeparationDeadloadLimit(
		ShovelProperties->SecondarySeparationDeadloadLimit);
	NativeBarrier.SetPenetrationDepthThreshold(ShovelProperties->PenetrationDepthThreshold);
	NativeBarrier.SetPenetrationForceScaling(ShovelProperties->PenetrationForceScaling);
	NativeBarrier.SetEnableParticleFreeDeformers(ShovelProperties->bEnableParticleFreeDeformers);
	NativeBarrier.SetAlwaysRemoveShovelContacts(ShovelProperties->bAlwaysRemoveShovelContacts);
	NativeBarrier.SetMaximumPenetrationForce(ShovelProperties->MaximumPenetrationForce);
	if (ShovelProperties->bOverride_ContactRegionThreshold)
	{
		NativeBarrier.SetContactRegionThreshold(ShovelProperties->ContactRegionThreshold);
	}
	if (ShovelProperties->bOverride_ContactRegionVerticalLimit)
	{
		NativeBarrier.SetContactRegionVerticalLimit(ShovelProperties->ContactRegionVerticalLimit);
	}
	NativeBarrier.SetEnableInnerShapeCreateDynamicMass(
		ShovelProperties->bEnableInnerShapeCreateDynamicMass);
	NativeBarrier.SetEnableParticleForceFeedback(ShovelProperties->bEnableParticleForceFeedback);
	NativeBarrier.SetParticleInclusionMultiplier(ShovelProperties->ParticleInclusionMultiplier);

	auto SetExcavationSettings =
		[this](EAGX_ExcavationMode Mode, const FAGX_ShovelExcavationSettings& Settings)
	{
		NativeBarrier.SetExcavationSettingsEnabled(Mode, Settings.bEnabled);
		NativeBarrier.SetExcavationSettingsEnableCreateDynamicMass(
			Mode, Settings.bEnableCreateDynamicMass);
		NativeBarrier.SetExcavationSettingsEnableForceFeedback(Mode, Settings.bEnableForceFeedback);
	};

	SetExcavationSettings(
		EAGX_ExcavationMode::Primary, ShovelProperties->PrimaryExcavationSettings);
	SetExcavationSettings(
		EAGX_ExcavationMode::DeformBack, ShovelProperties->DeformBackExcavationSettings);
	SetExcavationSettings(
		EAGX_ExcavationMode::DeformRight, ShovelProperties->DeformRightExcavationSettings);
	SetExcavationSettings(
		EAGX_ExcavationMode::DeformLeft, ShovelProperties->DeformLeftExcavationSettings);

	// Properties initialized by the AGX Dynamics Shovel constructor, i.e. body, edges, and cutting
	// direction, are not set here.

	return true;
}

#undef LOCTEXT_NAMESPACE
