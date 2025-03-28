// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "OpenPLX/PLX_Inputs.h"
#include "OpenPLX/PLX_Outputs.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/Object.h"
#include "agxOpenPLX/AgxCache.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <memory>
#include <unordered_set>
#include <vector>

namespace openplx
{
	namespace Physics3D
	{
		class System;
	}

	namespace Core
	{
		class Object;
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

class FPLXUtilities
{
public:
	static openplx::Core::ObjectPtr LoadModel(
		const FString& Filename, std::shared_ptr<agxopenplx::AgxCache> AGXCache);

	static bool HasInputs(openplx::Physics3D::System* System);
	static bool HasOutputs(openplx::Physics3D::System* System);

	static TArray<FPLX_Input> GetInputs(openplx::Physics3D::System* System);
	static TArray<FPLX_Output> GetOutputs(openplx::Physics3D::System* System);

	static EPLX_InputType GetInputType(const openplx::Physics::Signals::Input& Input);
	static EPLX_OutputType GetOutputType(const openplx::Physics::Signals::Output& Output);

	/**
	* Based on Object::getNestedObjects in OpenPLX, but calling that function crashes due to different allocators used.
	*/
	template <class T>
	static std::vector<std::shared_ptr<T>> GetNestedObjects(openplx::Core::Object& Object);

	static std::unordered_set<openplx::Core::ObjectPtr> GetNestedObjectFields(
		openplx::Core::Object& Object);
	static void GetNestedObjectFields(
		openplx::Core::Object& Object, std::unordered_set<openplx::Core::ObjectPtr>& Output);
	static std::vector<openplx::Core::ObjectPtr> GetObjectFields(openplx::Core::Object& Object);
};

template <class T>
std::vector<std::shared_ptr<T>> FPLXUtilities::GetNestedObjects(openplx::Core::Object& Object)
{
	std::vector<std::shared_ptr<T>> Output;
	std::unordered_set<openplx::Core::ObjectPtr> Objects = GetNestedObjectFields(Object);
	for (auto& Obj : Objects)
		if (auto Target = std::dynamic_pointer_cast<T>(Obj))
			Output.push_back(Target);

	return Output;
}
