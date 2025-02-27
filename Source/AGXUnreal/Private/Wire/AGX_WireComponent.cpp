// Copyright 2024, Algoryx Simulation AB.

#include "Wire/AGX_WireComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGXUnrealBarrier.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Wire/AGX_WireInstanceData.h"
#include "Wire/AGX_WireNode.h"
#include "Wire/AGX_WireUtilities.h"
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/WireNodeBarrier.h"
#include "Wire/WireParameterControllerBarrier.h"

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "CoreGlobals.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#if WITH_EDITOR
#include "Editor.h"
#endif
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialInterface.h"
#include "Math/UnrealMathUtility.h"

// Standard library includes.
#include <algorithm>
#include <tuple>

#include "Kismet/KismetSystemLibrary.h"

#define LOCTEXT_NAMESPACE "UAGX_WireComponent"

namespace AGX_WireComponent_helpers
{
	void SetLocalScope(FWireRoutingNode& Node, AActor* Owner)
	{
		Node.RigidBody.LocalScope = Owner;
		Node.Frame.Parent.LocalScope = Owner;
	}

	/**
	 * Establish the local scope for all Component References this Component owns. For more
	 * information see comments in AGX_ComponentReference.h.
	 */
	void SetLocalScope(UAGX_WireComponent& Wire)
	{
		AActor* Owner = FAGX_ObjectUtilities::GetRootParentActor(Wire);
		Wire.OwnedBeginWinch.BodyAttachment.LocalScope = Owner;
		Wire.OwnedEndWinch.BodyAttachment.LocalScope = Owner;
		for (FWireRoutingNode& Node : Wire.RouteNodes)
		{
			SetLocalScope(Node, Owner);
		}
	}
}

UAGX_WireComponent::UAGX_WireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
#endif

	// Add a pair of default nodes to make initial editing easier.
	AddNodeAtLocation(FVector::ZeroVector);
	AddNodeAtLocation(FVector(100.0f, 0.0f, 0.0f));

	AGX_WireComponent_helpers::SetLocalScope(*this);

	// Setup default visuals.
	static const TCHAR* WireMatAssetPath =
		TEXT("Material'/AGXUnreal/Wire/MI_GrayWire.MI_GrayWire'");
	RenderMaterial = FAGX_ObjectUtilities::GetAssetFromPath<UMaterialInterface>(WireMatAssetPath);
}

void UAGX_WireComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}
	Radius = InRadius;

	UpdateVisuals();
}

void UAGX_WireComponent::SetMinSegmentLength(float InMinSegmentLength)
{
	if (HasNative())
	{
		const float ResolutionPerUnitLength = 1.0f / InMinSegmentLength;
		NativeBarrier.SetResolutionPerUnitLength(ResolutionPerUnitLength);
	}
	MinSegmentLength = InMinSegmentLength;
}

FAGX_WireWinchRef UAGX_WireComponent::GetOwnedBeginWinch_BP()
{
	return {&OwnedBeginWinch};
}

bool UAGX_WireComponent::HasOwnedBeginWinch() const
{
	return BeginWinchType == EWireWinchOwnerType::Wire;
}

bool UAGX_WireComponent::HasBeginWinchComponent() const
{
	return GetBeginWinchComponent() != nullptr;
}

void UAGX_WireComponent::SetBeginWinchComponent(UAGX_WireWinchComponent* Winch)
{
	if (Winch == nullptr)
	{
		BeginWinchComponent = FComponentReference();
		return;
	}

	BeginWinchComponent.OtherActor = Winch->GetOwner();
	BeginWinchComponent.ComponentProperty = Winch->GetFName();
}

UAGX_WireWinchComponent* UAGX_WireComponent::GetBeginWinchComponent()
{
	return FAGX_ObjectUtilities::Get<UAGX_WireWinchComponent>(BeginWinchComponent, GetOwner());
}

const UAGX_WireWinchComponent* UAGX_WireComponent::GetBeginWinchComponent() const
{
	return FAGX_ObjectUtilities::Get<const UAGX_WireWinchComponent>(
		BeginWinchComponent, GetOwner());
}

FAGX_WireWinch* UAGX_WireComponent::GetBeginWinchComponentWinch()
{
	return const_cast<FAGX_WireWinch*>(
		const_cast<const UAGX_WireComponent*>(this)->GetBeginWinchComponentWinch());
}

const FAGX_WireWinch* UAGX_WireComponent::GetBeginWinchComponentWinch() const
{
	const UAGX_WireWinchComponent* WinchComponent = GetBeginWinchComponent();
	if (WinchComponent == nullptr)
	{
		return nullptr;
	}
	return &WinchComponent->WireWinch;
}

FAGX_WireWinchRef UAGX_WireComponent::GetBeginWinchComponentWinch_BP()
{
	return {GetBeginWinchComponentWinch()};
}

void UAGX_WireComponent::SetBorrowedBeginWinch(FAGX_WireWinchRef Winch)
{
	BorrowedBeginWinch = Winch.Winch;
}

