// Copyright 2023, Algoryx Simulation AB.

#include "Shapes/AGX_CylinderShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_ShapeUtilities.h"

// Unreal Engine includes.
#include "Engine/StaticMeshActor.h"

UAGX_CylinderShapeComponent::UAGX_CylinderShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	Height = 100.0f;
	Radius = 50.0f;
}

void UAGX_CylinderShapeComponent::SetRadius(float InRadius)
{
	if (HasNative())
	{
		NativeBarrier.SetRadius(InRadius);
	}

	Radius = InRadius;
	UpdateVisualMesh();
}

float UAGX_CylinderShapeComponent::GetRadius() const
{
	if (HasNative())
	{
		return NativeBarrier.GetRadius();
	}

	return Radius;
}

void UAGX_CylinderShapeComponent::SetHeight(float InHeight)
{
	if (HasNative())
	{
		NativeBarrier.SetHeight(InHeight);
	}

	Height = InHeight;
	UpdateVisualMesh();
}

float UAGX_CylinderShapeComponent::GetHeight() const
{
	if (HasNative())
	{
		return NativeBarrier.GetHeight();
	}

	return Height;
}

void UAGX_CylinderShapeComponent::SetPulley(bool bInPulley)
{
	bPulley = bInPulley;
	if (HasNative())
	{
		NativeBarrier.SetPulleyProperty(bPulley);
	}
}

void UAGX_CylinderShapeComponent::SetGypsy(bool bInGypsy)
{
	bGypsy = bInGypsy;
	if (HasNative())
	{
		NativeBarrier.SetGypsyProperty(bGypsy);
	}
}

UAGX_CylinderShapeComponent* UAGX_CylinderShapeComponent::CreateFromMeshActors(
	AActor* Parent, TArray<AStaticMeshActor*> InMeshes)
{
	if (Parent == nullptr)
	{
		return nullptr;
	}

	TArray<FAGX_MeshWithTransform> Meshes = AGX_MeshUtilities::ToMeshWithTransformArray(InMeshes);

	UAGX_CylinderShapeComponent* Cylinder = NewObject<UAGX_CylinderShapeComponent>(
		Parent, UAGX_CylinderShapeComponent::StaticClass(), "AGX_CylinderShape", RF_Transient);
	const bool Result = Cylinder->AutoFit(Meshes, Parent->GetWorld(), Cylinder->GetName());
	if (!Result)
	{
		// Logging done in AutoFit.
		Cylinder->DestroyComponent();
		return nullptr;
	}

	Parent->AddInstanceComponent(Cylinder);
	Cylinder->RegisterComponent();
	return Cylinder;
}

FShapeBarrier* UAGX_CylinderShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CylinderShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_CylinderShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_CylinderShapeComponent::GetNativeBarrier() const
{
	return &NativeBarrier;
}

FShapeBarrier* UAGX_CylinderShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

bool UAGX_CylinderShapeComponent::AutoFitFromVertices(const TArray<FVector>& Vertices)
{
	float RadiusBounding;
	float HeightBounding;
	FTransform TransformBounding;
	if (!FAGX_ShapeUtilities::ComputeOrientedCylinder(
			Vertices, RadiusBounding, HeightBounding, TransformBounding))
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Auto-fit on '%s' failed. Could not compute oriented cylinder with given "
				 "vertices."),
			*GetName());
		return false;
	}

	SetWorldTransform(TransformBounding);
	SetRadius(RadiusBounding);
	SetHeight(HeightBounding);
	return true;
}

FCylinderShapeBarrier* UAGX_CylinderShapeComponent::GetNativeCylinder()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_CylinderShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);

	NativeBarrier.SetHeight(Height * GetComponentScale().Y);
	NativeBarrier.SetRadius(Radius * GetComponentScale().X);

	NativeBarrier.SetPulleyProperty(bPulley);
	NativeBarrier.SetGypsyProperty(bGypsy);
}

void UAGX_CylinderShapeComponent::CopyFrom(
	const FCylinderShapeBarrier& Barrier, bool ForceOverwriteInstances)
{
	Super::CopyFrom(Barrier, ForceOverwriteInstances);
	AGX_COPY_PROPERTY_FROM(Height, Barrier.GetHeight(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(Radius, Barrier.GetRadius(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(bPulley, Barrier.GetPulleyProperty(), *this, ForceOverwriteInstances)
	AGX_COPY_PROPERTY_FROM(bGypsy, Barrier.GetGypsyProperty(), *this, ForceOverwriteInstances)
}

void UAGX_CylinderShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	const uint32 NumCircleSegments = 32;
	const uint32 NumHeightSegments = 1;

	AGX_MeshUtilities::MakeCylinder(
		OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices, OutMeshData.TexCoords,
		AGX_MeshUtilities::CylinderConstructionData(
			Radius, Height, NumCircleSegments, NumHeightSegments));
}

#if WITH_EDITOR

bool UAGX_CylinderShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	// Note: radius and height are intentionally ignored here since the SetRadius / SetHeight
	// functions are responsible to call the UpdateVisualMesh. This is done since calling e.g.
	// SetRadius from a Blueprint will NOT trigger the PostEditChangeProperty where the
	// UpdateVisualMesh is usually called from.
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName);
}

#endif

void UAGX_CylinderShapeComponent::CreateNative()
{
	check(!HasNative());
	NativeBarrier.AllocateNative();
	UpdateNativeProperties();
}

void UAGX_CylinderShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}

#if WITH_EDITOR

void UAGX_CylinderShapeComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_CylinderShapeComponent::InitPropertyDispatcher()
{
	// Cannot use the base class Property Dispatcher because there are name collisions for UProperty
	// names, for example Radius is in both Sphere and Cylinder.

	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, Radius),
		[](ThisClass* This) { This->SetRadius(This->Radius); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, Height),
		[](ThisClass* This) { This->SetHeight(This->Height); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, bPulley),
		[](ThisClass* This) { This->SetPulley(This->bPulley); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CylinderShapeComponent, bGypsy),
		[](ThisClass* This) { This->SetGypsy(This->bGypsy); });
}

void UAGX_CylinderShapeComponent::PostEditChangeChainProperty(
	struct FPropertyChangedChainEvent& Event)
{
	// Trigger Cylinder callbacks. Shape callbacks are handled by the base class through a separate
	// Property Dispatcher instance.
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}
#endif
