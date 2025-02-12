// Copyright 2024, Algoryx Simulation AB.

#include "RigidBodyBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Shapes/BoxShapeBarrier.h"
#include "Shapes/CylinderShapeBarrier.h"
#include "Shapes/CapsuleShapeBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Shapes/SphereShapeBarrier.h"
#include "Shapes/TrimeshShapeBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Vec3.h>
#include <agx/Quat.h>
#include <agxCollide/Box.h>
#include <agxCollide/Capsule.h>
#include <agxCollide/Cylinder.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Trimesh.h>
#include <agxSDK/MergeSplitHandler.h>
#include "EndAGXIncludes.h"

FRigidBodyBarrier::FRigidBodyBarrier()
	: NativeRef {new FRigidBodyRef}
{
}

FRigidBodyBarrier::FRigidBodyBarrier(std::unique_ptr<FRigidBodyRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
	MassProperties.BindTo(*NativeRef);
}

FRigidBodyBarrier::FRigidBodyBarrier(FRigidBodyBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FRigidBodyRef);
	Other.MassProperties.BindTo(*Other.NativeRef);

	MassProperties.BindTo(*NativeRef);
}

FRigidBodyBarrier::~FRigidBodyBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FRigidBodyRef.
}

void FRigidBodyBarrier::SetEnabled(bool Enabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enabled);
}

bool FRigidBodyBarrier::GetEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FRigidBodyBarrier::SetPosition(const FVector& PositionUnreal)
{
	check(HasNative());
	agx::Vec3 PositionAGX = ConvertDisplacement(PositionUnreal);
	NativeRef->Native->setPosition(PositionAGX);
}

FVector FRigidBodyBarrier::GetPosition() const
{
	check(HasNative());
	agx::Vec3 PositionAGX = NativeRef->Native->getPosition();
	return ConvertDisplacement(PositionAGX);
}

void FRigidBodyBarrier::SetRotation(const FQuat& RotationUnreal)
{
	check(HasNative());
	agx::Quat RotationAGX = Convert(RotationUnreal);
	NativeRef->Native->setRotation(RotationAGX);
}

FQuat FRigidBodyBarrier::GetRotation() const
{
	check(HasNative());
	agx::Quat RotationAGX = NativeRef->Native->getRotation();
	FQuat RotationUnreal = Convert(RotationAGX);
	return RotationUnreal;
}

void FRigidBodyBarrier::SetVelocity(const FVector& VelocityUnreal)
{
	check(HasNative());
	agx::Vec3 VelocityAGX = ConvertDisplacement(VelocityUnreal);
	NativeRef->Native->setVelocity(VelocityAGX);
}

FVector FRigidBodyBarrier::GetVelocity() const
{
	check(HasNative());
	agx::Vec3 VelocityAGX = NativeRef->Native->getVelocity();
	FVector VelocityUnreal = ConvertDisplacement(VelocityAGX);
	return VelocityUnreal;
}

void FRigidBodyBarrier::SetAngularVelocity(const FVector& AngularVelocityUnreal)
{
	check(HasNative());
	agx::Vec3 AngularVelocityAGX = ConvertAngularVelocity(AngularVelocityUnreal);
	NativeRef->Native->setAngularVelocity(AngularVelocityAGX);
}

FVector FRigidBodyBarrier::GetAngularVelocity() const
{
	check(HasNative());
	agx::Vec3 AngularVelocityAGX = NativeRef->Native->getAngularVelocity();
	FVector AngularVelocityUnreal = ConvertAngularVelocity(AngularVelocityAGX);
	return AngularVelocityUnreal;
}

void FRigidBodyBarrier::SetLinearVelocityDamping(const FVector& LinearVelocityDamping)
{
	check(HasNative());
	NativeRef->Native->setLinearVelocityDamping(ConvertFloat(LinearVelocityDamping));
}

FVector FRigidBodyBarrier::GetLinearVelocityDamping() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getLinearVelocityDamping());
}

void FRigidBodyBarrier::SetAngularVelocityDamping(const FVector& AngularVelocityDamping)
{
	check(HasNative());
	NativeRef->Native->setAngularVelocityDamping(ConvertFloat(AngularVelocityDamping));
}

FVector FRigidBodyBarrier::GetAngularVelocityDamping() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getAngularVelocityDamping());
}

FMassPropertiesBarrier& FRigidBodyBarrier::GetMassProperties()
{
	return MassProperties;
}

const FMassPropertiesBarrier& FRigidBodyBarrier::GetMassProperties() const
{
	return MassProperties;
}

void FRigidBodyBarrier::UpdateMassProperties()
{
	check(HasNative());
	NativeRef->Native->updateMassProperties();
}

