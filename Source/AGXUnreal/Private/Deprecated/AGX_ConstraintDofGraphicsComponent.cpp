// Copyright 2026, Algoryx Simulation AB.

/*
 * Deprecated: see internal issue 739.
 */

#include "Deprecated/AGX_ConstraintDofGraphicsComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintEnums.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
/// \todo Reduce includes!
#include "Misc/EngineVersionComparison.h"
#include "DynamicMeshBuilder.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "LocalVertexFactory.h"
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
#include "MaterialDomain.h"
#endif
#include "Materials/Material.h"
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
#include "Materials/MaterialRenderProxy.h"
#endif
#include "MaterialShared.h"
#include "Misc/EngineVersionComparison.h"
#include "SceneManagement.h"
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "RenderingThread.h"
#include "RenderResource.h"
#include "SceneInterface.h"
#include "SceneView.h"
#include "StaticMeshResources.h"
#include "UObject/ConstructorHelpers.h"
#include "VertexFactory.h"

/**
 * Holds vertex and index buffers for rendering.
 */
struct FAGX_ConstraintDofGraphicsGeometry
{
	EPrimitiveType Type = PT_TriangleList;

	FStaticMeshVertexBuffers VertexBuffers;
	FLocalVertexFactory VertexFactory;
	FDynamicMeshIndexBuffer32 IndexBuffer;

	FAGX_ConstraintDofGraphicsGeometry(
		EPrimitiveType InType, ERHIFeatureLevel::Type FeatureLevel, const char* BuffersDebugName)
		: Type(InType)
		, VertexFactory(FeatureLevel, BuffersDebugName)
	{
	}

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
 * Sections can be used to for example render different parts of the geometry with different
 * materials, or to render the same geometry multiple times with different transformations and
 * materials.
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

