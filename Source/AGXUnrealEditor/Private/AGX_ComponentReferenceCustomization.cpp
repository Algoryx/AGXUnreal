// Copyright 2025, Algoryx Simulation AB.

#include "AGX_ComponentReferenceCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ComponentReference.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_PropertyUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Utilities/AGX_Utilities.h"

// Unreal Engine includes.
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"

#define LOCTEXT_NAMESPACE "AGX_ComponentReferenceCustomization"

TSharedRef<IPropertyTypeCustomization> FAGX_ComponentReferenceCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ComponentReferenceCustomization());
}

namespace AGX_ComponentReferenceCustomization_helpers
{
	/**
	 * Get a combo-box compatible list of Component names of the compatible Components found in the
	 * Owner in the given Component Reference. That is, find out what options the user currently
	 * have to chose between.
	 * @param OutNames Combo-box compatible list of Component names.
	 * @param ComponentReference Get Components from this reference's Owner.
	 */
	void GetComponentNamesFromOwner(
		TArray<TSharedPtr<FName>>& OutNames, const FAGX_ComponentReference& ComponentReference)
	{
		TArray<UActorComponent*> CompatibleComponents =
			ComponentReference.GetCompatibleComponents();
		OutNames.Empty();
		for (const UActorComponent* Component : CompatibleComponents)
		{
			OutNames.Add(MakeShareable(new FName(Component->GetFName())));
		}
	}

	const UBlueprintGeneratedClass* GetBlueprint(IPropertyHandle& ComponentReferenceHandle)
	{
		// This assumes that the Component Reference is owned by an Actor Component.
		const UActorComponent* OwningComponent = Cast<UActorComponent>(
			FAGX_PropertyUtilities::GetParentObjectOfStruct(ComponentReferenceHandle));
		if (OwningComponent == nullptr)
		{
			return nullptr;
		}
		const UBlueprintGeneratedClass* Blueprint =
			Cast<UBlueprintGeneratedClass>(OwningComponent->GetOuter());
		return Blueprint;
	}

	/**
	 * Get a combo-box compatible list of Component names of the compatible Components found in the
	 * Blueprint that the given Property Handle is part of. That is, find out what options the user
	 * currently have to chose between.
	 * @param OutNames Combo-box compatible list of Component names.
	 * @param ComponentReferenceHandle Get Components from this reference's Owner.
	 * @param Type The UActorComponent subclass that the Component Reference can point to.
	 */
	void GetComponentNamesFromBlueprint(
		TArray<TSharedPtr<FName>>& OutNames, const UBlueprintGeneratedClass& Blueprint,
		TSubclassOf<UActorComponent> Type)
	{
		OutNames.Empty();

		// Search the Blueprint inheritance chain for SCS nodes of the wanted type.
		TArray<UBlueprint*> BlueprintChain;
		UBlueprint::GetBlueprintHierarchyFromClass(&Blueprint, BlueprintChain);
		for (const UBlueprint* BP : BlueprintChain)
		{
			for (const USCS_Node* Node : BP->SimpleConstructionScript->GetAllNodes())
			{
				if (Node->ComponentTemplate->IsA(Type))
				{
					const FString Name = [Node, BP]()
					{
						FString Name = Node->ComponentTemplate->GetName();
						Name.RemoveFromEnd(
							BP->SimpleConstructionScript->ComponentTemplateNameSuffix);
						return Name;
					}();

					OutNames.Add(MakeShareable(new FName(*Name)));
				}
			}
		}
	}
}

/**
 * Library of helper functions used by FAGX_ComponentReferenceCustomization that need access to
 * private members.
 */
