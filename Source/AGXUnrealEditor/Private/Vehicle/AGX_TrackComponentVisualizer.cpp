// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "AGX_RuntimeStyle.h"
#include "Vehicle/AGX_TrackComponent.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Materials/Material.h"
#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
#include "Materials/MaterialRenderProxy.h"
#endif
#include "SceneManagement.h"

// \note Material proxys are used below for drawing solids. If the material proxy handling causes
//       any problems you can turn them off by removing this define.
#define DRAW_SOLID_VISUALIZERS

namespace
{
	template <typename ObjClass>
	static FORCEINLINE ObjClass* LoadObjFromPath(const FName& Path)
	{
		if (Path == NAME_None)
			return nullptr;

		return Cast<ObjClass>(StaticLoadObject(ObjClass::StaticClass(), nullptr, *Path.ToString()));
	}

	static FORCEINLINE UMaterial* LoadMaterialFromPath(const FName& Path)
	{
		if (Path == NAME_None)
			return nullptr;

		return LoadObjFromPath<UMaterial>(Path);
	}

	void DrawRigidBodyFrames(
		const TArray<FTrackBarrier::FVectorAndRotator>& BodyTransforms, const FSceneView* View,
		FPrimitiveDrawInterface* PDI)
	{
		for (const FTrackBarrier::FVectorAndRotator& Transform : BodyTransforms)
		{
			// Get position and rotation.
			const auto& Position = std::get<0>(Transform);
			const auto& Rotation = std::get<1>(Transform);

			// Draw x, y, z axes.
			const float Scale {20.0f};
			const uint8 DepthPriority {100};
			const float Thickness {1.3F};
			DrawCoordinateSystem(PDI, Position, Rotation, Scale, DepthPriority, Thickness);
		}
	}

	void DrawHingeAxes(
		const TArray<FTrackBarrier::FVectorAndRotator>& HingeTransforms, const FSceneView* View,
		FPrimitiveDrawInterface* PDI)
	{
		// Unreal draws directional arrow along local X-axis, but AGX Dynamic's hinge axis
		// is the local z-axis. Therefore we need to rotate -90 degrees around the local y-axis.
		FRotationMatrix XToZAxisRot(FRotator(-90.0f, 0, 0));

		for (const FTrackBarrier::FVectorAndRotator& Transform : HingeTransforms)
		{
			// Get position and rotation.
			const auto& Position = std::get<0>(Transform);
			const auto& Rotation = std::get<1>(Transform);

			// Draw arrow.
			const float Length {30.0f};
			const float ArrowSize {3.0f};
			const uint8 DepthPriority {101};
			const float Thickness {0.8f};
			DrawDirectionalArrow(
				PDI, XToZAxisRot * FRotationTranslationMatrix(Rotation, Position),
				FLinearColor::White, Length, ArrowSize, DepthPriority, Thickness);
		}
	}

	void DrawMassCenters(
		const TArray<FVector>& MassCenters, FMaterialRenderProxy*& MaterialProxy,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		FLinearColor Magenta(1.0, 0.0, 1.0);

#ifdef DRAW_SOLID_VISUALIZERS
		// Create the material proxy if not yet created.
		if (MaterialProxy == nullptr)
		{
			MaterialProxy =
				new FColoredMaterialRenderProxy(GEngine->GeomMaterial->GetRenderProxy(), Magenta);
		}
#endif

		for (const FVector& Position : MassCenters)
		{
#ifdef DRAW_SOLID_VISUALIZERS
			// Draw solid sphere.
			// \todo Is there an easier or better way to draw a solid sphere from a component
			// visualizer?
			const FVector Radii(3.0f);
			const int32 NumSides {4};
			const int32 NumRings {4};
			const uint8 DepthPriority {102};
			DrawSphere(
				PDI, Position, FRotator::ZeroRotator, Radii, NumSides, NumRings, MaterialProxy,
				DepthPriority);
#else
			// Draw a thick-lined wire diamond that will appear as a solid world-space sized dot.
			const float Size {2.5f};
			const uint8 DepthPriority {102};
			const float Thickness {2.5f};
			DrawWireDiamond(
				PDI, FTranslationMatrix(Position), Size, Magenta, DepthPriority, Thickness);
#endif
		}
	}