double FRigidBodyBarrier::CalculateMass() const
{
	check(HasNative());
	return NativeRef->Native->calculateMass();
}

void FRigidBodyBarrier::SetCenterOfMassOffset(const FVector& Offset)
{
	check(HasNative());
	const agx::Vec3 OffsetAGX = ConvertDisplacement(Offset);
	NativeRef->Native->setCmLocalTranslate(OffsetAGX);
}

FVector FRigidBodyBarrier::GetCenterOfMassOffset() const
{
	check(HasNative());
	const agx::Vec3 OffsetAGX = NativeRef->Native->getCmLocalTranslate();
	const FVector Offset = ConvertDisplacement(OffsetAGX);
	return Offset;
}

FVector FRigidBodyBarrier::GetCenterOfMassPosition() const
{
	check(HasNative());
	const agx::Vec3 PosAGX = NativeRef->Native->getCmPosition();
	const FVector Pos = ConvertDisplacement(PosAGX);
	return Pos;
}

void FRigidBodyBarrier::SetName(const FString& NameUnreal)
{
	check(HasNative());
	agx::String NameAGX = Convert(NameUnreal);
	NativeRef->Native->setName(NameAGX);
}

FString FRigidBodyBarrier::GetName() const
{
	check(HasNative());
	FString NameUnreal(Convert(NativeRef->Native->getName()));
	return NameUnreal;
}

FGuid FRigidBodyBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

void FRigidBodyBarrier::SetMotionControl(EAGX_MotionControl MotionControlUnreal)
{
	check(HasNative());
	agx::RigidBody::MotionControl MotionControlAGX = Convert(MotionControlUnreal);
	NativeRef->Native->setMotionControl(MotionControlAGX);
}

EAGX_MotionControl FRigidBodyBarrier::GetMotionControl() const
{
	check(HasNative());
	agx::RigidBody::MotionControl MotionControlAGX = NativeRef->Native->getMotionControl();
	EAGX_MotionControl MotionControlUnreal = Convert(MotionControlAGX);
	return MotionControlUnreal;
}

void FRigidBodyBarrier::AddShape(FShapeBarrier* Shape)
{
	check(HasNative());
	NativeRef->Native->add(Shape->GetNative()->NativeGeometry);
}

void FRigidBodyBarrier::RemoveShape(FShapeBarrier* Shape)
{
	check(HasNative());
	NativeRef->Native->remove(Shape->GetNative()->NativeGeometry);
}

void FRigidBodyBarrier::AddForceAtCenterOfMass(const FVector& Force)
{
	check(HasNative());
	const agx::Vec3 ForceAGX = ConvertVector(Force);
	NativeRef->Native->addForce(ForceAGX);
}

void FRigidBodyBarrier::AddForceAtLocalLocation(const FVector& Force, const FVector& Location)
{
	check(HasNative());
	const agx::Vec3 ForceAGX = ConvertVector(Force);
	const agx::Vec3 LocationAGX = ConvertDisplacement(Location);
	NativeRef->Native->addForceAtLocalPosition(ForceAGX, LocationAGX);
}

void FRigidBodyBarrier::AddForceAtWorldLocation(const FVector& Force, const FVector& Location)
{
	check(HasNative());
	const agx::Vec3 ForceAGX = ConvertVector(Force);
	const agx::Vec3 LocationAGX = ConvertDisplacement(Location);
	NativeRef->Native->addForceAtPosition(ForceAGX, LocationAGX);
}

FVector FRigidBodyBarrier::GetForce() const
{
	check(HasNative());
	const agx::Vec3 ForceAGX = NativeRef->Native->getForce();
	return ConvertVector(ForceAGX);
}

void FRigidBodyBarrier::AddTorqueWorld(const FVector& Torque)
{
	check(HasNative());
	const agx::Vec3 TorqueAGX = ConvertTorque(Torque);
	NativeRef->Native->addTorque(TorqueAGX);
}

void FRigidBodyBarrier::AddTorqueLocal(const FVector& Torque)
{
	check(HasNative());
	const agx::Vec3 TorqueAGX = ConvertTorque(Torque);
	NativeRef->Native->addLocalTorque(TorqueAGX);
}

FVector FRigidBodyBarrier::GetTorque() const
{
	check(HasNative());
	const agx::Vec3 TorqueAGX = NativeRef->Native->getTorque();
	return ConvertTorque(TorqueAGX);
}

bool FRigidBodyBarrier::IsAutomaticallyMerged()
{
	check(HasNative());
	return agxSDK::MergeSplitHandler::isMergedByHandler(NativeRef->Native);
}