bool UAGX_WireComponent::HasBeginWinch() const
{
	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return true;
		case EWireWinchOwnerType::WireWinch:
			return HasBeginWinchComponent();
		case EWireWinchOwnerType::Other:
			return BorrowedBeginWinch != nullptr;
		case EWireWinchOwnerType::None:
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid Begin Winch Type found in Wire '%s' in '%s'. Cannot determine presence of "
			 "begin winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FAGX_WireWinch* UAGX_WireComponent::GetBeginWinch()
{
	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedBeginWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasBeginWinchComponent() ? GetBeginWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedBeginWinch != nullptr ? BorrowedBeginWinch : nullptr;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid Begin Winch Type found in Wire '%s' in '%s'. Cannot get begin winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetBeginWinch() const
{
	switch (BeginWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedBeginWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasBeginWinchComponent() ? GetBeginWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedBeginWinch;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid Begin Winch Type found in Wire '%s' in '%s'. Cannot get begin winch."),
		*GetName(), *GetLabelSafe(GetOwner()))
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetBeginWinch_BP()
{
	return {GetBeginWinch()};
}

bool UAGX_WireComponent::AttachOwnedBeginWinch()
{
	return AttachOwnedWinch(EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinch(UAGX_WireWinchComponent* Winch)
{
	return AttachWinch(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinch(FAGX_WireWinchRef Winch)
{
	return AttachWinch(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinchToComponent(UAGX_WireWinchComponent* Winch)
{
	return AttachWinchToComponent(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::AttachBeginWinchToOther(FAGX_WireWinchRef Winch)
{
	return AttachWinchToOther(Winch, EWireSide::Begin);
}

bool UAGX_WireComponent::DetachBeginWinch()
{
	return DetachWinch(EWireSide::Begin);
}

/*
 * End Winch.
 */

FAGX_WireWinchRef UAGX_WireComponent::GetOwnedEndWinch_BP()
{
	return {&OwnedEndWinch};
}

bool UAGX_WireComponent::HasOwnedEndWinch() const
{
	return EndWinchType == EWireWinchOwnerType::Wire;
}

bool UAGX_WireComponent::HasEndWinchComponent() const
{
	return GetEndWinchComponent() != nullptr;
}

void UAGX_WireComponent::SetEndWinchComponent(UAGX_WireWinchComponent* Winch)
{
	if (Winch == nullptr)
	{
		EndWinchComponent = FComponentReference();
		return;
	}

	EndWinchComponent.OtherActor = Winch->GetOwner();
	EndWinchComponent.ComponentProperty = Winch->GetFName();
}

UAGX_WireWinchComponent* UAGX_WireComponent::GetEndWinchComponent()
{
	return FAGX_ObjectUtilities::Get<UAGX_WireWinchComponent>(EndWinchComponent, GetOwner());
}

const UAGX_WireWinchComponent* UAGX_WireComponent::GetEndWinchComponent() const
{
	return FAGX_ObjectUtilities::Get<const UAGX_WireWinchComponent>(EndWinchComponent, GetOwner());
}

FAGX_WireWinch* UAGX_WireComponent::GetEndWinchComponentWinch()
{
	return const_cast<FAGX_WireWinch*>(
		const_cast<const UAGX_WireComponent*>(this)->GetEndWinchComponentWinch());
}

FAGX_WireWinchRef UAGX_WireComponent::GetEndWinchComponentWinch_BP()
{
	return {GetEndWinchComponentWinch()};
}

const FAGX_WireWinch* UAGX_WireComponent::GetEndWinchComponentWinch() const
{
	const UAGX_WireWinchComponent* WinchComponent = GetEndWinchComponent();
	if (WinchComponent == nullptr)
	{
		return nullptr;
	}
	return &WinchComponent->WireWinch;
}

void UAGX_WireComponent::SetBorrowedEndWinch(FAGX_WireWinchRef Winch)
{
	BorrowedEndWinch = Winch.Winch;
}

bool UAGX_WireComponent::HasEndWinch() const
{
	switch (EndWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return true;
		case EWireWinchOwnerType::WireWinch:
			return HasEndWinchComponent();
		case EWireWinchOwnerType::Other:
			return BorrowedEndWinch != nullptr;
		case EWireWinchOwnerType::None:
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid End Winch Type found in Wire '%s' in '%s'. Cannot determine presence of end "
			 "winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FAGX_WireWinch* UAGX_WireComponent::GetEndWinch()
{
	switch (EndWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedEndWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasEndWinchComponent() ? GetEndWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedEndWinch != nullptr ? BorrowedEndWinch : nullptr;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid End Winch Type found in Wire '%s' in '%s'. Cannot get end winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetEndWinch() const
{
	switch (EndWinchType)
	{
		case EWireWinchOwnerType::Wire:
			return &OwnedEndWinch;
		case EWireWinchOwnerType::WireWinch:
			return HasEndWinchComponent() ? GetEndWinchComponentWinch() : nullptr;
		case EWireWinchOwnerType::Other:
			return BorrowedEndWinch;
		case EWireWinchOwnerType::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid End Winch Type found in Wire '%s' in '%s'. Cannot get end winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetEndWinch_BP()
{
	return {GetEndWinch()};
}

bool UAGX_WireComponent::AttachOwnedEndWinch()
{
	return AttachOwnedWinch(EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinch(UAGX_WireWinchComponent* Winch)
{
	return AttachWinch(Winch, EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinch(FAGX_WireWinchRef Winch)
{
	return AttachWinch(Winch, EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinchToComponent(UAGX_WireWinchComponent* Winch)
{
	return AttachWinchToComponent(Winch, EWireSide::End);
}

bool UAGX_WireComponent::AttachEndWinchToOther(FAGX_WireWinchRef Winch)
{
	return AttachWinchToOther(Winch, EWireSide::End);
}

bool UAGX_WireComponent::DetachEndWinch()
{
	return DetachWinch(EWireSide::End);
}

/*
 * Side-agnostic winch.
 */

void UAGX_WireComponent::SetWinchType(EWireWinchOwnerType Type, EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			BeginWinchType = Type;
			return;
		case EWireSide::End:
			EndWinchType = Type;
			return;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Set Winch Type for Wire '%s' in '%s'. Doing "
					 "nothing."),
				*GetName(), *GetNameSafe(GetOwner()));
			return;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Winch Type for Wire '%s' in '%s'. Cannot set winch "
			 "type."),
		*GetName(), *GetLabelSafe(GetOwner()));
}

EWireWinchOwnerType UAGX_WireComponent::GetWinchType(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BeginWinchType;
		case EWireSide::End:
			return EndWinchType;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Get Winch Type for Wire '%s' in '%s'."), *GetName(),
				*GetNameSafe(GetOwner()));
			return EWireWinchOwnerType::None;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch Type for Wire '%s' in '%s'. Cannot determine "
			 "winch type."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return EWireWinchOwnerType::None;
}

FAGX_WireWinch* UAGX_WireComponent::GetOwnedWinch(EWireSide Side)
{
	return const_cast<FAGX_WireWinch*>(
		const_cast<const UAGX_WireComponent*>(this)->GetOwnedWinch(Side));
}

const FAGX_WireWinch* UAGX_WireComponent::GetOwnedWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return &OwnedBeginWinch;
		case EWireSide::End:
			return &OwnedEndWinch;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Owned Winch for Wire '%s' in '%s'. Cannot determine "
			 "owned winch to return."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetOwnedWinch_BP(EWireSide Side)
{
	return {GetOwnedWinch(Side)};
}

bool UAGX_WireComponent::SetBorrowedWinch(FAGX_WireWinchRef Winch, EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			SetBorrowedBeginWinch(Winch);
			return true;
		case EWireSide::End:
			SetBorrowedEndWinch(Winch);
			return true;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Set Borrowed Winch for Wire '%s' in '%s'. Doing "
					 "nothing."),
				*GetName(), *GetNameSafe(GetOwner()));
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Borrowed Winch for Wire '%s' in '%s'. Cannot set "
			 "winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

bool UAGX_WireComponent::HasWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return HasBeginWinch();
		case EWireSide::End:
			return HasEndWinch();
		case EWireSide::None:
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Has Winch for Wire '%s' in '%s'. Cannot determine winch "
			 "presence."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FAGX_WireWinch* UAGX_WireComponent::GetWinch(EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetBeginWinch();
		case EWireSide::End:
			return GetEndWinch();
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch for Wire '%s' in '%s'. Cannot get winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetBeginWinch();
		case EWireSide::End:
			return GetEndWinch();
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch for Wire '%s' in '%s'. Cannot get winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinchRef UAGX_WireComponent::GetWinch_BP(EWireSide Side)
{
	return {GetWinch(Side)};
}

bool UAGX_WireComponent::AttachOwnedWinch(EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to AttachOwnedWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	checkf(GetOwnedWinch(Side) != nullptr, TEXT("Did not get a winch despite valid side."));
	FAGX_WireWinch& Winch = *GetOwnedWinch(Side);
	SetWinchType(EWireWinchOwnerType::Wire, Side);
	if (HasNative())
	{
		FAGX_WireUtilities::ComputeSimulationPlacement(*this, Winch);
		FWireWinchBarrier* Barrier = Winch.GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot attach Wire '%s' in '%s' to it's owned %s winch because the AGX "
					 "Dynamics instance of the winch could not be created."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
		bool bAttached = NativeBarrier.Attach(*Barrier, Side == EWireSide::Begin);
		if (!bAttached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not attach %s side to "
					 "owned Wire Winch. See LogAGXDynamics for details."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			SetWinchType(EWireWinchOwnerType::None, EWireSide::Begin);
			return false;
		}
	}

	return true;
}

bool UAGX_WireComponent::AttachWinch(UAGX_WireWinchComponent* Winch, EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to AttachWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	if (Winch == nullptr)
	{
		SetWinchType(EWireWinchOwnerType::None, Side);
		SetWinchComponent(nullptr, Side);
		if (HasNative())
		{
			NativeBarrier.Detach(Side == EWireSide::Begin);
		}
		return true;
	}

	SetWinchType(EWireWinchOwnerType::WireWinch, Side);
	SetWinchComponent(Winch, Side);

	if (HasNative())
	{
		FWireWinchBarrier* Barrier = Winch->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot attach Wire '%s' in '%s' to the Wire Winch '%s' in '%s' because the "
					 "AGX Dynamics instance of the winch could not be created."),
				*GetName(), *GetNameSafe(GetOwner()), *Winch->GetName(),
				*GetNameSafe(Winch->GetOwner()));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
		const bool bAttached = NativeBarrier.Attach(*Barrier, Side == EWireSide::Begin);
		if (!bAttached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not attach %s side to "
					 "Wire Winch Component. See LogAGXDynamics for details."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
	}

	return true;
}

bool UAGX_WireComponent::AttachWinch(FAGX_WireWinchRef Winch, EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to AttachWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	if (!Winch.IsValid())
	{
		SetWinchType(EWireWinchOwnerType::None, Side);
		SetBorrowedWinch({nullptr}, Side);
		if (HasNative())
		{
			NativeBarrier.Detach(Side == EWireSide::Begin);
		}
		return true;
	}

	SetWinchType(EWireWinchOwnerType::Other, Side);
	SetBorrowedWinch(Winch, Side);
	if (HasNative())
	{
		FAGX_WireUtilities::ComputeSimulationPlacement(*Winch.Winch);
		FWireWinchBarrier* Barrier = Winch.Winch->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Cannot attach Wire '%s' in '%s' to custom Wire Winch because the AGX "
					 "Dynamics instance of the winch could not be created."),
				*GetName(), *GetNameSafe(GetOwner()));
			SetWinchType(EWireWinchOwnerType::None, Side);
			return false;
		}
		const bool bAttached = NativeBarrier.Attach(*Barrier, Side == EWireSide::Begin);
		if (!bAttached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not attach %s side to "
					 "custom Wire Winch. See LogAGXDynamics for details."),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side), *GetName(),
				*GetNameSafe(GetOwner()));
			return false;
		}
	}

	return true;
}

bool UAGX_WireComponent::AttachWinchToComponent(UAGX_WireWinchComponent* Winch, EWireSide Side)
{
	return AttachWinch(Winch, Side);
}

bool UAGX_WireComponent::AttachWinchToOther(FAGX_WireWinchRef Winch, EWireSide Side)
{
	return AttachWinch(Winch, Side);
}

bool UAGX_WireComponent::DetachWinch(EWireSide Side)
{
	if (Side != EWireSide::Begin && Side != EWireSide::End)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Invalid Side passed to DetachWinch for Wire '%s' in '%s'."),
			*GetName(), *GetNameSafe(GetOwner()));
		return false;
	}

	SetWinchType(EWireWinchOwnerType::None, Side);
	if (HasNative())
	{
		bool bDetached = NativeBarrier.Detach(Side == EWireSide::Begin);
		if (!bDetached)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("AGX Dynamics instance of Wire '%s' in '%s' could not detach %s side. See "
					 "LogAGXDynamics for details."),
				*GetName(), *GetNameSafe(GetOwner()),
				*StaticEnum<EWireSide>()->GetNameStringByValue((int64) Side));
			return false;
		}
	}
	return true;
}

bool UAGX_WireComponent::SetWinchOwnerType(EWireSide Side, EWireWinchOwnerType Type)
{
	switch (Side)
	{
		case EWireSide::Begin:
			BeginWinchType = Type;
			return true;
		case EWireSide::End:
			EndWinchType = Type;
			return true;
		case EWireSide::None:
			return true;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Winch Owner Type for Wire '%s' in '%s'. Cannot set "
			 "winch owner type."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

EWireWinchOwnerType UAGX_WireComponent::GetWinchOwnerType(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BeginWinchType;
		case EWireSide::End:
			return EndWinchType;
		case EWireSide::None:
			return EWireWinchOwnerType::None;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch Owner Type for Wire '%s' in '%s'. Cannot get "
			 "winch owner type."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return EWireWinchOwnerType::None;
}

UAGX_WireWinchComponent* UAGX_WireComponent::GetWinchComponent(EWireSide Side)
{
	return const_cast<UAGX_WireWinchComponent*>(
		const_cast<const UAGX_WireComponent*>(this)->GetWinchComponent(Side));
}

const UAGX_WireWinchComponent* UAGX_WireComponent::GetWinchComponent(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return GetBeginWinchComponent();
		case EWireSide::End:
			return GetEndWinchComponent();
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Winch Component for Wire '%s' in '%s'. Cannot get "
			 "winch component."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

bool UAGX_WireComponent::SetWinchComponent(UAGX_WireWinchComponent* Winch, EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			SetBeginWinchComponent(Winch);
			return true;
		case EWireSide::End:
			SetEndWinchComponent(Winch);
			return true;
		case EWireSide::None:
			UE_LOG(
				LogAGX, Warning,
				TEXT("Wire side None passed to Set Winch Component for Wire '%s' in '%s'. Doing "
					 "nothing."),
				*GetName(), *GetNameSafe(GetOwner()));
			return false;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Set Winch Component for Wire '%s' in '%s'. Cannot set "
			 "winch component."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return false;
}

FComponentReference* UAGX_WireComponent::GetWinchComponentReference(EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			return &BeginWinchComponent;
		case EWireSide::End:
			return &EndWinchComponent;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed Get Winch Component Reference for Wire '%s' in '%s'. Cannot "
			 "get winch component reference."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

FAGX_WireWinch* UAGX_WireComponent::GetBorrowedWinch(EWireSide Side)
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BorrowedBeginWinch;
		case EWireSide::End:
			return BorrowedEndWinch;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Borrowed Winch for Wire '%s' in '%s'. Cannot get "
			 "borrowed winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

const FAGX_WireWinch* UAGX_WireComponent::GetBorrowedWinch(EWireSide Side) const
{
	switch (Side)
	{
		case EWireSide::Begin:
			return BorrowedBeginWinch;
		case EWireSide::End:
			return BorrowedEndWinch;
		case EWireSide::None:
			return nullptr;
	}
	UE_LOG(
		LogAGX, Error,
		TEXT("Invalid wire side passed to Get Borrowed Winch for Wire '%s' in '%s'. Cannot get "
			 "borrowed winch."),
		*GetName(), *GetLabelSafe(GetOwner()));
	return nullptr;
}

namespace AGX_WireComponent_helpers
{
	void PrintNodeModifiedAlreadyInitializedWarning()
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Route node modification to already initialized wire. This route node will be "
				 "ignored."));
	}
}

FWireRoutingNode& UAGX_WireComponent::AddNode()
{
	int32 _;
	return AddNode(_);
}

FWireRoutingNode& UAGX_WireComponent::AddNode(int32& OutIndex)
{
	return AddNode(FWireRoutingNode(), OutIndex);
}

FWireRoutingNode& UAGX_WireComponent::CreateNode(int32& OutIndex)
{
	return AddNode(OutIndex);
}

FWireRoutingNode& UAGX_WireComponent::AddNode(const FWireRoutingNode& InNode)
{
	int32 _;
	return AddNode(InNode, _);
}

FWireRoutingNode& UAGX_WireComponent::AddNode(const FWireRoutingNode& InNode, int32& OutIndex)
{
	const int32 Index = RouteNodes.Num();
	OutIndex = Index;
	return AddNodeAtIndex(InNode, Index);
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtLocation(FVector InLocation)
{
	int32 _;
	return AddNodeAtLocation(InLocation, _);
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtLocation(FVector InLocation, int32& OutIndex)
{
	return AddNode(FWireRoutingNode(InLocation), OutIndex);
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtLocationAtIndex(FVector InLocation, int32 InIndex)
{
	return AddNodeAtIndex(FWireRoutingNode(InLocation), InIndex);
}

FWireRoutingNode& UAGX_WireComponent::AddNodeAtIndex(const FWireRoutingNode& InNode, int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	if (!RouteNodes.IsValidIndex(InIndex) && InIndex != RouteNodes.Num())
	{
		// Nodes may only be added at an index where there already is a node, or one-past-end.
		return InvalidRoutingNode;
	}
	RouteNodes.Insert(InNode, InIndex);
	FWireRoutingNode& NewNode = RouteNodes[InIndex];
	AActor* LocalScope = FAGX_ObjectUtilities::GetRootParentActor(this);
	AGX_WireComponent_helpers::SetLocalScope(NewNode, LocalScope);
	return NewNode;
}

void UAGX_WireComponent::SetNode(const int32 InIndex, const FWireRoutingNode InNode)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	if (!RouteNodes.IsValidIndex(InIndex))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Out-of-bounds index %d for route nodes array was passed to Set Node in Wire "
				 "Component '%s' in Actor '%s'"),
			InIndex, *GetName(), *GetLabelSafe(GetOwner()));
		return;
	}
	RouteNodes[InIndex] = InNode;
	AActor* LocalScope = FAGX_ObjectUtilities::GetRootParentActor(this);
	AGX_WireComponent_helpers::SetLocalScope(RouteNodes[InIndex], LocalScope);
}

void UAGX_WireComponent::RemoveNode(int32 InIndex)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes.RemoveAt(InIndex);
}

void UAGX_WireComponent::SetNodeLocalLocation(int32 InIndex, FVector InLocation)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	RouteNodes[InIndex].Frame.LocalLocation = InLocation;
}

void UAGX_WireComponent::SetNodeLocation(int32 InIndex, const FVector InLocation)
{
	if (HasNative())
	{
		AGX_WireComponent_helpers::PrintNodeModifiedAlreadyInitializedWarning();
	}
	USceneComponent* Parent = RouteNodes[InIndex].Frame.GetParentComponent();
	if (Parent == nullptr)
	{
		// No parent means the LocalLocation is relative to the Wire Component and thus InLocation
		// can be used as-is.
		RouteNodes[InIndex].Frame.LocalLocation = InLocation;
		return;
	}

	// Compute a local location relative to the parent that has the same world location as
	// InLocation in the Wire Component.
	const FTransform& ParentTransform = Parent->GetComponentTransform();
	const FTransform& WireTransform = GetComponentTransform();
	const FTransform& WireToParent = WireTransform.GetRelativeTransform(ParentTransform);
	const FVector LocationInParent = WireToParent.TransformPosition(InLocation);
	RouteNodes[InIndex].Frame.LocalLocation = LocationInParent;
}

bool UAGX_WireComponent::IsLumpedNode(const FAGX_WireNode& Node)
{
	if (!HasNative() || !Node.HasNative())
	{
		return false;
	}

	return NativeBarrier.IsLumpedNode(*Node.GetNative());
}

bool UAGX_WireComponent::IsInitialized() const
{
	if (!HasNative())
	{
		return false;
	}
	return NativeBarrier.IsInitialized();
}

double UAGX_WireComponent::GetRestLength() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRestLength();
	}
	if (RouteNodes.Num() <= 1)
	{
		return 0.0;
	}

	double Length {0.0};
	FVector PreviousLocation = RouteNodes[0].Frame.GetWorldLocation(*this);
	for (int32 I = 1; I < RouteNodes.Num(); ++I)
	{
		const FVector Location = RouteNodes[I].Frame.GetWorldLocation(*this);
		Length += FVector::Distance(PreviousLocation, Location);
		PreviousLocation = Location;
	}

	return Length;
}

float UAGX_WireComponent::GetRestLength_BP() const
{
	return static_cast<float>(GetRestLength());
}

double UAGX_WireComponent::GetMass() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMass();
	}
	if (ShapeMaterial == nullptr)
	{
		/// @note Can we find the density that AGX Dynamics will use for wires that don't have an
		/// explicit material set?
		return 0.0;
	}
	const double Area = PI * Radius * Radius; // Assume circular cross-section.
	const double Length = GetRestLength();
	const double Density = ShapeMaterial->Bulk.Density;
	const double Mass = Area * Length * Density;
	return Mass;
}

float UAGX_WireComponent::GetMass_BP() const
{
	return static_cast<float>(GetMass());
}

double UAGX_WireComponent::GetTension() const
{
	if (!HasNative())
	{
		return 0.0;
	}
	return NativeBarrier.GetTension();
}

float UAGX_WireComponent::GetTension_BP() const
{
	return static_cast<float>(GetTension());
}

bool UAGX_WireComponent::HasRenderNodes() const
{
	if (!HasNative())
	{
		return false;
	}
	return !NativeBarrier.GetRenderListEmpty();
}

bool UAGX_WireComponent::GetRenderListEmpty() const
{
	if (!HasNative())
	{
		return true;
	}
	return NativeBarrier.GetRenderListEmpty();
}

FAGX_WireRenderIterator UAGX_WireComponent::GetRenderBeginIterator() const
{
	if (!HasNative())
	{
		return {};
	}
	return {NativeBarrier.GetRenderBeginIterator()};
}

FAGX_WireRenderIterator UAGX_WireComponent::GetRenderEndIterator() const
{
	if (!HasNative())
	{
		return {};
	}
	return {NativeBarrier.GetRenderEndIterator()};
}

TArray<FVector> UAGX_WireComponent::GetRenderNodeLocations() const
{
	TArray<FVector> Result;
	for (auto It = GetRenderBeginIterator(), End = GetRenderEndIterator(); It != End; It.Inc())
	{
		const FAGX_WireNode Node = It.Get();
		Result.Add(Node.GetWorldLocation());
	}
	return Result;
}

#if WITH_EDITOR

void UAGX_WireComponent::OnRouteNodeParentMoved(
	USceneComponent* Component, EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	// If this is a callback from a parent we are no longer a child of, then unsubscribe.
	FWireRoutingNode* Node =
		RouteNodes.FindByPredicate([Component](const FWireRoutingNode& Node)
								   { return Node.Frame.GetParentComponent() == Component; });
	if (Node == nullptr)
	{
		// Component is not the parent of any node, unsubscribe.
		const FParentDelegate* Handle = DelegateHandles.Find(Component);
		if (Handle != nullptr && Handle->Parent.IsValid())
		{
			Component->TransformUpdated.Remove(Handle->DelegateHandle);
			DelegateHandles.Remove(Component);
		}
		return;
	}

	// At least one Routing Node has the moved Component as its parent.
	UpdateVisuals();
}

void UAGX_WireComponent::OnRouteNodeParentReplaced(
	const FCoreUObjectDelegates::FReplacementObjectMap& /*OldToNew*/)
{
	// Here we used to do incremental updates of the Delegate Handles table, but that doesn't work
	// for the rename case, i.e. when the a Blueprint Reconstruction happens due to the parent
	// Component being renamed in the Blueprint. In that case we can no longer find the old parent
	// anymore since there is no way of finding the supposedly new parent. So we are forced to do
	// a full synchronization.
	//
	// Perhaps not a major concern, the old callbacks will be found and cleared out at some point
	// in the future (perhaps). If this full synchronization call becomes a performance problem then
	// search the Git patch history for the string
	//
	//   Any changes made to the callback setup is a sign that we may need to update the wire
	//
	// to find the old code, or do something better than what we did then.
	SynchronizeParentMovedCallbacks();
	UpdateVisuals();
}

#endif

void UAGX_WireComponent::MarkVisualsDirty()
{
	UpdateVisuals();
}

namespace AGX_WireComponent_helpers
{
	UAGX_ShapeMaterial* GetOrCreateShapeMaterial(
		const FWireBarrier& Barrier, FAGX_ImportContext& Context)
	{
		const FShapeMaterialBarrier SMB = Barrier.GetMaterial();
		if (!SMB.HasNative())
			return nullptr;

		return FAGX_ImportRuntimeUtilities::GetOrCreateShapeMaterial(SMB, &Context);
	}

	bool ConfigureWinch(
		EWireSide Side, UAGX_WireComponent& Wire, const FWireBarrier& Barrier,
		FAGX_ImportContext& Context)
	{
		FWireWinchBarrier WinchBarrier = Barrier.GetWinch(Side);
		if (!WinchBarrier.HasNative())
		{
			Wire.SetWinchOwnerType(Side, EWireWinchOwnerType::None);
			return false;
		}

		FAGX_WireWinch* Winch = Wire.GetOwnedWinch(Side);

		// Get Owned Winch can never return nullptr when a valid Side is passed.
		Winch->CopyFrom(WinchBarrier);
		FRigidBodyBarrier WinchBodyBarrier = WinchBarrier.GetRigidBody();
		UAGX_RigidBodyComponent* WinchBodyComponent =
			Context.RigidBodies->FindRef(WinchBodyBarrier.GetGuid());

		// Ok for WinchBodyComponent to be nullptr. Means attached to the world.
		Winch->SetBodyAttachment(WinchBodyComponent);
		Wire.SetWinchOwnerType(Side, EWireWinchOwnerType::Wire);
		return true;
	}

	void TryCreateBodyFixedNode(
		FWireNodeBarrier NodeBarrier, TArray<FWireRoutingNode>& Route, FAGX_ImportContext& Context)
	{
		if (NodeBarrier.GetType() != EWireNodeType::BodyFixed)
			return;

		FRigidBodyBarrier NodeBodyBarrier = NodeBarrier.GetRigidBody();
		if (!NodeBodyBarrier.HasNative())
			return;

		UAGX_RigidBodyComponent* Body = Context.RigidBodies->FindRef(NodeBodyBarrier.GetGuid());
		if (Body == nullptr)
			return;

		if (Route.Num() > 0 && Route[0].NodeType == EWireNodeType::Free)
		{
			// In an initialized wire there may be a Free node right on top of the Body Fixe node.
			// The Body Fixed node represents the body itself, while the Free node represents the
			// part of the wire that approaches the body. While routing we only need the Body Fixed
			// node, it represents both concepts.
			Route.Pop();
		}

		FWireRoutingNode RouteNode;
		RouteNode.NodeType = EWireNodeType::BodyFixed;
		RouteNode.Frame.SetParentComponent(Body);
		RouteNode.Frame.LocalLocation = NodeBarrier.GetTranslate();

		RouteNode.SetBody(Body);
		Route.Add(RouteNode);
	}
}

void UAGX_WireComponent::CopyFrom(const FWireBarrier& Barrier, FAGX_ImportContext* Context)
{
	using namespace AGX_WireComponent_helpers;
	check(Barrier.HasNative());
	Radius = Barrier.GetRadius();
	MinSegmentLength = 1.0f / Barrier.GetResolutionPerUnitLength();
	LinearVelocityDamping = static_cast<float>(Barrier.GetLinearVelocityDamping());
	bCanCollide = Barrier.GetEnableCollisions();
	WireParameterController.CopyFrom(Barrier.GetParameterController());

	for (const FName& Group : Barrier.GetCollisionGroups())
		AddCollisionGroup(Group);

	const FMergeSplitPropertiesBarrier Msp =
		FMergeSplitPropertiesBarrier::CreateFrom(*const_cast<FWireBarrier*>(&Barrier));
	if (Msp.HasNative())
		MergeSplitProperties.CopyFrom(Msp, Context);

	ImportGuid = Barrier.GetGuid();
	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		GetOwner(), Barrier.GetName(), UAGX_WireComponent::StaticClass());
	Rename(*Name);

	auto ParameterControllerBarrier = Barrier.GetParameterController();
	if (ParameterControllerBarrier.HasNative())
		WireParameterController.CopyFrom(ParameterControllerBarrier);

	if (Context == nullptr || Context->Wires == nullptr || Context->ShapeMaterials == nullptr)
		return; // We are done.

	AGX_CHECK(!Context->Wires->Contains(ImportGuid));
	Context->Wires->Add(ImportGuid, this);

	ShapeMaterial = GetOrCreateShapeMaterial(Barrier, *Context);

	const bool bHaveBeginWinch = ConfigureWinch(EWireSide::Begin, *this, Barrier, *Context);
	const bool bHaveEndWinch = ConfigureWinch(EWireSide::End, *this, Barrier, *Context);

	if (!bHaveBeginWinch)
	{
		// Configure initial Body Fixe node. Some Body Fixed nodes are owned by the winch on that
		// side, do not create an explicit Body Fixed node in that case.
		TryCreateBodyFixedNode(Barrier.GetFirstNode(), RouteNodes, *Context);
	}

	// Configure "normal" route nodes.
	for (auto It = Barrier.GetRenderBeginIterator(), End = Barrier.GetRenderEndIterator();
		 It != End; It.Inc())
	{
		const FAGX_WireNode NodeAGX = It.Get();
		const EWireNodeType NodeType = [&NodeAGX, &Barrier, &It, Context]() -> EWireNodeType
		{
			if (Barrier.IsLumpedNode(It))
			{
				// Lumped nodes are special somehow, and should be created as free nodes.
				return EWireNodeType::Free;
			}
			switch (NodeAGX.GetType())
			{
				case EWireNodeType::Free:
					// Free nodes can be routed as-is.
					return EWireNodeType::Free;
				case EWireNodeType::Eye:
					// Eye nodes can be routed as-is.
					return EWireNodeType::Eye;
				case EWireNodeType::BodyFixed:
					// A Body Fixed node found by the render iterator is an implicitly created node
					// and should be routed as a Free node. It should not be attached to any of the
					// Rigid Bodies in the Component list.
					return EWireNodeType::Free;
				case EWireNodeType::Stop:
					// Stop nodes are used by winches, which we detect with GetWinch instead.
					return EWireNodeType::Other;
				default:
					// Any other node type is routed as a Free node for now. Special handling may
					// be needed in the future, if routing with additional node types become
					// supported.
					return EWireNodeType::Free;
			}
		}();

		if (NodeType == EWireNodeType::Other)
		{
			// Other nodes are used to signal "skip this node".
			continue;
		}

		FWireRoutingNode RouteNode;
		RouteNode.NodeType = NodeType;
		if (NodeType == EWireNodeType::Eye)
		{
			// Eye nodes are attached to a Rigid Body, so make Local Location relative to that body.
			FRigidBodyBarrier BodyBarrier = NodeAGX.GetRigidBody();
			UAGX_RigidBodyComponent* BodyComponent = Context->RigidBodies->FindRef(BodyBarrier.GetGuid());
			if (BodyComponent != nullptr)
			{
				RouteNode.SetBody(BodyComponent);
				RouteNode.Frame.SetParentComponent(BodyComponent);
				RouteNode.Frame.LocalLocation = NodeAGX.GetLocalLocation();
			}
		}
		else
		{
			// All other node types are placed relative to the Wire Component.
			RouteNode.Frame.SetParentComponent(nullptr);
			RouteNode.Frame.LocalLocation = NodeAGX.GetWorldLocation();
		}

		RouteNodes.Add(RouteNode);
	}

	if (!bHaveEndWinch)
	{
		// Configure ending Body Fixed node. Some Body Fixed nodes are owned by the winch on that
		// side, do not create an explicit Body Fixed node in that case.
		TryCreateBodyFixedNode(Barrier.GetLastNode(), RouteNodes, *Context);
	}
}

