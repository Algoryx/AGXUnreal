// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireRenderIterator.h"

#include "Wire/AGX_WireNode.h"

FAGX_WireRenderIterator::FAGX_WireRenderIterator(const FAGX_WireRenderIterator& InOther)
	: Barrier(InOther.Barrier)
{
}

FAGX_WireRenderIterator::FAGX_WireRenderIterator(FWireRenderIteratorBarrier&& InBarrier)
	: Barrier(std::move(InBarrier))
{
}

FAGX_WireRenderIterator& FAGX_WireRenderIterator::operator=(const FAGX_WireRenderIterator& InOther)
{
	Barrier = InOther.Barrier;
	return *this;
}

bool FAGX_WireRenderIterator::HasNative() const
{
	return Barrier.HasNative();
}

bool FAGX_WireRenderIterator::operator==(const FAGX_WireRenderIterator& InOther) const
{
	return Barrier == InOther.Barrier;
}

bool FAGX_WireRenderIterator::operator!=(const FAGX_WireRenderIterator& InOther) const
{
	return Barrier != InOther.Barrier;
}

FAGX_WireNode FAGX_WireRenderIterator::Get() const
{
	if (!HasNative())
	{
		return FAGX_WireNode();
	}
	return {Barrier.Get()};
}

FAGX_WireRenderIterator& FAGX_WireRenderIterator::Inc()
{
	Barrier.Inc();
	return *this;
}

FAGX_WireRenderIterator& FAGX_WireRenderIterator::Dec()
{
	Barrier.Dec();
	return *this;
}

FAGX_WireRenderIterator FAGX_WireRenderIterator::Next() const
{
	return {Barrier.Next()};
}

FAGX_WireRenderIterator FAGX_WireRenderIterator::Prev() const
{
	return {Barrier.Prev()};
}

/* Start of Blueprint Function Library. */

FAGX_WireNode UAGX_WireRenderIterator_FL::Get(UPARAM(Ref) FAGX_WireRenderIterator& Iterator)
{
	return Iterator.Get();
}

FAGX_WireRenderIterator UAGX_WireRenderIterator_FL::Next(UPARAM(Ref)
															 FAGX_WireRenderIterator& Iterator)
{
	return Iterator.Next();
}

FAGX_WireRenderIterator UAGX_WireRenderIterator_FL::Prev(UPARAM(Ref)
															 FAGX_WireRenderIterator& Iterator)
{
	return Iterator.Prev();
}
