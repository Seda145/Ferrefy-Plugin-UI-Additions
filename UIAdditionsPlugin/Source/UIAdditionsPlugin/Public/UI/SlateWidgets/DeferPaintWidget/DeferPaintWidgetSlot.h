/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"
#include "Components/PanelSlot.h"
#include "Templates/SharedPointer.h"
#include "Layout/Margin.h"

#include "DeferPaintWidgetSlot.generated.h"

class SDeferPaintWidget;
class UDeferPaintWidget;


UCLASS()
class UIADDITIONSPLUGIN_API UDeferPaintWidgetSlot : public UPanelSlot {
	GENERATED_BODY()

private:

	friend UDeferPaintWidget;

	//UPROPERTY()
		TWeakPtr<SDeferPaintWidget> DeferPaintWidget;

protected:

	UPROPERTY(EditAnywhere, Category = "Layout|DeferPaintWidgetSlot")
		FMargin Padding = FMargin(0, 0);

	UPROPERTY(EditAnywhere, Category = "Layout|DeferPaintWidgetSlot")
		TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	UPROPERTY(EditAnywhere, Category = "Layout|DeferPaintWidgetSlot")
		TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Fill;

public:

private:

protected:

public:

	// Setup

	UFUNCTION(BlueprintCallable, Category = "Layout|DeferPaintWidgetSlot")
		void SetPadding(FMargin InPadding);

	UFUNCTION(BlueprintCallable, Category = "Layout|DeferPaintWidgetSlot")
		void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	UFUNCTION(BlueprintCallable, Category = "Layout|DeferPaintWidgetSlot")
		void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

	// UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// End of UPanelSlot interface

	void BuildSlot(TSharedRef<SDeferPaintWidget> InDeferPaintWidget);

	virtual void ReleaseSlateResources(bool bInReleaseChildren) override;

#if WITH_EDITOR

	// UObject interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& InPropertyChangedEvent) override;
	// End of UObject interface

#endif

};
