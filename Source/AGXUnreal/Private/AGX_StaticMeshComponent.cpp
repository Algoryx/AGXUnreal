// Copyright 2025, Algoryx Simulation AB.

#include "AGX_StaticMeshComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engien includes.
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "PhysicsEngine/BodySetup.h"
#include "PhysicsEngine/AggregateGeom.h"

UAGX_StaticMeshComponent::UAGX_StaticMeshComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	MotionControl = EAGX_MotionControl::MC_DYNAMICS;
}

void UAGX_StaticMeshComponent::SetVelocity(FVector InVelocity)
{
	if (HasNative())
	{
		NativeBarrier.SetVelocity(InVelocity);
	}

	Velocity = InVelocity;
}

FVector UAGX_StaticMeshComponent::GetVelocity() const
{
	if (HasNative())
	{
		return NativeBarrier.GetVelocity();
	}

	return Velocity;
}

void UAGX_StaticMeshComponent::SetMotionControl(
	TEnumAsByte<enum EAGX_MotionControl> InMotionControl)
{
	MotionControl = InMotionControl;
	if (HasNative())
	{
		NativeBarrier.SetMotionControl(MotionControl);
	}
}

TEnumAsByte<enum EAGX_MotionControl> UAGX_StaticMeshComponent::GetMotionControl() const
{
	if (HasNative())
	{
		return NativeBarrier.GetMotionControl();
	}
	else
	{
		return MotionControl;
	}
}

bool UAGX_StaticMeshComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_StaticMeshComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_StaticMeshComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

FRigidBodyBarrier* UAGX_StaticMeshComponent::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

const FRigidBodyBarrier* UAGX_StaticMeshComponent::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &NativeBarrier;
}

FRigidBodyBarrier* UAGX_StaticMeshComponent::GetOrCreateNative()
{
	if (!HasNative())
	{
		if (GIsReconstructingBlueprintInstances)
		{
			// We're in a very bad situation. Someone need this Component's native but if we're in
			// the middle of a RerunConstructionScripts and this Component haven't been given its
			// Native yet then there isn't much we can do. We can't create a new one since we will
			// be given the actual Native soon, but we also can't return the actual Native right now
			// because it hasn't been restored from the Component Instance Data yet.
			//
			// For now we simply die in non-shipping (checkNoEntry is active) so unit tests will
			// detect this situation, and log error and return nullptr otherwise, so that the
			// application can at least keep running. It is unlikely that the simulation will behave
			// as intended.
			checkNoEntry();
			UE_LOG(
				LogAGX, Error,
				TEXT("A request for the AGX Dynamics instance for AGX Static Mesh '%s' in '%s' was "
					 "made but we are in the middle of a Blueprint Reconstruction and the "
					 "requested instance has not yet been restored. The instance cannot be "
					 "returned, which may lead to incorrect scene configuration."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return nullptr;
		}

		AllocateNative();
	}
	return GetNative();
}

void UAGX_StaticMeshComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
	{
		// Not allocating a new Native if currently reconstructing Blueprint instances because
		// if we should have a Native then one will be assigned to us by our Component Instance
		// Data.
		AllocateNative();
	}
}

void UAGX_StaticMeshComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (GIsReconstructingBlueprintInstances)
	{
		// Another Static Mesh will inherit this one's Native, so don't wreck it.
		// It's still safe to release the native since the Simulation will hold a reference if
		// necessary.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this))
		{
			Simulation->Remove(*this);
		}
	}

	if (HasNative())
	{
		GetNative()->ReleaseNative();
	}
}

void UAGX_StaticMeshComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ReadTransformFromNative();
	Velocity = NativeBarrier.GetVelocity();
}

TStructOnScope<FActorComponentInstanceData> UAGX_StaticMeshComponent::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

namespace AGX_StaticMeshComponent_helpers
{
// I would like to use RefreshCollisionShapes(PhysicsShapes, CollisionShapes) to
// reorder PhysicsShapes so that they match the new ordering of CollisionShapes.
// To do this I need a way to identify which of the new CollisionShapes each
// PhysicsShape belong to.
#if 0
	template <typename FCollisionShape>
	void RefreshCollisionShapes(
		TArray<FAGX_Shape>& PhysicsShapes, TArray<FCollisionShape>& CollisionShapes)
	{

	}
#endif
}

