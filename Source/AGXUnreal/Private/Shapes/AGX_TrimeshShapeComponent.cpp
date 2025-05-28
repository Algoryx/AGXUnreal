// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/AGX_TrimeshShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_MeshWithTransform.h"
#include "Import/AGX_ImportContext.h"
#include "Import/AGX_ImportSettings.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Utilities/AGX_StringUtilities.h"

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

namespace TrimshShapeComponent_helpers
{
	UStaticMesh* GetOrCreateStaticMesh(
		const FTrimeshShapeBarrier& Barrier, UMaterialInterface* Material,
		FAGX_ImportContext& Context)
	{
		AGX_CHECK(Context.CollisionStaticMeshes != nullptr);
		if (auto Existing = Context.CollisionStaticMeshes->FindRef(Barrier.GetGuid()))
			return Existing;

		TArray<FVector3f> Vertices;
		const auto VerticesAGX = Barrier.GetVertexPositions();
		Vertices.Reserve(VerticesAGX.Num());
		for (const FVector& Position : VerticesAGX)
		{
			Vertices.Add(FVector3f(Position));
		}

		const TArray<uint32> Indices = Barrier.GetVertexIndices();

		// One for each triangle, so we need to generate 3 unreal normals per agx normal.
		const auto NormalsAGX = Barrier.GetTriangleNormals();
		TArray<FVector3f> Normals;
		Normals.Reserve(NormalsAGX.Num() * 3);
		for (const FVector& Normal : NormalsAGX)
		{
			const auto N = FVector3f(Normal);
			for (int i = 0; i < 3; i++)
				Normals.Add(N);
		}

		const int32 NumVertices = Barrier.GetNumPositions();

		// Trimeshes have no UV information, so we set them to zero.
		TArray<FVector2D> UVs;
		UVs.SetNumZeroed(NumVertices);

		// Trimeshes have no tangents information, so we set them to zero.
		TArray<FVector3f> Tangents;
		Tangents.SetNumZeroed(Vertices.Num());

		const FString SourceName = Barrier.GetSourceName();
		const FString WantedName =
			SourceName.IsEmpty()
				? FString::Printf(TEXT("SM_CollisionMesh_%s"), *Barrier.GetGuid().ToString())
				: FString::Printf(TEXT("SM_%s"), *SourceName);

		const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			Context.Outer, WantedName, UStaticMesh::StaticClass());

#if WITH_EDITOR
		UStaticMesh* Mesh = AGX_MeshUtilities::CreateStaticMeshNoBuild(
			Vertices, Indices, Normals, UVs, Tangents, Name, *Context.Outer, Material);
#else
		UStaticMesh* Mesh = AGX_MeshUtilities::CreateStaticMesh(
			Vertices, Indices, Normals, UVs, Tangents, Name, *Context.Outer, Material);
#endif // WITH_EDITOR

		if (Mesh != nullptr)
			Context.CollisionStaticMeshes->Add(Barrier.GetGuid(), Mesh);

		return Mesh;
	}

	UStaticMeshComponent* CreateStaticMeshComponent(
		const FTrimeshShapeBarrier& Barrier, AActor& Owner, UMaterialInterface* Material,
		FAGX_ImportContext& Context)
	{
		AGX_CHECK(Material != nullptr);

		UStaticMesh* StaticMesh = GetOrCreateStaticMesh(Barrier, Material, Context);
		AGX_CHECK(StaticMesh != nullptr);
		if (StaticMesh == nullptr)
			return nullptr;

		FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(*StaticMesh, Context.SessionGuid);

		const FString ComponentName = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			&Owner, FString::Printf(TEXT("CollisionMesh_%s"), *Barrier.GetGuid().ToString()),
			nullptr);
		UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(&Owner, *ComponentName);
		FAGX_ImportRuntimeUtilities::OnComponentCreated(*Component, Owner, Context.SessionGuid);

		Component->SetMaterial(0, Material);
		Component->SetStaticMesh(StaticMesh);

		// The reason we let GetEnableCollisions determine whether or not the Collision Static Mesh
		// should be visible or not has to do with the behavior of agxViewer which we want to mimic.
		// If a shape in a agxCollide::Geometry which has canCollide == false is written to a AGX
		// archive and then read by agxViewer, the shape will not be visible (unless it has
		// RenderData).
		const bool Visible =
			Barrier.GetEnableCollisions() && Barrier.GetEnabled() && !Barrier.HasRenderData();
		Component->SetVisibility(Visible);

		Context.CollisionStaticMeshCom->Add(Barrier.GetShapeGuid(), Component);

		return Component;
	}

	UStaticMeshComponent* GetRenderDataMesh(const UAGX_TrimeshShapeComponent& Trimesh)
	{
		const TArray<UStaticMeshComponent*> Children =
			FAGX_ObjectUtilities::GetChildrenOfType<UStaticMeshComponent>(Trimesh, true);

		return Children.Num() > 0 ? Children[0] : nullptr;
	}
}

