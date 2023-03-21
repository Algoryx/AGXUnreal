// Copyright 2023, Algoryx Simulation AB.

#include "Shapes/AGX_BoxShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_ShapeUtilities.h"

// Unreal Engine includes.
#include "Engine/StaticMeshActor.h"

UAGX_BoxShapeComponent::UAGX_BoxShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	HalfExtent = FVector(50.0f, 50.0f, 50.0f);
}

void UAGX_BoxShapeComponent::SetHalfExtent(const FVector& InHalfExtent)
{
	if (HasNative())
	{
		NativeBarrier.SetHalfExtents(InHalfExtent);
	}

	HalfExtent = InHalfExtent;
	UpdateVisualMesh();
}

FVector UAGX_BoxShapeComponent::GetHalfExtent() const
{
	if (HasNative())
	{
		return NativeBarrier.GetHalfExtents();
	}

	return HalfExtent;
}

UAGX_BoxShapeComponent* UAGX_BoxShapeComponent::CreateFromMeshActors(
	AActor* Parent, TArray<AStaticMeshActor*> InMeshes)
{
	if (Parent == nullptr)
	{
		return nullptr;
	}

	TArray<FAGX_MeshWithTransform> Meshes = AGX_MeshUtilities::ToMeshWithTransformArray(InMeshes);

	UAGX_BoxShapeComponent* Box = NewObject<UAGX_BoxShapeComponent>(
		Parent, UAGX_BoxShapeComponent::StaticClass(), "AGX_BoxShape", RF_Transient);
	const bool Result = Box->AutoFit(Meshes, Parent->GetWorld(), Box->GetName());
	if (!Result)
	{
		// Logging done in AutoFit.
		Box->DestroyComponent();
		return nullptr;
	}

	Parent->AddInstanceComponent(Box);
	Box->RegisterComponent();
	return Box;
}

FShapeBarrier* UAGX_BoxShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_BoxShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_BoxShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_BoxShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_BoxShapeComponent::GetNativeBarrier() const
{
	return &NativeBarrier;
}

FBoxShapeBarrier* UAGX_BoxShapeComponent::GetNativeBox()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_BoxShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetHalfExtents(HalfExtent * GetComponentScale());
}

bool UAGX_BoxShapeComponent::AutoFitFromVertices(const TArray<FVector>& Vertices)
{
	FVector HalfExtentsBounding;
	FTransform TransformBounding;
	if (!FAGX_ShapeUtilities::ComputeOrientedBox(Vertices, HalfExtentsBounding, TransformBounding))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Auto-fit on '%s' failed. Could not compute oriented box with given vertices."),
			*GetName());
		return false;
	}

	SetWorldTransform(TransformBounding);
	SetHalfExtent(HalfExtentsBounding);
	return true;
}

void UAGX_BoxShapeComponent::CopyFrom(const FBoxShapeBarrier& Barrier, bool ForceOverwriteInstances)
{
	Super::CopyFrom(Barrier, ForceOverwriteInstances);
	AGX_COPY_PROPERTY_FROM(HalfExtent, Barrier.GetHalfExtents(), *this, ForceOverwriteInstances)
}

void UAGX_BoxShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	AGX_MeshUtilities::MakeCube(
		OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, OutMeshData.TexCoords,
		ToMeshVector(HalfExtent));
}

#if WITH_EDITOR

bool UAGX_BoxShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	// Note: halfExtent is intentionally ignored here since the SetHalfExtent function is
	// responsible to call the UpdateVisualMesh. This is done since calling e.g.
	// SetHalfExtent from a Blueprint will NOT trigger the PostEditChangeProperty where the
	// UpdateVisualMesh is usually called from.
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName);
}

#endif

void UAGX_BoxShapeComponent::CreateNative()
{
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

#if WITH_EDITOR

void UAGX_BoxShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_BoxShapeComponent::InitPropertyDispatcher()
{
	// Cannot use the UAGX_ShapeComponent Property Dispatcher because there are name collisions for
	// Shape-specific UProperty names, for example Radius is in both Sphere and Cylinder.
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_BoxShapeComponent, HalfExtent),
		[](ThisClass* This) { This->SetHalfExtent(This->HalfExtent); });
}

void UAGX_BoxShapeComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif

void UAGX_BoxShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}