	void DrawCollisionBoxes(
		const TArray<FTrackBarrier::FVectorRotatorRadii>& CollisionBoxes,
		bool bColorizeMergedBodies, const TArray<FLinearColor>& BodyColors,
		FMaterialRenderProxy*& CommonMaterialProxy,
		TMap<FLinearColor, FMaterialRenderProxy*>& MaterialProxyPerBodyColor,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		int i = 0;
		for (const auto& Box : CollisionBoxes)
		{
			// Get position, rotation, size.
			const auto& Position = std::get<0>(Box);
			const auto& Rotation = std::get<1>(Box);
			const auto& Radii = std::get<2>(Box);

#ifdef DRAW_SOLID_VISUALIZERS
			// Get or create material proxy.
			FMaterialRenderProxy* BoxMaterialProxy = nullptr;
			if (bColorizeMergedBodies && i < BodyColors.Num())
			{
				// Merged body dependent color.
				FLinearColor BoxColor = BodyColors[i];
				if (FMaterialRenderProxy** FoundProxy = MaterialProxyPerBodyColor.Find(BoxColor))
				{
					BoxMaterialProxy = *FoundProxy;
				}
				else
				{
					BoxMaterialProxy = new FColoredMaterialRenderProxy(
						GEngine->GeomMaterial->GetRenderProxy(), BoxColor);
					MaterialProxyPerBodyColor.Add(BoxColor, BoxMaterialProxy);
				}
			}
			else
			{
				// Common box color.
				if (CommonMaterialProxy == nullptr)
				{
					const FLinearColor TransparentGray(0, 0, 0, 0.3f);
					CommonMaterialProxy = new FColoredMaterialRenderProxy(
						GEngine->GeomMaterial->GetRenderProxy(), TransparentGray);
				}
				BoxMaterialProxy = CommonMaterialProxy;
			}
			check(BoxMaterialProxy);

			// Draw transparent solid box.
			// \todo Is there an easier or better way to draw a solid transparent box from a
			// component visualizer?
			DrawBox(
				PDI, FRotationTranslationMatrix(Rotation, Position), Radii, BoxMaterialProxy,
				SDPG_World);
#endif

			// Draw wire box.
			FLinearColor LineColor = FLinearColor(0, 0, 0, 0.2f); // black
			// \warning Too thick lines cause box too look larger than actual size
			float LineThickness = 0.4f;
#ifndef DRAW_SOLID_VISUALIZERS
			// Colorize the line, because there is no solid to show color.
			if (bColorizeMergedBodies && i < BodyColors.Num())
			{
				LineColor = BodyColors[i];
				LineThickness = 1.0f;
			}
#endif
			DrawWireBox(
				PDI, FRotationTranslationMatrix(Rotation, Position), FBox(-Radii, Radii), LineColor,
				103, LineThickness, 0, false);
			++i;
		}
	}

	void DrawTrackWheel(
		const FVector& Position, const FQuat& Rotation, float Radius, float Depth,
		FLinearColor Color, const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		if (Radius <= 0.f)
		{
			return;
		}

		const float HalfDepth = 0.5f * Depth;
		const int32 NumSides =
			FMath::GetMappedRangeValueClamped(TRange<float>(10, 60), TRange<float>(16, 64), Radius);

		// The rotation axis of an AGX Dynamics wheel is the y-axis (-Y in Unreal), but Unreal
		// draws an wire cylinder extended along the z-axis. Therefore, rotate around the X-axis
		// in order to align Z-axis with negative Y-axis (rotation done by swapping the axes below).

		// Draw wheel cylinder.
		DrawWireCylinder(
			PDI, Position, Rotation.GetAxisX(), Rotation.GetAxisZ(), -Rotation.GetAxisY(), Color,
			Radius, HalfDepth, NumSides, SDPG_Foreground);

		// Unreal draws directional arrow along local X-axis, but AGX Dynamic's wheel axis is the
		// local y-axis (-Y in Unreal). Therefore we need to rotate -90 degrees around the local
		// z-axis.
		FQuat ArrowRotation = Rotation * FQuat(FVector::UpVector, -HALF_PI);

		// Draw rotation axis.
		DrawDirectionalArrow(
			PDI, FQuatRotationTranslationMatrix(ArrowRotation, Position), Color, 0.6f * Depth,
			/*ArrowSize*/ 4.0f, SDPG_Foreground, /*Thickness*/ 1.5f);
	}

	void DrawTrackWheels(
		const TArray<FTrackBarrier::FVectorQuatRadius>& WheelTransforms,
		TArray<FLinearColor>& WheelColors, float WheelDepth, const FSceneView* View,
		FPrimitiveDrawInterface* PDI)
	{
		for (int32 I = 0; I < WheelTransforms.Num(); ++I)
		{
			// Get position, rotation, radius.
			const auto& Wheel = WheelTransforms[I];
			const auto& Position = std::get<0>(Wheel);
			const auto& Rotation = std::get<1>(Wheel);
			const auto& Radius = std::get<2>(Wheel);

			// Get color.
			const auto& Color = WheelColors[I];

			DrawTrackWheel(Position, Rotation, Radius, WheelDepth, Color, View, PDI);
		}
	}