bool UAGX_WireComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_WireComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_WireComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
	if (HasNative())
	{
		MergeSplitProperties.BindBarrierToOwner(*GetNative());
		WireParameterController.SetBarrier(NativeBarrier.GetParameterController());
	}
	else
	{
		WireParameterController.ClearBarrier();
	}
}

FWireBarrier* UAGX_WireComponent::GetOrCreateNative()
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
				TEXT("A request for the AGX Dynamics instance for Wire '%s' in '%s' was made but "
					 "we are in the middle of a Blueprint Reconstruction and the requested instance"
					 "has not yet been restored. The instance cannot be returned, which may lead to"
					 "incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		CreateNative();
	}
	check(HasNative()); /// \todo Consider better error handling than 'check'.
	return &NativeBarrier;
}

FWireBarrier* UAGX_WireComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FWireBarrier* UAGX_WireComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_WireComponent::PostInitProperties()
{
	Super::PostInitProperties();
	AGX_WireComponent_helpers::SetLocalScope(*this);

#if WITH_EDITOR
	InitPropertyDispatcher();

	// If the wire routing node frame parent's owner is a Blueprint instance then any
	// modification of that instance will cause a Blueprint Reconstruction. During
	// reconstruction all the Components will be destroyed and recreated. Unfortunately, Scene
	// Component does not use Actor Component Instance Data to transfer delegate callbacks, such
	// as Transform Updated, from the old object to the new one, so we need to do that
	// ourselves. Fortunately, the engine provides the On Object Replaced event that we can use
	// to do the transfer.
	//
	// Must be in Post Init Properties, not Post Load, because not all Components get the Post Load
	// callback. In particular, adding a new Component to an already existing Actor in a level will
	// not call Post Load.
	if (!ObjectsReplacedDelegateHandle.IsValid())
	{
		ObjectsReplacedDelegateHandle = FCoreUObjectDelegates::OnObjectsReplaced.AddUObject(
			this, &UAGX_WireComponent::OnRouteNodeParentReplaced);
	}
#endif
}

