/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "Layout/Margin.h"
#include "Widgets/SWidget.h"

#include "DeferPaintWidget.generated.h"

class SDeferPaintWidget;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USlateBrushAsset;
class UTexture2D;


	/* Wrapper which renders on top of everything else. */
UCLASS(meta=(DisplayName="Defer Paint"))
class UIADDITIONSPLUGIN_API UDeferPaintWidget : public UContentWidget
{
	GENERATED_BODY()

private:

	// Appearance

	UPROPERTY(EditAnywhere, Category = "Appearance")
		TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	UPROPERTY(EditAnywhere, Category = "Appearance")
		TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Fill;

	UPROPERTY(EditAnywhere, Category = "Appearance")
		FMargin Padding = FMargin(0, 0);

protected:

	// Content

	TSharedPtr<SDeferPaintWidget> MyDeferPaintWidget = nullptr;

public:

private:

protected:

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;

	virtual void OnSlotAdded(UPanelSlot* Slot) override;

	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	//~ End UWidget Interface

public:

	// Setup

	UDeferPaintWidget(const FObjectInitializer& ObjectInitializer);

	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	//~ End UWidget Interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface

#if WITH_EDITOR
	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	//~ End UObject Interface

	// Palette

	virtual const FText GetPaletteCategory() override;
#endif

	// Appearance

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
		EHorizontalAlignment GetHorizontalAlignment() const;

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetHorizontalAlignment(EHorizontalAlignment InHorizontalAlignment);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
		EVerticalAlignment GetVerticalAlignment() const;

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetVerticalAlignment(EVerticalAlignment InVerticalAlignment);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
		FMargin GetPadding() const;

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void SetPadding(FMargin InPadding);

};
