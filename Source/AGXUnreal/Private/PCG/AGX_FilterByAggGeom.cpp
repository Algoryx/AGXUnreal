// Copyright 2026, Algoryx Simulation AB.

#include "PCG/AGX_FilterByAggGeom.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"

// Unreal Engine includes.
#include "PCGContext.h"
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
	return MakeShared<FAGX_FilterByAggGeom>();
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

bool FAGX_FilterByAggGeom::ExecuteInternal(FPCGContext* Context) const
{
	const UAGX_FilterByAggGeomSettings* Settings =
		Context->GetInputSettings<UAGX_FilterByAggGeomSettings>();

	auto ShouldInclude = [Settings](EAGX_FilterAggGeomTypes Type)
	{ return Settings->ShouldInclude(Type); };

	// The input is of type 'Attribute Set', not point data. The most important attribute for this
	// filter is 'ComponentReference', which is a Soft Object Path to the Component instance that
	// should be filtered.
	TArray<FPCGTaggedData> Inputs =
		Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
	TArray<FPCGTaggedData> Outputs = Context->OutputData.TaggedData;

	for (const FPCGTaggedData& Input : Inputs)
	{
		FPCGTaggedData& Output = Outputs.Add_GetRef(Input);

		const UPCGData* InputData = Input.Data;
		// TODO How do I get the Static Mesh Component from InputData?
		// TODO I need to get a hold of the 'ComponentReference' attribute, which is a Soft Object
		//      Path, and dereference / load that to get a UStaticMeshComponent pointer.
		UStaticMeshComponent* MeshComponent = nullptr; // TODO Read from InputData.

		bool bInclude = false;
		FKAggregateGeom& AggGeom = MeshComponent->GetStaticMesh()->GetBodySetup()->AggGeom;
		if (ShouldInclude(EAGX_FilterAggGeomTypes::Sphere) && !AggGeom.SphereElems.IsEmpty())
		{
			bInclude = true;
		}
		if (ShouldInclude(EAGX_FilterAggGeomTypes::Box) && !AggGeom.BoxElems.IsEmpty())
		{
			bInclude = true;
		}
		if (ShouldInclude(EAGX_FilterAggGeomTypes::Sphyl) && !AggGeom.SphylElems.IsEmpty())
		{
			bInclude = true;
		}

		// TODO Copy input tagged data to output.

		if (bInclude)
		{
			Output.Pin = PCGPinConstants::DefaultInFilterLabel;
		}
		else
		{
			Output.Pin = PCGPinConstants::DefaultOutFilterLabel;
		}
	}
}

#undef LOCTEXT_NAMESPACE
