#include "AGXArchiveReader.h"

#include "RigidBodyBarrier.h"
#include "BoxShapeBarrier.h"
#include "SphereShapeBarrier.h"

#include "AGXBarrierFactories.h"


#include "BeginAGXIncludes.h"
#include <agx/RigidBody.h>
#include <agx/Encoding.h>
#include <agxSDK/Simulation.h>
#include <agxCollide/Geometry.h>
#include <agxCollide/Box.h>
#include "EndAGXIncludes.h"

struct FAGXArchiveContents
{
	// These store the AGX Dynamics objects restored from the archive.
	TArray<FRigidBodyBarrier> Bodies;
	TArray<FBoxShapeBarrier> Boxes;
	TArray<FSphereShapeBarrier> Spheres;

	// These pair one body with one of either box or sphere. Contains pointers
	// into the TArrays declared above.
	TArray<FBoxBody> BoxBodies;
	TArray<FSphereBody> SphereBodies;
};

namespace
{
	agx::String Convert(const FString& FilenameUnreal)
	{
		agx::String FilenameAGX(TCHAR_TO_UTF8(*FilenameUnreal));
		return FilenameAGX;
	}

	template <typename TCollection, typename TElement>
	bool Contains(TCollection const& Collection, TElement const& Element)
	{
		using std::begin;
		using std::end;
		return std::find(begin(Collection), end(Collection), Element) != end(Collection);
	}

	agxCollide::Shape* GetLeafShape(agxCollide::Shape& Shape)
	{
		static const agxCollide::Shape::Type SupportedShapes[] = {agxCollide::Shape::BOX, agxCollide::Shape::SPHERE};
		if (Contains(SupportedShapes, Shape.getType()))
		{
			return &Shape;
		}
		if (Shape.getType() == agxCollide::Shape::GROUP)
		{
			agxCollide::ShapeGroup* Group = dynamic_cast<agxCollide::ShapeGroup*>(&Shape);
			if (Group == nullptr)
			{
				// This really should not happen.
				return nullptr;
			}
			if (Group->getNumChildren() != size_t{1})
			{
				return nullptr;
			}
			agxCollide::Shape* Child = Group->getChild(size_t{0});
			if (Child == nullptr)
			{
				return nullptr;
			}
			return GetLeafShape(*Child);
		}
		return nullptr;
	}

	agxCollide::Shape* GetShape(agx::RigidBody& body)
	{
		const agxCollide::GeometryRefVector& Geometries = body.getGeometries();
		if (Geometries.size() != size_t{1})
		{
			return nullptr;
		}
		const agxCollide::Geometry* Geometry = Geometries.front();
		const agxCollide::ShapeRefVector& Shapes = Geometry->getShapes();
		if (Shapes.size() != size_t{1})
		{
			return nullptr;
		}
		agxCollide::Shape* Shape = Shapes.front();
		if (Shape == nullptr)
		{
			return nullptr;
		}
		return GetLeafShape(*Shape);
	}
}

FAGXArchiveReader::FAGXArchiveReader(const FString& Filename)
	: Contents{new FAGXArchiveContents}
{
	agxSDK::SimulationRef Simulation = new agxSDK::Simulation();
#if 0
	{
		agx::RigidBodyRef Body = new agx::RigidBody("MyBox");
		agxCollide::GeometryRef Geometry = new agxCollide::Geometry();
		agxCollide::BoxRef Box = new agxCollide::Box(agx::Vec3(agx::Real(1.0)));
		Geometry->add(Box);
		Body->add(Geometry);
		Simulation->add(Body);
	}
#else
	size_t NumRead = Simulation->read(Convert(Filename));
	if (NumRead == 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Could not read .agx file %s."), *Filename);
		return;
	}
#endif

	agx::RigidBodyRefVector& Bodies = Simulation->getRigidBodies();
	if (Bodies.size() > size_t{std::numeric_limits<int32>::max()})
	{
		UE_LOG(LogTemp, Log, TEXT(".agx file %s contains too many bodies."), *Filename);
		return;
	}
	int32 NumBodies = static_cast<int32>(Bodies.size());
	Contents->Bodies.Reserve(NumBodies);
	// These reserves are overly greedy since not all bodies are boxes and
	// spheres. Don't want reallocate since we store pointers into the arrays.
	// Count if necessary, or fill the pointer arrays after the data arrays.
	Contents->Boxes.Reserve(NumBodies);
	Contents->Spheres.Reserve(NumBodies);
	Contents->BoxBodies.Reserve(NumBodies);
	Contents->SphereBodies.Reserve(NumBodies);

	for (auto Body : Simulation->getRigidBodies())
	{
		if (Body == nullptr)
		{
			continue;
		}
		agxCollide::Shape* Shape = GetShape(*Body);
		if (Shape == nullptr)
		{
			continue;
		}
		Contents->Bodies.Add(CreateRigidBodyBarrier(Body));
		switch (Shape->getType())
		{
			case agxCollide::Shape::BOX:
			{
				agxCollide::Box* Box = dynamic_cast<agxCollide::Box*>(Shape);
				Contents->Boxes.Add(CreateBoxShapeBarrier(Box));
				Contents->BoxBodies.Add({&Contents->Bodies.Last(), &Contents->Boxes.Last()});
				break;
			}
			case agxCollide::Shape::SPHERE:
			{
				agxCollide::Sphere* Sphere = dynamic_cast<agxCollide::Sphere*>(Shape);
				Contents->Spheres.Add(CreateSphereShapeBarrier(Sphere));
				Contents->SphereBodies.Add({&Contents->Bodies.Last(), &Contents->Spheres.Last()});
				break;
			}
		}
	}
}

FAGXArchiveReader::~FAGXArchiveReader()
{
}

const TArray<FBoxBody>& FAGXArchiveReader::GetBoxBodies() const
{
	return Contents->BoxBodies;
}

const TArray<FSphereBody>& FAGXArchiveReader::GetSphereBodies() const
{
	return Contents->SphereBodies;
}