	FAGX_ConstraintDofGraphicsSection(
		const TSharedPtr<FAGX_ConstraintDofGraphicsGeometry>& InGeometry,
		UMaterialInterface* InMaterial, bool bInShowSelectionOutline, FMatrix InLocalTransform,
		ESceneDepthPriorityGroup InDepthPriority)
		: Geometry(InGeometry)
		, BeginIndex(0)
		, EndIndex(InGeometry ? static_cast<uint32>(InGeometry->IndexBuffer.Indices.Num()) : 0)
		, Material(InMaterial)
		, bShowSelectionOutline(bInShowSelectionOutline)
		, LocalTransform(InLocalTransform)
		, DepthPriority(InDepthPriority)
	{
	}

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
 * Render proxy for FAGX_ConstraintDofGraphics. Handles render resources. Accessed by both game and
 * render thread.
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
		, ViolatedMaterial(Component->GetViolatedMaterial())
#if UE_VERSION_OLDER_THAN(5, 7, 0)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
#else
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetShaderPlatform()))
#endif
		, bDrawOnlyIfSelected(true)
		, Constraint(Component->Constraint)
		, LockedDofs(Component->Constraint->GetLockedDofsBitmask())
		, FrameTransform1(Component->Constraint->BodyAttachment1.GetGlobalFrameMatrix())
		, FrameTransform2(Component->Constraint->BodyAttachment2.GetGlobalFrameMatrix())
		, AttachmentId(Component->AttachmentId)
	{
		CreateTranslationalArrows(Component);
		CreateRotationalArrows(Component);

		// Enqueue initialization of render resource
		for (const TSharedPtr<FAGX_ConstraintDofGraphicsGeometry>& Geometry : Geometries)
		{
			ENQUEUE_RENDER_COMMAND(FAGX_ConstraintDofGraphicsVertexBuffersInit)
			(
				[Geometry = Geometry.Get()](FRHICommandListImmediate& RHICmdList)
				{
#if UE_VERSION_OLDER_THAN(5, 3, 0)
					Geometry->VertexBuffers.PositionVertexBuffer.InitResource();
					Geometry->VertexBuffers.StaticMeshVertexBuffer.InitResource();
					Geometry->VertexBuffers.ColorVertexBuffer.InitResource();
#else
					Geometry->VertexBuffers.PositionVertexBuffer.InitResource(RHICmdList);
					Geometry->VertexBuffers.StaticMeshVertexBuffer.InitResource(RHICmdList);
					Geometry->VertexBuffers.ColorVertexBuffer.InitResource(RHICmdList);
#endif

					FLocalVertexFactory::FDataType Data;
					Geometry->VertexBuffers.PositionVertexBuffer.BindPositionVertexBuffer(
						&Geometry->VertexFactory, Data);
					Geometry->VertexBuffers.StaticMeshVertexBuffer.BindTangentVertexBuffer(
						&Geometry->VertexFactory, Data);
					Geometry->VertexBuffers.StaticMeshVertexBuffer.BindPackedTexCoordVertexBuffer(
						&Geometry->VertexFactory, Data);
					Geometry->VertexBuffers.StaticMeshVertexBuffer.BindLightMapVertexBuffer(
						&Geometry->VertexFactory, Data, /*LightMapIndex*/ 0);
					Geometry->VertexBuffers.ColorVertexBuffer.BindColorVertexBuffer(
						&Geometry->VertexFactory, Data);
#if UE_VERSION_OLDER_THAN(5, 4, 0)
					Geometry->VertexFactory.SetData(Data);
#else
					Geometry->VertexFactory.SetData(RHICmdList, Data);
#endif

#if UE_VERSION_OLDER_THAN(5, 3, 0)
					Geometry->VertexFactory.InitResource();
					Geometry->IndexBuffer.InitResource();
#else
					Geometry->VertexFactory.InitResource(RHICmdList);
					Geometry->IndexBuffer.InitResource(RHICmdList);
#endif
				});
		};
	}

	virtual ~FAGX_ConstraintDofGraphicsProxy()
	{
		for (const TSharedPtr<FAGX_ConstraintDofGraphicsGeometry>& Geometry : Geometries)
		{
			Geometry->VertexBuffers.PositionVertexBuffer.ReleaseResource();
			Geometry->VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			Geometry->VertexBuffers.ColorVertexBuffer.ReleaseResource();
			Geometry->IndexBuffer.ReleaseResource();
			Geometry->VertexFactory.ReleaseResource();
		}
	}

	void SetAttachmentFrameTransforms(
		const FMatrix& InFrameTransform1, const FMatrix& InFrameTransform2)
	{
		FrameTransform1 = InFrameTransform1;
		FrameTransform2 = InFrameTransform2;
	}