void UAGX_WireComponent::PostLoad()
{
	Super::PostLoad();

	// A Wire Component may contain Component References that don't yet exist when Post Init
	// Properties is run. For example, they may be part of the state that is read from drive on
	// level load, or cloned from a template. If those Component References don't specify an Owning
	// Actor explicitly then the implicit one, i.e. our outer Actor, should be used. The Begin- and
	// EndWinch don't need to do this because they always exists and is always set in Post Init
	// Properties.
	AGX_WireComponent_helpers::SetLocalScope(*this);

#if WITH_EDITOR
	// Condition on not Game World instead of is Editor World because we do not want this
	// block to run in Play In Editor sessions.
	if (GetWorld() != nullptr && !GetWorld()->IsGameWorld() && !HasAnyFlags(RF_ClassDefaultObject))
	{
		// While in the editor we don't update the wire rendering every tick and instead rely on
		// Transform Updated callbacks from the wire routing node frame parents. These callbacks
		// must be registered with each parent on start-up. We can't do that here because some of
		// the parents may not have been loaded yet. So we set up a callback to happen when the
		// level has finished loading, and hope that everything has been loaded by then.
		if (!MapLoadDelegateHandle.IsValid())
		{
			MapLoadDelegateHandle = FEditorDelegates::MapChange.AddWeakLambda(
				this,
				[this](uint32)
				{
					FEditorDelegates::MapChange.RemoveAll(this);
					SynchronizeParentMovedCallbacks();
					UpdateVisuals();
				});
		}
	}
#endif
}

