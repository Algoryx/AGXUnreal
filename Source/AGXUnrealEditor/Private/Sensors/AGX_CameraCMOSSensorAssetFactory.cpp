// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraCMOSSensorAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraCMOSSensor.h"

UAGX_CameraCMOSSensorAssetFactory::UAGX_CameraCMOSSensorAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_CameraCMOSSensor::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_CameraCMOSSensorAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_CameraCMOSSensor::StaticClass()));
	return NewObject<UAGX_CameraCMOSSensor>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