bool UAGX_StaticMeshComponent::ShouldCreatePhysicsState() const
{
	// Return true so that OnCreatePhysicsState is called when the underlying StaticMesh is changed.
	/**
	 * \note I'm not entirely sure on the consequences of doing this. I want to maintain my own
	 * physics state, which is the AGX Dynamics state. I do not want the PhysX code to start doing
	 * stuff because of this. And it's not even the actual AGX Dynamics state but the local state we
	 * keep in order to create the AGX Dynamics objects later, on BeginPlay.
	 *
	 * All I want is to keep the TArray<FAGX_Shape> containers in sync with the collision shapes
	 * stored in the StaticMesh asset.
	 */
	return true;
}

void UAGX_StaticMeshComponent::OnCreatePhysicsState()
{
	RefreshCollisionShapes();
	bPhysicsStateCreated = true;
}

#if WITH_EDITOR
void UAGX_StaticMeshComponent::PostInitProperties()
{
	Super::PostInitProperties();
	FAGX_PropertyChangedDispatcher<ThisClass>& Dispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (Dispatcher.IsInitialized())
	{
		return;
	}

	Dispatcher.Add(
		this->GetRelativeLocationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	Dispatcher.Add(
		this->GetRelativeRotationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	Dispatcher.Add(
		this->GetAbsoluteLocationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	Dispatcher.Add(
		this->GetAbsoluteRotationPropertyName(),
		[](ThisClass* This) { This->TryWriteTransformToNative(); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_StaticMeshComponent, Velocity),
		[](ThisClass* This) { This->SetVelocity(This->Velocity); });

	Dispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_StaticMeshComponent, MotionControl),
		[](ThisClass* This) { This->SetMotionControl(This->MotionControl); });

	Dispatcher.Add(
		GetMemberNameChecked_StaticMesh(),
		[](ThisClass* This)
		{
			// We have a new StaticMesh, replace the collision shapes for the old mesh with the  new
			// ones.
			/// \note This may not be necessary, it may be that OnCreatePhysicsState, which does the
			/// same work, is called in all cases where PostEditChangeProperty (this function) is
			/// called.
			This->RefreshCollisionShapes();
		});
}

void UAGX_StaticMeshComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_StaticMeshComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	if (!NativeBarrier.HasNative())
	{
		return;
	}

	// Not using the Set-functions here because we aren't editing a raw Property and don't want
	// to trigger a bunch of Unreal Engine code since we currently are in an Unreal Engine callback.
	// So go straight to the Barrier.
	TryWriteTransformToNative();
}
#endif

namespace AGX_StaticMeshComponent_helpers
{
	/**
	 * Swap all shape material assets for their material instance version.
	 * @param Shape The shape for which the material should be swapped.
	 * @param World The world in which material instances should be created.
	 */
	void SwapInMaterialInstance(FAGX_Shape& Shape, UWorld& World)
	{
		// No asset pointers should remain after a swap. If we fail to create an instance then we
		// should fall back to the default material, with a warning.
		UAGX_ShapeMaterial* Asset = Shape.Material;
		Shape.Material = nullptr;

		if (Asset == nullptr)
		{
			// If the asset material is nullptr then the instance material should also be nullptr,
			// AGX Dynamics will interpret that to mean the default material.
			return;
		}

		if (!World.IsGameWorld())
		{
			// Only create instances in game worlds, never in the editor world.
			return;
		}

		FShapeMaterialBarrier* Barrier = nullptr;
		UAGX_ShapeMaterial* Instance = Cast<UAGX_ShapeMaterial>(Asset->GetOrCreateInstance(&World));
		if (Instance == nullptr)
		{
			/// \todo Better error message.
			UE_LOG(LogAGX, Error, TEXT("Could not create a ShapeMaterialInstance."));
			return;
		}

		Shape.Material = Instance;
	}

	FAGX_Shape& GetShape(TArray<FAGX_Shape>& Shapes, int32 Index, FAGX_Shape& Default)
	{
		return Shapes.IsValidIndex(Index) ? Shapes[Index] : Default;
	}

	FVector GetHalfExtent(const FKBoxElem& Box)
	{
		return FVector(Box.X / 2.0f, Box.Y / 2.0f, Box.Z / 2.0f);
	}