struct FAGX_ComponentReferenceCustomizationOperations
{
	/**
	 * Create a combo-box displaying the contents of the ComponentNames array, which holds the
	 * Components of appropriate type that was found in the Owner or Blueprint.
	 * @param This The FAGX_ComponentReferenceCustomization instance calling the function.
	 * @return A combo-box widget displaying the contents of ComponentNames.
	 */
	static TSharedRef<SComboBox<TSharedPtr<FName>>> CreateNameComboBox(
		FAGX_ComponentReferenceCustomization& This,
		TSharedPtr<SComboBox<TSharedPtr<FName>>>& OutWidget)
	{
		// clang-format off
		return SAssignNew(OutWidget, SComboBox<TSharedPtr<FName>>)
			.Visibility_Lambda([&This](){ return FAGX_EditorUtilities::VisibleIf(This.bFoundComponents); })
			.OptionsSource(&This.ComponentNames)
			.OnGenerateWidget_Lambda(
				[](const TSharedPtr<FName>& Item)
				{
					return SNew(STextBlock)
						.Text(FText::FromName(*Item))
						.Font(IDetailLayoutBuilder::GetDetailFont());
				})
			.OnSelectionChanged(&This, &FAGX_ComponentReferenceCustomization::OnComboBoxChanged)
			.Content()
			[
				SNew(STextBlock)
				.Text_Lambda(
					[&This]()
					{
						return FText::FromName(This.SelectedComponent);
					})
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
		// clang-format on
	}
};

void FAGX_ComponentReferenceCustomization::CustomizeHeader(
	TSharedRef<IPropertyHandle> InComponentReferenceHandle, FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Slate callback functions may be called during setup and some of our callbacks manipulate
	// the Slate state, which we don't want to do during construction. Such code is therefore
	// guarded behind checks of this flag.
	bInCustomize = true;
	FAGX_Finalizer LeaveCustomize([this]() { this->bInCustomize = false; });

	if (!RefetchPropertyHandles(InComponentReferenceHandle))
	{
		// There is something wrong with the Property Handles that we need. Generate a warning text
		// in the header.

		// clang-format off
		HeaderRow
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoPropertyHandle", "Invalid property handle."))
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.MinDesiredWidth(250.0f)
		];
		// clang-format on

		return;
	}

	// Make sure we have a single valid Component Reference that is being customized. Support for
	// multi-select editing is a feature for the future.
	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		// clang-format off
		HeaderRow
		.WholeRowContent()
		[
			SNew(STextBlock)
			.Text(LOCTEXT(
				"MultipleShovelsSelected",
				"Multi-select editing of references not yet supported"))
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
			.MinDesiredWidth(250.0f)
		];
		// clang-format on
		return;
	}

	// Create and configure the Component name combo-box shown in the header.
	// clang-format off
	HeaderRow
	.NameContent()
	[
		ComponentReferenceHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(250.0f)
	[
		// The value content consists of a convenience combo-box and a label identifying the
		// currently selected Component.
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			FAGX_ComponentReferenceCustomizationOperations::CreateNameComboBox(*this, HeaderComboBoxPtr)
		]
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
				.Text(this, &FAGX_ComponentReferenceCustomization::GetHeaderText)
				.ToolTipText(this, &FAGX_ComponentReferenceCustomization::GetHeaderText)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.MinDesiredWidth(250.0f)
		]
	];
	// clang-format on
	SelectedComponent = GetName();
	RebuildComboBox();

	// The combo-box must be rebuilt when the search settings are changed.
	RebuildComboBoxDelegate.BindRaw(this, &FAGX_ComponentReferenceCustomization::RebuildComboBox);
	OwningActorHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);
	SearchChildActorsHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);
}