#if WITH_EDITOR

void UAGX_WireComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, Radius),
		[](ThisClass* Wire) { Wire->SetRadius(Wire->Radius); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, MinSegmentLength),
		[](ThisClass* Wire) { Wire->SetMinSegmentLength(Wire->MinSegmentLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, MergeSplitProperties),
		[](ThisClass* This) { This->MergeSplitProperties.OnPostEditChangeProperty(*This); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, bCanCollide),
		[](ThisClass* This) { This->SetCanCollide(This->bCanCollide); });

// Helper macro for registering Property Dispatches callbacks when modifying members of the
// Wire Parameter Controller struct.
#define ADD_PARAMETER_CONTROLLER_DISPACTCHER(PropertyName)                    \
	Dispatcher.Add(                                                           \
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, WireParameterController), \
		GET_MEMBER_NAME_CHECKED(FAGX_WireParameterController, PropertyName),  \
		[](ThisClass* This)                                                   \
		{                                                                     \
			if (This->HasNative())                                            \
			{                                                                 \
				This->WireParameterController.Set##PropertyName(              \
					This->WireParameterController.PropertyName);              \
			}                                                                 \
		})

	// Dispatchers for Properties owned by Wire Parameter Controller.
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(MaximumContactMovementOneTimestep);
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(MinimumDistanceBetweenNodes);
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(RadiusMultiplier);
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(ScaleConstant);
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(SplitTensionMultiplier);
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(StopNodeLumpMinDistanceFraction);
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(StopNodeReferenceDistance);
	ADD_PARAMETER_CONTROLLER_DISPACTCHER(WireContactDynamicsSolverDampingScale);

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RenderMaterial),
		[](ThisClass* Wire) { Wire->SetRenderMaterial(Wire->RenderMaterial); });

	// Begin Winch.

#if 0
	Add Begin Winch Type here, and do our best to handle it.
	Alternatively, disable that setting in the Detail Panel during runtime.
#endif

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, PulledInLength),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetPulledInLength(Wire->OwnedBeginWinch.PulledInLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bMotorEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetMotorEnabled(Wire->OwnedBeginWinch.bMotorEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, TargetSpeed),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetTargetSpeed(Wire->OwnedBeginWinch.TargetSpeed); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, MotorForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetMotorForceRange(Wire->OwnedBeginWinch.MotorForceRange); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bBrakeEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetBrakeEnabled(Wire->OwnedBeginWinch.bBrakeEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedBeginWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, BrakeForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedBeginWinch.SetBrakeForceRange(Wire->OwnedBeginWinch.BrakeForceRange); });

	/// @todo Find ways to do attach/detach during runtime from the Details Panel.

#if 0
	Add End Winch Type here, and do our best to handle it.
	Alternatively, disable that setting in the Detail Panel during runtime.
#endif

	// End Winch.

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, PulledInLength),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetPulledInLength(Wire->OwnedEndWinch.PulledInLength); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bMotorEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetMotorEnabled(Wire->OwnedEndWinch.bMotorEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, TargetSpeed),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetTargetSpeed(Wire->OwnedEndWinch.TargetSpeed); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, MotorForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetMotorForceRange(Wire->OwnedEndWinch.MotorForceRange); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, bBrakeEnabled),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetBrakeEnabled(Wire->OwnedEndWinch.bBrakeEnabled); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, OwnedEndWinch),
		GET_MEMBER_NAME_CHECKED(FAGX_WireWinch, BrakeForceRange),
		[](ThisClass* Wire)
		{ Wire->OwnedEndWinch.SetBrakeForceRange(Wire->OwnedEndWinch.BrakeForceRange); });

	/// @todo Find ways to do attach/detach from winch during runtime from the Details Panel.

	// Routing.

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes),
		[](ThisClass* Wire) { AGX_WireComponent_helpers::SetLocalScope(*Wire); });
}