	/**
	 * Get Track Preview Data from the Track Component, and convert it to formats
	 * that can be directly passed to our draw functions.
	 */
	void GetDebugDataFromTrackPreview(
		const UAGX_TrackComponent* TrackComponent,
		TArray<FTrackBarrier::FVectorAndRotator>* BodyTransforms,
		TArray<FTrackBarrier::FVectorRotatorRadii>* CollisionBoxes,
		TArray<FTrackBarrier::FVectorQuatRadius>* WheelTransforms,
		TArray<FLinearColor>* WheelColors)
	{
		check(TrackComponent);

		const FAGX_TrackPreviewData* Preview = TrackComponent->GetTrackPreview();

		// \todo Consider letting AGX_TrackComponent.cpp generate wheels preview data instead,
		//       along with the TrackPreviewData.

		// \todo Currently not detecting whether a Rigid Body Component or Frame Defining
		// Component has been moved, and marking Track Preview Data for update if so. In such
		// scenario the user currently has to manually update the preview data by clicking the
		// Update Preview button in the Details Panel of the Track Component.

		if (WheelTransforms != nullptr)
		{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			WheelTransforms->SetNum(TrackComponent->Wheels.Num(), /*bAllowShrinking*/ false);
#else
			WheelTransforms->SetNum(TrackComponent->Wheels.Num(), EAllowShrinking::No);
#endif
		}
		if (WheelColors != nullptr)
		{
#if UE_VERSION_OLDER_THAN(5, 5, 0)
			WheelColors->SetNum(TrackComponent->Wheels.Num(), false);
#else
			WheelColors->SetNum(TrackComponent->Wheels.Num(), EAllowShrinking::No);
#endif
		}

		for (int I = 0; I != TrackComponent->Wheels.Num(); ++I)
		{
			const FAGX_TrackWheel& Wheel = TrackComponent->Wheels[I];

			if (WheelTransforms != nullptr)
			{
				// \todo Can we cache rigid body even if not playing?
				UAGX_RigidBodyComponent* RigidBody = Wheel.RigidBody.GetRigidBody();
				FVector RelPos;
				FQuat RelRot;
				if (!RigidBody || !Wheel.GetTransformRelativeToBody(RelPos, RelRot))
				{
					continue;
				}
				FVector Pos = RigidBody->GetComponentTransform().TransformPositionNoScale(RelPos);
				FQuat Rot = RigidBody->GetComponentTransform().TransformRotation(RelRot);

				std::get<0>((*WheelTransforms)[I]) = Pos;
				std::get<1>((*WheelTransforms)[I]) = Rot;
				std::get<2>((*WheelTransforms)[I]) = Wheel.Radius;
			}

			if (WheelColors != nullptr)
			{
				switch (Wheel.Model)
				{
					case EAGX_TrackWheelModel::Sprocket:
						(*WheelColors)[I] = FLinearColor::Red;
						break;
					case EAGX_TrackWheelModel::Idler:
						(*WheelColors)[I] = FLinearColor::Blue;
						break;
					case EAGX_TrackWheelModel::Roller:
						(*WheelColors)[I] = FLinearColor::Green;
						break;
					default:
						(*WheelColors)[I] = FLinearColor::White;
				}
			}
		}

		if (BodyTransforms != nullptr)
		{
			if (Preview == nullptr)
			{
				BodyTransforms->SetNum(0);
			}
			else
			{
				BodyTransforms->SetNum(Preview->NodeTransforms.Num());

				for (int i = 0; i < Preview->NodeTransforms.Num(); ++i)
				{
					// Position
					std::get<0>((*BodyTransforms)[i]) = Preview->NodeTransforms[i].GetLocation();
					// Rotation
					std::get<1>((*BodyTransforms)[i]) =
						Preview->NodeTransforms[i].GetRotation().Rotator();
				}
			}
		}

		if (CollisionBoxes != nullptr)
		{
			if (Preview == nullptr)
			{
				CollisionBoxes->SetNum(0);
			}
			else
			{
				check(Preview->NodeTransforms.Num() == Preview->NodeHalfExtents.Num());

				CollisionBoxes->SetNum(Preview->NodeTransforms.Num());

				for (int i = 0; i < Preview->NodeTransforms.Num(); ++i)
				{
					FVector BodyFrameToBoxCenter =
						Preview->NodeTransforms[i].GetRotation().GetAxisZ() *
						Preview->NodeHalfExtents[i].Z;

					// Position
					std::get<0>((*CollisionBoxes)[i]) =
						Preview->NodeTransforms[i].GetLocation() + BodyFrameToBoxCenter;
					// Rotation
					std::get<1>((*CollisionBoxes)[i]) =
						Preview->NodeTransforms[i].GetRotation().Rotator();
					// Half Size
					std::get<2>((*CollisionBoxes)[i]) = Preview->NodeHalfExtents[i];
				}
			}
		}
	}
}

