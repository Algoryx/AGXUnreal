#include "AGX_RealDetails.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_Real.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "EditorSupportDelegates.h"
#include "Kismet2/ComponentEditorUtils.h"
#include "PropertyHandle.h"
#include "PropertyPathHelpers.h"
#include "ScopedTransaction.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/NumericTypeInterface.h"
#include "Widgets/Input/SSpinBox.h"

// System includes.
#include <limits>

#define LOCTEXT_NAMESPACE "FAGX_RealDetails"

TSharedRef<IPropertyTypeCustomization> FAGX_RealDetails::MakeInstance()
{
	return MakeShareable(new FAGX_RealDetails());
}

namespace AGX_RealDetails_helpers
{
	/**
	 * A INumericTypeInterface is responsible for converting between string representations and
	 * numeric representations in a widget. This one has support for scientific notation.
	 */
	class FAGX_RealInterface : public INumericTypeInterface<double>
	{
	public:
		static FString StaticToString(const double& Value)
		{
			FString Result = FString::Printf(TEXT("%g"), Value);
			return Result;
		}

		static TOptional<double> StaticFromString(const FString& InString)
		{
			TOptional<double> Result = FCString::Atod(*InString);
			if (!Result.IsSet())
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("FAGX_Real tried to convert string '%s' to double, but Atod failed."),
					*InString);
			}
			return Result;
		}

		virtual FString ToString(const double& Value) const override
		{
			return StaticToString(Value);
		}

		virtual TOptional<double> FromString(
			const FString& InString, const double& /*InExistingValue*/) override
		{
			return StaticFromString(InString);
		}

		virtual bool IsCharacterValid(TCHAR InChar) const override
		{
			auto IsValidLocalizedCharacter = [InChar]() -> bool
			{
				const FDecimalNumberFormattingRules& NumberFormattingRules =
					ExpressionParser::GetLocalizedNumberFormattingRules();
				return InChar == NumberFormattingRules.GroupingSeparatorCharacter ||
					   InChar == NumberFormattingRules.DecimalSeparatorCharacter ||
					   Algo::Find(NumberFormattingRules.DigitCharacters, InChar) != 0;
			};

			static const FString ValidChars = TEXT("1234567890eE-+");
			return InChar != 0 &&
				   (ValidChars.GetCharArray().Contains(InChar) || IsValidLocalizedCharacter());
		}

		/// Min Fractional Digits is not used by this Numeric Type Interface.
		virtual int32 GetMinFractionalDigits() const override
		{
			return 0;
		}

		/// Max Fractional Digits is not used by this Numeric Type Interface.
		virtual int32 GetMaxFractionalDigits() const override
		{
			return 0;
		}

		/// Min Fractional Digits is not used by this Numeric Type Interface.
		virtual void SetMinFractionalDigits(const TAttribute<TOptional<int32>>& NewValue) override
		{
		}

		/// Max Fractional Digits is not used by this Numeric Type Interface.
		virtual void SetMaxFractionalDigits(const TAttribute<TOptional<int32>>& NewValue) override
		{
		}
	};
}

namespace AGX_RealDetails_helpers
{
	TArray<FProperty*> MakePropertyChain(const TSharedPtr<IPropertyHandle>& Handle);
}