	void CopyShapeData(FSphereShapeBarrier& Destination, const FKSphereElem& Source)
	{
		Destination.SetRadius(Source.Radius);
		Destination.SetLocalPosition(Source.Center);
		Destination.SetLocalRotation(FQuat::Identity);
	}

	void CopyShapeData(FBoxShapeBarrier& Destination, const FKBoxElem& Source)
	{
		Destination.SetHalfExtents(GetHalfExtent(Source));
		Destination.SetLocalPosition(Source.Center);
		Destination.SetLocalRotation(Source.Rotation.Quaternion());
	}

	void CopyShapeData(FCapsuleShapeBarrier& Destination, const FKSphylElem& Source)
	{
		Destination.SetRadius(Source.Radius);
		Destination.SetHeight(Source.Length);
		Destination.SetLocalPosition(Source.Center);
		// Unreal Engine Sphyls are aligned along Z while AGX Dynamics capsules are aligned along Y.
		// By rolling 90 degrees (tilt-right assuming X forward) we transform from one to the other.
		Destination.SetLocalRotation((FRotator(0.0f, 0.0f, 90.0f) + Source.Rotation).Quaternion());
	}

	template <typename FBarrier, typename FCollisionShape>
	void CreateNativeShape(
		FBarrier& Barrier, const FCollisionShape& Collision, const FAGX_Shape& AgxConfig,
		UWorld& World)
	{
		Barrier.AllocateNative();
		check(Barrier.HasNative());
		CopyShapeData(Barrier, Collision);
		Barrier.SetEnableCollisions(AgxConfig.bCanCollide);
		if (AgxConfig.Material != nullptr)
		{
			FShapeMaterialBarrier* MaterialBarrier =
				AgxConfig.Material->GetOrCreateShapeMaterialNative(&World);
			if (MaterialBarrier != nullptr)
			{
				Barrier.SetMaterial(*MaterialBarrier);
			}
		}
		Barrier.SetName(Collision.GetName().ToString());
		Barrier.AddCollisionGroups(AgxConfig.CollisionGroups);
	}
}

void UAGX_StaticMeshComponent::AllocateNative()
{
	using namespace AGX_StaticMeshComponent_helpers;

	if (GetWorld() == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot create AGX Dynamics representation of AGX_StaticMeshComponent '%s' "
				 "because no world is available."),
			*GetName());
		return;
	}
	UWorld& World = *GetWorld();

	/// \todo Replace with early-out once we're confident that things work the way they should.
	check(!GIsReconstructingBlueprintInstances);
	check(!NativeBarrier.HasNative());
	check(SphereBarriers.Num() == 0);
	check(BoxBarriers.Num() == 0);
	check(CapsuleBarriers.Num() == 0);

	RefreshCollisionShapes();
	NativeBarrier.AllocateNative();

	NativeBarrier.SetVelocity(Velocity);
	NativeBarrier.SetMotionControl(MotionControl);

	SwapInMaterialInstance(DefaultShape, World);
	for (auto& Sphere : Spheres)
	{
		SwapInMaterialInstance(Sphere, World);
	}
	for (auto& Box : Boxes)
	{
		SwapInMaterialInstance(Box, World);
	}
	for (auto& Capsule : Capsules)
	{
		SwapInMaterialInstance(Capsule, World);
	}

	if (GetStaticMesh() != nullptr)
	{
#if UE_VERSION_OLDER_THAN(4, 27, 0)
		FKAggregateGeom& CollisionShapes = GetStaticMesh()->BodySetup->AggGeom;
#else
		FKAggregateGeom& CollisionShapes = GetStaticMesh()->GetBodySetup()->AggGeom;
#endif

		// Copy sphere data from the collision spheres to the barrier spheres.
		TArray<FKSphereElem>& CollisionSpheres = CollisionShapes.SphereElems;
		SphereBarriers.Reserve(CollisionSpheres.Num());
		for (int32 I = 0; I < CollisionSpheres.Num(); ++I)
		{
			FKSphereElem& Collision = CollisionSpheres[I];
			FAGX_Shape& Shape = GetShape(Spheres, I, DefaultShape);
			FSphereShapeBarrier Barrier;
			CreateNativeShape(Barrier, Collision, Shape, World);
			NativeBarrier.AddShape(&Barrier);
			SphereBarriers.Add(std::move(Barrier));
		}

		// Copy box data from the collision boxes to the barrier boxes.
		TArray<FKBoxElem>& CollisionBoxes = CollisionShapes.BoxElems;
		BoxBarriers.Reserve(CollisionBoxes.Num());
		for (int32 I = 0; I < CollisionBoxes.Num(); ++I)
		{
			FKBoxElem& Collision = CollisionBoxes[I];
			FAGX_Shape& Shape = GetShape(Boxes, I, DefaultShape);
			FBoxShapeBarrier Barrier;
			CreateNativeShape(Barrier, Collision, Shape, World);
			NativeBarrier.AddShape(&Barrier);
			BoxBarriers.Add(std::move(Barrier));
		}

		// Copy capsule data from the collision sphyls to the barrier capsules.
		TArray<FKSphylElem>& CollisionSphyls = CollisionShapes.SphylElems;
		CapsuleBarriers.Reserve(CollisionSphyls.Num());
		for (int32 I = 0; I < CollisionSphyls.Num(); ++I)
		{
			FKSphylElem& Collision = CollisionSphyls[I];
			FAGX_Shape& Shape = GetShape(Capsules, I, DefaultShape);
			FCapsuleShapeBarrier Barrier;
			CreateNativeShape(Barrier, Collision, Shape, World);
			NativeBarrier.AddShape(&Barrier);
			CapsuleBarriers.Add(std::move(Barrier));
		}
	}

	WriteTransformToNative();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Static Mesh '%s' in '%s' tried to get Simulation, but UAGX_Simulation::GetFrom "
				 "returned nullptr."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	Simulation->Add(*this);
}

