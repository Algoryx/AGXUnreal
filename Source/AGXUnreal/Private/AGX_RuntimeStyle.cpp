// Copyright 2025, Algoryx Simulation AB.

#include "AGX_RuntimeStyle.h"

#include "Runtime/Projects/Public/Interfaces/IPluginManager.h"
#include "Runtime/SlateCore/Public/Styling/SlateStyle.h"
#include "Runtime/SlateCore/Public/Styling/SlateStyleRegistry.h"
#include "Runtime/SlateCore/Public/Styling/SlateTypes.h"
#include "Framework/Application/SlateApplication.h"
#include "SlateOptMacros.h"

TSharedPtr<FSlateStyleSet> FAGX_RuntimeStyle::StyleInstance = nullptr;

void FAGX_RuntimeStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FAGX_RuntimeStyle::Shutdown()
{
	if (StyleInstance.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance.Get());
		ensure(StyleInstance.IsUnique());
		StyleInstance.Reset();
	}
}

void FAGX_RuntimeStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

TSharedPtr<class ISlateStyle> FAGX_RuntimeStyle::Get()
{
	return StyleInstance;
}

FName FAGX_RuntimeStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("AGX_RuntimeStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH(RelativePath, ...) \
	FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BOX_BRUSH(RelativePath, ...) \
	FSlateBoxBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define BORDER_BRUSH(RelativePath, ...) \
	FSlateBorderBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)
#define TTF_FONT(RelateivePath, ...) \
	FSlateFontInfo(Style->RootToContentDi(RelateivePath, TEXT(".ttf")), __VA_ARGS__)
#define OTF_FONT(RelateivePath, ...) \
	FSlateFontInfo(Style->RootToContentDir(RelateivePath, TEXT(".otf")), __VA_ARGS__)

namespace
{
	const FVector2D IconSize16(16.0f, 16.0f);
	const FVector2D IconSize40(40.0f, 40.0f);
	const FVector2D IconSize128(128.0f, 128.0f);
}

TSharedRef<class FSlateStyleSet> FAGX_RuntimeStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
	Style->SetContentRoot(
		IPluginManager::Get().FindPlugin("AGXUnreal")->GetContentDir() / TEXT("Runtime"));

	// Define icons and stuff here.

	return Style;
};

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT
