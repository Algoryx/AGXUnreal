// Copyright 2026, Algoryx Simulation AB.

#include "PCG/AGX_FilterByAggGeom.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"

// Unreal Engine includes.
#include "Components/StaticMeshComponent.h"
#include "Elements/PCGAddComponent.h"
#include "Metadata/Accessors/PCGAttributeAccessorHelpers.h"
#include "Metadata/PCGAttributePropertySelector.h"
#include "PCGContext.h"
#include "PCGParamData.h"
#include "PhysicsEngine/BodySetup.h"

#define LOCTEXT_NAMESPACE "AGX_FilterByAggGeom"

#if WITH_EDITOR

FName UAGX_FilterByAggGeomSettings::GetDefaultNodeName() const
{
	static const FName Name(TEXT("FilterByAggGeom"));
	return Name;
}

FText UAGX_FilterByAggGeomSettings::GetDefaultNodeTitle() const
{
	static const FText Title(LOCTEXT("NodeTitle", "Filter by AggGeom"));
	return Title;
}

FText UAGX_FilterByAggGeomSettings::GetNodeTooltipText() const
{
	static const FText Tooltip(LOCTEXT("NodeTooltipText", "Filter by AggGeom"));
	return Tooltip;
}

EPCGChangeType UAGX_FilterByAggGeomSettings::GetChangeTypeForProperty(
	const FName& InPropertyName) const
{
	return (InPropertyName == AGX_MEMBER_NAME(ToInclude))
			   ? EPCGChangeType::Settings
			   : Super::GetChangeTypeForProperty(InPropertyName);
}

#endif

FPCGElementPtr UAGX_FilterByAggGeomSettings::CreateElement() const
{
	return MakeShared<FAGX_FilterByAggGeomElement>();
}

EAGX_FilterAggGeomTypes UAGX_FilterByAggGeomSettings::GetToInclude() const
{
	return static_cast<EAGX_FilterAggGeomTypes>(ToInclude);
}

bool UAGX_FilterByAggGeomSettings::ShouldInclude(EAGX_FilterAggGeomTypes Type) const
{
	const uint32 T = static_cast<uint32>(Type);
	return (ToInclude & T) != 0;
}

