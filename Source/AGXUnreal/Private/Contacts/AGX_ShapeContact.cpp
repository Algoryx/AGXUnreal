// Copyright 2024, Algoryx Simulation AB.

#include "Contacts/AGX_ShapeContact.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_ShapeComponent.h"

// Unreal Engine includes.
#include "UObject/UObjectIterator.h"

FAGX_ShapeContact::FAGX_ShapeContact(const FShapeContactBarrier& InBarrier)
	: Barrier(InBarrier)
{
}

FAGX_ShapeContact::FAGX_ShapeContact(FShapeContactBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
}

bool FAGX_ShapeContact::HasNative() const
{
	return Barrier.HasNative();
}

namespace AGX_ShapeContact_helpers
{
	bool TestHasNative(const FAGX_ShapeContact& ShapeContact, const TCHAR* AttributeName)
	{
		if (ShapeContact.HasNative())
		{
			return true;
		}
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot get %s from a ShapeContact that doesn't have a native AGX Dynamics "
				 "representation"),
			AttributeName);
		return false;
	}
}

bool FAGX_ShapeContact::IsEnabled() const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Enabled")))
	{
		return false;
	}
	return Barrier.IsEnabled();
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody1() const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("First Body")))
	{
		return FRigidBodyBarrier();
	}
	return Barrier.GetBody1();
}

FRigidBodyBarrier FAGX_ShapeContact::GetBody2() const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Second Body")))
	{
		return FRigidBodyBarrier();
	}
	return Barrier.GetBody2();
}

FShapeBarrier FAGX_ShapeContact::GetShape1() const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("First Shape")))
	{
		return FShapeBarrier();
	}
	return Barrier.GetShape1();
}

FShapeBarrier FAGX_ShapeContact::GetShape2() const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Second Shape")))
	{
		return FShapeBarrier();
	}
	return Barrier.GetShape2();
}

bool FAGX_ShapeContact::Contains(const FRigidBodyBarrier& Body) const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Contains Body")))
		return false;
	if (!Body.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Rigid Body without native AGX Dynamics representation passed to Shape Contact > "
				 "Contains"));
		return false;
	}
	return Barrier.Contains(Body);
}

bool FAGX_ShapeContact::Contains(const FShapeBarrier& Shape) const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Contains Shape")))
		return false;
	if (!Shape.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape without native AGX Dynamics representation passed to Shape Contact > "
				 "Contains."));
		return false;
	}
	return Barrier.Contains(Shape);
}

int32 FAGX_ShapeContact::IndexOf(const FRigidBodyBarrier& Body) const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Index Of Body")))
		return false;
	if (!Body.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Rigid Body without native AGX Dynamics representation passed to Shape Contact > "
				 "Index Of"));
		return -1;
	}
	return Barrier.IndexOf(Body);
}

int32 FAGX_ShapeContact::IndexOf(const FShapeBarrier& Shape) const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Index Of Shape")))
		return false;
	if (!Shape.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape without native AGX Dynamics representation passed to Shape Contact > Index "
				 "Of"));
		return false;
	}
	return Barrier.IndexOf(Shape);
}

FVector FAGX_ShapeContact::CalculateRelativeVelocity(int32 PointIndex) const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Relative Velocity")))
	{
		return FVector::ZeroVector;
	}
	return Barrier.CalculateRelativeVelocity(PointIndex);
}

int32 FAGX_ShapeContact::GetNumContactPoints() const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Num Contact Points")))
	{
		return 0;
	}
	return Barrier.GetNumContactPoints();
}

TArray<FAGX_ContactPoint> FAGX_ShapeContact::GetContactPoints() const
{
	using namespace AGX_ShapeContact_helpers;
	TArray<FAGX_ContactPoint> ContactPoints;
	if (!TestHasNative(*this, TEXT("Contact Points")))
	{
		return ContactPoints;
	}
	TArray<FContactPointBarrier> ContactPointBarriers = Barrier.GetContactPoints();
	ContactPoints.Reserve(ContactPointBarriers.Num());
	for (FContactPointBarrier& ContactPointBarrier : ContactPointBarriers)
	{
		ContactPoints.Emplace(std::move(ContactPointBarrier));
	}
	return ContactPoints;
}