bool FRigidBodyBarrier::Split()
{
	check(HasNative());
	return agxSDK::MergeSplitHandler::split(NativeRef->Native);
}

void FRigidBodyBarrier::MoveTo(const FVector& Position, const FQuat& Rotation, double Duration)
{
	check(HasNative());
	const agx::Vec3 PosAGX = ConvertDisplacement(Position);
	const agx::Quat RotAGX = Convert(Rotation);
	NativeRef->Native->moveTo(PosAGX, RotAGX, Duration);
}

bool FRigidBodyBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

void FRigidBodyBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agx::RigidBody();
	MassProperties.BindTo(*NativeRef);
}

FRigidBodyRef* FRigidBodyBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FRigidBodyRef* FRigidBodyBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uintptr_t FRigidBodyBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FRigidBodyBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		this->ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		MassProperties.BindTo(*NativeRef);
		return;
	}

	NativeRef->Native = reinterpret_cast<agx::RigidBody*>(NativeAddress);
	MassProperties.BindTo(*NativeRef);
}

void FRigidBodyBarrier::ReleaseNative()
{
	NativeRef->Native = nullptr;
}

namespace RigidBodyBarrier_helpers
{
	template <typename TShape, typename BarrierCreateFunc>
	void CollectShapeOfType(
		const agxCollide::ShapeRefVector& Shapes, TArray<TShape>& OutShapes,
		agxCollide::Shape::Type ShapeType, BarrierCreateFunc CreateFunc)
	{
		for (const agxCollide::ShapeRef& Shape : Shapes)
		{
			if (Shape->getType() == ShapeType)
			{
				OutShapes.Add(CreateFunc(Shape));
			}
			else if (Shape->getType() == agxCollide::Shape::GROUP)
			{
				agxCollide::ShapeGroup* Group {Shape->as<agxCollide::ShapeGroup>()};
				CollectShapeOfType(Group->getChildren(), OutShapes, ShapeType, CreateFunc);
			}
		}
	}

	void CollectShapes(
		const agxCollide::ShapeRefVector& InShapes, TArray<FAnyShapeBarrier>& OutShapes)
	{
		for (const agxCollide::ShapeRef& Shape : InShapes)
		{
			if (Shape->getType() == agxCollide::Shape::GROUP)
			{
				agxCollide::ShapeGroup* Group = Shape->as<agxCollide::ShapeGroup>();
				CollectShapes(Group->getChildren(), OutShapes);
			}
			else
			{
				OutShapes.Add(AGXBarrierFactories::CreateAnyShapeBarrier(Shape));
			}
		}
	}

	TArray<FAnyShapeBarrier> GetAllShapes(agx::RigidBody& Body)
	{
		TArray<FAnyShapeBarrier> Shapes;
		for (const agxCollide::GeometryRef& Geometry : Body.getGeometries())
		{
			if (Geometry == nullptr)
			{
				continue;
			}

			CollectShapes(Geometry->getShapes(), Shapes);
		}
		return Shapes;
	}

	TArray<FSphereShapeBarrier> GetAllSpheres(agx::RigidBody& Body)
	{
		TArray<FSphereShapeBarrier> Spheres;
		for (const agxCollide::GeometryRef& Geometry : Body.getGeometries())
		{
			if (Geometry == nullptr)
			{
				continue;
			}

			auto CreateSphere = [](const agxCollide::ShapeRef& Shape) -> FSphereShapeBarrier
			{
				agxCollide::Sphere* Sphere {Shape->as<agxCollide::Sphere>()};
				return AGXBarrierFactories::CreateSphereShapeBarrier(Sphere);
			};

			CollectShapeOfType<FSphereShapeBarrier>(
				Geometry->getShapes(), Spheres, agxCollide::Shape::SPHERE, CreateSphere);
		}

		return Spheres;
	}

	TArray<FBoxShapeBarrier> GetAllBoxes(agx::RigidBody& Body)
	{
		TArray<FBoxShapeBarrier> Boxes;
		for (const agxCollide::GeometryRef& Geometry : Body.getGeometries())
		{
			if (Geometry == nullptr)
			{
				continue;
			}

			auto CreateBox = [](const agxCollide::ShapeRef& Shape) -> FBoxShapeBarrier
			{
				agxCollide::Box* Box {Shape->as<agxCollide::Box>()};
				return AGXBarrierFactories::CreateBoxShapeBarrier(Box);
			};

			CollectShapeOfType<FBoxShapeBarrier>(
				Geometry->getShapes(), Boxes, agxCollide::Shape::BOX, CreateBox);
		}

		return Boxes;
	}

