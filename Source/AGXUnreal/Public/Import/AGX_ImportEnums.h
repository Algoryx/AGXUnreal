// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/** Specifies what type of import is being performed. */
UENUM()
enum class EAGX_ImportType : uint8
{
	/** Imported type is Invalid. */
	Invalid,

	/** Imported type is an AGX Dynamics Archive. */
	Agx,

	/** Imported type is a OpenPLX model. */
	Plx,

	/** Imported type is a URDF (Unified Robotic Description Format) model. */
	Urdf
};

/* TODO: add decription */
UENUM()
enum class EAGX_ImportResult : uint8
{
	Success = 1 << 0,
	RecoverableErrorsOccured = 1 << 1,
	FatalError = 1 << 2,
	Invalid = 1 << 3
};

inline EAGX_ImportResult& operator|=(EAGX_ImportResult& InOutLhs, EAGX_ImportResult InRhs)
{
	uint8 Lhs = (uint8) InOutLhs;
	uint8 Rhs = (uint8) InRhs;
	uint8 result = Lhs | Rhs;
	InOutLhs = (EAGX_ImportResult) result;
	return InOutLhs;
}

inline EAGX_ImportResult operator&(EAGX_ImportResult InLhs, EAGX_ImportResult InRhs)
{
	uint8 Lhs = (uint8) InLhs;
	uint8 Rhs = (uint8) InRhs;
	uint8 Result = Lhs & Rhs;
	return (EAGX_ImportResult) Result;
}

inline bool IsUnrecoverableError(EAGX_ImportResult Result)
{
	return (uint8) (Result & EAGX_ImportResult::FatalError) != 0 ||
		   (uint8) (Result & EAGX_ImportResult::Invalid) != 0;
}
