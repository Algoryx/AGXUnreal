// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/RenderMaterial.h"

// Unreal Engine includes.
#include "Containers/Array.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// Standard library includes.
#include <memory>
#include <tuple>

struct FGeometryAndShapeRef;
class FShapeMaterialBarrier;

class FRenderDataBarrier;

/**
 * Barrier class that gives access to an AGX Dynamics (Geometry, Shape) pair.
 *
 * There are subclasses for each shape type.
 *
 * A Shape may come with Render Data, which may contain a Render Mesh and/or a Render Material.
 */
class AGXUNREALBARRIER_API FShapeBarrier
{
public:
	FShapeBarrier();
	FShapeBarrier(FShapeBarrier&& Other) noexcept;
	FShapeBarrier(std::unique_ptr<FGeometryAndShapeRef> Native);
	virtual ~FShapeBarrier();

	FShapeBarrier& operator=(FShapeBarrier&& Other) noexcept;

	bool HasNativeGeometry() const;
	bool HasNativeShape() const;
	virtual bool HasNative() const;
	void AllocateNative();
	void ReleaseNative();
	FGeometryAndShapeRef* GetNative();
	const FGeometryAndShapeRef* GetNative() const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object.
	void SetNativeAddress(uintptr_t NativeAddress);

	void SetIsSensor(bool IsSensor, bool GenerateContactData);
	bool GetIsSensor() const;
	bool GetIsSensorGeneratingContactData() const;

	void SetSurfaceVelocity(const FVector& SurfaceVelocity);
	FVector GetSurfaceVelocity() const;

	template <typename T>
	T* GetNativeShape();

	template <typename T>
	const T* GetNativeShape() const;

	void SetLocalPosition(const FVector& Position);
	void SetLocalRotation(const FQuat& Rotation);

	FVector GetLocalPosition() const;
	FQuat GetLocalRotation() const;
	std::tuple<FVector, FQuat> GetLocalPositionAndRotation() const;

	void SetWorldPosition(const FVector& Position);
	void SetWorldRotation(const FQuat& Rotation);

	FVector GetWorldPosition() const;
	FQuat GetWorldRotation() const;

	FTransform GetGeometryToShapeTransform() const;

	void SetName(const FString& Name);
	FString GetName() const;

	void ClearMaterial();
	void SetMaterial(const FShapeMaterialBarrier& Material);

	/// \todo Should GetMaterial() create a new FShapeMaterialBarrier, or get an existing somehow?
	/// If it creates a new FShapeMaterialBarrier we should implement comparison operators etc since
	/// multiple FShapeMaterialBarrier that points to the same native object should be logically
	/// seen as same object (similar to smart pointers).
	FShapeMaterialBarrier GetMaterial() const;

	void SetEnableCollisions(bool CanCollide);
	bool GetEnableCollisions() const;

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void AddCollisionGroup(const FName& GroupName);
	void AddCollisionGroups(const TArray<FName>& GroupNames);
	void RemoveCollisionGroup(const FName& GroupName);

	FGuid GetShapeGuid() const;
	FGuid GetGeometryGuid() const;

	/**
	 * Get all collision groups registered for this Shape.
	 *
	 * AGX Dynamics supports both name- and integer-based IDs while AGXUnreal
	 * only supports named groups. Any integer ID found is converted to the
	 * string representation of that integer using FString::FromInt.
	 *
	 * \return A list of all collision groups registered for this shape.
	 */
	TArray<FName> GetCollisionGroups() const;

	/**
	 * @return True if the native shape contains render data. The render data may be incomplete.
	 */
	bool HasRenderData() const;

	/**
	 * @return True if the native shape contains render data that also has triangle data.
	 */
	bool HasValidRenderData() const;

	/**
	 * Provide access to any render data that may be associated with the Shape. This data is
	 * optional and only exists if HasRenderData returns true.
	 *
	 * @return The render data associated with the shape.
	 */
	FRenderDataBarrier GetRenderData() const;

	/**
	 * @return True if the native shape contains render data and the render data has a render
	 * material.
	 */
	bool HasRenderMaterial() const;

	/**
	 * @return The native shape's render material, if there is one. Otherwise an empty fallback
	 * material.
	 */
	FAGX_RenderMaterial GetRenderMaterial() const;

protected:
	template <typename TFunc, typename... TPack>
	void AllocateNative(TFunc Factory, TPack... Params);

private:
	FShapeBarrier(const FShapeBarrier&) = delete;
	void operator=(const FShapeBarrier&) = delete;

private:
	/// \todo Are we allowed to have pure virtual classes in an Unreal plugin.
	///       Not allowed when inheriting from U/A classes, but we don't do that
	///       here.

	/**
	Called from AllocateNative. The subclass is responsible for creating the
	agxCollide::Shape instance, which is stored in NativeRef. FShapeBarrier
	creates the agxCollide::Geometry, in AllocateNative, and adds the
	agxCollide::Shape to it.
	*/
	virtual void AllocateNativeShape() {};
	virtual void ReleaseNativeShape() {};

protected:
	std::unique_ptr<FGeometryAndShapeRef> NativeRef;
};
