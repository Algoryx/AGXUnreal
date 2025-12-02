// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AgxAutomationCommon.h"
#include "Materials/AGX_TerrainMaterial.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "Engine/World.h"

/**
 * Test that Terrain Material properties are copied properly between asset, instance, and barrier.
 */
BEGIN_DEFINE_SPEC(
	FAGX_TerrainMaterialSpec, "AGXUnreal.Spec.TerrainMaterial",
	AgxAutomationCommon::DefaultTestFlags)
END_DEFINE_SPEC(FAGX_TerrainMaterialSpec)

namespace AGX_TerrainMaterialSpec_helpers
{
	/**
	 * Run a callback for every non-deprecated property of the given CPPType in the given struct.
	 * Intended to be called from an overload that takes a UObject as a starting-point.
	 *
	 * Visits nested struct properties recursively.
	 *
	 * Currently does not handle container properties such as arrays, maps, and sets.
	 *
	 * @param StructProperty Reflection data for the struct whose properties are being visited.
	 * @param StructMemory Pointer to the struct described by StructProperty.
	 * @param CPPType The name of the type that the callback should be called for.
	 * @param Callback Callback to call when a CPPType'd property is found.
	 */
	template <typename CallbackT>
	void VisitProperties(
		FStructProperty* StructProperty, void* StructMemory, const FString& CPPType,
		CallbackT& Callback)
	{
		UScriptStruct* Struct = StructProperty->Struct;
		for (TFieldIterator<FProperty> PropertyIt(Struct); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (Property->HasAnyPropertyFlags(CPF_Deprecated))
			{
				continue;
			}
			void* PropertyMemory = Property->ContainerPtrToValuePtr<void>(StructMemory);
			if (Property->GetCPPType() == CPPType)
			{
				Callback(PropertyMemory, *Property->GetName());
			}
			else if (FStructProperty* NestedStructProperty = CastField<FStructProperty>(Property))
			{
				VisitProperties(NestedStructProperty, PropertyMemory, CPPType, Callback);
			}
		}
	}

	/**
	 * Run a callback for every non-deprecated property of the given CPPType in the given object.
	 *
	 * Visits nested struct properties recursively.
	 *
	 * Currently does not handle container properties such as arrays, maps, and sets.
	 *
	 * @param Object The object whose properties are to be visited.
	 * @param CPPType The name of the type that the callback should be called for.
	 * @param Callback Callback to call when a CPPType'd property is found.
	 */
	template <typename CallbackT>
	void VisitProperties(UObject* Object, const FString& CPPType, CallbackT& Callback)
	{
		UClass* Class = Object->GetClass();
		for (TFieldIterator<FProperty> PropertyIt(Class); PropertyIt; ++PropertyIt)
		{
			FProperty* Property = *PropertyIt;
			if (Property->HasAnyPropertyFlags(CPF_Deprecated))
			{
				continue;
			}
			void* PropertyMemory = Property->ContainerPtrToValuePtr<void>(Object);
			if (Property->GetCPPType() == CPPType)
			{
				Callback(PropertyMemory, *Property->GetName());
			}
			else if (FStructProperty* NestedStructProperty = CastField<FStructProperty>(Property))
			{
				VisitProperties(NestedStructProperty, PropertyMemory, CPPType, Callback);
			}
		}
	}

	/**
	 * Set all AGX Real properties in the given object to increasing values starting at 0.0 and
	 * increasing by one for each AGX Real property found.
	 */
	void SetAllRealPropertiesToIncreasingValues(UObject* Object)
	{
		auto Callback = [NextValue = 0.0](void* Memory, const TCHAR* /*Name*/) mutable
		{
			FAGX_Real* Real = static_cast<FAGX_Real*>(Memory);
			Real->Value = NextValue;
			NextValue = NextValue + 1.0;
		};

		VisitProperties(Object, TEXT("FAGX_Real"), Callback);
	}

	/**
	 * Test that all FAGX_Real has the values that SetAllRealPropertiesToIncreasingValues would have
	 * set if run on this object.
	 */
	void TestAllRealPropertiesHaveIncreasingValues(UObject* Object, FAGX_TerrainMaterialSpec& Test)
	{
		auto Callback = [NextValue = 0.0, &Test](void* Memory, const TCHAR* Name) mutable
		{
			FAGX_Real* Real = static_cast<FAGX_Real*>(Memory);
			static const FString Message(TEXT("property (see separate LogAGX warning for which)"));
			if (!Test.TestEqual(Message, Real->Value, NextValue))
			{
				UE_LOG(
					LogAGX, Warning, TEXT("Failed increasing-values check for property '%s'."),
					Name);
			}
			NextValue = NextValue + 1.0;
		};

		VisitProperties(Object, TEXT("FAGX_Real"), Callback);
	}

	/**
	 * Dump a bunch of bytes from one place in memory to another. Uses a const-destination because
	 * that is what UAGX_TerrainMaterial::GetShapeMaterial*Properties returns. The object itself
	 * isn't actually const so this is safe. This is all a hack to get the unit test to pass, thus
	 * the long and unwieldy name.
	 */
	template <typename T>
	void RawByteCopyOverNotActuallyConst(const T& Destination, const T& Source)
	{
		memcpy((void*) &Destination, (void*) &Source, sizeof(T));
	}
}

