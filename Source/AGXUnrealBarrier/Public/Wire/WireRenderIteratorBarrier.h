// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireNodeBarrier.h"

// Standard library includes.
#include <memory>

// Testing if we can forward declare the AGX Dynamics type in the Barrier header file.
namespace agxWire
{
	class RenderIterator;
}

class AGXUNREALBARRIER_API FWireRenderIteratorBarrier
{
public:
	FWireRenderIteratorBarrier();
	FWireRenderIteratorBarrier(const FWireRenderIteratorBarrier& InOther);
	FWireRenderIteratorBarrier(FWireRenderIteratorBarrier&& InNative);
	FWireRenderIteratorBarrier(std::unique_ptr<agxWire::RenderIterator>&& InNative);
	~FWireRenderIteratorBarrier();

	/**
	 * Compare the native iterators for equality.
	 *
	 * Not that it's equality and not identity, so two separate Barriers with different are still
	 * considered equal if they identify the same underlying native Wire Node.
	 *
	 * @param Other The Render Iterator to compare against.
	 * @return True if both Render Iterators point to the same native Wire Node.
	 */
	bool operator==(const FWireRenderIteratorBarrier& Other) const;

	/**
	 * Compare the native iterators for inequality.
	 *
	 * Not that it's equality and not identity, so two separate Barriers with different are still
	 * considered equal if they identify the same underlying native Wire Node.
	 *
	 * @param Other The Render Iterator to compare against.
	 * @return True if the two Render Iterators point to different native Wire Nodes.
	 */
	bool operator!=(const FWireRenderIteratorBarrier& Other) const;

	FWireRenderIteratorBarrier& operator=(const FWireRenderIteratorBarrier& InOther);

	bool HasNative() const;

	FWireNodeBarrier Get() const;
	void Inc();
	void Dec();
	FWireRenderIteratorBarrier Next() const;
	FWireRenderIteratorBarrier Prev() const;

private:
	std::unique_ptr<agxWire::RenderIterator> Native;
};
