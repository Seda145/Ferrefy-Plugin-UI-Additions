/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputState.h"

#include "ButtonWidget.generated.h"


class UCentralButton;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnButtonInteraction, UButtonWidget*, InButtonWidget);


/*
* Simple widget which broadcasts when it is activated. Wraps around a slate button for button functionality.
* You can set a preference for an appearance (hovered / pressed).
* Offers methods to respond to a change in preferred and actual button interactive states.
* Simulates a hover visual when using a non mouse HID when this widget is on the focus path.
* To make the styling possible with the lack of access to Slate's button methods, styling for interactive states is read from cache and written to the "Normal style" channel.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UButtonWidget : public UUserWidget {
    GENERATED_BODY()

private:

    UPROPERTY(Transient)
    bool bAppearHovered = false;

    UPROPERTY(Transient)
    bool bAppearPressed = false;

    UPROPERTY(Transient)
    bool bIsOnFocusPath = false;

    UPROPERTY(Transient)
    bool bIsButtonHovered = false;

    UPROPERTY(Transient)
    bool bIsButtonPressed = false;

    UPROPERTY(Transient)
    bool bDesiresHoverAppearanceForFocus = false;
    
protected:

    /* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
    UCentralButton* ButtonWidget = nullptr;

    // Navigation

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Navigation")
    bool bFocusButtonWidgetOnMouseEnter = true;

public:

    // Delegates

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnButtonInteraction OnButtonPressed;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnButtonInteraction OnButtonReleased;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnButtonInteraction OnButtonHovered;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnButtonInteraction OnButtonUnhovered;

private:

protected:

    // Setup

    virtual void NativeOnInitialized() override;

    // Navigation

    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    // Appearance

    /* This method reads if we want to appear as hovered, hovered during focus with a non mouse HID, or pressed.
     * The button styles are swapped to the normal channel to get past a limited access to the inner button's data.
     * bInPlaySound can optionally be true if sound is not handled by an interaction with the inner button, for example when using "SetAppearPressed".
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Appearance")
    void UpdateButtonInteractionStateAppearance(bool bInPlaySound);

    virtual void UpdateButtonInteractionStateAppearance_Implementation(bool bInPlaySound);

    //Navigation

    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;

    virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

    // Delegates

    UFUNCTION()
    virtual void ActOnButtonPressed();    
    
    UFUNCTION()
    virtual void ActOnButtonReleased();

    UFUNCTION()
    virtual void ActOnButtonHovered();    
    
    UFUNCTION()
    virtual void ActOnButtonUnhovered();
    
    /* Tracks when the HID changes so we can simulate a hover style on focused widgets when using a HID other than a mouse. */
    UFUNCTION()
    virtual void ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex);

public:

    // Widgets

    /* Get the (U) Slate button nested within this UserWidget. This provides the button and focus functionality. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Widgets")
    UCentralButton* GetButtonWidget() const;

    // Appearance
    
    /* To implement a UI design in response to a change in appearance, when you need more control than the inner button's visuals offers. Useful for playing animations. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Appearance")
    void OnAppearNormal();

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
    bool GetAppearHovered() const;

    /* UpdateButtonInteractionStateAppearance will be called if this state changes. Use to prefer this appearance. */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetAppearHovered(bool bInAppearHovered, bool bInPlaySound);

    /* To implement a UI design in response to a change in appearance, when you need more control than the inner button's visuals offers. Useful for playing animations. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Appearance")
    void OnAppearHovered();

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
    bool GetAppearPressed() const;

    /* UpdateButtonInteractionStateAppearance will be called if this state changes. Use to prefer this appearance. */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetAppearPressed(bool bInAppearPressed, bool bInPlaySound);

    /* To implement a UI design in response to a change in appearance, when you need more control than the inner button's visuals offers. Useful for playing animations. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Appearance")
    void OnAppearPressed();

    // Navigation

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
    bool GetIsOnFocusPath() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
    bool GetIsButtonHovered() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
    bool GetIsButtonPressed() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
    bool GetDesiresHoverAppearanceForFocus() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
    bool GetFocusButtonWidgetOnMouseEnter() const;

    UFUNCTION(BlueprintCallable, Category = "Navigation")
    void SetFocusButtonWidgetOnMouseEnter(bool bInFocusButtonWidgetOnMouseEnter);

    /* Passes on bInIsEnabled to widgets bound to this widget. If disabled, this widget will block some methods used as in-game user interactions. */
    virtual void SetIsEnabled(bool bInIsEnabled) override;

};