void FAGX_TerrainMaterialSpec::Define()
{
	using namespace AGX_TerrainMaterialSpec_helpers;

	// Test that we copy all properties when we create runtime instances from assets on drive.
	Describe(
		"When copying properties from one Terrain Material to another",
		[this]()
		{
			It("should copy all properties",
			   [this]()
			   {
				   // Create source.
				   TObjectPtr<UAGX_TerrainMaterial> Source = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Source Terrain Material"));
				   SetAllRealPropertiesToIncreasingValues(Source);
				   TestAllRealPropertiesHaveIncreasingValues(Source, *this);

				   // Create and test destination.
				   TObjectPtr<UAGX_TerrainMaterial> Destination = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Destination Terrain Material"));
				   Destination->CopyTerrainMaterialProperties(Source);
				   TestAllRealPropertiesHaveIncreasingValues(Destination, *this);
			   });
		});

	// Test that we round-trip all properties to and then back from the native AGX Dynamics object.
	Describe(
		"When copying properties from a Terrain Material to a Barrier",
		[this]()
		{
			It("should copy all properties",
			   [this]()
			   {
				   // Create source, a stand-in for an on-drive asset.
				   TObjectPtr<UAGX_TerrainMaterial> Source = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Source Terrain Material"));
				   SetAllRealPropertiesToIncreasingValues(Source);

				   // AGX Dynamics puts limits on some properties. Make sure we are within those
				   // limits before creating the native.
				   double BankStatePhi0Test {Source->TerrainCompaction.BankStatePhi0};
				   double BankStatePhi0Agx {1.0};
				   Source->TerrainCompaction.BankStatePhi0 = BankStatePhi0Agx;

				   // Create a temporary world to hold our Terrain Material instance, so we don't
				   // put stuff that shouldn't be there in the main world.
				   TObjectPtr<UWorld> World = UWorld::CreateWorld(
					   EWorldType::Game, false, TEXT("Terrain Material Test World"),
					   GetTransientPackage());

				   // Create Barrier, i.e. the native AGX Dynamics object. This will copy properties
				   // Source -> Instance -> Barrier.
				   TObjectPtr<UAGX_TerrainMaterial> Instance =
					   UAGX_TerrainMaterial::CreateFromAsset(World, Source);
				   FTerrainMaterialBarrier* Barrier = Instance->GetTerrainMaterialNative();

				   // Create and copy into the destination.
				   TObjectPtr<UAGX_TerrainMaterial> Destination = NewObject<UAGX_TerrainMaterial>(
					   GetTransientPackage(), TEXT("Destination Terrain Material"));
				   Destination->CopyFrom(*Barrier);

				   // Special handling for properties that AGX Dynamics doesn't support arbitrary
				   // values for.

				   // Bank State Phi 0 has limits. Test the limited value and then set back to what
				   // the generic test harness expects.
				   TestEqual(
					   TEXT("Terrain Material Compaction Bank State Phi 0"),
					   Destination->TerrainCompaction.BankStatePhi0, BankStatePhi0Agx);
				   Destination->TerrainCompaction.BankStatePhi0 = BankStatePhi0Test;

				   // Adhesion Overlap Factor is aliased, both Terrain Bulk and Terrain Particles
				   // has it, and they write to the same AGX Dynamics memory. This means that when
				   // we read back from AGX Dynamics we do two reads of the same memory and expect
				   // two different values. That's not going to happen. Hard-copy the first property
				   // here so that the test don't fail. Since we also have the second property the
				   // copied value is still tested.
				   Destination->TerrainBulk.AdhesionOverlapFactor =
					   Source->TerrainBulk.AdhesionOverlapFactor;

				   // Bulk, Surface, and Wire properties, the old remains from when Terrain Material
				   // was a Shape Material, are not copied Barrier -> Asset, so we can't expect them
				   // to be automatically set to what the test harness expects. So just copy from
				   // the source. We don't actually care about these values, they are never used by
				   // the plugin.
				   //
				   // We do some ugly const_cast via void* here. I'm leaving it as-is because the
				   // data is guaranteed to be non-const at the declaration, this is data we don't
				   // care about, and this is only a test so the code isn't running on end-user
				   // machines.
				   RawByteCopyOverNotActuallyConst(
					   Destination->GetShapeMaterialBulkProperties(),
					   Source->GetShapeMaterialBulkProperties());
				   RawByteCopyOverNotActuallyConst(
					   Destination->GetShapeMaterialSurfaceProperties(),
					   Source->GetShapeMaterialSurfaceProperties());
				   RawByteCopyOverNotActuallyConst(
					   Destination->GetShapeMaterialWireProperties(),
					   Source->GetShapeMaterialWireProperties());

				   TestAllRealPropertiesHaveIncreasingValues(Destination, *this);

				   World->DestroyWorld(false);
			   });
		});
}
