// Copyright 2022, Algoryx Simulation AB.

#include "Shapes/AGX_TrimeshShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"

UAGX_TrimeshShapeComponent::UAGX_TrimeshShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MeshSourceLocation = TSL_PARENT_STATIC_MESH_COMPONENT;
	bOverrideMeshSourceLodIndex = true;
	MeshSourceLodIndex = 0;
}

FShapeBarrier* UAGX_TrimeshShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_TrimeshShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_TrimeshShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_TrimeshShapeComponent::GetNativeBarrier() const
{
	return &NativeBarrier;
}

FShapeBarrier* UAGX_TrimeshShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FTrimeshShapeBarrier* UAGX_TrimeshShapeComponent::GetNativeTrimesh()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_TrimeshShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeLocalTransform(NativeBarrier);
}

void UAGX_TrimeshShapeComponent::CopyFrom(const FTrimeshShapeBarrier& Barrier)
{
	/// \todo Where is all the triangle data copied?
	Super::CopyFrom(Barrier);
}

void UAGX_TrimeshShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& /*OutMeshData*/)
{
	// @todo Visualize the mesh in the Viewport. Currently the triangle data are only passed to
	// AGX Dynamics and can be used for collisions, but nothing is rendered.
}

#if WITH_EDITOR

bool UAGX_TrimeshShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	return Super::DoesPropertyAffectVisualMesh(PropertyName, MemberPropertyName) ||
		   MemberPropertyName ==
			   GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceLocation) ||
		   MemberPropertyName ==
			   GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceAsset);
}

void UAGX_TrimeshShapeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName().IsEqual(
			GET_MEMBER_NAME_CHECKED(UAGX_ShapeComponent, bIsSensor)))
	{
		if (UMeshComponent* Mesh = FindMeshComponent(MeshSourceLocation))
		{
			bIsSensor ? ApplySensorMaterial(*Mesh) : RemoveSensorMaterial(*Mesh);
		}
	}
	else if (PropertyChangedEvent.GetPropertyName().IsEqual(
				 GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceLocation)))
	{
		if (UMeshComponent* Mesh = FindMeshComponent(MeshSourceLocation))
		{
			bIsSensor ? ApplySensorMaterial(*Mesh) : RemoveSensorMaterial(*Mesh);
		}
	}
}

void UAGX_TrimeshShapeComponent::PreEditChange(FProperty* PropertyThatWillChange)
{
	Super::PreEditChange(PropertyThatWillChange);

	if (PropertyThatWillChange->GetName().Equals(
			GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceLocation).ToString()))
	{
		if (!bIsSensor)
		{
			return;
		}

		// Clean up sensor material of the old source MeshComponent if applied.
		if (UMeshComponent* Mesh = FindMeshComponent(MeshSourceLocation))
		{
			RemoveSensorMaterial(*Mesh);
		}
	}
}

bool UAGX_TrimeshShapeComponent::CanEditChange(
#if UE_VERSION_OLDER_THAN(4, 25, 0)
	const UProperty* InProperty
#else
	const FProperty* InProperty
#endif
	) const
{
	if (InProperty->GetFName() ==
		GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceAsset))
	{
		return MeshSourceLocation == TSL_STATIC_MESH_ASSET;
	}
	else
	{
		return Super::CanEditChange(InProperty);
	}
}

#endif

void UAGX_TrimeshShapeComponent::CreateNative()
{
	check(!HasNative());

	TArray<FVector> Vertices;
	TArray<FTriIndices> Indices;
	if (GetStaticMeshCollisionData(Vertices, Indices))
	{
		NativeBarrier.AllocateNative(Vertices, Indices, /*bClockwise*/ false, GetName());
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("TrimeshShapeComponent '%s' does not have a StaticMeshComponent to read triangle "
				 "data from. The generated native shape will be invalid."),
			*GetName());
		NativeBarrier.AllocateNative({}, {}, /*bClockwise*/ false, GetName());
	}

	UpdateNativeProperties();
}

void UAGX_TrimeshShapeComponent::ReleaseNative()
{
	check(HasNative());
	NativeBarrier.ReleaseNative();
}

UMeshComponent* UAGX_TrimeshShapeComponent::FindMeshComponent(
	TEnumAsByte<EAGX_StaticMeshSourceLocation> InMeshSourceLocation) const
{
	if (InMeshSourceLocation == TSL_CHILD_STATIC_MESH_COMPONENT)
	{
		TArray<USceneComponent*> Children;
		GetChildrenComponents(/*bIncludeAllDescendants*/ false, Children);

		for (int32 ChildIndex = 0; ChildIndex < Children.Num(); ++ChildIndex)
		{
			if (USceneComponent* Child = Children[ChildIndex])
			{
				if (Child->IsA(UStaticMeshComponent::StaticClass()))
				{
					return Cast<UStaticMeshComponent>(Child);
				}
			}
		}
	}
	else if (InMeshSourceLocation == TSL_PARENT_STATIC_MESH_COMPONENT)
	{
		TArray<USceneComponent*> Ancestors;
		GetParentComponents(Ancestors);

		for (int32 AncestorIndex = 0; AncestorIndex < Ancestors.Num(); ++AncestorIndex)
		{
			if (USceneComponent* Ancestor = Ancestors[AncestorIndex])
			{
				if (Ancestor->IsA(UStaticMeshComponent::StaticClass()))
				{
					return Cast<UStaticMeshComponent>(Ancestor);
				}
			}
		}
	}

	return nullptr;
}

bool UAGX_TrimeshShapeComponent::GetStaticMeshCollisionData(
	TArray<FVector>& OutVertices, TArray<FTriIndices>& OutIndices) const
{
	FAGX_MeshWithTransform Mesh;

	switch (MeshSourceLocation)
	{
		case EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT:
			Mesh = AGX_MeshUtilities::FindFirstChildMesh(*this);
			break;
		case EAGX_StaticMeshSourceLocation::TSL_PARENT_STATIC_MESH_COMPONENT:
			Mesh = AGX_MeshUtilities::FindFirstParentMesh(*this);
			break;
		case EAGX_StaticMeshSourceLocation::TSL_STATIC_MESH_ASSET:
			if (MeshSourceAsset != nullptr)
			{
				Mesh = FAGX_MeshWithTransform(MeshSourceAsset, GetComponentTransform());
			}
			break;
	}

	if (!Mesh.IsValid())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetStaticMeshCollisionData failed in '%s'. Unable to find static Mesh."),
			*GetName());
		return false;
	}

	const FTransform ComponentTransformNoScale =
		FTransform(GetComponentRotation(), GetComponentLocation());
	const uint32* LodIndex = bOverrideMeshSourceLodIndex ? &MeshSourceLodIndex : nullptr;

	return AGX_MeshUtilities::GetStaticMeshCollisionData(
		Mesh, ComponentTransformNoScale, OutVertices, OutIndices, LodIndex);
}
