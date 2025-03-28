// Copyright 2024, Algoryx Simulation AB.

#include "Shapes/AGX_SphereShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "Components/StaticMeshComponent.h"
#include "Import/AGX_ImportContext.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "PhysicsEngine/AggregateGeom.h"
#include "PhysicsEngine/BodySetup.h"

// Standard library includes.
#include <algorithm>

UAGX_SphereShapeComponent::UAGX_SphereShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Radius = 50.0f;
}

void UAGX_SphereShapeComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}

	Radius = InRadius;
	UpdateVisualMesh();
}

float UAGX_SphereShapeComponent::GetRadius() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRadius();
	}

	return Radius;
}

FShapeBarrier* UAGX_SphereShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_SphereShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_SphereShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_SphereShapeComponent::GetNativeBarrier() const
{
	return &NativeBarrier;
}

// Called by UAGX_ShapeComponent::BeginPlay.
FShapeBarrier* UAGX_SphereShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FSphereShapeBarrier* UAGX_SphereShapeComponent::GetNativeSphere()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_SphereShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetRadius(Radius * GetComponentScale().X);
}

void UAGX_SphereShapeComponent::CopyFrom(
	const FShapeBarrier& ShapeBarrier, FAGX_ImportContext* Context)
{
	Super::CopyFrom(ShapeBarrier, Context);
	const auto Barrier = static_cast<const FSphereShapeBarrier*>(&ShapeBarrier);
	Radius = Barrier->GetRadius();
}

void UAGX_SphereShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	AGX_MeshUtilities::MakeSphere(
		OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, OutMeshData.TexCoords,
		Radius, 32);
}

bool UAGX_SphereShapeComponent::SupportsShapeBodySetup()
{
	return true;
}

void UAGX_SphereShapeComponent::UpdateBodySetup()
{
	CreateShapeBodySetupIfNeeded();
	check(ShapeBodySetup->AggGeom.SphereElems.Num() == 1);
	ShapeBodySetup->AggGeom.SphereElems[0].Radius = std::max(UE_KINDA_SMALL_NUMBER, Radius);
}

void UAGX_SphereShapeComponent::AddShapeBodySetupGeometry()
{
	if (ShapeBodySetup != nullptr)
		ShapeBodySetup->AggGeom.SphereElems.Add(FKSphereElem());
}

#if WITH_EDITOR
bool UAGX_SphereShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	// Note: radius is intentionally ignored here since the SetRadius function is
	// responsible to call the UpdateVisualMesh. This is done since calling e.g.
	// SetRadius from a Blueprint will NOT trigger the PostEditChangeProperty where the
	// UpdateVisualMesh is usually called from.
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName);
}
#endif

void UAGX_SphereShapeComponent::CreateNative()
{
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_SphereShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}

#if WITH_EDITOR

void UAGX_SphereShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// Cannot use the UAGX_ShapeComponent Property Dispatcher because there are name collisions for
	// Shape-specific UProperty names, for example Radius is in both Sphere and Cylinder.
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_SphereShapeComponent, Radius),
		[](ThisClass* This) { This->SetRadius(This->Radius); });
}

void UAGX_SphereShapeComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif
