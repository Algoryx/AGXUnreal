// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved. 

/// \todo Reduce includes!
#include "AGX_ConstraintDofGraphicsComponent.h"
#include "RenderingThread.h"
#include "RenderResource.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "VertexFactory.h"
#include "MaterialShared.h"
#include "Engine/CollisionProfile.h"
#include "Materials/Material.h"
#include "LocalVertexFactory.h"
#include "SceneManagement.h"
#include "DynamicMeshBuilder.h"
#include "EngineGlobals.h"
#include "Engine/Engine.h"
#include "StaticMeshResources.h"
#include "UObject/ConstructorHelpers.h"

#include "AGX_MeshUtilities.h"
#include "Constraints/AGX_Constraint.h"
#include "Constraints/AGX_ConstraintEnums.h"

/**
 * Holds vertex and index buffers for rendering.
 */
struct FAGX_ConstraintDofGraphicsGeometry
{
	EPrimitiveType Type = PT_TriangleList;

	FStaticMeshVertexBuffers VertexBuffers;
	FLocalVertexFactory VertexFactory;
	FDynamicMeshIndexBuffer32 IndexBuffer;

	FAGX_ConstraintDofGraphicsGeometry(EPrimitiveType InType, ERHIFeatureLevel::Type FeatureLevel, const char* BuffersDebugName)
		: Type(InType), VertexFactory(FeatureLevel, BuffersDebugName)
	{ }

	uint32 NumIndexesPerPrimitive() const
	{
		return NumIndexesPerPrimitive(Type);
	}

	static uint32 NumIndexesPerPrimitive(EPrimitiveType Type)
	{
		switch (Type)
		{
		case PT_TriangleList:
			return 3;
		case PT_LineList:
			return 2;
		default:
			check(!"FAGX_ConstraintDofGraphicsGeometry does not support this primitive type!");
			return 1;
		}
	}
};

/**
 * Defines rendering of a section of a geometry, with a specific material and transformation.
 * Sections can be used to for example render different parts of the geometry with different materials,
 * or to render the same geometry multiple times with different transformations and materials.
 */
struct FAGX_ConstraintDofGraphicsSection
{
	TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry;
	uint32 BeginIndex; // First index to render in the geometry's index buffer.
	uint32 EndIndex; // One past last index to render in geometry's index buffer.
	UMaterialInterface* Material = nullptr;
	bool bShowSelectionOutline;
	FMatrix LocalTransform = FMatrix::Identity;
	ESceneDepthPriorityGroup DepthPriority;
	

	FAGX_ConstraintDofGraphicsSection(const TSharedPtr<FAGX_ConstraintDofGraphicsGeometry>& InGeometry,
		UMaterialInterface* InMaterial, bool bInShowSelectionOutline, FMatrix InLocalTransform,
		ESceneDepthPriorityGroup InDepthPriority)
		: 
	Geometry(InGeometry),
	BeginIndex(0),
	EndIndex(InGeometry ? static_cast<uint32>(InGeometry->IndexBuffer.Indices.Num()) : 0),
	Material(InMaterial),
	bShowSelectionOutline(bInShowSelectionOutline),
	LocalTransform(InLocalTransform),
	DepthPriority(InDepthPriority)
	{ }

	uint32 GetNumPrimitives() const
	{
		return Geometry ? (EndIndex - BeginIndex) / Geometry->NumIndexesPerPrimitive() : 0;
	}

	uint32 GetMinVertexIndex() const
	{
		return 0;
	}

	uint32 GetMaxVertexIndex() const
	{
		return Geometry ? Geometry->VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1 : 0;
	}
};

/**
 * Render proxy for FAGX_ConstraintDofGraphics. Handles render resources. Accessed by both game and render thread.
 */
class FAGX_ConstraintDofGraphicsProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FAGX_ConstraintDofGraphicsProxy(UAGX_ConstraintDofGraphicsComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel())),
		bDrawOnlyIfSelected(true),
		LockedDofs(Component->Constraint->GetLockedDofsBitmask()),
		FrameTransform1(Component->Constraint->BodyAttachment1.GetGlobalFrameMatrix()),
		FrameTransform2(Component->Constraint->BodyAttachment2.GetGlobalFrameMatrix())
	{
		CreateTranslationalArrows(Component);
		CreateRotationalArrows(Component);

		// Enqueue initialization of render resource
		for (const TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry : Geometries)
		{
			ENQUEUE_RENDER_COMMAND(FAGX_ConstraintDofGraphicsVertexBuffersInit)(
				[Geometry = Geometry.Get()](FRHICommandListImmediate& RHICmdList)
			{
				Geometry->VertexBuffers.PositionVertexBuffer.InitResource();
				Geometry->VertexBuffers.StaticMeshVertexBuffer.InitResource();
				Geometry->VertexBuffers.ColorVertexBuffer.InitResource();

				FLocalVertexFactory::FDataType Data;
				Geometry->VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(&Geometry->VertexFactory, Data, /*LightMapIndex*/ 0);
				Geometry->VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(&Geometry->VertexFactory, Data);
				Geometry->VertexFactory.SetData(Data);

				Geometry->VertexFactory.InitResource();
				Geometry->IndexBuffer.InitResource();
			});

			//AGX_MeshUtilities::PrintMeshToLog(Geometry->VertexBuffers, Geometry->IndexBuffer);
		};
	}

	virtual ~FAGX_ConstraintDofGraphicsProxy()
	{
		for (const TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry : Geometries)
		{
			Geometry->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			Geometry->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			Geometry->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			Geometry->IndexBuffer.ReleaseResource();
			Geometry->VertexFactory.ReleaseResource();
		}
	}

	void SetAttachmentFrameTransforms(const FMatrix& InFrameTransform1, const FMatrix& InFrameTransform2)
	{
		FrameTransform1 = InFrameTransform1;
		FrameTransform2 = InFrameTransform2;
	}

private:

	bool IsDofLocked(EDofFlag Dof)
	{
		return static_cast<uint8>(LockedDofs) & static_cast<uint8>(Dof);
	}

	UMaterialInterface* GetTranslationMaterial(UAGX_ConstraintDofGraphicsComponent* Component, EDofFlag Dof)
	{
		if (IsDofLocked(Dof))
		{
			return Component->GetLockedTranslationMaterial();
		}
		else
		{
			return Component->GetFreeTranslationMaterial();
		}
	}

	UMaterialInterface* GetRotationMaterial(UAGX_ConstraintDofGraphicsComponent* Component, EDofFlag Dof)
	{
		if (IsDofLocked(Dof))
		{
			return Component->GetLockedRotationMaterial();
		}
		else
		{
			return Component->GetFreeRotationMaterial();
		}
	}

	FMatrix GetScaleMatrix(UAGX_ConstraintDofGraphicsComponent* Component, EDofFlag Dof)
	{
		if (IsDofLocked(Dof))
		{
			return FScaleMatrix(0.65f);
		}
		else
		{
			return FMatrix::Identity;
		}
	}

	bool GetShowSelectionOutline(EDofFlag Dof)
	{
		return false; // looks best without it
		//return !IsDofLocked(Dof);
	}

	ESceneDepthPriorityGroup GetDepthPriority()
	{
		return SDPG_Foreground;
		//return SDPG_World;
	}

	/// Creates one Geometry for a cylindrical arrow mesh, and three sections using it, one per axis (x, y, z),
	// such that each arrow can chose material depending on the DOF being free or locked.
	void CreateTranslationalArrows(UAGX_ConstraintDofGraphicsComponent* Component)
	{
		// Create the geometry.

		const float ArrowLength = 100.0f;
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry = CreateTranslationalArrowGeometry(GetScene(), ArrowLength);
		Geometries.Add(Geometry);
		
		// Create sections using the geometry.

		const float TranslationOffset = 7.0f;

		// X-Axis
		Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(Geometry,
			GetTranslationMaterial(Component, EDofFlag::DOF_FLAG_TRANSLATIONAL_1),
			false, //GetShowSelectionOutline(EDofFlag::DOF_FLAG_TRANSLATIONAL_1),
			GetScaleMatrix(Component, EDofFlag::DOF_FLAG_TRANSLATIONAL_1) *
			FRotationMatrix(FRotator(-90.0f, 0.0f, 0.0f)) *
			FTranslationMatrix(FVector(TranslationOffset, 0.0f, 0.0f)),
			GetDepthPriority()));

		// Y-Axis
		Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(Geometry,
			GetTranslationMaterial(Component, EDofFlag::DOF_FLAG_TRANSLATIONAL_2),
			false, //GetShowSelectionOutline(EDofFlag::DOF_FLAG_TRANSLATIONAL_2),
			GetScaleMatrix(Component, EDofFlag::DOF_FLAG_TRANSLATIONAL_2) *
			FRotationMatrix(FRotator(0.0f, 0.0f, 90.0f)) *
			FTranslationMatrix(FVector(0.0f, TranslationOffset, 0.0f)),
			GetDepthPriority()));

		// Z-Axis
		Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(Geometry,
			GetTranslationMaterial(Component, EDofFlag::DOF_FLAG_TRANSLATIONAL_3),
			false, //GetShowSelectionOutline(EDofFlag::DOF_FLAG_TRANSLATIONAL_3),
			GetScaleMatrix(Component, EDofFlag::DOF_FLAG_TRANSLATIONAL_3) *
			FRotationMatrix(FRotator(0.0f, 0.0f, 0.0f)) *
			FTranslationMatrix(FVector(0.0f, 0.0f, TranslationOffset)),
			GetDepthPriority()));
	}

	static TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> CreateTranslationalArrowGeometry(FSceneInterface& Scene, float Length)
	{
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry = MakeShared<FAGX_ConstraintDofGraphicsGeometry>(
			PT_TriangleList, Scene.GetFeatureLevel(), "FAGX_ConstraintDofGraphicsGeometry");

		// Create geometry definition.

		const float RadiusOverLength = 0.025f;
		const float CylinderRadius = RadiusOverLength * Length;
		const float CylinderHeight = 0.85f * Length;
		const float ConeRadius = 2.0f * CylinderRadius;
		const float ConeHeight = 0.15f * Length;
		const bool bBottomCap = false;
		const uint32 NumSegments = 16;
		const FLinearColor Transparent(1, 1, 1, 0);
		const FLinearColor Opaque(1, 1, 1, 1);

		AGX_MeshUtilities::CylindricalArrowConstructionData ConstructionData(CylinderRadius, CylinderHeight, ConeRadius,
			ConeHeight, bBottomCap, NumSegments, Transparent, Opaque);
		
		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		ConstructionData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Fill buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		AGX_MeshUtilities::MakeCylindricalArrow(Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices,
			NumAddedIndices, ConstructionData);

		return Geometry;
	}

	/// Creates one Geometry for a bent arrow mesh, and three sections using it, one per axis (x, y, z),
	// such that each arrow can chose material depending on the DOF being free or locked.
	void CreateRotationalArrows(UAGX_ConstraintDofGraphicsComponent* Component)
	{
		// Create the geometry.

		const float Width = 7.0f;
		const float Radius = 13.0f;
		const float SegmentAngle = FMath::DegreesToRadians(260.0);
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry = CreateRotationalArrowGeometry(GetScene(), Width, Radius, SegmentAngle);
		Geometries.Add(Geometry);

		// Create sections using the geometry.

		const int32 NumLockedDofs =
			(IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_1) ? 1 : 0) +
			(IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_2) ? 1 : 0) +
			(IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_3) ? 1 : 0);

		const bool ShowLockedRotationalDofs = false;
		const bool MultipleVisible = ShowLockedRotationalDofs || NumLockedDofs > 1;
		const float TranslationOffset = MultipleVisible ? 10.0f : 27.0f; // more spacing for multiple visible arrows
		const float RotationOffset = -140.0f;

		// X-Axis
		if (!IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_1) || ShowLockedRotationalDofs)
		{
			Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(Geometry,
				GetRotationMaterial(Component, EDofFlag::DOF_FLAG_ROTATIONAL_1),
				GetShowSelectionOutline(EDofFlag::DOF_FLAG_ROTATIONAL_1),
				FRotationMatrix(FRotator(-140.0f, -90.0f, 0.0f)) *
				FTranslationMatrix(FVector(TranslationOffset, 0.0f, 0.0f)),
				GetDepthPriority()));
		}

		// Y-Axis
		if (!IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_2) || ShowLockedRotationalDofs)
		{
			Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(Geometry,
				GetRotationMaterial(Component, EDofFlag::DOF_FLAG_ROTATIONAL_2),
				GetShowSelectionOutline(EDofFlag::DOF_FLAG_ROTATIONAL_2),
				FRotationMatrix(FRotator(-140.0f, 0.0f, 0.0f)) *
				FTranslationMatrix(FVector(0.0f, TranslationOffset, 0.0f)),
				GetDepthPriority()));
		}

		// Z-Axis
		if (!IsDofLocked(EDofFlag::DOF_FLAG_ROTATIONAL_3) || ShowLockedRotationalDofs)
		{
			Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(Geometry,
				GetRotationMaterial(Component, EDofFlag::DOF_FLAG_ROTATIONAL_3),
				GetShowSelectionOutline(EDofFlag::DOF_FLAG_ROTATIONAL_3),
				FRotationMatrix(FRotator(0.0f, -60.0f, -90.0f)) *
				FTranslationMatrix(FVector(0.0f, 0.0f, TranslationOffset)),
				GetDepthPriority()));
		}
	}

	static TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> CreateRotationalArrowGeometry(FSceneInterface& Scene,
		float Width, float Radius, float SegmentAngle)
	{
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry = MakeShared<FAGX_ConstraintDofGraphicsGeometry>(
			PT_TriangleList, Scene.GetFeatureLevel(), "FAGX_ConstraintDofGraphicsGeometry");

		// Create geometry definition.

		const float RectangleWidth = Width;
		const float RectangleLength = 0.8 * (Radius * SegmentAngle);
		const float TriangleWidth = 2.0f * RectangleWidth;
		const float TriangleLength = 0.2 * (Radius * SegmentAngle);
		const float BendAngle = SegmentAngle;
		const uint32 NumSegments = 32;
		const FLinearColor Transparent(1, 1, 1, 0);
		const FLinearColor Opaque(1, 1, 1, 1);

		AGX_MeshUtilities::BendableArrowConstructionData ConstructionData(RectangleWidth, RectangleLength,
			TriangleWidth, TriangleLength, BendAngle, NumSegments, Transparent, Opaque);

		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		ConstructionData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Fill buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		AGX_MeshUtilities::MakeBendableArrow(Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices,
			NumAddedIndices, ConstructionData);

		return Geometry;
	}

	virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_AGX_ConstraintDofGraphics_GetDynamicMeshElements);

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f));

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		for (const TSharedPtr<FAGX_ConstraintDofGraphicsSection> Section : Sections)
		{
			FMaterialRenderProxy* MaterialProxy = bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];
					
					FMeshBatch& Mesh = Collector.AllocateMesh();
					Mesh.bWireframe = bWireframe;
					Mesh.MaterialRenderProxy = MaterialProxy;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.bCanApplyViewModeOverrides = false;
					Mesh.VertexFactory = &Section->Geometry->VertexFactory;
					Mesh.bSelectable = false;
					Mesh.Type = Section->Geometry->Type;
					Mesh.bUseSelectionOutline = Section->bShowSelectionOutline;
					Mesh.DepthPriorityGroup = Section->DepthPriority;

					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &Section->Geometry->IndexBuffer;
					BatchElement.FirstIndex = Section->BeginIndex;
					BatchElement.NumPrimitives = Section->GetNumPrimitives();
					BatchElement.MinVertexIndex = Section->GetMinVertexIndex();
					BatchElement.MaxVertexIndex = Section->GetMaxVertexIndex();

					FMatrix WorldMatrix = GetLocalToWorld().GetMatrixWithoutScale();
					//FMatrix WorldMatrix = FrameTransform1; // setting render matrix instead (see GetRenderMatrix())

					FMatrix ScreenScale = GetScreenSpaceScale(0.56f, 150.0f, 500.0f, 100.0f,
						FrameTransform1.GetOrigin(), View);
					
					FMatrix EffectiveLocalToWorld = Section->LocalTransform * ScreenScale * WorldMatrix;

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer =
						Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();

					DynamicPrimitiveUniformBuffer.Set(EffectiveLocalToWorld, EffectiveLocalToWorld, GetBounds(),
						GetLocalBounds(), true, false, UseEditorDepthTest());

					BatchElement.PrimitiveUniformBufferResource = &DynamicPrimitiveUniformBuffer.UniformBuffer;

					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}

	/// Will create a matrix that scales an object of original size 'OriginalWorldSize' to occupy the desired
	/// 'NormalizedScreenSpaceSize' fraction of the screen horizontally, but limiting it within MinWorldSize and
	/// MaxWorldSize. Result is not 100% correct, but it's consistent when moving around, so just tweak input!
	static FMatrix GetScreenSpaceScale(float NormalizedScreenSpaceSize, float MinWorldSize, float MaxWorldSize,
		float OriginalWorldSize, const FVector& WorldLocation, const FSceneView *View)
	{
		float Distance = (WorldLocation - View->ViewLocation).Size();
		float NormalizedScreenToWorld = 2.0f * Distance * FMath::Atan(FMath::DegreesToRadians(View->FOV) / 2.0f);
		float WorldSize = FMath::Clamp(NormalizedScreenSpaceSize * NormalizedScreenToWorld, MinWorldSize, MaxWorldSize);
		float Scale = WorldSize / OriginalWorldSize;

		return FScaleMatrix(Scale);
	}

	UMaterialInterface* GetSectionMaterial(UAGX_ConstraintDofGraphicsComponent* Component, uint32 SectionIndex)
	{
		if (UMaterialInterface* Material = Component->GetMaterial(SectionIndex))
		{
			return Material;
		}
		else
		{
			return UMaterial::GetDefaultMaterial(MD_Surface);
		}
	};

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
	{
		const bool bVisibleForSelection = !bDrawOnlyIfSelected || IsSelected();

		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View) && bVisibleForSelection;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		Result.bVelocityRelevance = IsMovable() && Result.bOpaqueRelevance && Result.bRenderInMainPass;
		
		MaterialRelevance.SetPrimitiveViewRelevance(Result);

		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	virtual bool IsUsingDistanceCullFade() const override
	{
		return MaterialRelevance.bUsesDistanceCullFade;
	}

	virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }

	uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

private:

	TArray<TSharedPtr<FAGX_ConstraintDofGraphicsGeometry>> Geometries;
	TArray<TSharedPtr<FAGX_ConstraintDofGraphicsSection>> Sections;
	FMaterialRelevance MaterialRelevance;
	const bool bDrawOnlyIfSelected = false;
	const EDofFlag LockedDofs;
	FMatrix FrameTransform1; // global transforms
	FMatrix FrameTransform2;
};

UAGX_ConstraintDofGraphicsComponent::UAGX_ConstraintDofGraphicsComponent(const FObjectInitializer& ObjectInitializer)
	:
Super(ObjectInitializer),
FreeTranslationMaterialIndex(-1),
FreeRotationMaterialIndex(-1),
LockedTranslationMaterialIndex(-1),
LockedRotationMaterialIndex(-1)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	bCastHiddenShadow = false;
	bVisualizeComponent = false;

	// Find materials.
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> FreeTranslationMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphics_Free"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> FreeRotationMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphicsRot_Free"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> LockedTranslationMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphics_Locked"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> LockedRotationMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphicsRot_Locked"));

		UMaterialInterface* FallbackMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

		UMaterialInterface* FreeTranslationMaterial = FreeTranslationMaterialFinder.Succeeded() ?
			Cast<UMaterialInterface>(FreeTranslationMaterialFinder.Object) : FallbackMaterial;

		UMaterialInterface* FreeRotationMaterial = FreeRotationMaterialFinder.Succeeded() ?
			Cast<UMaterialInterface>(FreeRotationMaterialFinder.Object) : FallbackMaterial;

		UMaterialInterface* LockedTranslationMaterial = LockedTranslationMaterialFinder.Succeeded() ?
			Cast<UMaterialInterface>(LockedTranslationMaterialFinder.Object) : FallbackMaterial;

		UMaterialInterface* LockedRotationMaterial = LockedRotationMaterialFinder.Succeeded() ?
			Cast<UMaterialInterface>(LockedRotationMaterialFinder.Object) : FallbackMaterial;

		FreeTranslationMaterialIndex = GetNumMaterials();
		SetMaterial(FreeTranslationMaterialIndex, FreeTranslationMaterial);

		FreeRotationMaterialIndex = GetNumMaterials();
		SetMaterial(FreeRotationMaterialIndex, FreeRotationMaterial);

		LockedTranslationMaterialIndex = GetNumMaterials();
		SetMaterial(LockedTranslationMaterialIndex, LockedTranslationMaterial);

		LockedRotationMaterialIndex = GetNumMaterials();
		SetMaterial(LockedRotationMaterialIndex, LockedRotationMaterial);
	}
}

UMaterialInterface* UAGX_ConstraintDofGraphicsComponent::GetFreeTranslationMaterial() const
{
	return GetMaterial(FreeTranslationMaterialIndex);
}

UMaterialInterface* UAGX_ConstraintDofGraphicsComponent::GetFreeRotationMaterial() const
{
	return GetMaterial(FreeRotationMaterialIndex);
}

UMaterialInterface* UAGX_ConstraintDofGraphicsComponent::GetLockedTranslationMaterial() const
{
	return GetMaterial(LockedTranslationMaterialIndex);
}

UMaterialInterface* UAGX_ConstraintDofGraphicsComponent::GetLockedRotationMaterial() const
{
	return GetMaterial(LockedRotationMaterialIndex);
}

void UAGX_ConstraintDofGraphicsComponent::OnBecameSelected()
{
	MarkRenderTransformDirty();
	MarkRenderDynamicDataDirty();
}

FPrimitiveSceneProxy* UAGX_ConstraintDofGraphicsComponent::CreateSceneProxy()
{
	if(!Constraint)
	{
		return nullptr;
	}

	return new FAGX_ConstraintDofGraphicsProxy(this);
}

int32 UAGX_ConstraintDofGraphicsComponent::GetNumMaterials() const
{
	return GetNumOverrideMaterials();
}

UMaterialInterface* UAGX_ConstraintDofGraphicsComponent::GetMaterial(int32 ElementIndex) const
{
	return Super::GetMaterial(ElementIndex);
}

void UAGX_ConstraintDofGraphicsComponent::GetUsedMaterials(TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	return Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
}

FBoxSphereBounds UAGX_ConstraintDofGraphicsComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	/// \todo Make more precise!

	FBoxSphereBounds NewBounds;
	NewBounds.BoxExtent = FVector(100.0f, 100.0f, 100.0f);
	NewBounds.Origin = LocalToWorld.GetLocation();
	NewBounds.SphereRadius = 150.0f;

	return NewBounds;
}

void UAGX_ConstraintDofGraphicsComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	/// \note Not using this data anymore. Instead we set render matrix directly using the frame (see GetRenderMatrix).

	// Update transform of the proxy to match the constraint attachment frame, if out-of-date!
	if (SceneProxy && Constraint && IsOwnerSelected())
	{
		FMatrix Frame1 = Constraint->BodyAttachment1.GetGlobalFrameMatrix();
		FMatrix Frame2 = Constraint->BodyAttachment2.GetGlobalFrameMatrix();

		FAGX_ConstraintDofGraphicsProxy* CastProxy = static_cast<FAGX_ConstraintDofGraphicsProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FSendConstraintDofGraphicsDynamicData)(
			[CastProxy, Frame1, Frame2](FRHICommandListImmediate& RHICmdList)
			{
				CastProxy->SetAttachmentFrameTransforms(Frame1, Frame2);
			});
	}
}

FMatrix UAGX_ConstraintDofGraphicsComponent::GetRenderMatrix() const
{
	if (Constraint)
	{
		return Constraint->BodyAttachment1.GetGlobalFrameMatrix();
	}
	else
	{
		return FMatrix::Identity;
	}
}