void FAGX_ComponentReferenceCustomization::CustomizeChildren(
	TSharedRef<IPropertyHandle> InComponentReferenceHandle, IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	using namespace AGX_ComponentReferenceCustomization_helpers;

	// Slate callback functions may be called during setup and some of our callbacks manipulate
	// the Slate state, which we don't want to do during construction. Such code is therefore
	// guarded behind checks of this flag.
	bInCustomize = true;
	FAGX_Finalizer LeaveCustomize([this]() { this->bInCustomize = false; });

	if (!RefetchPropertyHandles(InComponentReferenceHandle))
	{
		// There is something wrong with the Property Handles we need. Don't generate any widgets
		// since won't be able to do anything with them.
		//
		// Is there a better way to handle this case?
		return;
	}

	// Make sure we have a single valid Component Reference that is being customized. Support for
	// multi-select editing is a feature for the future.
	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		return;
	}

	// Create default widgets for the Properties that don't need any special attention.
	ChildBuilder.AddProperty(OwningActorHandle.ToSharedRef());
	ChildBuilder.AddProperty(SearchChildActorsHandle.ToSharedRef());

	// clang-format off
	FDetailWidgetRow& NameRow = ChildBuilder.AddCustomRow(FText::FromString("Component"));
	NameRow
	.NameContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Component"))
		.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
	];
	// clang-format on

	// clang-format off
	NameRow
	.ValueContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		[
			// Create the Component name combo-box shown among the children.
			FAGX_ComponentReferenceCustomizationOperations::CreateNameComboBox(*this, ComboBoxPtr)
		]
		+ SVerticalBox::Slot()
		[
			// Fallback text box that is shown if no compatible Components are found.
			SAssignNew(ComponentNameBoxPtr, SEditableTextBox)
				.Text_Lambda([this]() { return FText::FromName(SelectedComponent); })
				.OnTextCommitted(this, &FAGX_ComponentReferenceCustomization::OnComponentNameCommitted)
				.Visibility_Lambda([this]() { return FAGX_EditorUtilities::VisibleIf(!bFoundComponents); })
		]
	];
	// clang-format on

	SelectedComponent = GetName();
	RebuildComboBox();

	// The combo-box must be rebuilt when the search settings are changed.
	RebuildComboBoxDelegate.BindRaw(this, &FAGX_ComponentReferenceCustomization::RebuildComboBox);
	OwningActorHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);
	SearchChildActorsHandle->SetOnPropertyValueChanged(RebuildComboBoxDelegate);
}

FText FAGX_ComponentReferenceCustomization::GetHeaderText() const
{
	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		return LOCTEXT(
			"NoComponentReferece",
			"Component Reference Customization does not have a valid Component Reference");
	}

	// The header text is the name of the component and the name of the owning Actor.
	const AActor* OwningActor = GetOwningActor();
	const FName ComponentName = GetName();
	const FString ActorName = OwningActor ? GetLabelSafe(OwningActor) : TEXT("(Self)");
	return FText::Format(
		LOCTEXT("HeaderText", "{0} in {1}"), FText::FromName(ComponentName),
		FText::FromString(ActorName));
}

/**
 * RebuildComboBox is called whenever the list of available compatible Components is changed. This
 * happens when a new Owning Actor is selected, and when the Search Child Actors checkbox is
 * toggled.
 *
 * It searches the owning Actor or Blueprint for all compatible Components and puts their names into
 * the combo-box source array.
 */
void FAGX_ComponentReferenceCustomization::RebuildComboBox()
{
	using namespace AGX_ComponentReferenceCustomization_helpers;

	ComponentNames.Empty();

	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		return;
	}

	// Populate Component Names either from the Blueprint or from the owning Actor.
	const UBlueprintGeneratedClass* Blueprint = GetBlueprint(*ComponentReferenceHandle.Get());
	if (Blueprint != nullptr)
	{
		GetComponentNamesFromBlueprint(
			ComponentNames, *Blueprint, ComponentReference->ComponentType);
	}
	else
	{
		GetComponentNamesFromOwner(ComponentNames, *ComponentReference);
	}

	bFoundComponents = ComponentNames.Num() > 0;

	// Put the combo-box items in alphabetical order, to make it easier for the user to find what
	// they are looking for.
	ComponentNames.Sort([](const TSharedPtr<FName>& Lhs, const TSharedPtr<FName>& Rhs)
						{ return FNameLexicalLess()(*Lhs, *Rhs); });

	// "None" is the special name for nothing selected, like a nullptr or empty optional.
	// Compares equal to NAME_None.
	//
	// TODO Is "None" always correct, or is it localization language dependent?
	ComponentNames.Add(MakeShareable(new FName(TEXT("None"))));

	// We now know what options the user has to chose between. Let the combo-box know.
	// Don't always have all combo-boxes. Update the ones we have.
	for (auto& ComboBox : {HeaderComboBoxPtr, ComboBoxPtr})
	{
		if (ComboBox != nullptr)
		{
			ComboBox->RefreshOptions();
		}
	}

