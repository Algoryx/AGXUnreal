#include "AGX_VectorComponent.h"

// AGXUnreal includes.
#include "Utilities/AGX_MeshUtilities.h"

// Unreal Engine includes.
#include "DynamicMeshBuilder.h"
#include "Engine/Engine.h"
#include "LocalVertexFactory.h"
#include "Materials/Material.h"
#include "PrimitiveSceneProxy.h"
#include "StaticMeshResources.h"
#include "Misc/EngineVersionComparison.h"

namespace
{
	constexpr float DEFAULT_SCREEN_SIZE {0.0025f};
	constexpr float ARROW_RADIUS_FACTOR {0.03f};
	constexpr float ARROW_HEAD_FACTOR {0.2f};
	constexpr float ARROW_HEAD_ANGLE {20.f};

	/**
	 * Represents an UAGX_VectorComponent to the scene manager.
	 */
	class FArrowSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FArrowSceneProxy(UAGX_VectorComponent* Component)
			: FPrimitiveSceneProxy(Component)
			, VertexFactory(GetScene().GetFeatureLevel(), "FArrowSceneProxy")
			, ArrowColor(Component->ArrowColor)
			, ArrowSize(Component->ArrowSize)
		{
			bWillEverBeLit = false;

			const float HeadAngle = FMath::DegreesToRadians(ARROW_HEAD_ANGLE);
			const float TotalLength = ArrowSize;
			const float HeadLength = TotalLength * ARROW_HEAD_FACTOR;
			const float ShaftRadius = TotalLength * ARROW_RADIUS_FACTOR;
			const float ShaftLength =
				(TotalLength - HeadLength) * 1.1f; // 10% overlap between shaft and head
			const FVector ShaftCenter = FVector(0.5f * ShaftLength, 0, 0);

			TArray<FDynamicMeshVertex> OutVerts;
			const uint32 NumConeSides {32u};
			const uint32 NumCylinderSides {16u};
			AGX_MeshUtilities::MakeCone(
				HeadAngle, HeadAngle, -HeadLength, TotalLength, NumConeSides, OutVerts,
				IndexBuffer.Indices);
			AGX_MeshUtilities::MakeCylinder(
				ShaftCenter, FVector(0, 0, 1), FVector(0, 1, 0), FVector(1, 0, 0), ShaftRadius,
				0.5f * ShaftLength, NumCylinderSides, OutVerts, IndexBuffer.Indices);

			VertexBuffers.InitFromDynamicVertex(&VertexFactory, OutVerts);

			// Enqueue initialization of render resource
			BeginInitResource(&IndexBuffer);
		}

		virtual ~FArrowSceneProxy()
		{
			VertexBuffers.PositionVertexBuffer.ReleaseResource();
			VertexBuffers.StaticMeshVertexBuffer.ReleaseResource();
			VertexBuffers.ColorVertexBuffer.ReleaseResource();
			IndexBuffer.ReleaseResource();
			VertexFactory.ReleaseResource();
		}

