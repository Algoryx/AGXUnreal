// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/PLXControlMetadata.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/OpenPLX_Utilities.h"
#include "Utilities/PLXUtilitiesInternal.h"
#include "Utilities/ScopedAGXContainer.h"

// OpenPLX includes.
#include "BeginAGXIncludes.h"
#include "openplx/Control/Virtual/AffineTransform1DInput.h"
#include "openplx/Control/Virtual/AffineTransform1DOutput.h"
#include "openplx/Control/Virtual/BroadcastInput.h"
#include "openplx/Marshalling.h"
#include "openplx/Object.h"
#include "openplx/Physics/Signals/Input.h"
#include "openplx/Physics/Signals/Marshalling.h"
#include "openplx/Physics/Signals/Output.h"
#include "EndAGXIncludes.h"

// Standard library includes.
#include <map>
#include <string>
#include <unordered_set>

namespace PLXControlMetadata
{
	using FFields = std::map<std::string, openplx::FieldType>;
	using FPath = std::unordered_set<const openplx::Core::Object*>;

	template <typename TypeT>
	struct TMetadata
	{
		TypeT Type {TypeT::Unknown};
		EOpenPLX_ValueLayout Layout {EOpenPLX_ValueLayout::Unsupported};
		EOpenPLX_VirtualType VirtualType {EOpenPLX_VirtualType::None};
		FFields Fields;
	};

	using FInputMetadata = TMetadata<EOpenPLX_InputType>;
	using FOutputMetadata = TMetadata<EOpenPLX_OutputType>;

	// Keep cycle detection local to the current branch, allowing shared targets in other branches.
	struct FVisit
	{
		FPath& Path;
		const openplx::Core::Object* Object;
		~FVisit()
		{
			Path.erase(Object);
		}
	};

	EOpenPLX_ValueLayout GetLayout(const FFields& Fields)
	{
		using Layout = EOpenPLX_ValueLayout;
		using FieldType = openplx::FieldType;
		if (Fields.size() == 1)
		{
			switch (Fields.begin()->second)
			{
				case FieldType::Real:
					return Layout::Real;
				case FieldType::Bool:
					return Layout::Boolean;
				case FieldType::Int:
					return Layout::Integer;
				case FieldType::UInt:
					return Layout::UnsignedInteger;
				default:
					return Layout::Unsupported;
			}
		}
		if (Fields == FFields {{"min", FieldType::Real}, {"max", FieldType::Real}})
			return Layout::RangeReal;
		if (Fields == FFields {{"x", FieldType::Real}, {"y", FieldType::Real}})
			return Layout::Vector2;
		if (Fields ==
			FFields {{"x", FieldType::Real}, {"y", FieldType::Real}, {"z", FieldType::Real}})
			return Layout::Vector3;
		if (Fields ==
			FFields {{"r", FieldType::Real}, {"p", FieldType::Real}, {"y", FieldType::Real}})
			return Layout::RPY;
		return Layout::Unsupported;
	}

	template <typename NativeT, typename TypeT>
	TMetadata<TypeT> ResolveConcrete(const NativeT& Native, TypeT Type)
	{
		TMetadata<TypeT> Result;
		Result.Type = Type;
		Result.Layout = FOpenPLX_Utilities::GetValueLayout(Type);
		// Sensor layouts contain nested/windowed data and use their existing dedicated views.
		if (Result.Layout == EOpenPLX_ValueLayout::IMU ||
			Result.Layout == EOpenPLX_ValueLayout::Lidar)
			return Result;
		const auto Definition = Native.marshalling();
		if (Definition == nullptr)
			return {};

		openplx::Marshalling Marshalling;
		if (!Marshalling.add_fields_from_object(Definition))
			return {};
		for (const auto& [Name, Field] : Marshalling.get_field_map())
		{
			if (Field.field_array_type != openplx::FieldArrayType::None)
				return {};
			Result.Fields.emplace(Name, Field.field_type);
		}
		if (Result.Layout == EOpenPLX_ValueLayout::Unsupported)
		{
			if (!Marshalling.get_nested_map().empty())
				return Result;
			Result.Layout = GetLayout(Result.Fields);
			if (Result.Layout != EOpenPLX_ValueLayout::Unsupported)
				Result.Type = TypeT::Unknown;
		}
		return Result;
	}

	EOpenPLX_InputType GetScalarType(EOpenPLX_InputType Type)
	{
		if (FOpenPLX_Utilities::GetValueLayout(Type) == EOpenPLX_ValueLayout::Real)
			return Type;
		if (Type == EOpenPLX_InputType::ForceRangeInput)
			return EOpenPLX_InputType::Force1DInput;
		if (Type == EOpenPLX_InputType::TorqueRangeInput)
			return EOpenPLX_InputType::Torque1DInput;
		// A vector component can require axis-specific coordinate conversion, not just units.
		return EOpenPLX_InputType::Unknown;
	}

