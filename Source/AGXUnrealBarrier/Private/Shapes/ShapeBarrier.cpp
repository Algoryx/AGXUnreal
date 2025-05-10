// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/ShapeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/RenderDataBarrier.h"
#include "Shapes/RenderDataRef.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/CollisionGroupManager.h"
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

FShapeBarrier::FShapeBarrier()
	: NativeRef {new FGeometryAndShapeRef}
{
}

FShapeBarrier::FShapeBarrier(FShapeBarrier&& Other) noexcept
	: NativeRef {std::move(Other.NativeRef)}
{
}

FShapeBarrier::FShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native)
	: NativeRef {std::move(Native)}
{
}

FShapeBarrier::~FShapeBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FGeometryAndShapeRef.
}

FShapeBarrier& FShapeBarrier::operator=(FShapeBarrier&& Other) noexcept
{
	NativeRef = std::move(Other.NativeRef);
	return *this;
}

bool FShapeBarrier::HasNative() const
{
	return HasNativeGeometry() && HasNativeShape();
}

bool FShapeBarrier::HasNativeGeometry() const
{
	return NativeRef->NativeGeometry != nullptr;
}

bool FShapeBarrier::HasNativeShape() const
{
	return NativeRef->NativeShape != nullptr;
}

void FShapeBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->NativeGeometry = new agxCollide::Geometry();
	AllocateNativeShape();
	NativeRef->NativeGeometry->add(NativeRef->NativeShape);
}

void FShapeBarrier::ReleaseNative()
{
	check(HasNative());
	ReleaseNativeShape();
	NativeRef->NativeGeometry = nullptr;
}

FGeometryAndShapeRef* FShapeBarrier::GetNative()
{
	return NativeRef.get();
}

const FGeometryAndShapeRef* FShapeBarrier::GetNative() const
{
	return NativeRef.get();
}

uintptr_t FShapeBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->NativeShape.get());
}

void FShapeBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		this->ReleaseNative();
	}

	NativeRef->NativeShape = reinterpret_cast<agxCollide::Shape*>(NativeAddress);
	if (NativeRef->NativeShape != nullptr)
	{
		NativeRef->NativeGeometry = NativeRef->NativeShape->getGeometry();
	}
	else
	{
		NativeRef->NativeGeometry = nullptr;
	}
}

bool FShapeBarrier::GetIsSensorGeneratingContactData() const
{
	check(HasNative());
	return NativeRef->NativeGeometry->isSensorGeneratingContactData();
}

void FShapeBarrier::SetSurfaceVelocity(const FVector& SurfaceVelocity)
{
	check(HasNative());
	agx::Vec3f SurfaceVelocityAGX = ConvertFloatDisplacement(SurfaceVelocity);
	NativeRef->NativeGeometry->setSurfaceVelocity(SurfaceVelocityAGX);
}

FVector FShapeBarrier::GetSurfaceVelocity() const
{
	check(HasNative());
	agx::Vec3f SurfaceVelocityAGX = NativeRef->NativeGeometry->getSurfaceVelocity();
	FVector SurfaceVelocity = ConvertDisplacement(SurfaceVelocityAGX);
	return SurfaceVelocity;
}

void FShapeBarrier::SetIsSensor(bool IsSensor, bool GenerateContactData)
{
	check(HasNative());
	NativeRef->NativeGeometry->setSensor(IsSensor, GenerateContactData);
}

bool FShapeBarrier::GetIsSensor() const
{
	check(HasNative());
	return NativeRef->NativeGeometry->isSensor();
}

void FShapeBarrier::SetLocalPosition(const FVector& Position)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalPosition(ConvertDisplacement(Position));
}

void FShapeBarrier::SetLocalRotation(const FQuat& Rotation)
{
	check(HasNative());
	NativeRef->NativeGeometry->setLocalRotation(Convert(Rotation));
}

FVector FShapeBarrier::GetLocalPosition() const
{
	return std::get<0>(GetLocalPositionAndRotation());
}

FQuat FShapeBarrier::GetLocalRotation() const
{
	return std::get<1>(GetLocalPositionAndRotation());
}

void FShapeBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->NativeGeometry->setName(Convert(Name));
}

FString FShapeBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->NativeGeometry->getName());
}

void FShapeBarrier::SetMaterial(const FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	NativeRef->NativeGeometry->setMaterial(Material.GetNative()->Native);
}

