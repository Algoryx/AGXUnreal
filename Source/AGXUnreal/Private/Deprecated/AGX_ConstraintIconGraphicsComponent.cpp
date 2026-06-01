// Copyright 2026, Algoryx Simulation AB.

/*
 * Deprecated: see internal issue 739.
 */

/// \todo Reduce includes!
#include "Deprecated/AGX_ConstraintIconGraphicsComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "AGX_UE4Compatibility.h"
#include "Constraints/AGX_BallConstraintComponent.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintEnums.h"
#include "Constraints/AGX_CylindricalConstraintComponent.h"
#include "Constraints/AGX_DistanceConstraintComponent.h"
#include "Constraints/AGX_HingeConstraintComponent.h"
#include "Constraints/AGX_LockConstraintComponent.h"
#include "Constraints/AGX_PrismaticConstraintComponent.h"
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
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
#include "PrimitiveViewRelevance.h"
#include "PrimitiveSceneProxy.h"
#include "RenderingThread.h"
#include "RenderResource.h"
#include "SceneInterface.h"
#include "SceneManagement.h"
#include "SceneView.h"
#include "StaticMeshResources.h"
#include "UObject/ConstructorHelpers.h"
#include "VertexFactory.h"

/**
 * Holds vertex and index buffers for rendering.
 */
struct FAGX_ConstraintIconGraphicsGeometry
{
	EPrimitiveType Type = PT_TriangleList;

	FStaticMeshVertexBuffers VertexBuffers;
	FLocalVertexFactory VertexFactory;
	FDynamicMeshIndexBuffer32 IndexBuffer;

