// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_GenericHorizontalSweepParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"

UAGX_GenericHorizontalSweepParametersAssetFactory::UAGX_GenericHorizontalSweepParametersAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_GenericHorizontalSweepParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_GenericHorizontalSweepParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_GenericHorizontalSweepParameters::StaticClass()));
	return NewObject<UAGX_GenericHorizontalSweepParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