FAGX_ContactPoint FAGX_ShapeContact::GetContactPoint(int32 Index) const
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(*this, TEXT("Contact Point")))
	{
		return FAGX_ContactPoint();
	}
	return Barrier.GetContactPoint(Index);
}

/*
 * Function Library implementation starts here.
 */

namespace AGX_ShapeContact_helpers
{
	bool IsValidPointIndex(
		const FAGX_ShapeContact& ShapeContact, int32 PointIndex, const TCHAR* AttributeName)
	{
		const int32 NumContactPoints = ShapeContact.GetNumContactPoints();
		if (PointIndex >= 0 && PointIndex < NumContactPoints)
		{
			return true;
		}
		UE_LOG(
			LogAGX, Error,
			TEXT("Trying to access %s from contact point at index %d in a ShapeContact that only "
				 "has %d contact points."),
			AttributeName, PointIndex, NumContactPoints);
		return false;
	}

	bool CheckHasNativeAndValidPointIndex(
		const FAGX_ShapeContact& ShapeContact, int32 PointIndex, const TCHAR* AttributeName)
	{
		return TestHasNative(ShapeContact, AttributeName) &&
			   IsValidPointIndex(ShapeContact, PointIndex, AttributeName);
	}

	bool IsMatch(UAGX_ShapeComponent* Shape, const FGuid& Guid)
	{
		if (!Shape || !Shape->HasNative() || !Guid.IsValid())
		{
			return false;
		}

		return Shape->GetNative()->GetGeometryGuid() == Guid;
	}

	bool IsMatch(UAGX_RigidBodyComponent* Body, const FGuid& Guid)
	{
		if (!Body || !Body->HasNative() || !Guid.IsValid())
		{
			return false;
		}

		return Body->GetNative()->GetGuid() == Guid;
	}

	template <typename T>
	T* GetFromGuid(const FGuid& Guid)
	{
		/// @todo Are we sure this will work? That is, are we guaranteed that it will find a T
		/// within the same World as we got the GUID from? I don't think so. And it seems really
		/// expensive to iterate all objects in the entire universe.
		///
		/// We should consider keeping a table of (GUID -> Unreal Engine class instances) instead,
		/// alternatively store a pointer to the Unreal Engine class instance somewhere in the
		/// AGX Dynamics object. For some things there is setCustomData, but not all things.
		for (TObjectIterator<T> ObjectIt; ObjectIt; ++ObjectIt)
		{
			T* Obj = *ObjectIt;
			if (IsMatch(Obj, Guid))
			{
				return Obj;
			}
		}

		return nullptr;
	}
}

bool UAGX_ShapeContact_FL::HasNative(FAGX_ShapeContact& ShapeContact)
{
	return ShapeContact.HasNative();
}

bool UAGX_ShapeContact_FL::IsEnabled(FAGX_ShapeContact& ShapeContact)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Is Enabled")))
	{
		return false;
	}
	return ShapeContact.IsEnabled();
}

UAGX_ShapeComponent* UAGX_ShapeContact_FL::GetFirstShape(FAGX_ShapeContact& ShapeContact)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("First Shape")))
	{
		return nullptr;
	}
	if (!ShapeContact.GetShape1().HasNative())
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContact.GetShape1().GetGeometryGuid());
}

UAGX_ShapeComponent* UAGX_ShapeContact_FL::GetSecondShape(FAGX_ShapeContact& ShapeContact)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Second Shape")))
	{
		return nullptr;
	}
	if (!ShapeContact.GetShape2().HasNative())
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_ShapeComponent>(ShapeContact.GetShape2().GetGeometryGuid());
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetFirstBody(FAGX_ShapeContact& ShapeContact)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("First Body")))
	{
		return nullptr;
	}
	if (!ShapeContact.GetBody1().HasNative())
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContact.GetBody1().GetGuid());
}

UAGX_RigidBodyComponent* UAGX_ShapeContact_FL::GetSecondBody(FAGX_ShapeContact& ShapeContact)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Second Body")))
	{
		return nullptr;
	}
	if (!ShapeContact.GetBody2().HasNative())
	{
		return nullptr;
	}
	return GetFromGuid<UAGX_RigidBodyComponent>(ShapeContact.GetBody2().GetGuid());
}