	FAGX_ConstraintIconGraphicsGeometry(
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
				check(!"FAGX_ConstraintIconGraphicsGeometry does not support this primitive type!");
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
struct FAGX_ConstraintIconGraphicsSection
{
	TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry;
	uint32 BeginIndex; // First index to render in the geometry's index buffer.
	uint32 EndIndex; // One past last index to render in geometry's index buffer.
	UMaterialInterface* Material = nullptr;
	bool bShowSelectionOutline;
	FMatrix LocalTransform = FMatrix::Identity;
	ESceneDepthPriorityGroup DepthPriority;

	FAGX_ConstraintIconGraphicsSection(
		const TSharedPtr<FAGX_ConstraintIconGraphicsGeometry>& InGeometry,
		UMaterialInterface* InMaterial, bool bInShowSelectionOutline, FMatrix InLocalTransform,
		ESceneDepthPriorityGroup InDepthPriority, uint32 InBeginIndex = 0, int32 InEndIndex = -1)
		: Geometry(InGeometry)
		, BeginIndex(InBeginIndex)
		, EndIndex(
			  InEndIndex >= 0
				  ? InEndIndex
				  : (InGeometry ? static_cast<uint32>(InGeometry->IndexBuffer.Indices.Num()) : 0))
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
 * Render proxy for FAGX_ConstraintIconGraphics. Handles render resources. Accessed by both game and
 * render thread.
 */
class FAGX_ConstraintIconGraphicsProxy final : public FPrimitiveSceneProxy
{
public:
	SIZE_T GetTypeHash() const override
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FAGX_ConstraintIconGraphicsProxy(UAGX_ConstraintIconGraphicsComponent* Component)
		: FPrimitiveSceneProxy(Component)
		#if UE_VERSION_OLDER_THAN(5, 7, 0)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
#else
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetShaderPlatform()))
#endif
		, bDrawOnlyIfUnselected(true)
		, Constraint(Component->Constraint)
		, LockedDofs(Component->Constraint->GetLockedDofsBitmask())
	{
		FrameTransform1 = Component->Constraint->BodyAttachment1.GetGlobalFrameMatrix();
		FrameTransform2 = Component->Constraint->BodyAttachment2.GetGlobalFrameMatrix();
		/// \todo Use inheritance instead of this branching below.
		/// \todo IsA() should probably not be used if future constraints will derive these
		/// spawnable constraints.
		USceneComponent* Parent = Component->GetAttachParent();
		if (Parent->IsA<UAGX_BallConstraintComponent>())
		{
			CreateBallConstraint(Component);
		}
		else if (Parent->IsA<UAGX_CylindricalConstraintComponent>())
		{
			CreateCylindricalConstraint(Component);
		}
		else if (Parent->IsA<UAGX_DistanceConstraintComponent>())
		{
			/// \todo
		}
		else if (Parent->IsA<UAGX_HingeConstraintComponent>())
		{
			/// \todo
		}
		else if (Parent->IsA<UAGX_LockConstraintComponent>())
		{
			/// \todo
		}
		else if (Parent->IsA<UAGX_PrismaticConstraintComponent>())
		{
			// Using low-res cylindrical for boxy look. \todo Make dedicated!
			CreateCylindricalConstraint(Component, 4);
		}

		// Enqueue initialization of render resource
		for (const TSharedPtr<FAGX_ConstraintIconGraphicsGeometry>& Geometry : Geometries)
		{
			ENQUEUE_RENDER_COMMAND(FAGX_ConstraintIconGraphicsVertexBuffersInit)
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
		}
	}

	virtual ~FAGX_ConstraintIconGraphicsProxy()
	{
		for (const TSharedPtr<FAGX_ConstraintIconGraphicsGeometry>& Geometry : Geometries)
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

	bool GetShowSelectionOutline(EDofFlag Dof)
	{
		return false; // looks best without it
		// return !IsDofLocked(Dof);
	}

	ESceneDepthPriorityGroup GetDepthPriority()
	{
		return SDPG_Foreground;
		// return SDPG_World; /// \note Also need to turn off "Disable Depth Test" on material for
		// it to be cullable.
	}

	void CreateBallConstraint(UAGX_ConstraintIconGraphicsComponent* Component)
	{
		// Create the geometry.

		TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry =
			MakeShared<FAGX_ConstraintIconGraphicsGeometry>(
				PT_TriangleList, GetScene().GetFeatureLevel(),
				"FAGX_ConstraintIconGraphicsGeometry");

		// Create geometry definition.

		const float Radius = 20.0f;
		const uint32 NumSegments = 24;

		const FLinearColor Opaque(1, 1, 1, 1);
		const FLinearColor SemiTransparent(1, 1, 1, 0.5);

		AGX_MeshUtilities::DiskArrayConstructionData DiskArrayData(
			Radius, NumSegments, 0.0f, 3, true, SemiTransparent, SemiTransparent,
			{FTransform3f(FRotator3f(90.0f, 0.0f, 0.0f)),
			 FTransform3f(FRotator3f(0.0f, 90.0f, 0.0f)),
			 FTransform3f(FRotator3f(0.0f, 0.0f, 90.0f))});

		AGX_MeshUtilities::SphereConstructionData SphereData(Radius, NumSegments);

		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		DiskArrayData.AppendBufferSizes(NumVertices, NumIndices);
		SphereData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(
			NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Create Sections and Fill Geometry buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(
				Geometry, Component->GetOuterShellMaterial(), true, FMatrix::Identity,
				GetDepthPriority(), NumAddedIndices, NumAddedIndices + DiskArrayData.Indices));

			AGX_MeshUtilities::MakeDiskArray(
				Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices, NumAddedIndices,
				DiskArrayData);
		}

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(
				Geometry, Component->GetInnerShellMaterial(), true, FMatrix::Identity,
				GetDepthPriority(), NumAddedIndices, NumAddedIndices + SphereData.Indices));

			AGX_MeshUtilities::MakeSphere(
				Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices, NumAddedIndices,
				SphereData);
		}