TArray<FProperty*> AGX_RealDetails_helpers::MakePropertyChain(
	const TSharedPtr<IPropertyHandle>& Handle)
{
	TArray<FProperty*> PropertyChain;

	TArray<UObject*> SelectedObjects;
	Handle->GetOuterObjects(SelectedObjects);
	if (SelectedObjects.Num() == 0)
	{
		// If nothing is selected then nothing will be changed, so we don't need to do anything.
		return PropertyChain;
	}

	const FString ValuePath = Handle->GeneratePathToProperty();
	TArray<FString> PropertyChainNames;
	ValuePath.ParseIntoArray(PropertyChainNames, TEXT("."));
	UStruct* OuterClass = SelectedObjects[0]->GetClass();
	for (const FString& PropertyChainName : PropertyChainNames)
	{
		// Build a list of Properties along the ValuePath from one of the UObjects down to the
		// double inside the FAGX_Real. Each step need to know the type that the next Property
		// is inside in order to find it, so we keep OuterClass between loop iterations.
		//
		// If OuterClass is nullptr then that means that we hit an intermediate Property that wasn't
		// a struct, which is an unsupported case for now.
		AGX_CHECK(OuterClass != nullptr);
		if (OuterClass == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("When constructing Property chain for '%s', could not determine outer class "
					 "for '%s'. Property changed callbacks may not be triggered correctly."),
				*ValuePath, *PropertyChainName);
			/// @todo Unclear what we should do here. We can chose to just break and let the rest of
			/// the function continue executing. This will cause an incomplete path to be passed to
			/// PostEditChangeChainProperty. Better than nothing, I guess, since we've already
			/// modified the value, but any logic that depend on the path being correct will not
			/// trigger in response to the new value. That's bad. Should we generate this path first
			/// and do nothing if we fail?
			break;
		}

		FName Name(*PropertyChainName);
		FFieldVariant NextField = FindFProperty<FProperty>(OuterClass, Name);
		FProperty* NextProperty = NextField.Get<FProperty>();
		if (NextProperty == nullptr)
		{
			UE_LOG(
				LogAGX, Warning, TEXT("Got no Property for '%s' in '%s'."), *PropertyChainName,
				*ValuePath);
			/// @todo Unclear what we should do here. We can chose to just break and let the rest of
			/// the function continue executing. This will cause an incomplete path to be passed to
			/// PostEditChangeChainProperty. Better than nothing, I guess, since we've already
			/// modified the value, but any logic that depend on the path being correct will not
			/// trigger in response to the new value. That's bad. Should we generate this path first
			/// and do nothing if we fail?
			break;
		}

		PropertyChain.Add(NextProperty);

		if (FStructProperty* StructProp = CastField<FStructProperty>(NextProperty))
		{
			OuterClass = StructProp->Struct;
		}
		else
		{
			// This better be the very last link in the chain. Otherwise, the next iteration of this
			// loop will terminate prematurely with an error message.
			OuterClass = nullptr;
		}
	}

	return PropertyChain;
}

namespace AGX_RealDetails_helpers
{
	double MetaDataOrDefault(
		const TSharedPtr<IPropertyHandle>& Handle, const FName& Key, double Default);
}

double AGX_RealDetails_helpers::MetaDataOrDefault(
	const TSharedPtr<IPropertyHandle>& Handle, const FName& Key, double Default)
{
	TArray<FProperty*> PropertyChain = MakePropertyChain(Handle);
	for (int32 I = PropertyChain.Num() - 1; I >= 0; --I)
	{
		FProperty* Property = PropertyChain[I];
		if (Property->HasMetaData(Key))
		{
			const FString& MetaData = Property->GetMetaData(Key);
			double Value = FCString::Atod(*MetaData); // Is there any error checking we can do here?
			return Value;
		}
	}

	return Default;
}

