/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"
#include "Components/PanelSlot.h"
#include "Templates/SharedPointer.h"
#include "Layout/Margin.h"

#include "LazyWidgetSlot.generated.h"

class SLazyWidget;
class ULazyWidget;


UCLASS()
class UIADDITIONSPLUGIN_API ULazyWidgetSlot : public UPanelSlot {
	GENERATED_BODY()

private:

	friend ULazyWidget;

	//UPROPERTY()
		TWeakPtr<SLazyWidget> LazyWidget;

protected:

	UPROPERTY(EditAnywhere, Category = "Layout|LazyWidgetSlot")
		FMargin Padding = FMargin(0, 0);

	UPROPERTY(EditAnywhere, Category = "Layout|LazyWidgetSlot")
		TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	UPROPERTY(EditAnywhere, Category = "Layout|LazyWidgetSlot")
		TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Fill;

public:

private:

protected:

public:

	// Setup

	UFUNCTION(BlueprintCallable, Category = "Layout|LazyWidgetSlot")
		void SetPadding(FMargin InPadding);

	UFUNCTION(BlueprintCallable, Category = "Layout|LazyWidgetSlot")
		void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	UFUNCTION(BlueprintCallable, Category = "Layout|LazyWidgetSlot")
		void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

	// UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// End of UPanelSlot interface

	void BuildSlot(TSharedRef<SLazyWidget> InLazyWidget);

	virtual void ReleaseSlateResources(bool bInReleaseChildren) override;

#if WITH_EDITOR

	// UObject interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& InPropertyChangedEvent) override;
	// End of UObject interface

#endif

};
