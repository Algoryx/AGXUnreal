// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Layout/Visibility.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"

class FAGX_ConstraintBodyAttachmentCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(
		TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	virtual void CustomizeChildren(
		TSharedRef<class IPropertyHandle> StructPropertyHandle,
		class IDetailChildrenBuilder& StructBuilder,
		IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	FText GetRigidBodyLabel() const;

	bool HasRigidBody() const;

	bool HasFrameDefiningActor() const;

	void CreateAndSetFrameDefiningActor();

private:
	TSharedPtr<class IPropertyHandle> BodyAttachmentProperty = nullptr;
	TSharedPtr<class IPropertyHandle> RigidBodyProperty = nullptr;
	TSharedPtr<class IPropertyHandle> FrameDefiningComponentProperty = nullptr;
};