void FShapeBarrier::ClearMaterial()
{
	check(HasNative());
	NativeRef->NativeGeometry->setMaterial(nullptr);
}

FShapeMaterialBarrier FShapeBarrier::GetMaterial() const
{
	check(HasNative());
	agx::Material* Material = NativeRef->NativeGeometry->getMaterial();
	return AGXBarrierFactories::CreateShapeMaterialBarrier(Material);
}

void FShapeBarrier::SetEnableCollisions(bool CanCollide)
{
	check(HasNative());
	NativeRef->NativeGeometry->setEnableCollisions(CanCollide);
}

bool FShapeBarrier::GetEnableCollisions() const
{
	check(HasNative());
	return NativeRef->NativeGeometry->getEnableCollisions();
}

void FShapeBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->NativeGeometry->setEnable(Enabled);
}

bool FShapeBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->NativeGeometry->getEnable();
}

void FShapeBarrier::AddCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Add collision group as (hashed) unsigned int.
	NativeRef->NativeGeometry->addGroup(StringTo32BitFnvHash(GroupName.ToString()));
}

void FShapeBarrier::AddCollisionGroups(const TArray<FName>& GroupNames)
{
	check(HasNative());
	for (auto& GroupName : GroupNames)
	{
		AddCollisionGroup(GroupName);
	}
}

void FShapeBarrier::RemoveCollisionGroup(const FName& GroupName)
{
	check(HasNative());

	// Remove collision group as (hashed) unsigned int.
	NativeRef->NativeGeometry->removeGroup(StringTo32BitFnvHash(GroupName.ToString()));
}

FGuid FShapeBarrier::GetShapeGuid() const
{
	check(HasNative());
	return Convert(NativeRef->NativeShape->getUuid());
}

FGuid FShapeBarrier::GetGeometryGuid() const
{
	check(HasNative());
	return Convert(NativeRef->NativeGeometry->getUuid());
}

FGuid FShapeBarrier::GetGuid() const
{
	return GetShapeGuid();
}

TArray<FName> FShapeBarrier::GetCollisionGroups() const
{
	check(HasNative());
	TArray<FName> Result;
	const agxCollide::GroupIdCollection Groups = NativeRef->NativeGeometry->findGroupIdCollection();
	for (const agx::Name& Name : Groups.getNames())
	{
		Result.Add(FName(*Convert(Name)));
	}
	for (const agx::UInt32 Id : Groups.getIds())
	{
		Result.Add(FName(*FString::FromInt(Id)));
	}
	return Result;
}

bool FShapeBarrier::HasRenderData() const
{
	check(HasNative());
	return NativeRef->NativeShape->getRenderData() != nullptr;
}

bool FShapeBarrier::HasValidRenderData() const
{
	check(HasNative());
	if (agxCollide::RenderData* RenderDataAGX = NativeRef->NativeShape->getRenderData())
	{
		return RenderDataAGX->getIndexArray().size() > 0;
	}

	return false;
}

FRenderDataBarrier FShapeBarrier::GetRenderData() const
{
	check(HasNative());

	if (!HasRenderData())
	{
		return FRenderDataBarrier();
	}

	const agxCollide::RenderData* RenderDataAGX = NativeRef->NativeShape->getRenderData();
	return FRenderDataBarrier(std::make_unique<FRenderDataRef>(RenderDataAGX));
}

bool FShapeBarrier::HasRenderMaterial() const
{
	check(HasNative());
	return HasRenderData() && NativeRef->NativeShape->getRenderData()->hasRenderMaterial();
}