		Geometries.Add(Geometry);
	}

	void CreateCylindricalConstraint(
		UAGX_ConstraintIconGraphicsComponent* Component, uint32 NumSegments = 16)
	{
		// Create the geometry.

		TSharedPtr<FAGX_ConstraintIconGraphicsGeometry> Geometry =
			MakeShared<FAGX_ConstraintIconGraphicsGeometry>(
				PT_TriangleList, GetScene().GetFeatureLevel(),
				"FAGX_ConstraintIconGraphicsGeometry");

		// Create geometry definition.

		const float InnerRadius = 9.0f;
		const float OuterRadius = 16.0f;
		const FLinearColor Opaque(1, 1, 1, 1);
		const FLinearColor Transparent(1, 1, 1, 0);

		AGX_MeshUtilities::DiskArrayConstructionData DiskArrayData(
			InnerRadius, NumSegments, 5.0f, 18, true, Opaque, Transparent);
		AGX_MeshUtilities::CylinderConstructionData InnerCylinderData(
			InnerRadius, 110.0f, NumSegments, 2, Opaque, Transparent);
		AGX_MeshUtilities::CylinderConstructionData OuterCylinderData(
			OuterRadius, 50.0f, NumSegments, 1, Opaque, Opaque);

		// Allocate buffer sizes.

		uint32 NumVertices = 0;
		uint32 NumIndices = 0;

		DiskArrayData.AppendBufferSizes(NumVertices, NumIndices);
		InnerCylinderData.AppendBufferSizes(NumVertices, NumIndices);
		OuterCylinderData.AppendBufferSizes(NumVertices, NumIndices);

		Geometry->VertexBuffers.PositionVertexBuffer.Init(NumVertices);
		Geometry->VertexBuffers.StaticMeshVertexBuffer.Init(
			NumVertices, /*NumTexCoordsPerVertex*/ 1);
		Geometry->VertexBuffers.ColorVertexBuffer.Init(NumVertices);
		Geometry->IndexBuffer.Indices.AddUninitialized(NumIndices);

		// Create Sections and Fill Geometry buffers.

		uint32 NumAddedVertices = 0;
		uint32 NumAddedIndices = 0;

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(
				Geometry, Component->GetInnerDisksMaterial(), true, FMatrix::Identity,
				GetDepthPriority(), NumAddedIndices, NumAddedIndices + DiskArrayData.Indices));

			AGX_MeshUtilities::MakeDiskArray(
				Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices, NumAddedIndices,
				DiskArrayData);
		}

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(
				Geometry, Component->GetInnerShellMaterial(), true, FMatrix::Identity,
				GetDepthPriority(), NumAddedIndices, NumAddedIndices + InnerCylinderData.Indices));

			AGX_MeshUtilities::MakeCylinder(
				Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices, NumAddedIndices,
				InnerCylinderData);
		}

		{
			Sections.Add(MakeShared<FAGX_ConstraintIconGraphicsSection>(
				Geometry, Component->GetOuterShellMaterial(), true, FMatrix::Identity,
				GetDepthPriority(), NumAddedIndices, NumAddedIndices + OuterCylinderData.Indices));

			AGX_MeshUtilities::MakeCylinder(
				Geometry->VertexBuffers, Geometry->IndexBuffer, NumAddedVertices, NumAddedIndices,
				OuterCylinderData);
		}

		Geometries.Add(Geometry);
	}

	virtual void GetDynamicMeshElements(
		const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
		uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_AGX_ConstraintIconGraphics_GetDynamicMeshElements);

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f));

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		for (const TSharedPtr<FAGX_ConstraintIconGraphicsSection>& Section : Sections)
		{
			FMaterialRenderProxy* MaterialProxy =
				bWireframe ? WireframeMaterialInstance : Section->Material->GetRenderProxy();

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
					Mesh.bSelectable = true;
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

					const UAGX_Simulation* Simulation = GetDefault<UAGX_Simulation>();

					// Note: the ScaleMax as calculated below is a bit arbitrary, but was chosen to
					// match default behavior for the default
					// ConstraintVisualizationScalingDistanceMax value. It will scaled linearly with
					// it which turns out to work nicely.
					const float ScaleMax =
						Simulation->ConstraintVisualizationScalingDistanceMax * 0.25f;

					FMatrix ScreenScale = GetScreenSpaceScale(
						0.2f, 30.0f, ScaleMax, 200.0f, WorldMatrix.GetOrigin(), View);

					/// todo ScreenScale does not seem to have effect in-game. Must have missed
					/// something...

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
						Collector.GetRHICommandList(),
						EffectiveLocalToWorld, EffectiveLocalToWorld, GetBounds(), GetLocalBounds(),
						true, false, Unknown);
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
		UAGX_ConstraintIconGraphicsComponent* Component, uint32 SectionIndex)
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
		const bool bVisibleForSelection =
			IsSelected() && !Constraint->IsSelected() && Constraint->IsVisible();

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
	TArray<TSharedPtr<FAGX_ConstraintIconGraphicsGeometry>> Geometries;
	TArray<TSharedPtr<FAGX_ConstraintIconGraphicsSection>> Sections;
	FMaterialRelevance MaterialRelevance;
	const bool bDrawOnlyIfUnselected = true;
	UAGX_ConstraintComponent* Constraint;
	const EDofFlag LockedDofs;
	FMatrix FrameTransform1; // global transforms
	FMatrix FrameTransform2;
};