void FAGX_RealDetails::CustomizeHeader(
	TSharedRef<IPropertyHandle> InRealHandle, FDetailWidgetRow& InHeaderRow,
	IPropertyTypeCustomizationUtils& InStructCustomizationUtils)
{
	using namespace AGX_RealDetails_helpers;

	StructHandle = InRealHandle;
	ValueHandle = StructHandle->GetChildHandle(TEXT("Value"));

	const float SliderMin = MetaDataOrDefault(StructHandle, TEXT("SliderMin"), 0.0f);
	const float SliderMax = MetaDataOrDefault(StructHandle, TEXT("SliderMax"), 100.0);
	const float SliderExponent = MetaDataOrDefault(StructHandle, TEXT("SliderExponent"), 1.0f);

	// clang-format off
	InHeaderRow
	.NameContent()
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(StructHandle->GetPropertyDisplayName())
		.ToolTipText(StructHandle->GetToolTipText())
	]
	.ValueContent()
	[
		SNew(SOverlay)
		+ SOverlay::Slot()
		[
			// The spin box is the main way the user interacts with an FAGX_Real in the Details Panel.
			SNew(SSpinBox<double>)
			.TypeInterface(MakeShareable(new AGX_RealDetails_helpers::FAGX_RealInterface))
			.MinValue(std::numeric_limits<double>::lowest())
			.MinSliderValue(SliderMin)
			.MaxValue(std::numeric_limits<double>::max())
			.MaxSliderValue(SliderMax)
			.SliderExponent(SliderExponent)
			.OnValueChanged(this, &FAGX_RealDetails::OnSpinChanged)
			.OnValueCommitted(this, &FAGX_RealDetails::OnSpinCommitted)
			.Value(this, &FAGX_RealDetails::GetDoubleValue)
			.Visibility(this, &FAGX_RealDetails::VisibleWhenSingleSelection)
		]
		+ SOverlay::Slot()
		[
			// Fallback text box used when multiple FAGX_Reals are selected.
			SNew(SEditableText)
			.OnTextChanged(this, &FAGX_RealDetails::OnTextChanged)
			.OnTextCommitted(this, &FAGX_RealDetails::OnTextCommitted)
			.Text(this, &FAGX_RealDetails::GetTextValue)
			.OnTextCommitted(this, &FAGX_RealDetails::OnTextCommitted)
			.OnTextChanged(this, &FAGX_RealDetails::OnTextChanged)
			.Visibility(this, &FAGX_RealDetails::VisibleWhenMultiSelection)
		]
		+ SOverlay::Slot()
		[
			// Fallback text box used when no FAGX_Real is selected.
			SNew(SEditableText)
			.IsReadOnly(true)
			.Text(LOCTEXT("NoSelection", "NoSelection"))
			.Visibility(this, &FAGX_RealDetails::VisibleWhenNoSelectionOrInvalidHandle)
		]
	];
	// clang-format on
}

void FAGX_RealDetails::CustomizeChildren(
	TSharedRef<IPropertyHandle> RealHandle, IDetailChildrenBuilder& StructBuilder,
	IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	// By having no children we only occupy a single row in the Details Panel, just like regular
	// double Properties.
}

namespace AGX_RealDetails_helpers
{
	template <typename TCmp>
	EVisibility VisibleWhenNumSelected(const TSharedPtr<IPropertyHandle>& Handle, TCmp Cmp)
	{
		if (!Handle.IsValid() || !Handle->IsValidHandle())
		{
			return EVisibility::Collapsed;
		}
		const int32 Num = Handle->GetNumPerObjectValues();
		return FAGX_EditorUtilities::VisibleIf(Cmp(Num));
	}
}

EVisibility FAGX_RealDetails::VisibleWhenSingleSelection() const
{
	return AGX_RealDetails_helpers::VisibleWhenNumSelected(
		StructHandle, [](int32 Num) { return Num == 1; });
}

EVisibility FAGX_RealDetails::VisibleWhenMultiSelection() const
{
	return AGX_RealDetails_helpers::VisibleWhenNumSelected(
		StructHandle, [](int32 Num) { return Num > 1; });
}

EVisibility FAGX_RealDetails::VisibleWhenNoSelectionOrInvalidHandle() const
{
	return AGX_RealDetails_helpers::VisibleWhenNumSelected(
		StructHandle, [](int32 Num) { return Num == 0; });
}

double FAGX_RealDetails::GetDoubleValue() const
{
	if (!ValueHandle.IsValid() || !ValueHandle->IsValidHandle())
	{
		// The Spin Box will not be displayed while the handle is invalid, so it doesn't matter what
		// we return here.
		return -1.0;
	}
	double Value;
	FPropertyAccess::Result Status = ValueHandle->GetValue(Value);
	switch (Status)
	{
		case FPropertyAccess::Success:
			return Value;
		case FPropertyAccess::MultipleValues:
			// The Spin Box will not be displayed while there are multiple objects selected, so it
			// doesn't matter what we return here.
			return -2.0;
		case FPropertyAccess::Fail:
			UE_LOG(
				LogAGX, Error,
				TEXT("Failed to read value for '%s', Details Panel may show incorrect data."),
				*ValueHandle->GeneratePathToProperty());
			return -3.0;
		default:
			// Should never get here.
			UE_LOG(
				LogAGX, Error,
				TEXT("Unknown property access status from IPropertyHandle::GetValue for '%s'. "
					 "Details Panel may show incorrect data."),
				*ValueHandle->GeneratePathToProperty());
			return -4.0;
	}
}