private:
	bool IsDofLocked(EDofFlag Dof)
	{
		return static_cast<uint8>(LockedDofs) & static_cast<uint8>(Dof);
	}

	UMaterialInterface* GetTranslationMaterial(
		UAGX_ConstraintDofGraphicsComponent* Component, EDofFlag Dof)
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

	UMaterialInterface* GetRotationMaterial(
		UAGX_ConstraintDofGraphicsComponent* Component, EDofFlag Dof)
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
		// return !IsDofLocked(Dof);
	}

	ESceneDepthPriorityGroup GetDepthPriority()
	{
		return SDPG_Foreground;
		// return SDPG_World;
	}

	/// Creates one Geometry for a cylindrical arrow mesh, and three sections using it, one per axis
	/// (x, y, z),
	// such that each arrow can chose material depending on the DOF being free or locked.
	void CreateTranslationalArrows(UAGX_ConstraintDofGraphicsComponent* Component)
	{
		// Create the geometry.

		const float ArrowLength = 100.0f;
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry =
			CreateTranslationalArrowGeometry(GetScene(), ArrowLength);
		Geometries.Add(Geometry);

		// Create sections using the geometry.

		const float TranslationOffset = 7.0f;

		// X-Axis
		Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(
			Geometry, GetTranslationMaterial(Component, EDofFlag::DofFlagTranslational1),
			false, // GetShowSelectionOutline(EDofFlag::DofFlagTranslational1),
			GetScaleMatrix(Component, EDofFlag::DofFlagTranslational1) *
				FRotationMatrix(FRotator(-90.0f, 0.0f, 0.0f)) *
				FTranslationMatrix(FVector(TranslationOffset, 0.0f, 0.0f)),
			GetDepthPriority()));

		// Y-Axis
		Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(
			Geometry, GetTranslationMaterial(Component, EDofFlag::DofFlagTranslational2),
			false, // GetShowSelectionOutline(EDofFlag::DofFlagTranslational2),
			GetScaleMatrix(Component, EDofFlag::DofFlagTranslational2) *
				FRotationMatrix(FRotator(0.0f, 0.0f, 90.0f)) *
				FTranslationMatrix(FVector(0.0f, TranslationOffset, 0.0f)),
			GetDepthPriority()));

		// Z-Axis
		Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(
			Geometry, GetTranslationMaterial(Component, EDofFlag::DofFlagTranslational3),
			false, // GetShowSelectionOutline(EDofFlag::DofFlagTranslational3),
			GetScaleMatrix(Component, EDofFlag::DofFlagTranslational3) *
				FRotationMatrix(FRotator(0.0f, 0.0f, 0.0f)) *
				FTranslationMatrix(FVector(0.0f, 0.0f, TranslationOffset)),
			GetDepthPriority()));
	}

	static TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> CreateTranslationalArrowGeometry(
		FSceneInterface& Scene, float Length)
	{
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry =
			MakeShared<FAGX_ConstraintDofGraphicsGeometry>(
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

		AGX_MeshUtilities::CylindricalArrowConstructionData ConstructionData(
			CylinderRadius, CylinderHeight, ConeRadius, ConeHeight, bBottomCap, NumSegments,
			Transparent, Opaque);

		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		ConstructionData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(
			NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Fill buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		AGX_MeshUtilities::MakeCylindricalArrow(
			Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices, NumAddedIndices,
			ConstructionData);

		return Geometry;
	}

	/// Creates one Geometry for a bent arrow mesh, and three sections using it, one per axis (x, y,
	/// z),
	// such that each arrow can chose material depending on the DOF being free or locked.
	void CreateRotationalArrows(UAGX_ConstraintDofGraphicsComponent* Component)
	{
		// Create the geometry.

		const float Width = 7.0f;
		const float Radius = 13.0f;
		const float SegmentAngle = FMath::DegreesToRadians(260.0);
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry =
			CreateRotationalArrowGeometry(GetScene(), Width, Radius, SegmentAngle);
		Geometries.Add(Geometry);

		// Create sections using the geometry.

		const int32 NumLockedDofs = (IsDofLocked(EDofFlag::DofFlagRotational1) ? 1 : 0) +
									(IsDofLocked(EDofFlag::DofFlagRotational2) ? 1 : 0) +
									(IsDofLocked(EDofFlag::DofFlagRotational3) ? 1 : 0);

		const bool ShowLockedRotationalDofs = false;
		const bool MultipleVisible = ShowLockedRotationalDofs || NumLockedDofs > 1;
		const float TranslationOffset =
			MultipleVisible ? 10.0f : 27.0f; // more spacing for multiple visible arrows
		const float RotationOffset = -140.0f;

		// X-Axis
		if (!IsDofLocked(EDofFlag::DofFlagRotational1) || ShowLockedRotationalDofs)
		{
			Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(
				Geometry, GetRotationMaterial(Component, EDofFlag::DofFlagRotational1),
				GetShowSelectionOutline(EDofFlag::DofFlagRotational1),
				FRotationMatrix(FRotator(-140.0f, -90.0f, 0.0f)) *
					FTranslationMatrix(FVector(TranslationOffset, 0.0f, 0.0f)),
				GetDepthPriority()));
		}

		// Y-Axis
		if (!IsDofLocked(EDofFlag::DofFlagRotational2) || ShowLockedRotationalDofs)
		{
			Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(
				Geometry, GetRotationMaterial(Component, EDofFlag::DofFlagRotational2),
				GetShowSelectionOutline(EDofFlag::DofFlagRotational2),
				FRotationMatrix(FRotator(-140.0f, 0.0f, 0.0f)) *
					FTranslationMatrix(FVector(0.0f, TranslationOffset, 0.0f)),
				GetDepthPriority()));
		}

		// Z-Axis
		if (!IsDofLocked(EDofFlag::DofFlagRotational3) || ShowLockedRotationalDofs)
		{
			Sections.Add(MakeShared<FAGX_ConstraintDofGraphicsSection>(
				Geometry, GetRotationMaterial(Component, EDofFlag::DofFlagRotational3),
				GetShowSelectionOutline(EDofFlag::DofFlagRotational3),
				FRotationMatrix(FRotator(0.0f, -60.0f, -90.0f)) *
					FTranslationMatrix(FVector(0.0f, 0.0f, TranslationOffset)),
				GetDepthPriority()));
		}
	}

	static TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> CreateRotationalArrowGeometry(
		FSceneInterface& Scene, float Width, float Radius, float SegmentAngle)
	{
		TSharedPtr<FAGX_ConstraintDofGraphicsGeometry> Geometry =
			MakeShared<FAGX_ConstraintDofGraphicsGeometry>(
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

		AGX_MeshUtilities::BendableArrowConstructionData ConstructionData(
			RectangleWidth, RectangleLength, TriangleWidth, TriangleLength, BendAngle, NumSegments,
			Transparent, Opaque);

		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		ConstructionData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(
			NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Fill buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		AGX_MeshUtilities::MakeBendableArrow(
			Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices, NumAddedIndices,
			ConstructionData);

		return Geometry;
	}

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_AGX_ConstraintDofGraphics_GetDynamicMeshElements);

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		// Check if violated, if so pick violated material instead of Section->Material.
		bool bViolated = Constraint->AreFramesInViolatedState();
		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f));

		FMaterialRenderProxy* ViolatedMaterialInstance = ViolatedMaterial->GetRenderProxy();

		auto SelectMaterial =
			[bWireframe, bViolated, WireframeMaterialInstance, ViolatedMaterialInstance,
			 this](FAGX_ConstraintDofGraphicsSection& Section) -> FMaterialRenderProxy*
		{
			if (bWireframe)
			{
				return WireframeMaterialInstance;
			}
			else if (bViolated && AttachmentId == 2)
			{
				return ViolatedMaterialInstance;
			}
			else
			{
				return Section.Material->GetRenderProxy();
			}
		};

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		for (const TSharedPtr<FAGX_ConstraintDofGraphicsSection>& Section : Sections)
		{
			FMaterialRenderProxy* MaterialProxy = SelectMaterial(*Section);
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
					// FMatrix WorldMatrix = FrameTransform1; // setting render matrix instead (see
					// GetRenderMatrix())

					FMatrix ScreenScale = GetScreenSpaceScale(
						0.56f, 150.0f, 500.0f, 100.0f, WorldMatrix.GetOrigin(), View);

					FMatrix EffectiveLocalToWorld =
						Section->LocalTransform * ScreenScale * WorldMatrix;

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer =
						Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();
#if UE_VERSION_OLDER_THAN(4, 23, 0)
					DynamicPrimitiveUniformBuffer.Set(
						EffectiveLocalToWorld, EffectiveLocalToWorld, GetBounds(), GetLocalBounds(),
						true, false, UseEditorDepthTest());
#elif UE_VERSION_OLDER_THAN(5, 1, 0)
					/// \todo Replace Unknown with proper name or use some getter function to get a
					/// proper value.
					bool Unknown = false;
					DynamicPrimitiveUniformBuffer.Set(
						EffectiveLocalToWorld, EffectiveLocalToWorld, GetBounds(), GetLocalBounds(),
						true, false, DrawsVelocity(), Unknown);
#elif UE_VERSION_OLDER_THAN(5, 4, 0)
					/// \todo Replace Unknown with proper name or use some getter function to get a
					/// proper value.
					bool Unknown = false;
					DynamicPrimitiveUniformBuffer.Set(
						EffectiveLocalToWorld, EffectiveLocalToWorld, GetBounds(), GetLocalBounds(),
						true, false, Unknown);
#else
					/// \todo Replace Unknown with proper name or use some getter function to get a
					/// proper value.
					bool Unknown = false;
					DynamicPrimitiveUniformBuffer.Set(
						Collector.GetRHICommandList(), EffectiveLocalToWorld, EffectiveLocalToWorld,
						GetBounds(), GetLocalBounds(), true, false, Unknown);
#endif
					BatchElement.PrimitiveUniformBufferResource =
						&DynamicPrimitiveUniformBuffer.UniformBuffer;

					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}
	}

	/// Will create a matrix that scales an object of original size 'OriginalWorldSize' to occupy
	/// the desired 'NormalizedScreenSpaceSize' fraction of the screen horizontally, but limiting it
	/// within MinWorldSize and MaxWorldSize. Result is not 100% correct, but it's consistent when
	/// moving around, so just tweak input!
	static FMatrix GetScreenSpaceScale(
		float NormalizedScreenSpaceSize, float MinWorldSize, float MaxWorldSize,
		float OriginalWorldSize, const FVector& WorldLocation, const FSceneView* View)
	{
		float Distance = (WorldLocation - View->ViewLocation).Size();
		float NormalizedScreenToWorld =
			2.0f * Distance * FMath::Atan(FMath::DegreesToRadians(View->FOV) / 2.0f);
		float WorldSize = FMath::Clamp(
			NormalizedScreenSpaceSize * NormalizedScreenToWorld, MinWorldSize, MaxWorldSize);
		float Scale = WorldSize / OriginalWorldSize;

		return FScaleMatrix(Scale);
	}

	UMaterialInterface* GetSectionMaterial(
		UAGX_ConstraintDofGraphicsComponent* Component, uint32 SectionIndex)
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
		const bool bSelected = IsSelected() && Constraint->IsSelected() && Constraint->IsVisible();
		const bool bPrimary = AttachmentId == 1;
		const bool bVisibleForSelection =
			bSelected && (bPrimary || Constraint->AreFramesInViolatedState());

		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View) && bVisibleForSelection;
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		Result.bVelocityRelevance = IsMovable() &&
#if UE_VERSION_OLDER_THAN(4, 25, 0)
									Result.bOpaqueRelevance &&
