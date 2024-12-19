// Copyright 2024, Algoryx Simulation AB.

#include "AMOR/AGX_MergeSplitPropertiesBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AGXToUeContext.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "UObject/Class.h"

FAGX_MergeSplitPropertiesBase& FAGX_MergeSplitPropertiesBase::operator=(
	const FAGX_MergeSplitPropertiesBase& Other)
{
	bEnableMerge = Other.bEnableMerge;
	bEnableSplit = Other.bEnableSplit;
	return *this;
}

bool FAGX_MergeSplitPropertiesBase::operator==(const FAGX_MergeSplitPropertiesBase& Other) const
{
	return HasNative() == Other.HasNative() && Other.bEnableMerge == bEnableMerge &&
		   Other.bEnableSplit == bEnableSplit;
}

void FAGX_MergeSplitPropertiesBase::SetEnableMerge(bool bEnable)
{
	bEnableMerge = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableMerge(bEnable);
	}
}

bool FAGX_MergeSplitPropertiesBase::GetEnableMerge() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnableMerge();
	}

	return bEnableMerge;
}

void FAGX_MergeSplitPropertiesBase::SetEnableSplit(bool bEnable)
{
	bEnableSplit = bEnable;
	if (HasNative())
	{
		NativeBarrier.SetEnableSplit(bEnable);
	}
}

bool FAGX_MergeSplitPropertiesBase::GetEnableSplit() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnableSplit();
	}

	return bEnableSplit;
}

bool FAGX_MergeSplitPropertiesBase::HasNative() const
{
	return NativeBarrier.HasNative();
}

const FMergeSplitPropertiesBarrier* FAGX_MergeSplitPropertiesBase::GetNative() const
{
	return HasNative() ? &NativeBarrier : nullptr;
}

FMergeSplitPropertiesBarrier* FAGX_MergeSplitPropertiesBase::GetNative()
{
	return HasNative() ? &NativeBarrier : nullptr;
}

void FAGX_MergeSplitPropertiesBase::CopyFrom(
	const FMergeSplitPropertiesBarrier& Barrier, FAGX_AGXToUeContext* /*Context*/)
{
	bEnableMerge = Barrier.GetEnableMerge();
	bEnableSplit = Barrier.GetEnableSplit();
}
