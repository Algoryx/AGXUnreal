// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Cable/AGX_CableEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_CableNodeInfo.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_CableNodeInfo
{
	GENERATED_BODY()

	FAGX_CableNodeInfo() = default;

	FAGX_CableNodeInfo(
		EAGX_CableNodeType InNodeType, const FTransform& InWorldTransform,
		bool bInLockRotationToBody)
		: NodeType(InNodeType)
		, WorldTransform(InWorldTransform)
		, LockRotationToBody(bInLockRotationToBody)
	{
	}

	/**
	 * The type of node, e.g., Free, Body Fixed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Cable Node Info")
	EAGX_CableNodeType NodeType {EAGX_CableNodeType::Free};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Cable Node Info")
	FTransform WorldTransform;

	/**
	 * Whether or not this node has locked rotation in relation to the attached Body.
	 * Only relevant for 'BodyFixed' Node Type.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Cable Node Info",
		Meta = (EditCondition = "NodeType == EAGX_CableNodeType::BodyFixed"))
	bool LockRotationToBody {false};

	/**
	 * AGX Native uuid of attached Rigid Body. Only assigned in some contexts, for example during
	 * Play.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Cable Node Info",
		Meta = (EditCondition = "NodeType == EAGX_CableNodeType::BodyFixed"))
	FGuid BodyGuid;
};
