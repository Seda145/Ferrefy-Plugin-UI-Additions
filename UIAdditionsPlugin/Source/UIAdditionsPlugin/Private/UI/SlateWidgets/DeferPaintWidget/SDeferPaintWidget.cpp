/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "SDeferPaintWidget.h"


static FName SDeferPaintWidgetTypeName("SDeferPaintWidget");

// Setup

SDeferPaintWidget::SDeferPaintWidget() {
	if (GetType() == SDeferPaintWidgetTypeName) {
		SetVisibility(EVisibility::SelfHitTestInvisible);
		bCanSupportFocus = false;
	}
}

void SDeferPaintWidget::Construct(const SDeferPaintWidget::FArguments& InArgs) {
	// Only do this if we're exactly an SDeferPaintWidget
	if (GetType() == SDeferPaintWidgetTypeName) {
		SetCanTick(false);
		SetVisibility(EVisibility::SelfHitTestInvisible);
		bCanSupportFocus = false;
	}

	ChildSlot
		.HAlign(InArgs._HAlign)
		.VAlign(InArgs._VAlign)
		.Padding(InArgs._Padding)
		[InArgs._Content.Widget];
}

// Content

void SDeferPaintWidget::SetContent(TSharedRef<SWidget> InContent) {
	ChildSlot[InContent];
}

const TSharedRef<SWidget>& SDeferPaintWidget::GetContent() const {
	return ChildSlot.GetWidget();
}

void SDeferPaintWidget::ClearContent() {
	ChildSlot.DetachWidget();
}

// Appearance

void SDeferPaintWidget::SetHAlign(EHorizontalAlignment InHAlign) {
	ChildSlot.SetHorizontalAlignment(InHAlign);
}

void SDeferPaintWidget::SetVAlign(EVerticalAlignment InVAlign) {
	ChildSlot.SetVerticalAlignment(InVAlign);
}

void SDeferPaintWidget::SetPadding(const TAttribute<FMargin>& InPadding) {
	ChildSlot.SetPadding(InPadding);
}

int32 SDeferPaintWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const {
	// This causes the widget to render after everything else
	OutDrawElements.QueueDeferredPainting(FSlateWindowElementList::FDeferredPaint(ChildSlot.GetWidget(), Args, AllottedGeometry, InWidgetStyle, bParentEnabled));
	return SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, ShouldBeEnabled(bParentEnabled));
}