	TArray<FCapsuleShapeBarrier> GetAllCapsules(agx::RigidBody& Body)
	{
		TArray<FCapsuleShapeBarrier> Capsules;
		for (const agxCollide::GeometryRef& Geometry : Body.getGeometries())
		{
			if (Geometry == nullptr)
			{
				continue;
			}

			auto CreateCapsule = [](const agxCollide::ShapeRef& Shape) -> FCapsuleShapeBarrier
			{
				agxCollide::Capsule* Capsule {Shape->as<agxCollide::Capsule>()};
				return AGXBarrierFactories::CreateCapsuleShapeBarrier(Capsule);
			};

			CollectShapeOfType<FCapsuleShapeBarrier>(
				Geometry->getShapes(), Capsules, agxCollide::Shape::CAPSULE, CreateCapsule);
		}

		return Capsules;
	}

	TArray<FCylinderShapeBarrier> GetAllCylinders(agx::RigidBody& Body)
	{
		TArray<FCylinderShapeBarrier> Cylinders;
		for (const agxCollide::GeometryRef& Geometry : Body.getGeometries())
		{
			if (Geometry == nullptr)
			{
				continue;
			}

			auto CreateCylinder = [](const agxCollide::ShapeRef& Shape) -> FCylinderShapeBarrier
			{
				agxCollide::Cylinder* Cylinder {Shape->as<agxCollide::Cylinder>()};
				return AGXBarrierFactories::CreateCylinderShapeBarrier(Cylinder);
			};

			CollectShapeOfType<FCylinderShapeBarrier>(
				Geometry->getShapes(), Cylinders, agxCollide::Shape::CYLINDER, CreateCylinder);
		}

		return Cylinders;
	}

	TArray<FTrimeshShapeBarrier> GetAllTrimeshes(agx::RigidBody& Body)
	{
		TArray<FTrimeshShapeBarrier> Trimeshes;
		for (const agxCollide::GeometryRef& Geometry : Body.getGeometries())
		{
			if (Geometry == nullptr)
			{
				continue;
			}

			auto CreateTrimesh = [](const agxCollide::ShapeRef& Shape) -> FTrimeshShapeBarrier
			{
				agxCollide::Trimesh* Trimesh {Shape->as<agxCollide::Trimesh>()};
				return AGXBarrierFactories::CreateTrimeshShapeBarrier(Trimesh);
			};

			CollectShapeOfType<FTrimeshShapeBarrier>(
				Geometry->getShapes(), Trimeshes, agxCollide::Shape::TRIMESH, CreateTrimesh);

			// We have to collect all Convex shapes as well, which inherits from Trimesh.
			// We have no special handling/support for Convex shapes so they are treaded as regular
			// Trimeshes.
			CollectShapeOfType<FTrimeshShapeBarrier>(
				Geometry->getShapes(), Trimeshes, agxCollide::Shape::CONVEX, CreateTrimesh);
		}

		return Trimeshes;
	}
}

TArray<FAnyShapeBarrier> FRigidBodyBarrier::GetShapes() const
{
	if (!HasNative())
	{
		return TArray<FAnyShapeBarrier>();
	}
	return RigidBodyBarrier_helpers::GetAllShapes(*NativeRef->Native);
}

TArray<FSphereShapeBarrier> FRigidBodyBarrier::GetSphereShapes() const
{
	if (!HasNative())
	{
		return TArray<FSphereShapeBarrier>();
	}
	return RigidBodyBarrier_helpers::GetAllSpheres(*NativeRef->Native);
}

TArray<FBoxShapeBarrier> FRigidBodyBarrier::GetBoxShapes() const
{
	if (!HasNative())
	{
		return TArray<FBoxShapeBarrier>();
	}
	return RigidBodyBarrier_helpers::GetAllBoxes(*NativeRef->Native);
}

TArray<FCylinderShapeBarrier> FRigidBodyBarrier::GetCylinderShapes() const
{
	if (!HasNative())
	{
		return TArray<FCylinderShapeBarrier>();
	}
	return RigidBodyBarrier_helpers::GetAllCylinders(*NativeRef->Native);
}

TArray<FCapsuleShapeBarrier> FRigidBodyBarrier::GetCapsuleShapes() const
{
	if (!HasNative())
	{
		return TArray<FCapsuleShapeBarrier>();
	}
	return RigidBodyBarrier_helpers::GetAllCapsules(*NativeRef->Native);
}

TArray<FTrimeshShapeBarrier> FRigidBodyBarrier::GetTrimeshShapes() const
{
	if (!HasNative())
	{
		return TArray<FTrimeshShapeBarrier>();
	}
	return RigidBodyBarrier_helpers::GetAllTrimeshes(*NativeRef->Native);
}