UAGX_ConstraintIconGraphicsComponent::UAGX_ConstraintIconGraphicsComponent(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, OuterShellMaterialIndex(-1)
	, InnerShellMaterialIndex(-1)
	, InnerDisksMaterialIndex(-1)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);

	bCastHiddenShadow = false;

	if (USceneComponent* Parent = GetAttachParent())
	{
		// Enabling the SceneComponents's white blob for constraint types with no visualization yet.
		/// \todo Set bVisualizeComponent to false when these constraints' graphics are implemented
		/// by in the proxy.
#if WITH_EDITORONLY_DATA
		bVisualizeComponent = Parent->IsA<UAGX_DistanceConstraintComponent>() ||
							  Parent->IsA<UAGX_HingeConstraintComponent>() ||
							  Parent->IsA<UAGX_LockConstraintComponent>();
#endif
	}

	// Find materials.
	{
		static ConstructorHelpers::FObjectFinder<UMaterialInterface> OuterShellMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintIconGraphics_OuterShell"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> InnerShellMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintIconGraphics_InnerShell"));

		static ConstructorHelpers::FObjectFinder<UMaterialInterface> InnerDisksMaterialFinder(
			TEXT("/AGXUnreal/Runtime/Materials/M_ConstraintIconGraphics_InnerDisks"));

		UMaterialInterface* FallbackMaterial = UMaterial::GetDefaultMaterial(MD_Surface);

		UMaterialInterface* OuterShellMaterial =
			OuterShellMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(OuterShellMaterialFinder.Object)
				: FallbackMaterial;

		UMaterialInterface* InnerShellMaterial =
			InnerShellMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(InnerShellMaterialFinder.Object)
				: FallbackMaterial;

		UMaterialInterface* InnerDisksMaterial =
			InnerDisksMaterialFinder.Succeeded()
				? Cast<UMaterialInterface>(InnerDisksMaterialFinder.Object)
				: FallbackMaterial;

		OuterShellMaterialIndex = GetNumMaterials();
		SetMaterial(OuterShellMaterialIndex, OuterShellMaterial);

		InnerShellMaterialIndex = GetNumMaterials();
		SetMaterial(InnerShellMaterialIndex, InnerShellMaterial);

		InnerDisksMaterialIndex = GetNumMaterials();
		SetMaterial(InnerDisksMaterialIndex, InnerDisksMaterial);
	}
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetOuterShellMaterial() const
{
	return GetMaterial(OuterShellMaterialIndex);
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetInnerShellMaterial() const
{
	return GetMaterial(InnerShellMaterialIndex);
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetInnerDisksMaterial() const
{
	return GetMaterial(InnerDisksMaterialIndex);
}

void UAGX_ConstraintIconGraphicsComponent::OnBecameSelected()
{
	MarkRenderTransformDirty();
	MarkRenderDynamicDataDirty();
}

FPrimitiveSceneProxy* UAGX_ConstraintIconGraphicsComponent::CreateSceneProxy()
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
	return new FAGX_ConstraintIconGraphicsProxy(this);
}

int32 UAGX_ConstraintIconGraphicsComponent::GetNumMaterials() const
{
	return GetNumOverrideMaterials();
}

UMaterialInterface* UAGX_ConstraintIconGraphicsComponent::GetMaterial(int32 ElementIndex) const
{
	return Super::GetMaterial(ElementIndex);
}

void UAGX_ConstraintIconGraphicsComponent::GetUsedMaterials(
	TArray<UMaterialInterface*>& OutMaterials, bool bGetDebugMaterials) const
{
	return Super::GetUsedMaterials(OutMaterials, bGetDebugMaterials);
}

FBoxSphereBounds UAGX_ConstraintIconGraphicsComponent::CalcBounds(
	const FTransform& LocalToWorld) const
{
	/// \todo Make more precise!

	FBoxSphereBounds NewBounds;
	NewBounds.BoxExtent = FVector(100.0f, 100.0f, 100.0f);
	NewBounds.Origin = LocalToWorld.GetLocation();
	NewBounds.SphereRadius = 150.0f;

	return NewBounds;
}

void UAGX_ConstraintIconGraphicsComponent::SendRenderDynamicData_Concurrent()
{
	Super::SendRenderDynamicData_Concurrent();

	/// \note Not using this data anymore. Instead we set render matrix directly using the frame
	/// (see GetRenderMatrix).

	// Update transform of the proxy to match the constraint attachment frame, if out-of-date!
	if (SceneProxy && Constraint && IsOwnerSelected())
	{
		FMatrix Frame1 = Constraint->BodyAttachment1.GetGlobalFrameMatrix();
		FMatrix Frame2 = Constraint->BodyAttachment2.GetGlobalFrameMatrix();

		FAGX_ConstraintIconGraphicsProxy* CastProxy =
			static_cast<FAGX_ConstraintIconGraphicsProxy*>(SceneProxy);
		ENQUEUE_RENDER_COMMAND(FSendConstraintIconGraphicsDynamicData)
		([CastProxy, Frame1, Frame2](FRHICommandListImmediate& RHICmdList)
		 { CastProxy->SetAttachmentFrameTransforms(Frame1, Frame2); });
	}
}

FMatrix UAGX_ConstraintIconGraphicsComponent::GetRenderMatrix() const
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