bool FAGX_FilterByAggGeomElement::ExecuteInternal(FPCGContext* Context) const
{
	// Get the settings, which will tell us what types of primitives we should be filtering on.
	const UAGX_FilterByAggGeomSettings* Settings =
		Context->GetInputSettings<UAGX_FilterByAggGeomSettings>();

	// Local variables to reduce typing later.
	const bool bIncludeSphere = Settings->ShouldInclude(EAGX_FilterAggGeomTypes::Sphere);
	const bool bIncludeBox = Settings->ShouldInclude(EAGX_FilterAggGeomTypes::Box);
	const bool bIncludeSphyl = Settings->ShouldInclude(EAGX_FilterAggGeomTypes::Sphyl);

	// Get all inputs that contain attribute sets, which is what Get Actor Data will output in
	// Get Components Reference mode.
	TArray<FPCGTaggedData> Inputs =
		Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);

	// Shorter name for the output data, to reduce typing later.
	TArray<FPCGTaggedData>& Outputs = Context->OutputData.TaggedData;

	// Loop over all the inputs and for each input loop over all the entries in the attribute set.
	// Each entry corresponds to a Component Reference that "should" point to a Static Mesh
	// Component. If it does then check if the Static Mesh Component has at least one primitive
	// collision of a type we are filtering for. If so, add it to the in-output, otherwise add it
	// to the out-output.
	for (const FPCGTaggedData& Input : Inputs)
	{
		// Get the attribute set data, skip this input if it was some other type.
		// (Should never happen since we used GetAllParams.)
		const UPCGParamData* ParamData = Cast<UPCGParamData>(Input.Data);
		if (ParamData == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("FAGX_FilterByAggGeomElement::ExecuteInternal got an input that is not a "
					 "UPCGParamData."));
			continue;
		}

		// Create an attribute selector that selects the Component Reference attribute.
		FPCGAttributePropertyInputSelector Selector;
		Selector.SetAttributeName(PCGAddComponentConstants::ComponentReferenceAttribute);

		// Extract all Component Reference attribute values from the current input's attribute set.
		TArray<FSoftObjectPath> ComponentRefs;
		if (!PCGAttributeAccessorHelpers::ExtractAllValues<FSoftObjectPath>(
				ParamData, Selector, ComponentRefs, Context,
				EPCGAttributeAccessorFlags::AllowBroadcastAndConstructible))
		{
			PCGE_LOG(
				Warning, GraphAndLog,
				LOCTEXT(
					"ExtractAllValuesFailed",
					"FAGX_FilterByAggGeomElement::ExecuteInternal got ParamData from which "
					"FSoftObjectPaths could not be extracted."));
			UE_LOG(
				LogAGX, Warning,
				TEXT("FAGX_FilterByAggGeomElement::ExecuteInternal got ParamData from which "
					 "FSoftObjectPaths could not be extracted."));
			continue;
		}

		// The attribute set entries that match the filter, i.e. has at least one collision
		// primitive of a wanted type. This is the in-output.
		TArray<PCGMetadataEntryKey> InEntryKeys;

		// The attribute set entries that does not match the filter, i.e. does not have any of the
		// wanted collision primitive types. This is the out-output.
		TArray<PCGMetadataEntryKey> OutEntryKeys;

		// Loop over the Component References in the current attribute set and determine if the
		// Static Mesh Component pointed to has any of the wanted primitive collision types or not.
		for (int32 I = 0; I < ComponentRefs.Num(); ++I)
		{
			// Get the Static Mesh Component, if any. If the Component isn't a Static Mesh Component
			// then it is put in the out-output.
			UStaticMeshComponent* MeshComponent =
				Cast<UStaticMeshComponent>(ComponentRefs[I].ResolveObject());
			bool bInclude = false;
			if (MeshComponent != nullptr)
			{
				// We have a Static Mesh Component. Check if it has any of the collision primitives
				// we are looking for.
				if (const UBodySetup* BodySetup = MeshComponent->GetBodySetup())
				{
					const FKAggregateGeom& AggGeom = BodySetup->AggGeom;
					bInclude |= bIncludeSphere && !AggGeom.SphereElems.IsEmpty();
					bInclude |= bIncludeBox && !AggGeom.BoxElems.IsEmpty();
					bInclude |= bIncludeSphyl && !AggGeom.SphylElems.IsEmpty();
				}
			}

			if (bInclude)
			{
				// This Static Mesh should be included.
				InEntryKeys.Add(I);
			}
			else
			{
				// This Static Mesh should not be included.
				OutEntryKeys.Add(I);
			}
		}

		const UPCGMetadata* InputMetadata = ParamData->Metadata;

		// Create a new attribute set for the in-output, i.e. the entries that matched the filter.
		UPCGParamData* InFilterParamData = FPCGContext::NewObject_AnyThread<UPCGParamData>(Context);

		// Copy the entries whose Static Mesh passed the include filter from the input to the
		// in-output.
		InFilterParamData->Metadata->InitializeAsCopy(
			FPCGMetadataInitializeParams(InputMetadata, &InEntryKeys));

		// Create a Tagged Data object to hold our newly created in-output attribute set and name it
		// according to the pin naming convention for filter nodes.
		FPCGTaggedData& InOutput = Outputs.Emplace_GetRef(Input);
		InOutput.Data = InFilterParamData;
		InOutput.Pin = PCGPinConstants::DefaultInFilterLabel;

		// Create a new attribute set for the out-output, i.e. the entries that did not match the
		// filter.
		UPCGParamData* OutFilterParamData =
			FPCGContext::NewObject_AnyThread<UPCGParamData>(Context);

		// Copy the entries whose Static Mesh did not pass the include filter from the input to the
		// out-output.
		OutFilterParamData->Metadata->InitializeAsCopy(
			FPCGMetadataInitializeParams(InputMetadata, &OutEntryKeys));

		// Create a Tagged Data object to hold our newly created out-output attribute set and name
		// it according to the pin naming convention for filter nodes.
		FPCGTaggedData& OutOutput = Outputs.Emplace_GetRef(Input);
		OutOutput.Data = OutFilterParamData;
		OutOutput.Pin = PCGPinConstants::DefaultOutFilterLabel;
	}

	return true;
}

#undef LOCTEXT_NAMESPACE