bool UAGX_ShapeContact_FL::ContainsRigidBody(
	FAGX_ShapeContact& ShapeContact, UAGX_RigidBodyComponent* RigidBody, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Contains Rigid Body")))
	{
		bSuccess = false;
		return false;
	}
	if (!RigidBody->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Rigid Body without a native AGX Dynamics representation passed to Shape Contact "
				 "> Contains."));
		bSuccess = false;
		return false;
	}
	bSuccess = true;
	return ShapeContact.Contains(*RigidBody->GetNative());
}

bool UAGX_ShapeContact_FL::ContainsShape(
	FAGX_ShapeContact& ShapeContact, UAGX_ShapeComponent* Shape, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Contains Shape")))
	{
		bSuccess = false;
		return false;
	}
	if (!Shape->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape without a native AGX Dynamics representation passed to Shape Contact > "
				 "Contains."));
		bSuccess = false;
		return false;
	}
	bSuccess = true;
	return ShapeContact.Contains(*Shape->GetNative());
}

int32 UAGX_ShapeContact_FL::IndexOfRigidBody(
	FAGX_ShapeContact& ShapeContact, UAGX_RigidBodyComponent* RigidBody, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Index Of Rigid Body")))
	{
		bSuccess = false;
		return -1;
	}
	if (!RigidBody->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Rigid Body without native AGX Dynamics representaiton passed to Shape Contact > "
				 "Index Of"));
		bSuccess = false;
		return -1;
	}
	const int32 Index = ShapeContact.IndexOf(*RigidBody->GetNative());
	bSuccess = Index != -1;
	return Index;
}

int32 UAGX_ShapeContact_FL::IndexOfShape(
	FAGX_ShapeContact& ShapeContact, UAGX_ShapeComponent* Shape, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Index Of Shape")))
	{
		bSuccess = false;
		return -1;
	}
	if (!Shape->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Shape without native AGX Dynamics representaiton passed to Shape Contact > "
				 "Index Of"));
		bSuccess = false;
		return -1;
	}
	const int32 Index = ShapeContact.IndexOf(*Shape->GetNative());
	bSuccess = Index != -1;
	return Index;
}

int32 UAGX_ShapeContact_FL::GetNumContactPoints(FAGX_ShapeContact& ShapeContact)
{
	return GetNumPoints(ShapeContact);
}

int UAGX_ShapeContact_FL::GetNumPoints(UPARAM(Ref) FAGX_ShapeContact& ShapeContact)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Num Contact Points")))
	{
		return 0;
	}
	return ShapeContact.GetNumContactPoints();
}

int UAGX_ShapeContact_FL::GetLastPointIndex(UPARAM(Ref) FAGX_ShapeContact& ShapeContact)
{
	return GetNumPoints(ShapeContact) - 1;
}

bool UAGX_ShapeContact_FL::HasPointNative(FAGX_ShapeContact& ShapeContact, int32 PointIndex)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Has Native")))
	{
		return false;
	}
	return ShapeContact.GetContactPoint(PointIndex).HasNative();
}

bool UAGX_ShapeContact_FL::SetPointEnabled(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool bEnabled)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Set Enabled")))
	{
		return false;
	}
	ShapeContact.GetContactPoint(PointIndex).SetEnabled(bEnabled);
	return true;
}

bool UAGX_ShapeContact_FL::IsPointEnabled(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Is Enabled")))
	{
		bSuccess = false;
		return false;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).IsEnabled();
}

bool UAGX_ShapeContact_FL::SetPointLocation(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector Location)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Location")))
	{
		return false;
	}
	ShapeContact.GetContactPoint(PointIndex).SetLocation(Location);
	return true;
}

FVector UAGX_ShapeContact_FL::GetPointLocation(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Location")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetLocation();
}

bool UAGX_ShapeContact_FL::SetPointNormal(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector Normal)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Normal")))
	{
		return false;
	}
	ShapeContact.GetContactPoint(PointIndex).SetNormal(Normal);
	return true;
}

FVector UAGX_ShapeContact_FL::GetPointNormal(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Normal")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetNormal();
}

bool UAGX_ShapeContact_FL::SetPointTangentU(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector TangentU)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Tangent U")))
		return false;
	ShapeContact.GetContactPoint(PointIndex).SetTangentU(TangentU);
	return true;
}