FText FAGX_RealDetails::GetTextValue() const
{
	if (!ValueHandle.IsValid() || !ValueHandle->IsValidHandle())
	{
		// The Editable Text will not be displayed while the handle is invalid,
		// so it doesn't matter what we return here.
		return LOCTEXT("NothingSelected", "Nothing Selected");
	}

	double Value = 0.0;
	FPropertyAccess::Result Status = ValueHandle->GetValue(Value);
	switch (Status)
	{
		case FPropertyAccess::Success:
			// Don't expect we will ever get here since the spin box, and not the text field, is
			// shown when only a single object is selected. Perhaps we get here if all selected
			// objects have the same value.
			return FText::FromString(
				AGX_RealDetails_helpers::FAGX_RealInterface::StaticToString(Value));
		case FPropertyAccess::MultipleValues:
			// The Editable Text will not be displayed while there are multiple
			// objects selected, so it doesn't matter what we return here.
			return LOCTEXT("MultipleValues", "Multiple Values");
		case FPropertyAccess::Fail:
			UE_LOG(
				LogAGX, Error,
				TEXT("Failed to read value for '%s', Details Panel may show incorrect data."),
				*ValueHandle->GeneratePathToProperty());
			return LOCTEXT("CouldNotReadValue", "<could not read value>");
		default:
			// Should never get here.
			UE_LOG(
				LogAGX, Error,
				TEXT("Unknown property access status from IPropertyHandle::GetValue for '%s'. "
					 "Details Panel may show incorrect data."),
				*ValueHandle->GeneratePathToProperty());
			return LOCTEXT("UnknownPropertyAccessStatus", "<unknown property access status>");
	}
}

namespace AGX_RealDetails_helpers
{
	void NewValueSet(
		double NewValue, TSharedPtr<IPropertyHandle> StructHandle,
		TSharedPtr<IPropertyHandle> ValueHandle);
}

