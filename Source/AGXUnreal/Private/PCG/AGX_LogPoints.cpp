// Copyright 2026, Algoryx Simulation AB.

#include "PCG/AGX_LogPoints.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Data/PCGPointData.h"
#include "PCGContext.h"

#define LOCTEXT_NAMESPACE "AGX_LogPoints"

#if WITH_EDITOR
FName UAGX_LogPointsSettings::GetDefaultNodeName() const
{
	static const FName Name(TEXT("LogPoints"));
	return Name;
}

FText UAGX_LogPointsSettings::GetDefaultNodeTitle() const
{
	static const FText Title(LOCTEXT("NodeTitle", "Log Points"));
	return Title;
}

FText UAGX_LogPointsSettings::GetNodeTooltipText() const
{
	static const FText Tooltip(LOCTEXT("NodeTooltip", "Log Points"));
	return Tooltip;
}

EPCGSettingsType UAGX_LogPointsSettings::GetType() const
{
	return EPCGSettingsType::PointOps;
}
#endif // WITH_EDITOR

TArray<FPCGPinProperties> UAGX_LogPointsSettings::InputPinProperties() const
{
	return Super::DefaultPointInputPinProperties();
}

TArray<FPCGPinProperties> UAGX_LogPointsSettings::OutputPinProperties() const
{
	return Super::DefaultPointOutputPinProperties();
}

FPCGElementPtr UAGX_LogPointsSettings::CreateElement() const
{
	return MakeShared<FAGX_LogPointsElement>();
}

bool FAGX_LogPointsElement::ExecuteInternal(FPCGContext* Context) const
{
	const UAGX_LogPointsSettings* Settings = Context->GetInputSettings<UAGX_LogPointsSettings>();

	TArray<FPCGTaggedData> InputData = Context->InputData.GetAllSpatialInputs();
	for (const FPCGTaggedData& InputDatum : InputData)
	{
		const UPCGPointData* PointData = Cast<UPCGPointData>(InputDatum.Data);
		if (PointData == nullptr)
			continue;

		const TArray<FPCGPoint>& Points = PointData->GetPoints();
		for (const FPCGPoint& Point : Points)
		{
			ProcessPoint(Point, *Settings);
		}
	}

	Context->OutputData = Context->InputData;
	return true;
}

void FAGX_LogPointsElement::ProcessPoint(const FPCGPoint& Point, const UAGX_LogPointsSettings& Settings) const
{
	FString Prefix = Settings.bWithPrefix ? Settings.Prefix : TEXT("");
	UE_LOG(LogAGX, Warning, TEXT("%s Point: Position=%s"), *Prefix, *Point.Transform.GetLocation().ToString());
}

#undef LOCTEXT_NAMESPACE