// This was an attempt to be helpful, but it makes it impossible to select None, it immediately
// switches back to the first valid Component. Is there a way to detect and handle the I-want-None
// case?
#if 0
	// If we didn't have a selection before, then pick the first one. This may be the None FName
	// added at the end above, if there are no valid Components in the owning Actor or Blueprint.
	if (SelectedComponent == NAME_None)
	{
		SelectedComponent = *ComponentNames[0];
	}
#endif

	// Find which combo-box item matches the current selection and select it.
	bool SelectionFound = false;
	for (TSharedPtr<FName>& ComponentName : ComponentNames)
	{
		if (*ComponentName == SelectedComponent)
		{
			for (auto& ComboBox : {HeaderComboBoxPtr, ComboBoxPtr})
			{
				if (ComboBox != nullptr)
				{
					ComboBox->SetSelectedItem(ComponentName);
				}
			}
			SelectionFound = true;
			break;
		}
	}
	if (!SelectionFound)
	{
		// Not sure what to do here. The Component Reference references something that does not
		// exist in the current Owning Actor. Not an error, but something the user should fix before
		// starting a simulation if they want to reference something.
		//
		// Or should we fix it for them, by selecting one of the valid options? Will lead to angry
		// users when they accidentally switch from an Actor with many valid Components and then
		// switch back again. If we "fixed" the state on the first switch then we have no way of
		// restoring the original/wanted state when the user switches back. They are forced to
		// scroll through the long list again. And hope they know from the top of their head what
		// the correct selection was. Or at least have a way to find out what they should select.
		// And that they notice that the selection changed.
		//
		// This is so bad. Don't do this.
		//
		// How should this state instead be presented to the user?
		// Should the non-existing name be added to the list?
		// If so, how do we make it clear that it is an invalid selection?
		// If not, how do we show that the actual selection isn't among the options?
		// Red text/background?
		//
		// For now select the first name, despite the tirade above. Do something better when we have
		// got the basics working and can start tweaking things like this.
		//
		// There is always at least one valid option in ComponentNames. If there are no valid
		// Components in the owning Actor then the None FName is selected.
		SelectedComponent = *ComponentNames[0];
		for (auto& ComboBox : {HeaderComboBoxPtr, ComboBoxPtr})
		{
			if (ComboBox != nullptr)
			{
				ComboBox->SetSelectedItem(ComponentNames[0]);
			}
		}
	}
}

void FAGX_ComponentReferenceCustomization::OnComboBoxChanged(
	TSharedPtr<FName> NewSelection, ESelectInfo::Type SelectionInfo)
{
	FName CurrentValue;
	FPropertyAccess::Result Result = NameHandle->GetValue(CurrentValue);

	SelectedComponent = NewSelection.IsValid() ? *NewSelection : NAME_None;

	// Only set the new value if it is an actual new value. This is to prevent infinite recursion
	// when setting the value through the Property Handle triggers a Blueprint Reconstruction,
	// which causes the Details panel to be rebuilt, which creates a new Component Reference
	// Customization, which triggers a new call here, which we don't want triggering a new
	// set on the Property Handle because that will trigger an assert in Unreal Editor code.
	if (Result == FPropertyAccess::Success && SelectedComponent != CurrentValue)
	{
		if (bInCustomize)
		{
			// We are currently in the process of creating the Details panel, possibly as a
			// consequence of a Blueprint Reconstruction. We do not want to do this recursively,
			// only one Blueprint Reconstruction is allowed at a time, so do not call
			// NameHandle->SetValue. This is not an actual edit, so the value we would set is that
			// same as the current value anyway.
		}
		else
		{
			// This may trigger a Blueprint Reconstruction. If it does then the object being
			// customized will be deleted and the entire Details panel replaced. Once we return
			// from SetValue we must detect if it is safe to continue working with the objects we
			// have and bail out if not.
			NameHandle->SetValue(SelectedComponent);
		}
	}

	if (ComponentReferenceHandle.IsValid() && ComponentReferenceHandle->IsValidHandle() &&
		ComponentReferenceHandle->GetNumPerObjectValues() == 0)
	{
		// It looks like Blueprint Reconstruction happened, which means that both the object we
		// were editing and the entire Details panel has been destroyed. We can do nothing more.
		return;
	}

	// Even if we couldn't call SetValue because of the object creation loop we still want to
	// update the Component Reference with the new name, if we have a Component Reference.
	FAGX_ComponentReference* Reference = GetComponentReference();
	if (Reference == nullptr)
	{
		return;
	}
	Reference->Name = SelectedComponent;
}

void FAGX_ComponentReferenceCustomization::OnComponentNameCommitted(
	const FText& InText, ETextCommit::Type InCommitType)
{
	// TODO This function need similar safety checks as OnComboBoxChanged. Make a helper function?

	SelectedComponent = FName(*InText.ToString());
	if (NameHandle.IsValid() && NameHandle->IsValidHandle())
	{
		NameHandle->SetValue(SelectedComponent);
	}

	FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference != nullptr)
	{
		// If/when we do caching in the Component Reference, then the cache needs to be invalidated
		// here.
	}
}

bool FAGX_ComponentReferenceCustomization::RefetchPropertyHandles(
	const TSharedRef<IPropertyHandle>& InComponentReferenceHandle)
{
	ComponentReferenceHandle = InComponentReferenceHandle;
	if (!ComponentReferenceHandle.IsValid() || !ComponentReferenceHandle->IsValidHandle())
	{
		ClearPropertyHandles();
		return false;
	}

	OwningActorHandle = ComponentReferenceHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ComponentReference, OwningActor));
	NameHandle = ComponentReferenceHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ComponentReference, Name));
	SearchChildActorsHandle = ComponentReferenceHandle->GetChildHandle(
		GET_MEMBER_NAME_CHECKED(FAGX_ComponentReference, bSearchChildActors));
	for (auto& Handle : {OwningActorHandle, NameHandle, SearchChildActorsHandle})
	{
		if (!Handle.IsValid() || !Handle->IsValidHandle())
		{
			ClearPropertyHandles();
			return false;
		}
	}

	return true;
}

void FAGX_ComponentReferenceCustomization::ClearPropertyHandles()
{
	ComponentReferenceHandle = nullptr;
	OwningActorHandle = nullptr;
	NameHandle = nullptr;
	SearchChildActorsHandle = nullptr;
}

FAGX_ComponentReference* FAGX_ComponentReferenceCustomization::GetComponentReference() const
{
	if (!ComponentReferenceHandle.IsValid())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: The Component Reference Handle is not valid."));
		return nullptr;
	}
	if (!ComponentReferenceHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: The Component Reference Handle is not a valid "
				 "handle."));
		return nullptr;
	}

	// This will fail with FPropertyAccess::MultipleValues if multiple Component References is
	// selected. Support for multi-select editing is a feature for the future.
	void* UntypedPointer = nullptr;
	const FPropertyAccess::Result Result = ComponentReferenceHandle->GetValueData(UntypedPointer);
	if (Result != FPropertyAccess::Success)
	{
#if 0
		// This gives linker error despite having all the DependencyModuleNames I think are
		// necessary. I guess it is because FPropertyAccess::Result isn't marked with UENUM.
		const UEnum* ResultEnum = StaticEnum<FPropertyAccess::Result>();
		const FName ResultName = ResultEnum->GetNameByValue(Result).ToString();
#else
		// Doing it the manual way instead.
		FString ResultName = [Result]()
		{
			switch (Result)
			{
				case FPropertyAccess::Fail:
					return TEXT("Fail");
				case FPropertyAccess::Success:
					return TEXT("Success");
				case FPropertyAccess::MultipleValues:
					return TEXT("MultipleValues");
				default:
					return TEXT("(unknown");
			}
		}();
#endif

		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Failed to read value data from Property "
				 "Handle. Got result %s."),
			*ResultName);
		return nullptr;
	}
	if (UntypedPointer == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Got null Component Reference from the handle."));
		// Note that we consider a nullptr a failure even though we successfully read it. Is this
		// a problem? When should we need to know that the actual Property is a nullptr. Can it be?
		// The Component Reference is a struct, not a pointer.
		return nullptr;
	}

	return static_cast<FAGX_ComponentReference*>(UntypedPointer);
}