void UAGX_WireComponent::PreEditChange(FEditPropertyChain& PropertyAboutToChange)
{
	UObject::PreEditChange(PropertyAboutToChange);

	// Here I would like to detect if the change is about to give a routing node a new parent
	// and if so unsubscribe from the Transform Updated event on the old parent, but I don't
	// know how to find the index in the Route Nodes list of the routing node that is about to
	// be given a new parent. In the PostEditChangeChainProperty callback we get a
	// FPropertyChangedChainEvent instead of a FEditPropertyChain and the event knows the array
	// index because it has been created from an FPropertyHandle, which has access to
	// FPropertyNode, instead of from an FProperty. The FProperty doesn't seem to know the array
	// index.
}

void UAGX_WireComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	FEditPropertyChain::TDoubleLinkedListNode* const PropertyNode = Event.PropertyChain.GetHead();
	if (PropertyNode != nullptr)
	{
		const FName Member = PropertyNode->GetValue()->GetFName();
		if (DoesPropertyAffectVisuals(Member))
		{
			UpdateVisuals();
		}

		if (Member == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes))
		{
			// We have no way of knowing what the old parent was, or if the changed node was the
			// last to have it has its parent, so the best we can do is to synchronize all node
			// parents. We used to have an attempt at a more incremental approach here but that
			// didn't work out because of this. To find the old implementation search the Git
			// patch history for
			//
			//    Did a node get a new parent, meaning we must set up a new parent-moved
			//    callback?
			SynchronizeParentMovedCallbacks();
		}
	}

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the
	// owning Actor. That means that this object will be removed from the Actor and destroyed.
	// We want to apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif

void UAGX_WireComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		// Do not create a native AGX Dynamics object if GIsReconstructingBlueprintInstances is
		// set. That means that we're being created as part of a Blueprint Reconstruction and we
		// will soon be assigned the native that the reconstructed Wire Component had, if any.
		CreateNative();
		check(HasNative()); /// @todo Consider better error handling than check.

		MergeSplitProperties.OnBeginPlay(*this);
	}
}

void UAGX_WireComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateVisuals();
}

void UAGX_WireComponent::CreateMergeSplitProperties()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_WireComponent::CreateMergeSplitProperties was called "
				 "on Wire '%s' that does not have a Native AGX Dynamics object. Only call "
				 "this function "
				 "during play."),
			*GetName());
		return;
	}

	if (!MergeSplitProperties.HasNative())
	{
		MergeSplitProperties.CreateNative(*this);
	}
}

TStructOnScope<FActorComponentInstanceData> UAGX_WireComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_WireInstanceData>(this);
}

void UAGX_WireComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_WireComponent will inherit this one's Native Barrier, so don't wreck it.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);
		}
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
		WireParameterController.ClearBarrier();
	}
}

#if WITH_EDITOR
void UAGX_WireComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	UpdateVisuals();
}
#endif

void UAGX_WireComponent::OnRegister()
{
	Super::OnRegister();

	// During level load any Blueprint Instances will be created by running the Construction
	// Script. Any routing nodes created by that script will not be seen by Post Init Properties
	// and Post Load, so their Local Scope must be set here.
	AGX_WireComponent_helpers::SetLocalScope(*this);

#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
		FName NewName = MakeUniqueObjectName(
			SpriteComponent->GetOuter(), SpriteComponent->GetClass(), TEXT("WireIcon"));
		SpriteComponent->Rename(*NewName.ToString(), nullptr, REN_DontCreateRedirectors);
		SpriteComponent->SetSprite(
			LoadObject<UTexture2D>(nullptr, TEXT("/AGXUnreal/Editor/Icons/wire_64x64")));
		SpriteComponent->SetRelativeScale3D(FVector(2.0, 2.0, 2.0));
	}
#endif

	if (VisualCylinders == nullptr || VisualSpheres == nullptr)
		CreateVisuals();

	UpdateVisuals();
}

void UAGX_WireComponent::DestroyComponent(bool bPromoteChildren)
{
	if (VisualCylinders != nullptr)
		VisualCylinders->DestroyComponent();

	if (VisualSpheres != nullptr)
		VisualSpheres->DestroyComponent();

#if WITH_EDITOR
	if (MapLoadDelegateHandle.IsValid())
	{
		FEditorDelegates::MapChange.Remove(MapLoadDelegateHandle);
	}
	if (ObjectsReplacedDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::OnObjectsReplaced.Remove(ObjectsReplacedDelegateHandle);
	}
	for (TTuple<USceneComponent*, FParentDelegate>& Entry : DelegateHandles)
	{
		if (!Entry.Value.Parent.IsValid())
		{
			continue;
		}
		Entry.Value.Parent->TransformUpdated.Remove(Entry.Value.DelegateHandle);
	}
#endif

	Super::DestroyComponent(bPromoteChildren);
}

namespace AGX_WireComponent_helpers
{
	class FAGXWireNotifyBuffer : public FAGXNotifyListener
	{
	public:
		virtual ~FAGXWireNotifyBuffer() = default;

		virtual void OnMessage(const FString& InMessage, ELogVerbosity::Type Verbosity) override
		{
			if (Message != "")
			{
				Message += "\n";
			}
			Message += InMessage;
		}

	public:
		FString Message;
	};

	/**
	 * Given a location in one frame of reference, return the same world location relative to
	 * another frame of reference.
	 *
	 * @param SourceTransform The frame of reference in which LocalLocation is given.
	 * @param TargetTransform The frame of reference in which we want the same world location.
	 * @param LocalLocation The location in the source frame of reference.
	 * @return The location in the target frame of reference.
	 */
	FVector MoveLocationBetweenLocalTransforms(
		const FTransform& SourceTransform, const FTransform& TargetTransform,
		const FVector& LocalLocation)
	{
		// A.GetRelativeTransform(B) produces a transformation that goes from B to A. That is,
		// it tells us where A is in relation to B. Transforming the zero vector with that
		// transform will produce the vector pointing from B to A. Transforming any other vector
		// will produce the vector pointing from B to the tip of the first vector when placed
		// with its base at A. In our case we want the vector pointing from TargetTransform so
		// that should be our B, i.e. the parameter.
		FTransform SourceToTarget = SourceTransform.GetRelativeTransform(TargetTransform);
		return SourceToTarget.TransformPosition(LocalLocation);
	}

	std::tuple<FRigidBodyBarrier*, FVector> GetBodyAndLocalLocation(
		const FWireRoutingNode& RouteNode, const FTransform& WireTransform)
	{
		UAGX_RigidBodyComponent* BodyComponent = RouteNode.RigidBody.GetRigidBody();
		if (BodyComponent == nullptr)
		{
			return {nullptr, FVector::ZeroVector};
		}
		FRigidBodyBarrier* NativeBody = BodyComponent->GetOrCreateNative();
		check(NativeBody);
		const FVector LocalLocation =
			RouteNode.Frame.GetLocationRelativeTo(*BodyComponent, WireTransform);
		return {NativeBody, LocalLocation};
	}
}

namespace AGX_WireComponent_helpers
{
	void CreateNativeWireOwnedWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		check(Wire.GetOwnedWinch(Side) != nullptr);
		FAGX_WireWinch& Winch = *Wire.GetOwnedWinch(Side);
		FAGX_WireUtilities::ComputeSimulationPlacement(Wire, Winch);
		FWireWinchBarrier* Barrier = Winch.GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not create AGX Dynamics instance for wire-owned winch on Wire '%s' "
					 "in '%s'"),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()));
			return;
		}

		Wire.GetNative()->AddWinch(*Barrier);
	}

	void CreateNativeWireWinchOwnedWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		if (Side == EWireSide::None)
		{
			return;
		}

// Unreal Engine 5.2 require that the message passed to UE_LOG is constexpr.
// Not sure how to make the constexpr-ness of a string literal survive through a call to a lambda
// function, so falling back to a macro.
//
// Is there a better solution?
#if UE_VERSION_OLDER_THAN(5, 2, 0)

		// Message must contain four %s ordered as Wire name, Wire owner name, Winch name, Winch
		// owner name.
		auto LogError = [&Wire, Side](auto& Message)
		{
			const FComponentReference* Reference = Wire.GetWinchComponentReference(Side);
			const FString WinchName = Reference->ComponentProperty.ToString();
			const FString ActorName = GetLabelSafe(FAGX_ObjectUtilities::GetActor(*Reference));
			UE_LOG(
				LogAGX, Error, Message, *Wire.GetName(), *GetLabelSafe(Wire.GetOwner()), *WinchName,
				*ActorName);
		};

#else

#define LogError(Message)                                                                    \
	const FComponentReference* Reference = Wire.GetWinchComponentReference(Side);            \
	const FString WinchName = Reference->ComponentProperty.ToString();                       \
	const FString ActorName = GetLabelSafe(FAGX_ObjectUtilities::GetActor(*Reference));      \
	UE_LOG(                                                                                  \
		LogAGX, Error, Message, *Wire.GetName(), *GetLabelSafe(Wire.GetOwner()), *WinchName, \
		*ActorName);

