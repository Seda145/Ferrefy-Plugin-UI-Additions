/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "SLazyWidget.h"


static FName SLazyWidgetTypeName("SLazyWidget");

// Setup

SLazyWidget::SLazyWidget() {
	SetCanTick(false);
}

void SLazyWidget::Construct(const SLazyWidget::FArguments& InArgs) {
	ChildSlot
		.HAlign(InArgs._HAlign)
		.VAlign(InArgs._VAlign)
		.Padding(InArgs._Padding)
		[InArgs._Content.Widget];
}

// Content

void SLazyWidget::SetContent(TSharedRef<SWidget> InContent) {
	ChildSlot[InContent];
}

const TSharedRef<SWidget>& SLazyWidget::GetContent() const {
	return ChildSlot.GetWidget();
}

void SLazyWidget::ClearContent() {
	ChildSlot.DetachWidget();
}

// Appearance

void SLazyWidget::SetHAlign(EHorizontalAlignment InHAlign) {
	ChildSlot.SetHorizontalAlignment(InHAlign);
}

void SLazyWidget::SetVAlign(EVerticalAlignment InVAlign) {
	ChildSlot.SetVerticalAlignment(InVAlign);
}

void SLazyWidget::SetPadding(const TAttribute<FMargin>& InPadding) {
	ChildSlot.SetPadding(InPadding);
}
