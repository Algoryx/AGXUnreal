// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLXModelRegistry.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/OpenPLX/OpenPLXRefs.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"
#include "Utilities/PLXUtilities.h"


// Standard library includes.
#include <limits>

FPLXModelRegistry::FPLXModelRegistry()
	: Native(std::make_unique<FPLXModelDataArray>())
{
}

FPLXModelRegistry::~FPLXModelRegistry()
{
}

bool FPLXModelRegistry::HasNative() const
{
	return Native != nullptr;
}

void FPLXModelRegistry::ReleaseNative()
{
	Native = nullptr;
}

namespace FPLXModelRegistry_helpers
{
	FPLXModelRegistry::Handle Convert(size_t Val)
	{
		if (Val > std::numeric_limits<FPLXModelRegistry::Handle>::max())
		{
			// This should never ever happen. It means we have more than
			// int32::max number of models in the world.
			AGX_CHECK(false);
			return std::numeric_limits<FPLXModelRegistry::Handle>::max();
		}

		return static_cast<FPLXModelRegistry::Handle>(Val);
	}

	size_t Convert(FPLXModelRegistry::Handle Val)
	{
		check(Val >= 0);
		return static_cast<size_t>(Val);
	}

	using InputMap = std::unordered_map<std::string, std::shared_ptr<openplx::Physics::Signals::Input>>;

	InputMap MapInputs(openplx::Physics3D::System* System)
	{
		InputMap Inputs;
		if (System == nullptr)
			return Inputs;

		for (auto& Input : System->getNestedObjects<openplx::Physics::Signals::Input>())
		{
			if (Input == nullptr)
				continue;

			AGX_CHECK(!Inputs.contains(Input->getName()));
			Inputs.insert({Input->getName(), Input});
		}

		return Inputs;
	}
}

FPLXModelRegistry::Handle FPLXModelRegistry::Register(const FString& PLXFile)
{
	check(HasNative());

	Handle Handle = GetFrom(PLXFile);
	if (Handle == InvalidHandle) // We have never seen this PLX Model before.
		Handle = LoadNewModel(PLXFile);

	return Handle;
}

template <typename T>
T* FPLXModelRegistry::GetModelDataImpl(Handle Handle) const
{
	check(HasNative());
	if (Handle == InvalidHandle)
		return nullptr;

	const size_t Index = FPLXModelRegistry_helpers::Convert(Handle);
	if (Index >= Native->ModelData.size())
		return nullptr;

	return &Native->ModelData[Index];
}

const FPLXModelData* FPLXModelRegistry::GetModelData(Handle Handle) const
{
	return GetModelDataImpl<const FPLXModelData>(Handle);
}

FPLXModelData* FPLXModelRegistry::GetModelData(Handle Handle)
{
	return GetModelDataImpl<FPLXModelData>(Handle);
}

FPLXModelRegistry::Handle FPLXModelRegistry::GetFrom(const FString& PLXFile) const
{
	auto It = KnownModels.find(Convert(PLXFile));
	return It != KnownModels.end() ? It->second : InvalidHandle;
}

FPLXModelRegistry::Handle FPLXModelRegistry::LoadNewModel(const FString& PLXFile)
{
	// Here we create a new slot in the PLXModelData array with the PLX model tree as well
	// as some other required objects like the AGX Cache.
	AGX_CHECK(GetFrom(PLXFile) == InvalidHandle);

	FPLXModelData NewModel;
	NewModel.PLXModel = FPLXUtilities::LoadModel(PLXFile, NewModel.AGXCache);
	if (NewModel.PLXModel == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not read OpenPLX file '%s'. The Log category LogAGXDynamics may include "
				 "more details."),
			*PLXFile);
		return InvalidHandle;
	}

	auto System = std::dynamic_pointer_cast<openplx::Physics3D::System>(NewModel.PLXModel);
	if (System == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Could not get OpenPLX system from file '%s'. The OpenPLX model will not be loaded."),
			*PLXFile);
		return InvalidHandle;
	}

	NewModel.Inputs = FPLXModelRegistry_helpers::MapInputs(System.get());	
	const Handle NewHandle = FPLXModelRegistry_helpers::Convert(Native->ModelData.size());
	Native->ModelData.emplace_back(std::move(NewModel));
	KnownModels.insert({Convert(PLXFile), NewHandle});

	return NewHandle;
}