#endif

		UAGX_WireWinchComponent* WinchComponent = Wire.GetWinchComponent(Side);
		if (WinchComponent == nullptr)
		{
			LogError(
				TEXT("Wire '%s' in '%s' did not find a Wire Winch named '%s' in '%s'. AGX Dynamics "
					 "instance will not be created."));
			return;
		}

		FWireWinchBarrier* Barrier = WinchComponent->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			LogError(
				TEXT("Wire '%s' in '%s' could not create AGX Dynamics instance for Wire Winch '%s' "
					 "in '%s'."));
			return;
		}

		Wire.GetNative()->AddWinch(*Barrier);
	}

	void CreateNativeWireBorrowedWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		if (Side == EWireSide::None)
		{
			return;
		}

		FAGX_WireWinch* Winch = Wire.GetBorrowedWinch(Side);
		if (Winch == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Wire Winch Owner Type on '%s' in '%s' has been set to Other but no Wire "
					 "Winch as been assigned to Borrowed Winch. No AGX Dynamics winch instance "
					 "will be created."),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()));
			return;
		}

		FWireWinchBarrier* Barrier = Winch->GetOrCreateNative();
		if (Barrier == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Could not create AGX Dynamics instance for a borrowed winch on '%s' in "
					 "'%s'."),
				*Wire.GetName(), *GetLabelSafe(Wire.GetOwner()));
			return;
		}

		Wire.GetNative()->AddWinch(*Barrier);
	}

	void CreateNativeWinch(UAGX_WireComponent& Wire, EWireSide Side)
	{
		EWireWinchOwnerType WinchType = Wire.GetWinchOwnerType(Side);
		switch (WinchType)
		{
			case EWireWinchOwnerType::Wire:
				CreateNativeWireOwnedWinch(Wire, Side);
				break;
			case EWireWinchOwnerType::WireWinch:
				CreateNativeWireWinchOwnedWinch(Wire, Side);
				break;
			case EWireWinchOwnerType::Other:
				CreateNativeWireBorrowedWinch(Wire, Side);
				break;
			case EWireWinchOwnerType::None:
				// Nothing to do here.
				break;
		}
	}
}

void UAGX_WireComponent::SetWireParameterController(
	const FAGX_WireParameterController& InWireParameterController)
{
	WireParameterController = InWireParameterController;
	// Make sure the Wire Parameter Controller is bound to this wire.
	if (HasNative())
	{
		WireParameterController.SetBarrier(NativeBarrier.GetParameterController());
		WireParameterController.WritePropertiesToNative();
	}
	else
	{
		WireParameterController.ClearBarrier();
	}
}

bool UAGX_WireComponent::SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial)
{
	UAGX_ShapeMaterial* ShapeMaterialOrig = ShapeMaterial;
	ShapeMaterial = InShapeMaterial;

	if (!HasNative())
	{
		// Not in play, we are done.
		return true;
	}

	// UpdateNativeMaterial is responsible for creating an instance of none exists and do the
	// asset/instance swap.
	if (!UpdateNativeMaterial())
	{
		// Something went wrong, restore original ShapeMaterial.
		ShapeMaterial = ShapeMaterialOrig;
		return false;
	}

	return true;
}

void UAGX_WireComponent::SetCanCollide(bool CanCollide)
{
	if (HasNative())
	{
		GetNative()->SetEnableCollisions(CanCollide);
	}

	bCanCollide = CanCollide;
}

bool UAGX_WireComponent::GetCanCollide() const
{
	if (HasNative())
	{
		return GetNative()->GetEnableCollisions();
	}

	return bCanCollide;
}

void UAGX_WireComponent::AddCollisionGroup(FName GroupName)
{
	if (GroupName.IsNone())
	{
		return;
	}

	if (CollisionGroups.Contains(GroupName))
		return;

	CollisionGroups.Add(GroupName);
	if (HasNative())
		NativeBarrier.AddCollisionGroup(GroupName);
}

void UAGX_WireComponent::RemoveCollisionGroupIfExists(FName GroupName)
{
	if (GroupName.IsNone())
		return;

	auto Index = CollisionGroups.IndexOfByKey(GroupName);
	if (Index == INDEX_NONE)
		return;

	CollisionGroups.RemoveAt(Index);
	if (HasNative())
		NativeBarrier.RemoveCollisionGroup(GroupName);
}

void UAGX_WireComponent::SetRenderMaterial(UMaterialInterface* Material)
{
	RenderMaterial = Material;

	if (VisualCylinders != nullptr)
		VisualCylinders->SetMaterial(0, RenderMaterial);

	if (VisualSpheres != nullptr)
		VisualSpheres->SetMaterial(0, RenderMaterial);
}

void UAGX_WireComponent::CreateNative()
{
	using namespace AGX_WireComponent_helpers;

	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	const float ResolutionPerUnitLength = 1.0f / MinSegmentLength;
	NativeBarrier.AllocateNative(Radius, ResolutionPerUnitLength);
	check(HasNative()); /// @todo Consider better error handling than 'check'.
	WireParameterController.SetBarrier(NativeBarrier.GetParameterController());

	if (!UpdateNativeMaterial())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UpdateNativeMaterial returned false in AGX_WireComponent. "
				 "Ensure the selected Shape Material is valid."));
	}

	NativeBarrier.SetLinearVelocityDamping(LinearVelocityDamping);
	NativeBarrier.SetEnableCollisions(bCanCollide);
	NativeBarrier.AddCollisionGroups(CollisionGroups);
	WireParameterController.WritePropertiesToNative();

	const FTransform LocalToWorld = GetComponentTransform();

	// Collection of error messages related to the wire setup/configuration that the user made.
	// These are presented as a dialog box at the end of the initialization process.
	// The intention is to add the equivalent edit-time checks and display in the wire's Details
	// Panel so the user can be informed before clicking Play.
	TArray<FString> ErrorMessages;

	if (HasBeginWinch())
	{
		AGX_WireComponent_helpers::CreateNativeWinch(*this, EWireSide::Begin);
	}

	AActor* const Owner = FAGX_ObjectUtilities::GetRootParentActor(GetOwner());
	// Create AGX Dynamics simulation nodes and initialize the wire.
	for (int32 I = 0; I < RouteNodes.Num(); ++I)
	{
		FWireRoutingNode& RouteNode = RouteNodes[I];
		FWireNodeBarrier NodeBarrier;

		check(RouteNode.Frame.Parent.LocalScope == Owner);
		check(RouteNode.RigidBody.LocalScope == Owner);

		switch (RouteNode.NodeType)
		{
			case EWireNodeType::Free:
			{
				const FVector WorldLocation = RouteNode.Frame.GetWorldLocation(*this);
				NodeBarrier.AllocateNativeFreeNode(WorldLocation);
				break;
			}
			case EWireNodeType::Eye:
			{
				FRigidBodyBarrier* Body;
				FVector Location;
				std::tie(Body, Location) = GetBodyAndLocalLocation(RouteNode, LocalToWorld);
				if (Body == nullptr)
				{
					ErrorMessages.Add(FString::Printf(
						TEXT("Wire node at index %d has invalid body. Creating Free Node "
							 "instead "
							 "of Eye Node."),
						I));
					const FVector WorldLocation = RouteNode.Frame.GetWorldLocation(*this);
					NodeBarrier.AllocateNativeFreeNode(WorldLocation);
					break;
				}
				NodeBarrier.AllocateNativeEyeNode(*Body, Location);
				break;
			}
			case EWireNodeType::BodyFixed:
			{
				FRigidBodyBarrier* Body;
				FVector Location;
				std::tie(Body, Location) = GetBodyAndLocalLocation(RouteNode, LocalToWorld);
				if (Body == nullptr)
				{
					ErrorMessages.Add(FString::Printf(
						TEXT("Wire node at index %d has invalid body. Creating Free Node "
							 "instead "
							 "for Body Fixed Node."),
						I));
					const FVector WorldLocation = RouteNode.Frame.GetWorldLocation(*this);
					NodeBarrier.AllocateNativeFreeNode(WorldLocation);
					break;
				}
				NodeBarrier.AllocateNativeBodyFixedNode(*Body, Location);
				break;
			}
			case EWireNodeType::Other:
				UE_LOG(
					LogAGX, Warning,
					TEXT("Found unexpected node type in Wire '%s', part of actor '%s', at "
						 "index %d. "
						 "Node ignored."),
					*GetName(), *GetLabelSafe(GetOwner()), I);
				break;
		}
		NativeBarrier.AddRouteNode(NodeBarrier);
	}

	if (HasEndWinch())
	{
		AGX_WireComponent_helpers::CreateNativeWinch(*this, EWireSide::End);
	}

	if (ErrorMessages.Num() > 0)
	{
		FString Message = FString::Printf(
			TEXT("Errors detected during initialization of wire '%s' in '%s':\n"), *GetName(),
			*GetLabelSafe(GetOwner()));
		for (const FString& Line : ErrorMessages)
		{
			Message += Line + '\n';
		}
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
	}

	{
		FAGXWireNotifyBuffer Messages;
		UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
		if (Simulation == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Wire '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
					 "returned "
					 "nullptr."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return;
		}

		Simulation->Add(*this);

		if (!IsInitialized())
		{
			const FString WireName = GetName();
			const FString OwnerName = GetLabelSafe(GetOwner());
			const FString Message = FString::Printf(
				TEXT("Invalid wire configuration for '%s' in '%s':\n%s"), *WireName, *OwnerName,
				*Messages.Message);

			FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		}
	}
}

