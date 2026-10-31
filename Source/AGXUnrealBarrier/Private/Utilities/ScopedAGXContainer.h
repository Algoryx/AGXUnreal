// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "agxOpenPLX/AllocationUtils.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ScopedAGXContainer_helpers
{
	// Extend only after verifying that moving the container transfers its native allocation.
	template <typename ContainerT>
	struct TSupportedContainer : std::is_same<ContainerT, std::string>
	{
	};

	template <typename ValueT>
	struct TSupportedContainer<std::vector<ValueT>> : std::true_type
	{
	};
}

/**
 * Owns a container received from AGX and releases its storage using a native cleanup function.
 * Moving in transfers the native buffer; destruction empties it inside AGX before the member's
 * normal destructor runs. Contents are read-only to prevent introducing Unreal allocations.
 *
 * Use only for containers whose storage and owned elements were allocated by the native library.
 * The selected freeContainerMemory instantiation must be exported by that library. This does not
 * make later copies or transfers across DLL boundaries allocator-safe.
 */
template <typename ContainerT, void (*FreeMemory)(ContainerT&)>
class TScopedAGXContainer
{
	static_assert(
		ScopedAGXContainer_helpers::TSupportedContainer<ContainerT>::value,
		"Scoped AGX containers support std::string and std::vector with the default allocator.");
	static_assert(
		std::is_nothrow_move_constructible_v<ContainerT>,
		"Moving a native container into its scope owner must not throw.");

public:
	explicit TScopedAGXContainer(ContainerT&& InContainer) noexcept
		: Container(std::move(InContainer))
	{
	}

	~TScopedAGXContainer() noexcept
	{
		FreeMemory(Container);
	}

	TScopedAGXContainer(const TScopedAGXContainer&) = delete;
	TScopedAGXContainer& operator=(const TScopedAGXContainer&) = delete;
	TScopedAGXContainer(TScopedAGXContainer&&) = delete;
	TScopedAGXContainer& operator=(TScopedAGXContainer&&) = delete;

	const ContainerT& Get() const& noexcept
	{
		return Container;
	}

	const ContainerT& Get() const&& = delete;

private:
	ContainerT Container;
};

/** Adopt a native-returned rvalue whose cleanup is exported by agxUtil. */
template <typename ContainerT>
auto MakeScopedAGXContainer(ContainerT&& Container) noexcept
{
	static_assert(
		!std::is_lvalue_reference_v<ContainerT>,
		"Pass a native-returned container rvalue; existing lvalues are not adopted implicitly.");
	using ValueT = std::remove_reference_t<ContainerT>;
	return TScopedAGXContainer<ValueT, &agxUtil::freeContainerMemory<ValueT>>(std::move(Container));
}

/** Adopt a native-returned rvalue whose cleanup is exported by agxopenplx. */
template <typename ContainerT>
auto MakeScopedOpenPLXContainer(ContainerT&& Container) noexcept
{
	static_assert(
		!std::is_lvalue_reference_v<ContainerT>,
		"Pass a native-returned container rvalue; existing lvalues are not adopted implicitly.");
	using ValueT = std::remove_reference_t<ContainerT>;
	return TScopedAGXContainer<ValueT, &agxopenplx::freeContainerMemory<ValueT>>(
		std::move(Container));
}