void UAGX_TrimeshShapeComponent::CopyFrom(
	const FShapeBarrier& ShapeBarrier, FAGX_ImportContext* Context)
{
	using namespace TrimshShapeComponent_helpers;

	Super::CopyFrom(ShapeBarrier, Context);
	if (Context == nullptr || Context->CollisionStaticMeshes == nullptr ||
		Context->Outer == nullptr)
		return; // We are done.

	if (Context->Settings != nullptr && Context->Settings->bIgnoreDisabledTrimeshes &&
		!ShapeBarrier.GetEnableCollisions())
	{
		return; // We should not create a collision mesh.
	}

	// At this point, there might exists a StaticMeshComponent as a child to this Trimesh
	// representing the RenderData (created in Super::CopyFrom). We want to create another
	// StaticMeshComponent for representing the collision shape and make that parent to the
	// RenderData StaticMeshComponent. I.e. we want a TrimeshComponent -> StaticMeshComponent
	// (RenderData) -> StaticMeshComponent (collision) hierarchy.
	UStaticMeshComponent* RenderMeshCom = GetRenderDataMesh(*this);

	UMaterialInterface* Material = RenderMeshCom != nullptr
									   ? RenderMeshCom->GetMaterial(0)
									   : AGX_MeshUtilities::GetDefaultRenderMaterial(false);

	UStaticMeshComponent* MeshCom = CreateStaticMeshComponent(
		*static_cast<const FTrimeshShapeBarrier*>(&ShapeBarrier), *GetOwner(), Material, *Context);
	if (MeshCom == nullptr)
		return;

	const auto AttachmentRule = FAttachmentTransformRules::KeepRelativeTransform;
	MeshCom->AttachToComponent(this, AttachmentRule);

	if (RenderMeshCom != nullptr)
		RenderMeshCom->AttachToComponent(MeshCom, AttachmentRule);

	MeshSourceLocation = EAGX_StaticMeshSourceLocation::TSL_CHILD_STATIC_MESH_COMPONENT;
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
	if (PropertyThatWillChange == nullptr)
	{
		return;
	}

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
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty->GetFName() ==
		GET_MEMBER_NAME_CHECKED(UAGX_TrimeshShapeComponent, MeshSourceAsset))
	{
		return MeshSourceLocation == TSL_STATIC_MESH_ASSET;
	}

	return SuperCanEditChange;
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
			TEXT("TrimeshShapeComponent '%s' in '%s' does not have a StaticMeshComponent to read "
				 "triangle data from. The generated native shape will be invalid."),
			*GetName(), *GetLabelSafe(GetOwner()));
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
			TEXT("GetStaticMeshCollisionData failed for '%s' in '%s'. Unable to find static Mesh."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	const FTransform ComponentTransformNoScale =
		FTransform(GetComponentRotation(), GetComponentLocation());
	const uint32* LodIndex = bOverrideMeshSourceLodIndex ? &MeshSourceLodIndex : nullptr;

	return AGX_MeshUtilities::GetStaticMeshCollisionData(
		Mesh, ComponentTransformNoScale, OutVertices, OutIndices, LodIndex);
}
