/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "DeferPaintWidgetSlot.h"
#include "SDeferPaintWidget.h"
#include "DeferPaintWidget.h"
#include "Widgets/SNullWidget.h"
#include "ObjectEditorUtils.h"


// Setup

void UDeferPaintWidgetSlot::ReleaseSlateResources(bool bInReleaseChildren) {
	Super::ReleaseSlateResources(bInReleaseChildren);
	DeferPaintWidget.Reset();
}

void UDeferPaintWidgetSlot::BuildSlot(TSharedRef<SDeferPaintWidget> InDeferPaintWidget) {
	DeferPaintWidget = InDeferPaintWidget;

	DeferPaintWidget.Pin()->SetPadding(Padding);
	DeferPaintWidget.Pin()->SetHAlign(HorizontalAlignment);
	DeferPaintWidget.Pin()->SetVAlign(VerticalAlignment);

	DeferPaintWidget.Pin()->SetContent(Content ? Content->TakeWidget() : SNullWidget::NullWidget);
}

void UDeferPaintWidgetSlot::SetPadding(FMargin InPadding) {
	CastChecked<UDeferPaintWidget>(Parent)->SetPadding(InPadding);
}

void UDeferPaintWidgetSlot::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment) {
	CastChecked<UDeferPaintWidget>(Parent)->SetHorizontalAlignment(InHorizontalAlignment);
}

void UDeferPaintWidgetSlot::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment) {
	CastChecked<UDeferPaintWidget>(Parent)->SetVerticalAlignment(InVerticalAlignment);
}

void UDeferPaintWidgetSlot::SynchronizeProperties() {
	if (DeferPaintWidget.IsValid()) {
		SetPadding(Padding);
		SetHorizontalAlignment(HorizontalAlignment);
		SetVerticalAlignment(VerticalAlignment);
	}
}

#if WITH_EDITOR

void UDeferPaintWidgetSlot::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent) {
	Super::PostEditChangeProperty(InPropertyChangedEvent);

	static bool IsReentrant = false;

	if (!IsReentrant) {
		IsReentrant = true;

		if (InPropertyChangedEvent.Property) {
			static const FName PaddingName("Padding");
			static const FName HorizontalAlignmentName("HorizontalAlignment");
			static const FName VerticalAlignmentName("VerticalAlignment");

			if (UDeferPaintWidget* ParentWidget = CastChecked<UDeferPaintWidget>(Parent)) {
				if (InPropertyChangedEvent.Property->GetFName() == PaddingName) {
					FObjectEditorUtils::MigratePropertyValue(this, PaddingName, ParentWidget, PaddingName);
				}
				else if (InPropertyChangedEvent.Property->GetFName() == HorizontalAlignmentName) {
					FObjectEditorUtils::MigratePropertyValue(this, HorizontalAlignmentName, ParentWidget, HorizontalAlignmentName);
				}
				else if (InPropertyChangedEvent.Property->GetFName() == VerticalAlignmentName) {
					FObjectEditorUtils::MigratePropertyValue(this, VerticalAlignmentName, ParentWidget, VerticalAlignmentName);
				}
			}
		}

		IsReentrant = false;
	}
}

#endif
