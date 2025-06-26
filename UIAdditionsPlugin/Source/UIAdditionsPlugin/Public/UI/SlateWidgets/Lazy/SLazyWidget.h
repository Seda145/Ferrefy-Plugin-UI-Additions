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


/*
* Currently nothing to do here. There is no proper way to detect if a widget is actually visible and on-screen relative to its ancestors, making checking for it here pointless. OnPaint is called regardless. Lazy load functionality lives in the UWidget.
*/


class UIADDITIONSPLUGIN_API SLazyWidget : public SCompoundWidget {

private:

protected:

public:

private:

protected:

public:

	// Setup

	SLATE_BEGIN_ARGS(SLazyWidget)
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

	SLazyWidget();

	void Construct(const FArguments& InArgs);

	// Content

	virtual void SetContent(TSharedRef<SWidget> InContent);

	const TSharedRef<SWidget>& GetContent() const;

	void ClearContent();

	// Appearance


	void SetHAlign(EHorizontalAlignment InHAlign);

	void SetVAlign(EVerticalAlignment InVAlign);

	void SetPadding(const TAttribute<FMargin>& InPadding);

};
