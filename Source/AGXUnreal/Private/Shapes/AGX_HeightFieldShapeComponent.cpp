// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/AGX_HeightFieldShapeComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_LogCategory.h"
#include "Terrain/AGX_HeightFieldBoundsComponent.h"
#include "Utilities/AGX_HeightFieldUtilities.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "Landscape.h"
#include "Misc/EngineVersionComparison.h"

UAGX_HeightFieldShapeComponent::UAGX_HeightFieldShapeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	HeightFieldBounds =
		CreateDefaultSubobject<UAGX_HeightFieldBoundsComponent>(TEXT("HeightFieldBounds"));
}

FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNative()
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNative() const
{
	if (!NativeBarrier.HasNative())
	{
		// Cannot use HasNative in the test above because it is implemented
		// in terms of GetNative, i.e., this function. Asking the barrier instead.
		return nullptr;
	}
	return &NativeBarrier;
}

FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNativeBarrier()
{
	return &NativeBarrier;
}

const FShapeBarrier* UAGX_HeightFieldShapeComponent::GetNativeBarrier() const
{
	return &NativeBarrier;
}

FShapeBarrier* UAGX_HeightFieldShapeComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		CreateNative();
	}
	return &NativeBarrier;
}

FHeightFieldShapeBarrier* UAGX_HeightFieldShapeComponent::GetNativeHeightField()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

void UAGX_HeightFieldShapeComponent::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	Super::UpdateNativeProperties();

	UpdateNativeGlobalTransform();

	/// \todo What is the height field equivalent of this?
	// NativeBarrier.SetHalfExtents(HalfExtent * GetComponentScale());
}

void UAGX_HeightFieldShapeComponent::CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
{
	/// \todo What is the height field equivalent of this?
	// AGX_MeshUtilities::MakeCube(OutMeshData.Vertices, OutMeshData.Normals, OutMeshData.Indices,
	// HalfExtent);
}

void UAGX_HeightFieldShapeComponent::UpdateNativeGlobalTransform()
{
	// We override this function because the parents class' version of it does not allow HasNative
	// to be false, which is common for a Height Field Component.
	if (HasNative())
	{
		UAGX_ShapeComponent::UpdateNativeGlobalTransform();
	}
}

#if WITH_EDITOR

bool UAGX_HeightFieldShapeComponent::DoesPropertyAffectVisualMesh(
	const FName& PropertyName, const FName& MemberPropertyName) const
{
	// Height fields does not have a visual mesh, so there there is nothing to affect.
	return false;
}

#endif

void UAGX_HeightFieldShapeComponent::CreateNative()
{
	UE_LOG(LogAGX, Log, TEXT("Allocating native object for HeightFieldShapeComponent."));
	check(!HasNative());
	if (SourceLandscape == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("HeightFieldComponent hasn't been given a source Landscape. Will not be included "
				 "in the simulation."));
		return;
	}

	TOptional<UAGX_HeightFieldBoundsComponent::FHeightFieldBoundsInfo> BoxBounds =
		HeightFieldBounds->GetLandscapeAdjustedBounds();
	if (!BoxBounds.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to create Height Field native; the given Bounds was invalid."));
		return;
	}

	const FVector StartPos = BoxBounds->Transform.TransformPositionNoScale(-BoxBounds->HalfExtent);
	NativeBarrier = AGX_HeightFieldUtilities::CreateHeightField(
		*SourceLandscape, StartPos, BoxBounds->HalfExtent.X * 2.0, BoxBounds->HalfExtent.Y * 2.0);
	check(HasNative());

	const FTransform Transform = BoxBounds->Transform;
	SetWorldTransform(Transform);
	check(HasNative());
	UpdateNativeProperties();
}

void UAGX_HeightFieldShapeComponent::ReleaseNative()
{
	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

void UAGX_HeightFieldShapeComponent::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
	if (HeightFieldBounds)
	{
		HeightFieldBounds->DestroyComponent();
	}
}

void UAGX_HeightFieldShapeComponent::Serialize(FArchive& Archive)
{
	Super::Serialize(Archive);
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::HeightFieldUsesBounds))
	{
		HeightFieldBounds->bInfiniteBounds = true;
	}
}
