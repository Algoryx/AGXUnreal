// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FAGXCommonModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