namespace AGX_StaticMeshComponent_helpers
{
	void ResizeShapeArray(TArray<FAGX_Shape>& Shapes, FAGX_Shape& DefaultShape, int32 Num)
	{
		int32 OldNum = Shapes.Num();
		Shapes.SetNum(Num);
		for (int32 I = OldNum; I < Num; ++I)
		{
			Shapes[I] = DefaultShape;
		}
	}
}

void UAGX_StaticMeshComponent::RefreshCollisionShapes()
{
	using namespace AGX_StaticMeshComponent_helpers;

	if (GetStaticMesh() == nullptr)
	{
		Spheres.SetNum(0);
		Boxes.SetNum(0);
		Capsules.SetNum(0);
		return;
	}

#if UE_VERSION_OLDER_THAN(4, 27, 0)
	FKAggregateGeom& CollisionShapes = GetStaticMesh()->BodySetup->AggGeom;
#else
	FKAggregateGeom& CollisionShapes = GetStaticMesh()->GetBodySetup()->AggGeom;
#endif
	TArray<FKSphereElem>& CollisionSpheres = CollisionShapes.SphereElems;
	TArray<FKBoxElem>& CollisionBoxes = CollisionShapes.BoxElems;
	TArray<FKSphylElem>& CollisionSphyls = CollisionShapes.SphylElems;

	ResizeShapeArray(Spheres, DefaultShape, CollisionSpheres.Num());
	ResizeShapeArray(Boxes, DefaultShape, CollisionBoxes.Num());
	ResizeShapeArray(Capsules, DefaultShape, CollisionSphyls.Num());
}

void UAGX_StaticMeshComponent::ReadTransformFromNative()
{
	check(HasNative());
	const FVector NewLocation = NativeBarrier.GetPosition();
	const FQuat NewRotation = NativeBarrier.GetRotation();

	/// \todo Consider supporting other transformation targets, such as parent and root.
	/// Should we? If so, why?
	const FVector OldLocation = GetComponentLocation();
	const FVector LocationDelta = NewLocation - OldLocation;
	MoveComponent(LocationDelta, NewRotation, false);
}

void UAGX_StaticMeshComponent::WriteTransformToNative()
{
	check(HasNative());
	NativeBarrier.SetPosition(GetComponentLocation());
	NativeBarrier.SetRotation(GetComponentQuat());
}

void UAGX_StaticMeshComponent::TryWriteTransformToNative()
{
	if (!HasNative())
	{
		return;
	}
	WriteTransformToNative();
}
