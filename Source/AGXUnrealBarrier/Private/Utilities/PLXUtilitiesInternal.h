// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSDK/Assembly.h>
#include "EndAGXIncludes.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/Error.h"
#include "openplx/Object.h"
#include "agxOpenPLX/AgxCache.h"
#include "agxOpenPLX/AllocationUtils.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>
#include <unordered_set>
#include <string>
#include <utility>
#include <vector>

class FConstraintBarrier;
class FSimulationBarrier;
struct FOpenPLXMappingBarriersCollection;
struct FRigidBodyBarrier;

namespace openplx
{
	namespace Physics3D
	{
		class System;
	}

	namespace Physics
	{
		namespace Signals
		{
			class Input;
			class Output;
		}
	}
}

class FPLXUtilitiesInternal
{
public:
	static openplx::Core::ObjectPtr LoadModel(
		const FString& Filename, std::shared_ptr<agxopenplx::AgxCache> AGXCache);

	static bool HasInputs(openplx::Physics3D::System* System);
	static bool HasOutputs(openplx::Physics3D::System* System);

	static TArray<FOpenPLX_Input> GetInputs(openplx::Physics3D::System* System);
	static TArray<FOpenPLX_Output> GetOutputs(openplx::Physics3D::System* System);

	static EOpenPLX_InputType GetInputType(const openplx::Physics::Signals::Input& Input);
	static EOpenPLX_OutputType GetOutputType(const openplx::Physics::Signals::Output& Output);

	/**
	 * Takes a list of OpenPLX bundle paths and builds a single ; separated std::string from them
	 * which is used in some places in the agxopenplx namespace in AGX Dynamics.
	 */
	static std::string BuildBundlePathsString(const TArray<FString>& Paths);

	/**
	 * Based on Object::getNestedObjects in OpenPLX, but calling that function crashes due to
	 * different allocators used.
	 */
	template <typename T>
	static std::vector<std::shared_ptr<T>> GetNestedObjects(openplx::Core::Object& Object);

	/**
	 * Based on Object::getEntries in OpenPLX, but calling that function crashes due to
	 * different allocators used.
	 * Return all attribute name,value pairs of the specified type T.
	 */
	template <typename T>
	static std::vector<std::pair<std::string, std::shared_ptr<T>>> GetEntries(
		openplx::Core::Object& Object);

	static std::unordered_set<openplx::Core::ObjectPtr> GetNestedObjectFields(
		openplx::Core::Object& Object);
	static void GetNestedObjectFields(
		openplx::Core::Object& Object, std::unordered_set<openplx::Core::ObjectPtr>& Output);
	static std::vector<openplx::Core::ObjectPtr> GetObjectFields(openplx::Core::Object& Object);

	static TArray<FString> GetErrorStrings(const openplx::Errors& Errors);

	// Returns true if errors was found.
	static bool LogErrorsSafe(openplx::Errors&& Errors, const FString& ErrorMessagePostfix = "");

	/**
	 * Given an OpenPLX System, and all relevant AGX objects part of the simulated model instance,
	 * this function creates all runtime-mapped objects (such as DriveTrain) and returns an
	 * agxSDK::Assembly containing all AGX Dynamics objects needed for the
	 * agxopenplx mapping classes. This functions will
	 * also add objects that are created by it to the passed Simulation (as well as to the returned
	 * Assembly).
	 */
	static agxSDK::AssemblyRef MapRuntimeObjects(
		std::shared_ptr<openplx::Physics3D::System> System, FSimulationBarrier& Simulation,
		const FOpenPLXMappingBarriersCollection& Barriers);

	/**
	 * On OpenPlx, a default PowerLine is created holding all DriveTrains in the model. This has a
	 * specific name which this function returns.
	 */
	static std::string GetDefaultPowerLineName();
};

template <typename T>
std::vector<std::shared_ptr<T>> FPLXUtilitiesInternal::GetNestedObjects(
	openplx::Core::Object& Object)
{
	std::vector<std::shared_ptr<T>> Output;
	std::unordered_set<openplx::Core::ObjectPtr> Objects = GetNestedObjectFields(Object);
	for (auto& Obj : Objects)
		if (auto Target = std::dynamic_pointer_cast<T>(Obj))
			Output.push_back(Target);

	return Output;
}

template <typename T>
std::vector<std::pair<std::string, std::shared_ptr<T>>> FPLXUtilitiesInternal::GetEntries(
	openplx::Core::Object& Object)
{
	std::vector<std::pair<std::string, std::shared_ptr<T>>> Output;
	std::vector<std::pair<std::string, openplx::Core::Any>> AllEntries;
	Object.extractEntriesTo(AllEntries);
	for (auto& Entry : AllEntries)
	{
		if (!Entry.second.isObject())
			continue;
		auto Obj = std::dynamic_pointer_cast<T>(Entry.second.asObject());
		if (Obj == nullptr)
			continue;

		Output.push_back({Entry.first, Obj});
	}

	agxopenplx::freeContainerMemory(AllEntries);
	return Output;
}