FAGX_RenderMaterial FShapeBarrier::GetRenderMaterial() const
{
	check(HasNative());

	FAGX_RenderMaterial RenderMaterialUnreal;
	if (!HasRenderMaterial())
	{
		// Default-created FAGX_RenderMaterial has all bHas-properties set to false.
		return RenderMaterialUnreal;
	}

	const agxCollide::RenderData* RenderDataAgx = NativeRef->NativeShape->getRenderData();
	const agxCollide::RenderMaterial* RenderMaterialAgx = RenderDataAgx->getRenderMaterial();

	RenderMaterialUnreal.Guid = Convert(RenderMaterialAgx->getUuid());

	{
		agx::String NameAGX = RenderMaterialAgx->getName();
		RenderMaterialUnreal.Name = NameAGX.empty() ? NAME_None : FName(*Convert(NameAGX));

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(NameAGX);
	}

	if ((RenderMaterialUnreal.bHasDiffuse = RenderMaterialAgx->hasDiffuseColor()) == true)
	{
		agx::Vec4 DiffuseAgx(RenderMaterialAgx->getDiffuseColor());
		RenderMaterialUnreal.Diffuse = Convert(DiffuseAgx);
	}
	if ((RenderMaterialUnreal.bHasAmbient = RenderMaterialAgx->hasAmbientColor()) == true)
	{
		agx::Vec4 AmbientAgx(RenderMaterialAgx->getAmbientColor());
		RenderMaterialUnreal.Ambient = Convert(AmbientAgx);
	}
	if ((RenderMaterialUnreal.bHasEmissive = RenderMaterialAgx->hasEmissiveColor()) == true)
	{
		agx::Vec4 EmissiveAgx(RenderMaterialAgx->getEmissiveColor());
		RenderMaterialUnreal.Emissive = Convert(EmissiveAgx);
	}
	if ((RenderMaterialUnreal.bHasShininess = RenderMaterialAgx->hasShininess()) == true)
	{
		RenderMaterialUnreal.Shininess = RenderMaterialAgx->getShininess();
	}
	return RenderMaterialUnreal;
}

FRigidBodyBarrier FShapeBarrier::GetRigidBody() const
{
	check(HasNative());
	return AGXBarrierFactories::CreateRigidBodyBarrier(NativeRef->NativeGeometry->getRigidBody());
}

namespace
{
	agxCollide::ShapeIterator FindShape(agxCollide::Geometry* Geometry, agxCollide::Shape* Shape)
	{
		agxCollide::ShapeIterator Iterator(Geometry);
		while (Iterator.isValid() && Iterator.getShape() != Shape)
		{
			Iterator.next();
		}
		if (!Iterator.isValid())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Found invalid FShapeBarrier. The native Geometry does not contain the native "
					 "Shape."));
		}
		return Iterator;
	}
}

std::tuple<FVector, FQuat> FShapeBarrier::GetLocalPositionAndRotation() const
{
	check(HasNative());
	agxCollide::ShapeIterator Iterator =
		FindShape(NativeRef->NativeGeometry, NativeRef->NativeShape);
	if (!Iterator.isValid())
	{
		return {FVector::ZeroVector, FQuat::Identity};
	}

	// The ShapeTransform is always Identity when the Native objects have been
	// created from AGXUnreal objects. However, it can be a non-Identity transform
	// during import from e.g. an .agx archive. Split this implementation if this step
	// is shown to be a performance problem.
	const agx::AffineMatrix4x4& GeometryTransform = NativeRef->NativeGeometry->getLocalTransform();
	const agx::AffineMatrix4x4& ShapeTransform = Iterator.getLocalTransform();
	const agx::AffineMatrix4x4 ShapeRelativeBody = ShapeTransform * GeometryTransform;
	return {
		ConvertDisplacement(ShapeRelativeBody.getTranslate()),
		Convert(ShapeRelativeBody.getRotate())};
}

void FShapeBarrier::SetWorldPosition(const FVector& Position)
{
	check(HasNative());
	NativeRef->NativeGeometry->setPosition(ConvertDisplacement(Position));
}

void FShapeBarrier::SetWorldRotation(const FQuat& Rotation)
{
	check(HasNative());
	NativeRef->NativeGeometry->setRotation(Convert(Rotation));
}

FVector FShapeBarrier::GetWorldPosition() const
{
	check(HasNative());
	return ConvertDisplacement(NativeRef->NativeGeometry->getPosition());
}

FQuat FShapeBarrier::GetWorldRotation() const
{
	check(HasNative());
	return Convert(NativeRef->NativeGeometry->getRotation());
}

FTransform FShapeBarrier::GetGeometryToShapeTransform() const
{
	check(HasNative());
	agxCollide::ShapeIterator Iterator =
		FindShape(NativeRef->NativeGeometry, NativeRef->NativeShape);
	if (!Iterator.isValid())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Found a FShapeBarrier whose native Geometry doesn't contain the native Shape. "
				 "Cannot get geometry-to-shape transform."));
		return FTransform();
	}

	const agx::AffineMatrix4x4& TransformAGX = Iterator.getLocalTransform();
	const FTransform Transform = Convert(TransformAGX);
	return Transform;
}
