#pragma once

#include "CoreMinimal.h"

UENUM()
enum EAGX_TransformTarget
{
	TT_SELF = 0 UMETA(DisplayName = "Self",	ToolTip="Synchronize AGX Dynamics transformations the local transformation."),
	TT_PARENT = 1 UMETA(DisplayName = "Parent", ToolTip = "Synchronize AGX Dynamics transformations to the local transformation of the parent."),
	TT_ROOT = 2 UMETA(DisplayName = "Root", ToolTip = "Synchronize AGX Dynamics transformations to the root.")
};