AActor* FAGX_ComponentReferenceCustomization::GetOwningActor() const
{
	if (!OwningActorHandle.IsValid())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: The Owning Actor handle is not valid."));
		return nullptr;
	}

	if (!OwningActorHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT(
				"ComponentReferenceCustomization: The Owning Actor Handle is not a valid handle."));
		return nullptr;
	}

	void* UntypedPointer = nullptr;
	const FPropertyAccess::Result Result = OwningActorHandle->GetValueData(UntypedPointer);
	if (Result != FPropertyAccess::Success)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Failed to read value data from Owning Actor "
				 "Handle. Got result %d."),
			Result);
		return nullptr;
	}
	if (UntypedPointer == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Got null Owning Actor location from the "
				 "handle."));
		return nullptr;
	}
	AActor* OwningActor = *static_cast<AActor**>(UntypedPointer);

	// Sanity check against reading through the Component Reference. Should produce the same result.
	FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_ComponentReferenceCustomization: Found an Owning Actor but not a Component "
				 "Reference. Thats surprising."));
	}
	else if (ComponentReference->OwningActor != OwningActor)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceHandle and OwningActorHandle disagree on the Owning Actor.\n"
				 "  From handle: %p.\n  From Component Reference: %p."),
			OwningActor, ComponentReference->OwningActor);
	}

	return OwningActor;
}

AActor* FAGX_ComponentReferenceCustomization::GetScope() const
{
	FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference == nullptr)
	{
		return nullptr;
	}
	return ComponentReference->GetScope();
}

FName FAGX_ComponentReferenceCustomization::GetName() const
{
	if (!NameHandle.IsValid())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: The Name handle is not valid."));
		return NAME_None;
	}

	if (!NameHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: The Name Handle is not a valid handle."));
		return NAME_None;
	}

	void* UntypedPointer = nullptr;
	const FPropertyAccess::Result Result = NameHandle->GetValueData(UntypedPointer);
	if (Result != FPropertyAccess::Success)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Failed to read value data from Name "
				 "Handle. Got result %d."),
			Result);
		return NAME_None;
	}
	if (UntypedPointer == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ComponentReferenceCustomization: Got null Name location from the "
				 "handle."));
		return NAME_None;
	}
	FName Name = *static_cast<FName*>(UntypedPointer);

	// Sanity check against reading through the Component Reference.
	const FAGX_ComponentReference* ComponentReference = GetComponentReference();
	if (ComponentReference != nullptr && Name != ComponentReference->Name)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("NameHandle and ComponentReferenceHandle disagree on what the Name is."));
	}

	return Name;
}

bool FAGX_ComponentReferenceCustomization::GetSearchChildActors() const
{
	if (!SearchChildActorsHandle.IsValid() || !SearchChildActorsHandle->IsValidHandle())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_ComponentReferenceCustomization: Cannot read Search Child Actors property "
				 "from handle."));
		return false;
	}

	bool bValue = false;
	SearchChildActorsHandle->GetValue(bValue);
	return bValue;
}

#undef LOCTEXT_NAMESPACE