#else
									Result.bOpaque &&
#endif
									Result.bRenderInMainPass;

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

	virtual uint32 GetMemoryFootprint(void) const override
	{
		return (sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return (FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:
	TArray<TSharedPtr<FAGX_ConstraintDofGraphicsGeometry>> Geometries;
	TArray<TSharedPtr<FAGX_ConstraintDofGraphicsSection>> Sections;
	UMaterialInterface* ViolatedMaterial;
	FMaterialRelevance MaterialRelevance;
	const bool bDrawOnlyIfSelected = false;
	UAGX_ConstraintComponent* Constraint;
	const EDofFlag LockedDofs;
	FMatrix FrameTransform1; // global transforms
	FMatrix FrameTransform2;
	int32 AttachmentId;
};

UAGX_ConstraintDofGraphicsComponent::UAGX_ConstraintDofGraphicsComponent(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, AttachmentId(1)
	, FreeTranslationMaterialIndex(-1)
	, FreeRotationMaterialIndex(-1)
	, LockedTranslationMaterialIndex(-1)
	, LockedRotationMaterialIndex(-1)
	, ViolatedMaterialIndex(-1)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	bCastHiddenShadow = false;
#if WITH_EDITORONLY_DATA
	bVisualizeComponent = false;
#endif

	// Find materials.
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> FreeTranslationMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphics_Free"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> FreeRotationMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphicsRot_Free"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface>
			LockedTranslationMaterialFinder(
				TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphics_Locked"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> LockedRotationMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphicsRot_Locked"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> ViolatedMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintDofGraphics_Violated"));

		UMaterialInterface* FallbackMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

		UMaterialInterface* FreeTranslationMaterial =
			FreeTranslationMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(FreeTranslationMaterialFinder.Object)
				: FallbackMaterial;

		UMaterialInterface* FreeRotationMaterial =
			FreeRotationMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(FreeRotationMaterialFinder.Object)
				: FallbackMaterial;

		UMaterialInterface* LockedTranslationMaterial =
			LockedTranslationMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(LockedTranslationMaterialFinder.Object)
				: FallbackMaterial;

		UMaterialInterface* LockedRotationMaterial =
			LockedRotationMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(LockedRotationMaterialFinder.Object)
				: FallbackMaterial;

		UMaterialInterface* ViolatedMaterial =
			ViolatedMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(ViolatedMaterialFinder.Object)
				: FallbackMaterial;

		FreeTranslationMaterialIndex = GetNumMaterials();
		SetMaterial(FreeTranslationMaterialIndex, FreeTranslationMaterial);

		FreeRotationMaterialIndex = GetNumMaterials();
		SetMaterial(FreeRotationMaterialIndex, FreeRotationMaterial);

		LockedTranslationMaterialIndex = GetNumMaterials();
		SetMaterial(LockedTranslationMaterialIndex, LockedTranslationMaterial);

		LockedRotationMaterialIndex = GetNumMaterials();
		SetMaterial(LockedRotationMaterialIndex, LockedRotationMaterial);

		ViolatedMaterialIndex = GetNumMaterials();
		SetMaterial(ViolatedMaterialIndex, ViolatedMaterial);
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

UMaterialInterface* UAGX_ConstraintDofGraphicsComponent::GetViolatedMaterial() const
{
	return GetMaterial(ViolatedMaterialIndex);
}

void UAGX_ConstraintDofGraphicsComponent::OnBecameSelected()
{
	MarkRenderTransformDirty();
	MarkRenderDynamicDataDirty();
}

FPrimitiveSceneProxy* UAGX_ConstraintDofGraphicsComponent::CreateSceneProxy()
{
	if (!Constraint)
	{
		return nullptr;
	}

	if (GetWorld()->bPostTickComponentUpdate)
	{
		// Based on
		// https://github.com/kestrelm/Creature_UE4/blob/master/CreatureEditorAndPlugin/CreaturePlugin/Source/CreaturePlugin/Private/CreatureMeshComponent.cpp
		//
		// Not sure that SendRenderDynamicData_Concurrent is what we really want
		// to do in this case, but I know that MarkRenderDynamicDataDirty isn't
		// it because it does check(!bPostTickComponentUpdate).
		SendRenderDynamicData_Concurrent();
	}
	else
	{
		MarkRenderDynamicDataDirty();
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

void UAGX_ConstraintDofGraphicsComponent::GetUsedMaterials(
	TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	return Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
}

FBoxSphereBounds UAGX_ConstraintDofGraphicsComponent::CalcBounds(
	const FTransform& LocalToWorld) const
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

	/// \note Not using this data anymore. Instead we set render matrix directly using the frame
	/// (see GetRenderMatrix).

	// Update transform of the proxy to match the constraint attachment frame, if out-of-date!
	if (SceneProxy && Constraint && IsOwnerSelected())
	{
		FMatrix Frame1 = Constraint->BodyAttachment1.GetGlobalFrameMatrix();
		FMatrix Frame2 = Constraint->BodyAttachment2.GetGlobalFrameMatrix();

		FAGX_ConstraintDofGraphicsProxy* CastProxy =
			static_cast<FAGX_ConstraintDofGraphicsProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FSendConstraintDofGraphicsDynamicData)
		([CastProxy, Frame1, Frame2](FRHICommandListImmediate& RHICmdList)
		 { CastProxy->SetAttachmentFrameTransforms(Frame1, Frame2); });
	}
}

FMatrix UAGX_ConstraintDofGraphicsComponent::GetRenderMatrix() const
{
	if (Constraint)
	{
		switch (AttachmentId)
		{
			case 1:
				return Constraint->BodyAttachment1.GetGlobalFrameMatrix();
			case 2:
				return Constraint->BodyAttachment2.GetGlobalFrameMatrix();
			default:
				checkNoEntry();
				return FMatrix::Identity;
		}
	}
	else
	{
		return FMatrix::Identity;
	}
}