	EOpenPLX_OutputType GetScalarType(EOpenPLX_OutputType Type)
	{
		if (FOpenPLX_Utilities::GetValueLayout(Type) == EOpenPLX_ValueLayout::Real)
			return Type;
		if (Type == EOpenPLX_OutputType::ForceRangeOutput)
			return EOpenPLX_OutputType::Force1DOutput;
		if (Type == EOpenPLX_OutputType::TorqueRangeOutput)
			return EOpenPLX_OutputType::Torque1DOutput;
		return EOpenPLX_OutputType::Unknown;
	}

	template <typename TypeT, typename AffineT>
	TMetadata<TypeT> ResolveAffine(
		const TMetadata<TypeT>& Source, const AffineT& Affine, EOpenPLX_VirtualType VirtualType)
	{
		TMetadata<TypeT> Result;
		Result.VirtualType = VirtualType;
		const auto FieldNameAGX = MakeScopedAGXContainer(Affine.field_name());
		const auto Field = Source.Fields.find(FieldNameAGX.Get());
		if (Field == Source.Fields.end() || Field->second != openplx::FieldType::Real)
			return Result;
		Result.Fields.emplace(*Field);
		Result.Layout = EOpenPLX_ValueLayout::Real;
		Result.Type = GetScalarType(Source.Type);
		return Result;
	}

	FInputMetadata ResolveInput(const openplx::Physics::Signals::Input& Input, FPath& Path)
	{
		if (!Path.insert(&Input).second)
			return {};
		FVisit Visit {Path, &Input};
		const auto* Broadcast =
			dynamic_cast<const openplx::Control::Virtual::BroadcastInput*>(&Input);
		if (Broadcast != nullptr)
		{
			FInputMetadata Result;
			Result.VirtualType = EOpenPLX_VirtualType::BroadcastInput;
			bool bFirst = true;
			const auto TargetsAGX = MakeScopedOpenPLXContainer(Broadcast->targets());
			for (const auto& Target : TargetsAGX.Get())
			{
				if (Target == nullptr)
					return {};
				const FInputMetadata Branch = ResolveInput(*Target, Path);
				if (Branch.Layout == EOpenPLX_ValueLayout::Unsupported)
					return {};
				if (bFirst)
				{
					Result.Type = Branch.Type;
					Result.Layout = Branch.Layout;
					Result.Fields = Branch.Fields;
					bFirst = false;
				}
				else
				{
					if (Result.Fields != Branch.Fields || Result.Layout != Branch.Layout)
						return {};
					if (Result.Type != Branch.Type)
						Result.Type = EOpenPLX_InputType::Unknown;
				}
			}
			return Result;
		}
		const auto* Affine =
			dynamic_cast<const openplx::Control::Virtual::AffineTransform1DInput*>(&Input);
		if (Affine != nullptr)
		{
			const auto Target = Affine->target();
			return ResolveAffine(
				Target != nullptr ? ResolveInput(*Target, Path) : FInputMetadata {}, *Affine,
				EOpenPLX_VirtualType::AffineTransform1DInput);
		}
		return ResolveConcrete(Input, FPLXUtilitiesInternal::GetInputType(Input));
	}

	FOutputMetadata ResolveOutput(const openplx::Physics::Signals::Output& Output, FPath& Path)
	{
		if (!Path.insert(&Output).second)
			return {};
		FVisit Visit {Path, &Output};
		const auto* Affine =
			dynamic_cast<const openplx::Control::Virtual::AffineTransform1DOutput*>(&Output);
		if (Affine != nullptr)
		{
			const auto Source = Affine->source();
			return ResolveAffine(
				Source != nullptr ? ResolveOutput(*Source, Path) : FOutputMetadata {}, *Affine,
				EOpenPLX_VirtualType::AffineTransform1DOutput);
		}
		return ResolveConcrete(Output, FPLXUtilitiesInternal::GetOutputType(Output));
	}

	void Resolve(const openplx::Physics::Signals::Input& Native, FOpenPLX_Input& Input)
	{
		FPath Path;
		const FInputMetadata Metadata = ResolveInput(Native, Path);
		Input.Type = Metadata.Type;
		Input.VirtualType = Metadata.VirtualType;
		Input.ValueLayout = Metadata.Layout;
	}

	void Resolve(const openplx::Physics::Signals::Output& Native, FOpenPLX_Output& Output)
	{
		FPath Path;
		const FOutputMetadata Metadata = ResolveOutput(Native, Path);
		Output.Type = Metadata.Type;
		Output.VirtualType = Metadata.VirtualType;
		Output.ValueLayout = Metadata.Layout;
	}
}
