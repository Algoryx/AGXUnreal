// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Standard library includes.
#include <memory>
#include <string>
#include <unordered_map>

class FSimulationBarrier;

struct FAssemblyRef;
struct FPLXModelData;
struct FPLXModelDataArray;

class AGXUNREALBARRIER_API FPLXModelRegistry
{
public:
	using Handle = int32;
	inline static constexpr Handle InvalidHandle = -1;

	FPLXModelRegistry();
	~FPLXModelRegistry();

	bool HasNative() const;
	void ReleaseNative();

	/**
	 * The PLXFile is the absolute path of a OpenPLX model to be loaded.
	 *
	 * The Handle returned can be used to later access the loaded OpenPLX model. This Handle will be
	 * shared by all who register the same PLX file, for example when the same PLX model is
	 * instanced many times in the same Level.
	 */
	Handle Register(const FString& PLXFile);

	/**
	 * Important note: the lifetime of the returned FPLXModelData is only guaranteed during direct
	 * usage in local scope. It is not thread safe. Do not store this pointer for later use.
	 */
	const FPLXModelData* GetModelData(Handle Handle) const;
	FPLXModelData* GetModelData(Handle Handle);

private:
	FPLXModelRegistry(const FPLXModelRegistry&) = delete;
	void operator=(const FPLXModelRegistry&) = delete;

	template <typename T>
	T* GetModelDataImpl(Handle Handle) const;

	Handle GetFrom(const FString& PLXFile) const;
	Handle LoadNewModel(const FString& PLXFile);

	std::unique_ptr<FPLXModelDataArray> Native;
	std::unordered_map<std::string, Handle> KnownModels;
};
