// Copyright 2023, Algoryx Simulation AB.

#include "Shapes/AGX_CapsuleShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_ShapeUtilities.h"

// Unreal Engine includes.
#include "Engine/StaticMeshActor.h"

UAGX_CapsuleShapeComponent::UAGX_CapsuleShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Height = 100.0f;
	Radius = 50.0f;
}

void UAGX_CapsuleShapeComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}

	Radius = InRadius;
	UpdateVisualMesh();
}

float UAGX_CapsuleShapeComponent::GetRadius() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRadius();
	}

	return Radius;
}

UAGX_CapsuleShapeComponent* UAGX_CapsuleShapeComponent::CreateFromMeshActors(
	AActor* Parent, TArray<AStaticMeshActor*> InMeshes)
{
	if (Parent == nullptr)
	{
		return nullptr;
	}

	TArray<FAGX_MeshWithTransform> Meshes = AGX_MeshUtilities::ToMeshWithTransformArray(InMeshes);

	UAGX_CapsuleShapeComponent* Capsule = NewObject<UAGX_CapsuleShapeComponent>(
		Parent, UAGX_CapsuleShapeComponent::StaticClass(), "AGX_CapsuleShape", RF_Transient);
	const bool Result = Capsule->AutoFit(Meshes, Parent->GetWorld(), Capsule->GetName());
	if (!Result)
	{
		// Logging done in AutoFit.
		Capsule->DestroyComponent();
		return nullptr;
	}

	Parent->AddInstanceComponent(Capsule);
	Capsule->RegisterComponent();
	return Capsule;
}

void UAGX_CapsuleShapeComponent::SetHeight(float InHeight)
{
	if (HasNative())
	{
		NativeBarrier.SetHeight(InHeight);
	}

	Height = InHeight;
	UpdateVisualMesh();
}

float UAGX_CapsuleShapeComponent::GetHeight() const
{
	if (HasNative())
	{
		return NativeBarrier.GetHeight();
	}

	return Height;
}

FShapeBarrier* UAGX_CapsuleShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CapsuleShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_CapsuleShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CapsuleShapeComponent::GetNativeBarrier() const
{
	return &NativeBarrier;
}

FShapeBarrier* UAGX_CapsuleShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

bool UAGX_CapsuleShapeComponent::AutoFitFromVertices(const TArray<FVector>& Vertices)
{
	float RadiusBounding;
	float HeightBounding;
	FTransform TransformBounding;
	if (!FAGX_ShapeUtilities::ComputeOrientedCapsule(
			Vertices, RadiusBounding, HeightBounding, TransformBounding))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Auto-fit on '%s' failed. Could not compute oriented capsule with given "
				 "vertices."),
			*GetName());
		return false;
	}

	SetWorldTransform(TransformBounding);
	SetRadius(RadiusBounding);
	SetHeight(HeightBounding);
	return true;
}

FCapsuleShapeBarrier* UAGX_CapsuleShapeComponent::GetNativeCapsule()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_CapsuleShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetHeight(Height * GetComponentScale().Y);
	NativeBarrier.SetRadius(Radius * GetComponentScale().X);
}

void UAGX_CapsuleShapeComponent::CopyFrom(
	const FCapsuleShapeBarrier& Barrier, bool ForceOverwriteInstances)
{
	Super::CopyFrom(Barrier, ForceOverwriteInstances);
	AGX_COPY_PROPERTY_FROM(Height, Barrier.GetHeight(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(Radius, Barrier.GetRadius(), *this, ForceOverwriteInstances)
}

void UAGX_CapsuleShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	const uint32 NumCircleSegments = 32;
	const uint32 NumHeightSegments = 1;

	AGX_MeshUtilities::MakeCapsule(
		OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, OutMeshData.TexCoords,
		AGX_MeshUtilities::CapsuleConstructionData(
			Radius, Height, NumCircleSegments, NumHeightSegments));
}

#if WITH_EDITOR

bool UAGX_CapsuleShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	// Note: radius and height are intentionally ignored here since the SetRadius / SetHeight
	// functions are responsible to call the UpdateVisualMesh. This is done since calling e.g.
	// SetRadius from a Blueprint will NOT trigger the PostEditChangeProperty where the
	// UpdateVisualMesh is usually called from.
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName);
}

#endif

void UAGX_CapsuleShapeComponent::CreateNative()
{
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_CapsuleShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}

#if WITH_EDITOR

void UAGX_CapsuleShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();

	// Cannot use the UAGX_ShapeComponent Property Dispatcher because there are name collisions for
	// Shape-specific UProperty names, for example Radius is in both Sphere and Cylinder.
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher = FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CapsuleShapeComponent, Radius),
		[](ThisClass* This) { This->SetRadius(This->Radius); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CapsuleShapeComponent, Height),
		[](ThisClass* This) { This->SetHeight(This->Height); });
}

void UAGX_CapsuleShapeComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

#endif
