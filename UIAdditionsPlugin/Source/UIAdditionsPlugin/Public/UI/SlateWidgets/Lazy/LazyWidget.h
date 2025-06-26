/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Components/ContentWidget.h"
#include "Layout/Margin.h"
#include "Widgets/SWidget.h"

#include "LazyWidget.generated.h"

class UUserWidget;
class SLazyWidget;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USlateBrushAsset;
class UTexture2D;
struct FStreamableHandle;


/** New lazy load delegates which are blueprint accessible. 
* Lives here as there is likely no other reason to respond to a change in a streaming status
in Blueprint besides when you want to show progress in widgets, load icon or text feedback. 
*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGenericWidgetLazyLoadEvent);


/* Wrapper which loads and unloads content to its slot from a soft pointer when requested, when added to a visible UserWidget and when its outer UserWidget broadcasts a change in visibility. */
UCLASS(meta=(DisplayName="Lazy"))
class UIADDITIONSPLUGIN_API ULazyWidget : public UContentWidget
{
	GENERATED_BODY()

private:

	// Setup 

	UPROPERTY(EditAnywhere, Category = "Setup")
		TSoftClassPtr<UUserWidget> LazyContent = nullptr;

	/* Load content async or sync */
	UPROPERTY(EditAnywhere, Category = "Setup")
		bool bLoadAsync = true;

	/* Load / Unload content when the outer UserWidget visibility changes. */
	UPROPERTY(EditAnywhere, Category = "Setup")
		bool bLoadControlledByOuterVisibility = true;

	/* Unload content when the loaded content visibility changes. */
	UPROPERTY(EditAnywhere, Category = "Setup")
		bool bLoadControlledBySelfVisibility = true;

	/* Slate uses resources for all hidden / removed widgets until they are garbage collected. */
	UPROPERTY(EditAnywhere, Category = "Setup")
		bool bCollectGarbageOnUnload = false;

#if WITH_EDITORONLY_DATA

	UPROPERTY(EditAnywhere, Category = "Preview")
		bool bLoadInEditor = true;

#endif // WITH_EDITORONLY_DATA

	// Appearance

	UPROPERTY(EditAnywhere, Category = "Appearance")
		TEnumAsByte<EHorizontalAlignment> HorizontalAlignment = EHorizontalAlignment::HAlign_Fill;

	UPROPERTY(EditAnywhere, Category = "Appearance")
		TEnumAsByte<EVerticalAlignment> VerticalAlignment = EVerticalAlignment::VAlign_Fill;

	UPROPERTY(EditAnywhere, Category = "Appearance")
		FMargin Padding = FMargin(0, 0);

	// Lazy

	UPROPERTY()
		UUserWidget* OuterUserWidget = nullptr;

	TSharedPtr<FStreamableHandle> StreamingHandle;

protected:

	// Lazy

	TSharedPtr<SLazyWidget> MyLazyWidget = nullptr;

public:

	// Delegates

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnGenericWidgetLazyLoadEvent OnLazyLoadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnGenericWidgetLazyLoadEvent OnLazyLoadCancelled;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnGenericWidgetLazyLoadEvent OnLazyLoadUpdates;
	
	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnGenericWidgetLazyLoadEvent OnLoadComplete;

private:

	/* Private helper to update with loaded content. */
	void SetLoadedContent(const TSubclassOf<UUserWidget> InContentClass);

	// Delegates

	UFUNCTION()
		void ActOnOuterVisibilityChanged(ESlateVisibility InVisibility);

protected:

	// Setup

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;

	virtual void OnSlotAdded(UPanelSlot* InSlot) override;

	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;
	// End UPanelWidget

	//~ Begin UWidget Interface
	virtual TSharedRef<SWidget> RebuildWidget() override;

	virtual void OnWidgetRebuilt() override;
	//~ End UWidget Interface

	// Lazy

	/* Loads content from the soft pointer returned by GetLazyContent, if conditions are met. */
	virtual void ConditionalLoadOrUnloadContent();

public:

	// Setup

	ULazyWidget(const FObjectInitializer& InObjectInitializer);

	//~ Begin UWidget Interface
	virtual void SynchronizeProperties() override;
	//~ End UWidget Interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bInReleaseChildren) override;
	//~ End UVisual Interface

	//~ Begin UObject Interface
	virtual void PostLoad() override;
	//~ End UObject Interface

#if WITH_EDITOR

	//~ Begin UObject Interface
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& InPropertyChangedEvent) override;
	//~ End UObject Interface

	// Palette

	virtual const FText GetPaletteCategory() override;

#endif

	// Appearance

	virtual void SetVisibility(ESlateVisibility InVisibility) override;

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

	// Lazy

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lazy")
		bool GetLoadAsync() const;

	UFUNCTION(BlueprintCallable, Category = "Lazy")
		void SetLoadAsync(bool bInLoadAsync);

	/* Load / Unload content when the outer UserWidget visibility changes. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lazy")
		bool GetLoadControlledByOuterVisibility() const;

	/* Load / Unload content when the outer UserWidget visibility changes. */
	UFUNCTION(BlueprintCallable, Category = "Lazy")
		void SetLoadControlledByOuterVisibility(bool bInLoadControlledByOuterVisibility);

	/* Unload content when the loaded content visibility changes. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lazy")
		bool GetLoadControlledBySelfVisibility() const;

	/* Unload content when the loaded content visibility changes. */
	UFUNCTION(BlueprintCallable, Category = "Lazy")
		void SetLoadControlledBySelfVisibility(bool bInLoadControlledBySelfVisibility);

	/* Slate uses resources for all hidden / removed widgets until they are garbage collected. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lazy")
		bool GetCollectGarbageOnUnload() const;

	/* Slate uses resources for all hidden / removed widgets until they are garbage collected. */
	UFUNCTION(BlueprintCallable, Category = "Lazy")
		void SetCollectGarbageOnUnload(bool bInCollectGarbageOnUnload);

	/* Returns a soft pointer to a set class which can be loaded or unloaded on demand. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Lazy")
		TSoftClassPtr<UUserWidget> GetLazyContent() const;

	/* Sets a soft pointer to a class which can be loaded or unloaded on demand. This does not load it. */
	UFUNCTION(BlueprintCallable, Category = "Lazy")
		void SetLazyContent(const TSoftClassPtr<UUserWidget> InLazyContent);

	/* Clears any loaded content on this panel. */
	UFUNCTION(BlueprintCallable, Category = "Lazy")
		void UnloadContent();

	/* Loads content from the soft pointer returned by GetLazyContent. */
	UFUNCTION(BlueprintCallable, Category = "Lazy")
		void LoadContent();

};