FAGX_TrackComponentVisualizer::FAGX_TrackComponentVisualizer()
	: MassCenterMaterialProxy(nullptr)
	, CollisionBoxMaterialProxy(nullptr)
{
}

FAGX_TrackComponentVisualizer::~FAGX_TrackComponentVisualizer()
{
	if (CollisionBoxMaterialProxy != nullptr)
	{
		delete CollisionBoxMaterialProxy;
		CollisionBoxMaterialProxy = nullptr;
	}

	if (MassCenterMaterialProxy != nullptr)
	{
		delete MassCenterMaterialProxy;
		MassCenterMaterialProxy = nullptr;
	}

	for (auto& entry : CollisionBoxMaterialProxies)
	{
		delete entry.Value;
		entry.Value = nullptr;
	}
}

void FAGX_TrackComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_TrackComponent* TrackComponent = Cast<const UAGX_TrackComponent>(Component);

	// \todo It appears the same ComponentVisualizer is used for multiple UAGX_TrackComponents.
	// We should probably adjust our cache management to reflect this!
	// Though, in reality it is not THAT bad with current implementation, because the main purpose
	// of the cache is so avoid re-allocation of the container, not to remember its actual element
	// values (which are always overwritten anyway). Because the size (node count) is usually the
	// same for multiple tracks (i.e. left and right track) the cached container can be reused
	// without any re-allocation occuring.

	if (!IsValid(TrackComponent) || !TrackComponent->bEnabled ||
		!TrackComponent->bShowEditorDebugGraphics)
	{
		return;
	}

	// \todo Is there a better way to determine if we are in the BP Actor Editor?
	const bool bIsInBlueprintEditor =
		TrackComponent->GetOutermost()->GetName().Equals("/Engine/Transient");

	// By default DrawVisualization is called when the component's Actor is selected in the Level
	// Editor or when the Component is selected in the BP Actor Editor. If there's many constraints
	// etc in the same actor the Level Editor's viewport gets very messy. Therefore, with the
	// following boolean we only draw the visualization if the component is selected in the Level
	// Editor, not the actor.
	const bool bDrawVisualizer = TrackComponent->IsSelectedInEditor() || bIsInBlueprintEditor;
	if (!bDrawVisualizer)
	{
		return;
	}

	if (TrackComponent->GetWorld() && TrackComponent->GetWorld()->IsGameWorld())
	{
		// Draw in-game visualization using AGX Dynamics native data.
		if (!TrackComponent->HasNative())
		{
			return;
		}

		const FTrackBarrier* TrackBarrier = TrackComponent->GetNative();
		check(TrackBarrier);

		TrackBarrier->GetDebugData(
			&BodyTransformsCache, &HingeTransformsCache, &MassCentersCache, &CollisionBoxesCache,
			&BodyColorsCache, &WheelTransformsCache, &WheelColorsCache);

		// Draw rigid body frames.
		DrawRigidBodyFrames(BodyTransformsCache, View, PDI);

		// Draw hinge axes
		DrawHingeAxes(HingeTransformsCache, View, PDI);

		// Draw mass centers
		DrawMassCenters(MassCentersCache, MassCenterMaterialProxy, View, PDI);

		// Draw collision boxes
		DrawCollisionBoxes(
			CollisionBoxesCache, TrackComponent->bColorizeMergedBodies, BodyColorsCache,
			CollisionBoxMaterialProxy, CollisionBoxMaterialProxies, View, PDI);

		// Draw track wheels.
		DrawTrackWheels(
			WheelTransformsCache, WheelColorsCache, 0.6f * TrackComponent->Width, View, PDI);
	}
	else
	{
		// Draw outside-game visualization using preview data.

		// Get rendering data from Track Preview Data.
		GetDebugDataFromTrackPreview(
			TrackComponent, &BodyTransformsCache, &CollisionBoxesCache, &WheelTransformsCache,
			&WheelColorsCache);

		// Draw rigid body frames.
		DrawRigidBodyFrames(BodyTransformsCache, View, PDI);

		// Draw collision boxes
		DrawCollisionBoxes(
			CollisionBoxesCache, TrackComponent->bColorizeMergedBodies, BodyColorsCache,
			CollisionBoxMaterialProxy, CollisionBoxMaterialProxies, View, PDI);

		// Draw track wheels.
		DrawTrackWheels(
			WheelTransformsCache, WheelColorsCache, 0.6f * TrackComponent->Width, View, PDI);
	}
}

#undef DRAW_SOLID_VISUALIZERS
