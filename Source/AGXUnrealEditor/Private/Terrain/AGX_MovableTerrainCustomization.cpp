// Copyright 2024, Algoryx Simulation AB.

#include "Terrain/AGX_MovableTerrainCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_MovableTerrainCustomization"

TSharedRef<IDetailCustomization> FAGX_MovableTerrainCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_MovableTerrainCustomization);
}
void FAGX_MovableTerrainCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;


	IDetailCategoryBuilder& EditorCategory = DetailBuilder->EditCategory("AGX Editor");
	// Retrieve the UObject associated with the details panel
	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;
	DetailBuilder->GetObjectsBeingCustomized(ObjectsBeingCustomized);

	if (ObjectsBeingCustomized.Num() > 0)
	{
		// Assuming a single component is being customized
		if (UAGX_MovableTerrainComponent* Component =
				Cast<UAGX_MovableTerrainComponent>(ObjectsBeingCustomized[0]))
		{
			FText RebuildMeshesText = LOCTEXT("RebuildMesh", "Rebuild Mesh");

			EditorCategory.AddCustomRow(RebuildMeshesText)
				.NameContent()[SNew(STextBlock)
								   .Text(RebuildMeshesText)
								   .Font(IDetailLayoutBuilder::GetDetailFont())]
				.ValueContent()[SNew(SButton)
									.Text(RebuildMeshesText)
									.OnClicked_Lambda(
										[Component]() -> FReply
										{
											// Call RecreateMeshesEditor
											Component->RecreateMeshesEditor();
											return FReply::Handled();
										})];
		}
	}

	DetailBuilder->SortCategories(
	[](const TMap<FName, IDetailCategoryBuilder*>& CategoryMap)
	{
		for (const TPair<FName, IDetailCategoryBuilder*>& Pair : CategoryMap)
		{
			int32 SortOrder = Pair.Value->GetSortOrder();
			const FName CategoryName = Pair.Key;

			if (CategoryName == "AGX Editor")
			{
				SortOrder += 1;
			}
			else if (CategoryName == "AGX Movable Terrain")
			{
				SortOrder += 4;
			}
			else if (CategoryName == "AGX Terrain")
			{
				SortOrder += 3;
			}
			else if (CategoryName == "AGX Terrain Rendering")
			{
				SortOrder += 4;
			}
			else
			{
				const int32 ValueSortOrder = Pair.Value->GetSortOrder();
				if (ValueSortOrder >= SortOrder && ValueSortOrder < SortOrder + 10)
				{
					SortOrder += 10;
				}
				else
				{
					continue;
				}
			}

			Pair.Value->SetSortOrder(SortOrder);
		}
	});
}

#undef LOCTEXT_NAMESPACE