		virtual void GetDynamicMeshElements(
			const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily,
			uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			FMatrix EffectiveLocalToWorld = GetLocalToWorld();

			auto ArrowMaterialRenderProxy = new FColoredMaterialRenderProxy(
				GEngine->ArrowMaterial->GetRenderProxy(), ArrowColor, "GizmoColor");

			Collector.RegisterOneFrameMaterialProxy(ArrowMaterialRenderProxy);

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					// Calculate the view-dependent scaling factor.
					/// \todo ViewScale not ported from prototype project yet.
					float ViewScale = 1.0f;

					// Draw the mesh.
					FMeshBatch& Mesh = Collector.AllocateMesh();
					FMeshBatchElement& BatchElement = Mesh.Elements[0];
					BatchElement.IndexBuffer = &IndexBuffer;
					Mesh.bWireframe = false;
					Mesh.VertexFactory = &VertexFactory;
					Mesh.MaterialRenderProxy = ArrowMaterialRenderProxy;

					FDynamicPrimitiveUniformBuffer& DynamicPrimitiveUniformBuffer =
						Collector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();

#if UE_VERSION_OLDER_THAN(4, 23, 0)
					DynamicPrimitiveUniformBuffer.Set(
						FScaleMatrix(ViewScale) * EffectiveLocalToWorld,
						FScaleMatrix(ViewScale) * EffectiveLocalToWorld, GetBounds(),
						GetLocalBounds(), true, false, UseEditorDepthTest());
#else
					DynamicPrimitiveUniformBuffer.Set(
						FScaleMatrix(ViewScale) * EffectiveLocalToWorld,
						FScaleMatrix(ViewScale) * EffectiveLocalToWorld, GetBounds(),
						GetLocalBounds(), true, false, DrawsVelocity(), LpvBiasMultiplier);
#endif

					BatchElement.PrimitiveUniformBufferResource =
						&DynamicPrimitiveUniformBuffer.UniformBuffer;

					BatchElement.FirstIndex = 0;
					BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
					BatchElement.MinVertexIndex = 0;
					BatchElement.MaxVertexIndex =
						VertexBuffers.PositionVertexBuffer.GetNumVertices() - 1;
					Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
					Mesh.Type = PT_TriangleList;
					Mesh.DepthPriorityGroup = SDPG_World;
					Mesh.bCanApplyViewModeOverrides = false;
					Collector.AddMesh(ViewIndex, Mesh);
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance =
				IsShown(View) && (View->Family->EngineShowFlags.BillboardSprites);
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			Result.bVelocityRelevance =
				IsMovable() && Result.bOpaqueRelevance && Result.bRenderInMainPass;
			return Result;
		}

		virtual void OnTransformChanged() override
		{
			Origin = GetLocalToWorld().GetOrigin();
		}

		virtual uint32 GetMemoryFootprint(void) const override
		{
			return sizeof(*this) + GetAllocatedSize();
		}

		uint32 GetAllocatedSize() const
		{
			return FPrimitiveSceneProxy::GetAllocatedSize();
		}

	private:
		FStaticMeshVertexBuffers VertexBuffers;
		FDynamicMeshIndexBuffer32 IndexBuffer;
		FLocalVertexFactory VertexFactory;

		FVector Origin;
		FColor ArrowColor;
		float ArrowSize;
	};
}

FVector UAGX_VectorComponent::GetVectorDirection() const
{
	FVector localDirection = FVector::ForwardVector * ArrowSize * RelativeScale3D;
	return GetComponentQuat().RotateVector(localDirection);
}

FVector UAGX_VectorComponent::GetVectorDirectionNormalized() const
{
	FVector localDirection = FVector::ForwardVector;
	return GetComponentQuat().RotateVector(localDirection).GetSafeNormal();
}

FVector UAGX_VectorComponent::GetVectorOrigin() const
{
	return GetComponentLocation();
}

FVector UAGX_VectorComponent::GetVectorTarget() const
{
	return GetVectorOrigin() + GetVectorDirection();
}

FTwoVectors UAGX_VectorComponent::GetInLocal(FTransform const& WorldToLocal) const
{
	FVector LocalOrigin = WorldToLocal.TransformPosition(GetVectorOrigin());
	FVector LocalTarget = WorldToLocal.TransformPosition(GetVectorTarget());
	return {LocalOrigin, LocalTarget};
}

FPrimitiveSceneProxy* UAGX_VectorComponent::CreateSceneProxy()
{
	return new FArrowSceneProxy(this);
}

FBoxSphereBounds UAGX_VectorComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	float Radius = ArrowSize * ARROW_RADIUS_FACTOR;
	return FBoxSphereBounds(
			   FBox(FVector(0.0f, -Radius, -Radius), FVector(ArrowSize, Radius, Radius)))
		.TransformBy(LocalToWorld);
}