void AGX_RealDetails_helpers::NewValueSet(
	double NewValue, TSharedPtr<IPropertyHandle> StructHandle,
	TSharedPtr<IPropertyHandle> ValueHandle)
{
	if (!StructHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Cannot commit new Spin value to AGX Real, the handle is invalid."));
		return;
	}

	const FString ValuePath = ValueHandle->GeneratePathToProperty();
#if 0
	/// @todo Learn when we are allowed to cache the value path and when not.
	/// Passing it both to GetPropertyValue and SetPropertyValue for both the selected objects and
	/// template instances causes a crash due to a failed assert.
	///     LogCore: Assertion failed:
	///     InContainer == InPropertyPath.GetCachedContainer()
	///     File:Runtime/PropertyPath/Public/PropertyPathHelpers.h
	///     Line: 354
	/// I assume that means the cached path was used in a context where it shouldn't.
	/// Worst-case scenario is that a cached path is only valid for a single UObject, which makes
	/// it mostly usesless here.
	FCachedPropertyPath CachedValuePath(ValuePath);
#endif

	FProperty* ValueProperty = ValueHandle->GetProperty();

	// Get the selected objects. These are the objects that we should manipulate directly.
	TArray<UObject*> SelectedObjects;
	ValueHandle->GetOuterObjects(SelectedObjects);
	if (SelectedObjects.Num() == 0)
	{
		// If nothing is selected then nothing will be changed, so we don't need to do anything.
		return;
	}

	// This is a user-level interaction, so start a new undo/redo transaction. Any object that we
	// call Modify or PreEditChange on until this object goes out of scope will be included in a
	// single undo/redo step.
	FScopedTransaction Transaction(FText::Format(
		LOCTEXT("SpinTransaction", "Edit {0}"), StructHandle->GetPropertyDisplayName()));

	// Let the selected objects know that we are about to modify them, which will include them in
	// the current undo/redo transaction.
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PreEditChange(ValueProperty);
	}

	// Store the current values in the selected objects so that we can compare them against values
	// in template instances later.
	TArray<double> OldValues;
	OldValues.Reserve(SelectedObjects.Num());
	for (UObject* SelectedObject : SelectedObjects)
	{
		double OldValue;
		const bool bOldValueValid =
			PropertyPathHelpers::GetPropertyValue(SelectedObject, ValuePath, OldValue);
		if (!bOldValueValid)
		{
			/// @todo Is there something better we can do here?
			UE_LOG(
				LogAGX, Error, TEXT("Could not current '%s' value from '%s'. Assuming 0.0."),
				*ValuePath, *SelectedObject->GetName());
			OldValue = 0.0;
		}
		OldValues.Add(OldValue);
	}

	// Write the new value to each selected object.
	for (UObject* SelectedObject : SelectedObjects)
	{
		const bool bNewValueSet =
			PropertyPathHelpers::SetPropertyValue(SelectedObject, ValuePath, NewValue);
		if (!bNewValueSet)
		{
			/// @todo Is there something better we can do here?
			UE_LOG(
				LogAGX, Error, TEXT("Could not set '%s' on '%s'. Value remain unchanged."),
				*ValuePath, *SelectedObject->GetName());
		}
	}

	// Let selected objects know that we are done modifying them.
	FPropertyChangedEvent ChangedEvent(
		ValueProperty, EPropertyChangeType::ValueSet, MakeArrayView(SelectedObjects));
	TArray<FProperty*> PropertyChainArray = MakePropertyChain(ValueHandle);
	FEditPropertyChain PropertyChain;
	for (FProperty* PropertyChainLink : PropertyChainArray)
	{
		PropertyChain.AddTail(PropertyChainLink);
	}
	FPropertyChangedChainEvent ChainEvent(PropertyChain, ChangedEvent);
	for (UObject* SelectedObject : SelectedObjects)
	{
		SelectedObject->PostEditChangeChainProperty(ChainEvent);
	}

	// For any selected template, propagate the change to all template instances that has the same
	// value as the selected object had.
	for (int32 I = 0; I < SelectedObjects.Num(); ++I)
	{
		UObject* SelectedObject = SelectedObjects[I];
		if (!SelectedObject->IsTemplate())
		{
			continue;
		}

		const double OldValue = OldValues[I];

		TArray<UObject*> Instances;
		SelectedObject->GetArchetypeInstances(Instances);
		for (UObject* Instance : Instances)
		{
			double CurrentValue;
			PropertyPathHelpers::GetPropertyValue(Instance, ValuePath, CurrentValue);
			if (CurrentValue == OldValue)
			{
				Instance->PreEditChange(ValueProperty);
				PropertyPathHelpers::SetPropertyValue(Instance, ValuePath, NewValue);
				Instance->PostEditChangeChainProperty(ChainEvent);
			}
		}
	}

	// Let the various parts of the editor know about the change. I have no idea what we're
	// supposed to call here. Is anything missing? Should something be removed?
	FEditorSupportDelegates::RedrawAllViewports.Broadcast();
	FEditorSupportDelegates::RefreshPropertyWindows.Broadcast();
}

void FAGX_RealDetails::OnSpinChanged(double NewValue)
{
	// Consider doing input validation here, and make the background red if invalid.
}

void FAGX_RealDetails::OnSpinCommitted(double NewValue, ETextCommit::Type CommitInfo)
{
	AGX_RealDetails_helpers::NewValueSet(NewValue, StructHandle, ValueHandle);
}

void FAGX_RealDetails::OnTextChanged(const FText& NewText)
{
	// Consider doing input validation here, and make the background red if invalid.
}

void FAGX_RealDetails::OnTextCommitted(const FText& NewText, ETextCommit::Type CommitInfo)
{
	TOptional<double> NewValue =
		AGX_RealDetails_helpers::FAGX_RealInterface::StaticFromString(NewText.ToString());
	if (!NewValue.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Cannot commit new Text value to AGX Real, '%s' is not a valid double."),
			*NewText.ToString());
		return;
	}
	AGX_RealDetails_helpers::NewValueSet(NewValue.GetValue(), StructHandle, ValueHandle);
}

#undef LOCTEXT_NAMESPACE