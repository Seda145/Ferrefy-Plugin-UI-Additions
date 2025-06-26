/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "LazyWidgetSlot.h"
#include "SLazyWidget.h"
#include "LazyWidget.h"
#include "Widgets/SNullWidget.h"
#include "ObjectEditorUtils.h"


// Setup

void ULazyWidgetSlot::ReleaseSlateResources(bool bInReleaseChildren) {
	Super::ReleaseSlateResources(bInReleaseChildren);
	LazyWidget.Reset();
}

void ULazyWidgetSlot::BuildSlot(TSharedRef<SLazyWidget> InLazyWidget) {
	LazyWidget = InLazyWidget;

	LazyWidget.Pin()->SetPadding(Padding);
	LazyWidget.Pin()->SetHAlign(HorizontalAlignment);
	LazyWidget.Pin()->SetVAlign(VerticalAlignment);

	LazyWidget.Pin()->SetContent(Content ? Content->TakeWidget() : SNullWidget::NullWidget);
}

void ULazyWidgetSlot::SetPadding(FMargin InPadding) {
	CastChecked<ULazyWidget>(Parent)->SetPadding(InPadding);
}

void ULazyWidgetSlot::SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment) {
	CastChecked<ULazyWidget>(Parent)->SetHorizontalAlignment(InHorizontalAlignment);
}

void ULazyWidgetSlot::SetVerticalAlignment(EVerticalAlignment InVerticalAlignment) {
	CastChecked<ULazyWidget>(Parent)->SetVerticalAlignment(InVerticalAlignment);
}

void ULazyWidgetSlot::SynchronizeProperties() {
	if (LazyWidget.IsValid()) {
		SetPadding(Padding);
		SetHorizontalAlignment(HorizontalAlignment);
		SetVerticalAlignment(VerticalAlignment);
	}
}

#if WITH_EDITOR

void ULazyWidgetSlot::PostEditChangeProperty(FPropertyChangedEvent& InPropertyChangedEvent) {
	Super::PostEditChangeProperty(InPropertyChangedEvent);

	static bool IsReentrant = false;

	if (!IsReentrant) {
		IsReentrant = true;

		if (InPropertyChangedEvent.Property) {
			static const FName PaddingName("Padding");
			static const FName HorizontalAlignmentName("HorizontalAlignment");
			static const FName VerticalAlignmentName("VerticalAlignment");

			if (ULazyWidget* ParentWidget = CastChecked<ULazyWidget>(Parent)) {
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