FVector UAGX_ShapeContact_FL::GetPointTangentU(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Tangent U")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetTangentU();
}

bool UAGX_ShapeContact_FL::SetPointTangentV(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector TangentV)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Tangent V")))
		return false;
	ShapeContact.GetContactPoint(PointIndex).SetTangentV(TangentV);
	return true;
}

FVector UAGX_ShapeContact_FL::GetPointTangentV(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Tangent V")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetTangentV();
}

bool UAGX_ShapeContact_FL::SetPointDepth(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, double Depth)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Depth")))
		return false;
	ShapeContact.GetContactPoint(PointIndex).SetDepth(Depth);
	return true;
}

double UAGX_ShapeContact_FL::GetPointDepth(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Depth")))
	{
		bSuccess = false;
		return 0.0;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetDepth();
}

FVector UAGX_ShapeContact_FL::GetPointWitnessPoint(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, int32 ShapeIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Witness Point")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetWitnessPoint(ShapeIndex);
}

bool UAGX_ShapeContact_FL::SetPointArea(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, double Area)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Area")))
	{
		return false;
	}
	ShapeContact.GetContactPoint(PointIndex).SetArea(Area);
	return true;
}

double UAGX_ShapeContact_FL::GetPointArea(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Area")))
	{
		bSuccess = false;
		return 0.0f;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetArea();
}

bool UAGX_ShapeContact_FL::SetPointSurfaceVelocity(
	UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector SurfaceVelocity)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Surface Velocity")))
	{
		return false;
	}
	ShapeContact.GetContactPoint(PointIndex).SetSurfaceVelocity(SurfaceVelocity);
	return true;
}

FVector UAGX_ShapeContact_FL::GetPointSurfaceVelocity(
	UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Surface Velocity")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetSurfaceVelocity();
}

FVector UAGX_ShapeContact_FL::GetPointForce(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Force")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetForce();
}

double UAGX_ShapeContact_FL::GetPointForceMagnitude(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Force Magnitude")))
	{
		bSuccess = false;
		return 0.0;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetForceMagnitude();
}

FVector UAGX_ShapeContact_FL::GetPointNormalForce(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Normal Force")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetNormalForce();
}

double UAGX_ShapeContact_FL::GetPointNormalForceMagnitude(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Normal Force Magnitude")))
	{
		bSuccess = false;
		return 0.0;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetNormalForceMagnitude();
}

FVector UAGX_ShapeContact_FL::GetPointTangentialForce(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Tangential Force")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetTangentialForce();
}

double UAGX_ShapeContact_FL::GetPointTangentialForceMagnitude(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(
			ShapeContact, PointIndex, TEXT("Tangential Force Magnitude")))
	{
		bSuccess = false;
		return 0.0;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetTangentialForceMagnitude();
}

double UAGX_ShapeContact_FL::GetPointTangentialForceUMagnitude(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(
			ShapeContact, PointIndex, TEXT("Tangential Force U Magnitude")))
	{
		bSuccess = false;
		return 0.0;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetTangentialForceUMagnitude();
}

double UAGX_ShapeContact_FL::GetPointTangentialForceVMagnitude(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(
			ShapeContact, PointIndex, TEXT("Tangential Force V Magnitude")))
	{
		bSuccess = false;
		return 0.0;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetTangentialForceVMagnitude();
}

FVector UAGX_ShapeContact_FL::GetPointLocalForce(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Local Force")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetLocalForce();
}

double UAGX_ShapeContact_FL::GetPointLocalForceComponent(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, EAGX_ContactForceComponents Component,
	bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!CheckHasNativeAndValidPointIndex(ShapeContact, PointIndex, TEXT("Local Force Component")))
	{
		bSuccess = false;
		return 0.0;
	}
	bSuccess = true;
	return ShapeContact.GetContactPoint(PointIndex).GetLocalForce(Component);
}

FVector UAGX_ShapeContact_FL::CalculateRelativeVelocity(
	FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess)
{
	using namespace AGX_ShapeContact_helpers;
	if (!TestHasNative(ShapeContact, TEXT("Relative Velocity")))
	{
		bSuccess = false;
		return FVector::ZeroVector;
	}
	bSuccess = true;
	return ShapeContact.CalculateRelativeVelocity(PointIndex);
}