void UAGX_WireComponent::CreateVisuals()
{
	VisualCylinders =
		NewObject<UInstancedStaticMeshComponent>(this, FName(TEXT("VisualCylinders")));
	VisualCylinders->SetCanEverAffectNavigation(false);
	VisualCylinders->RegisterComponent();
	VisualCylinders->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	static const TCHAR* CylinderAssetPath =
		TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualCylinder.SM_WireVisualCylinder'");
	VisualCylinders->SetStaticMesh(
		FAGX_ObjectUtilities::GetAssetFromPath<UStaticMesh>(CylinderAssetPath));
	VisualCylinders->SetMaterial(0, RenderMaterial);

	VisualSpheres = NewObject<UInstancedStaticMeshComponent>(this, FName(TEXT("VisualSpheres")));
	VisualSpheres->SetCanEverAffectNavigation(false);
	VisualSpheres->RegisterComponent();
	VisualSpheres->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	static const TCHAR* SphereAssetPath =
		TEXT("StaticMesh'/AGXUnreal/Wire/SM_WireVisualSphere.SM_WireVisualSphere'");
	VisualSpheres->SetStaticMesh(
		FAGX_ObjectUtilities::GetAssetFromPath<UStaticMesh>(SphereAssetPath));
	VisualSpheres->SetMaterial(0, RenderMaterial);
}

bool UAGX_WireComponent::UpdateNativeMaterial()
{
	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("UpdateNativeMaterial called on Wire '%s' but it does not have a "
				 "native AGX Dynamics representation."),
			*GetName());
		return false;
	}

	if (ShapeMaterial == nullptr)
	{
		if (HasNative())
		{
			GetNative()->ClearMaterial();
		}
		return true;
	}

	UWorld* World = GetWorld();
	UAGX_ShapeMaterial* MaterialInstance =
		static_cast<UAGX_ShapeMaterial*>(ShapeMaterial->GetOrCreateInstance(World));
	check(MaterialInstance);

	if (ShapeMaterial != MaterialInstance)
	{
		ShapeMaterial = MaterialInstance;
	}

	FShapeMaterialBarrier* MaterialBarrier =
		MaterialInstance->GetOrCreateShapeMaterialNative(World);
	check(MaterialBarrier);
	NativeBarrier.SetMaterial(*MaterialBarrier);

	return true;
}

#if WITH_EDITOR
bool UAGX_WireComponent::DoesPropertyAffectVisuals(const FName& MemberPropertyName) const
{
	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, VisualCylinders))
		return true;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, VisualSpheres))
		return true;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, RouteNodes))
		return true;

	if (MemberPropertyName == GET_MEMBER_NAME_CHECKED(UAGX_WireComponent, Radius))
		return true;

	return false;
}
#endif

TArray<FVector> UAGX_WireComponent::GetNodesForRendering() const
{
	TArray<FVector> NodeLocations;
	if (HasRenderNodes())
	{
		NodeLocations = GetRenderNodeLocations();
	}
	else
	{
		NodeLocations.Reserve(RouteNodes.Num());
		const FTransform& ComponentTransform = GetComponentTransform();
		for (const auto& Node : RouteNodes)
		{
			const FVector WorldLocation = Node.Frame.GetWorldLocation(*this);
			NodeLocations.Add(WorldLocation);
		}
	}

	return NodeLocations;
}

bool UAGX_WireComponent::ShouldRenderSelf() const
{
	return VisualCylinders != nullptr && VisualSpheres != nullptr && ShouldRender();
}

void UAGX_WireComponent::UpdateVisuals()
{
	if (!ShouldRenderSelf())
	{
		const bool bHasVisualCylinders =
			VisualCylinders != nullptr && VisualCylinders->GetInstanceCount() > 0;
		const bool bHasVisualSpheres =
			VisualSpheres != nullptr && VisualSpheres->GetInstanceCount() > 0;

		if (bHasVisualCylinders || bHasVisualSpheres)
			SetVisualsInstanceCount(0);

		return;
	}

	// Workaround, the RenderMaterial does not propagate properly in SetRenderMaterial() in
	// Blueprints, so we assign it here.
	if (VisualCylinders->GetMaterial(0) != RenderMaterial)
		VisualCylinders->SetMaterial(0, RenderMaterial);

	if (VisualSpheres->GetMaterial(0) != RenderMaterial)
		VisualSpheres->SetMaterial(0, RenderMaterial);

	TArray<FVector> NodeLocations = GetNodesForRendering();
	RenderSelf(NodeLocations);
}

void UAGX_WireComponent::RenderSelf(const TArray<FVector>& Points)
{
	if (Points.Num() <= 1)
		return;

	const int32 NumSegments = Points.Num() - 1;
	SetVisualsInstanceCount(NumSegments);

	VisualCylinders->UpdateComponentToWorld();
	VisualSpheres->UpdateComponentToWorld();

	const double ScaleXY = static_cast<double>(Radius) * 0.01 * 2.0;
	FTransform SphereTransform {FTransform::Identity};
	FTransform CylTransform {FTransform::Identity};
	SphereTransform.SetScale3D(FVector(ScaleXY, ScaleXY, ScaleXY));
	for (int i = 0; i < NumSegments; i++)
	{
		const FVector& StartLocation = Points[i];
		const FVector& EndLocation = Points[i + 1];
		const FVector MidPoint = (StartLocation + EndLocation) * 0.5;
		const FVector DeltaVec = EndLocation - StartLocation;
		const FRotator Rot = UKismetMathLibrary::MakeRotFromZ(DeltaVec);

		CylTransform.SetLocation(MidPoint);
		CylTransform.SetRotation(Rot.Quaternion());
		const auto Distance = (DeltaVec).Length();
		CylTransform.SetScale3D(FVector(ScaleXY, ScaleXY, Distance * 0.01));
		VisualCylinders->UpdateInstanceTransform(i, CylTransform, true, true);

		SphereTransform.SetLocation(StartLocation);
		VisualSpheres->UpdateInstanceTransform(i, SphereTransform, true, true);
	}
}

void UAGX_WireComponent::SetVisualsInstanceCount(int32 Num)
{
	Num = std::max(0, Num);

	auto SetNum = [](UInstancedStaticMeshComponent& C, int32 N)
	{
		while (C.GetInstanceCount() < N)
		{
			C.AddInstance(FTransform());
		}
		while (C.GetInstanceCount() > N)
		{
			C.RemoveInstance(C.GetInstanceCount() - 1);
		}
	};

	if (VisualCylinders != nullptr)
		SetNum(*VisualCylinders, Num);

	if (VisualSpheres != nullptr)
		SetNum(*VisualSpheres, Num);
}

#if WITH_EDITOR

void UAGX_WireComponent::SynchronizeParentMovedCallbacks()
{
	// We currently have no reliable way to detect when we should no longer be tracking
	// a parent. This is a cleanup-pass that should not be necessary but is until we figure
	// out how to reliably remove our callback from the parent's TransformUpdated event, and
	// remove an entry from the Delegate Handles list when the parent no longer exists.
	//
	// This implementation takes a nuke-all rebuild-all approach. We used to have an incremental
	// implementation that was a bit more complicated. If this synchronization becomes a
	// performance problem then the old incremental implementation can be found by searching the
	// Git patch history for the string
	//
	//   Find parents that aren't a parent to any routing node anymore, and parents that has
	//
	// or build something new.

	// Remove all old callbacks.
	for (TTuple<USceneComponent*, FParentDelegate>& ParentHandle : DelegateHandles)
	{
		if (!ParentHandle.Value.Parent.IsValid())
		{
			continue;
		}
		ParentHandle.Value.Parent->TransformUpdated.Remove(ParentHandle.Value.DelegateHandle);
	}
	DelegateHandles.Empty(RouteNodes.Num());

	// Add callbacks for the current parents.
	for (FWireRoutingNode& RouteNode : RouteNodes)
	{
		USceneComponent* Parent = RouteNode.Frame.Parent.GetComponent<USceneComponent>();
		if (!IsValid(Parent) || DelegateHandles.Contains(Parent))
		{
			continue;
		}
		DelegateHandles.Add(
			Parent, {Parent, Parent->TransformUpdated.AddUObject(
								 this, &UAGX_WireComponent::OnRouteNodeParentMoved)});
	}
}

void UAGX_WireComponent::SynchronizeRendering()
{
	MarkVisualsDirty();
	SynchronizeParentMovedCallbacks();
}

#endif

#undef LOCTEXT_NAMESPACE
