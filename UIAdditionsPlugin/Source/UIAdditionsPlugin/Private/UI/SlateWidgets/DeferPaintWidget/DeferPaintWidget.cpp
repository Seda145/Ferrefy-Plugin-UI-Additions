/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "DeferPaintWidget.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "SDeferPaintWidget.h"
#include "DeferPaintWidgetSlot.h"
#include "ObjectEditorUtils.h"

#define LOCTEXT_NAMESPACE "UIAdditionsPlugin"


// Setup

UDeferPaintWidget::UDeferPaintWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	bIsVariable = false;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UDeferPaintWidget::ReleaseSlateResources(bool bReleaseChildren) {
	Super::ReleaseSlateResources(bReleaseChildren);

	MyDeferPaintWidget.Reset();
}

TSharedRef<SWidget> UDeferPaintWidget::RebuildWidget() {
	MyDeferPaintWidget = SNew(SDeferPaintWidget);

	if (GetChildrenCount() > 0) 	{
		Cast<UDeferPaintWidgetSlot>(GetContentSlot())->BuildSlot(MyDeferPaintWidget.ToSharedRef());
	}

	return MyDeferPaintWidget.ToSharedRef();
}

void UDeferPaintWidget::SynchronizeProperties() {
	Super::SynchronizeProperties();
	if (MyDeferPaintWidget.IsValid()) {
		MyDeferPaintWidget->SetHAlign(HorizontalAlignment);
		MyDeferPaintWidget->SetVAlign(VerticalAlignment);
		MyDeferPaintWidget->SetPadding(Padding);
	}
}

UClass* UDeferPaintWidget::GetSlotClass() const {
	return UDeferPaintWidgetSlot::StaticClass();
}

void UDeferPaintWidget::OnSlotAdded(UPanelSlot* InSlot) {
	// Copy the content properties into the new slot so that it matches what has been setup
	// so far by the user.
	UDeferPaintWidgetSlot* DeferPaintWidgetSlot = CastChecked<UDeferPaintWidgetSlot>(InSlot);
	DeferPaintWidgetSlot->Padding = Padding;
	DeferPaintWidgetSlot->HorizontalAlignment = HorizontalAlignment;
	DeferPaintWidgetSlot->VerticalAlignment = VerticalAlignment;

	// Add the child to the live slot if it already exists
	if (MyDeferPaintWidget.IsValid()) {
		// Construct the underlying slot.
		DeferPaintWidgetSlot->BuildSlot(MyDeferPaintWidget.ToSharedRef());
	}
}

void UDeferPaintWidget::OnSlotRemoved(UPanelSlot* InSlot) {
	// Remove the widget from the live slot if it exists.
	if (MyDeferPaintWidget.IsValid()) {
		MyDeferPaintWidget->SetContent(SNullWidget::NullWidget);
	}
}

void UDeferPaintWidget::PostLoad() {
	Super::PostLoad();
	if (GetChildrenCount() > 0) {
		if (UPanelSlot* PanelSlot = GetContentSlot()) {
			UDeferPaintWidgetSlot* DeferPaintWidgetSlot = Cast<UDeferPaintWidgetSlot>(PanelSlot);
			if (DeferPaintWidgetSlot == nullptr) {
				DeferPaintWidgetSlot = NewObject<UDeferPaintWidgetSlot>(this);
				DeferPaintWidgetSlot->Content = GetContent();
				DeferPaintWidgetSlot->Content->Slot = DeferPaintWidgetSlot;
				Slots[0] = DeferPaintWidgetSlot;
			}
		}
	}
}

#if WITH_EDITOR

void UDeferPaintWidget::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static bool IsReentrant = false;

	if (!IsReentrant) {
		IsReentrant = true;

		if (PropertyChangedEvent.Property) {
			static const FName PaddingName("Padding");
			static const FName HorizontalAlignmentName("HorizontalAlignment");
			static const FName VerticalAlignmentName("VerticalAlignment");

			if (UDeferPaintWidgetSlot* DeferPaintWidgetSlot = Cast<UDeferPaintWidgetSlot>(GetContentSlot())) {
				if (PropertyChangedEvent.Property->GetFName() == PaddingName) {
					FObjectEditorUtils::MigratePropertyValue(this, PaddingName, DeferPaintWidgetSlot, PaddingName);
				}
				else if (PropertyChangedEvent.Property->GetFName() == HorizontalAlignmentName) {
					FObjectEditorUtils::MigratePropertyValue(this, HorizontalAlignmentName, DeferPaintWidgetSlot, HorizontalAlignmentName);
				}
				else if (PropertyChangedEvent.Property->GetFName() == VerticalAlignmentName) {
					FObjectEditorUtils::MigratePropertyValue(this, VerticalAlignmentName, DeferPaintWidgetSlot, VerticalAlignmentName);
				}
			}
		}

		IsReentrant = false;
	}
}

// Palette

const FText UDeferPaintWidget::GetPaletteCategory() {
	return LOCTEXT("Advanced", "Advanced");
}

#endif

// Appearance

EHorizontalAlignment UDeferPaintWidget::GetHorizontalAlignment() const {
	return HorizontalAlignment;
}

void UDeferPaintWidget::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment) {
	HorizontalAlignment = InHorizontalAlignment;
	if (MyDeferPaintWidget.IsValid()) {
		MyDeferPaintWidget->SetHAlign(GetHorizontalAlignment());
	}
}

EVerticalAlignment UDeferPaintWidget::GetVerticalAlignment() const {
	return VerticalAlignment;
}

void UDeferPaintWidget::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment) {
	VerticalAlignment = InVerticalAlignment;
	if (MyDeferPaintWidget.IsValid()) {
		MyDeferPaintWidget->SetVAlign(GetVerticalAlignment());
	}
}

FMargin UDeferPaintWidget::GetPadding() const {
	return Padding;
}

void UDeferPaintWidget::SetPadding(FMargin InPadding) {
	Padding = InPadding;
	if (MyDeferPaintWidget.IsValid()) {
		MyDeferPaintWidget->SetPadding(GetPadding());
	}
}


#undef LOCTEXT_NAMESPACE
