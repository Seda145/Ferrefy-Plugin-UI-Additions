/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Templates/SharedPointer.h"
#include "Misc/Attribute.h"
#include "Styling/SlateColor.h"
#include "Widgets/SWidget.h"
#include "Layout/Margin.h"
#include "Widgets/SCompoundWidget.h"
#include "Types/SlateEnums.h"


class UIADDITIONSPLUGIN_API SDeferPaintWidget : public SCompoundWidget {

private:

protected:

private:

protected:

public:

	// Setup

	SLATE_BEGIN_ARGS(SDeferPaintWidget)
		: _Content()
		, _HAlign(HAlign_Fill)
		, _VAlign(VAlign_Fill)
		, _Padding(FMargin(0.f))
	{}

	SLATE_DEFAULT_SLOT(FArguments, Content)

	SLATE_ARGUMENT(EHorizontalAlignment, HAlign)
	SLATE_ARGUMENT(EVerticalAlignment, VAlign)
	SLATE_ATTRIBUTE(FMargin, Padding)

	SLATE_END_ARGS()

	SDeferPaintWidget();

	void Construct(const FArguments& InArgs);

	// Content

	virtual void SetContent(TSharedRef<SWidget> InContent);

	const TSharedRef<SWidget>& GetContent() const;

	void ClearContent();

	// Appearance

	void SetHAlign(EHorizontalAlignment InHAlign);

	void SetVAlign(EVerticalAlignment InVAlign);

	void SetPadding(const TAttribute<FMargin>& InPadding);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

};
