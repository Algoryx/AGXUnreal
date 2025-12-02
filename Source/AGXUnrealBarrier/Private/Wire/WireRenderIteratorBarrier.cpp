// Copyright 2025, Algoryx Simulation AB.

#include "Wire/WireRenderIteratorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/Wire/WireNodeRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxWire/RenderIterator.h>
#include "EndAGXIncludes.h"

FWireRenderIteratorBarrier::FWireRenderIteratorBarrier()
{
}

FWireRenderIteratorBarrier::FWireRenderIteratorBarrier(const FWireRenderIteratorBarrier& InOther)
	: Native(std::make_unique<agxWire::RenderIterator>(*InOther.Native))
{
}

FWireRenderIteratorBarrier::FWireRenderIteratorBarrier(
	std::unique_ptr<agxWire::RenderIterator>&& InNative)
	: Native(std::move(InNative))
{
}

FWireRenderIteratorBarrier::FWireRenderIteratorBarrier(FWireRenderIteratorBarrier&& InNative)
	: Native(std::move(InNative.Native))
{
}

FWireRenderIteratorBarrier& FWireRenderIteratorBarrier::operator=(
	const FWireRenderIteratorBarrier& InOther)
{
	if (InOther.HasNative())
	{
		Native = std::make_unique<agxWire::RenderIterator>(*InOther.Native);
	}
	else
	{
		Native = nullptr;
	}
	return *this;
}

bool FWireRenderIteratorBarrier::operator==(const FWireRenderIteratorBarrier& Other) const
{
	if (Native == nullptr && Other.Native == nullptr)
	{
		return true;
	}
	if ((Native == nullptr) != (Other.Native == nullptr))
	{
		return false;
	}
	return *Native == *Other.Native;
}

bool FWireRenderIteratorBarrier::operator!=(const FWireRenderIteratorBarrier& Other) const
{
	return !(*this == Other);
}

bool FWireRenderIteratorBarrier::HasNative() const
{
	return Native.get() != nullptr;
}

FWireRenderIteratorBarrier::~FWireRenderIteratorBarrier()
{
}

FWireNodeBarrier FWireRenderIteratorBarrier::Get() const
{
	check(HasNative());
	agxWire::Node* Node = Native->get();
	return {std::make_unique<FWireNodeRef>(Node)};
}

void FWireRenderIteratorBarrier::Inc()
{
	Native->inc();
}

void FWireRenderIteratorBarrier::Dec()
{
	Native->dec();
}

FWireRenderIteratorBarrier FWireRenderIteratorBarrier::Next() const
{
	return {std::make_unique<agxWire::RenderIterator>(Native->next())};
}

FWireRenderIteratorBarrier FWireRenderIteratorBarrier::Prev() const
{
	return {std::make_unique<agxWire::RenderIterator>(Native->prev())};
}
