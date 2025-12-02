// Copyright 2025, Algoryx Simulation AB.

#include "CollisionGroups/AGX_CollisionGroupPair.h"

bool FAGX_CollisionGroupPair::IsEqual(const FName& GroupA, const FName& GroupB) const
{
	return (Group1.IsEqual(GroupA) && Group2.IsEqual(GroupB)) ||
		   (Group1.IsEqual(GroupB) && Group2.IsEqual(GroupA));
}

bool FAGX_CollisionGroupPair::IsIn(const TArray<FAGX_CollisionGroupPair>& Pairs) const
{
	for (const auto& Pair : Pairs)
	{
		// IsEqual checks both permutations.
		if (IsEqual(Pair.Group1, Pair.Group2))
			return true;
	}

	return false;
}
