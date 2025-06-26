/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "SlateUtils.h"
#include "Blueprint/UserWidget.h"

#include "HoverFeedbackWidget.generated.h"


class UNamedSlot;
class SWidget;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHoverFeedbackAppearanceChanged);


/*
* This widget is used to implement a response on demand or when hovering / focusing content, used as outline / background for inner ContentSlot.
* The result is similar to hovering a button, but this appearance is highly customizable.
* It responds to changes in the currently used input device, so that it can simulate a hover appearance for gamepads as well.
* Possible uses are:
* - Outline widgets during a tutorial.
* - When using a gamepad, UI otherwise unresponsive to focus (no outline, no hover etc.) can be now visualize as the hover effect.
* - Design and uses are much more flexible than a hardcoded Slate style offers (button etc.).
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UHoverFeedbackWidget : public UUserWidget {
    GENERATED_BODY()

private:

    // Input

    bool bIsMouseEntered = false;

    // Focus

    bool bIsOnFocusPath = false;

    // Appearance

    bool bIsUsingHoverAppearanceForFocus = false;

    bool bAppearHoveredByDefault = false;

protected:

    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UNamedSlot* ContentSlot = nullptr;

public:

    // Delegates

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
        FOnHoverFeedbackAppearanceChanged OnHoverFeedbackAppearanceChanged;

private:

protected:

    // Setup

    virtual void NativeOnInitialized() override;

    // Appearance

    /* Checks by current state if the widget should appear as normal or hovered. */
    void UpdateFeedbackAppearance();

    /* Blueprint implements the design state for the normal apperance. */
    UFUNCTION(BlueprintImplementableEvent)
        void AppearNormal();

    /* Blueprint implements the design state for the hovered apperance. */
    UFUNCTION(BlueprintImplementableEvent)
        void AppearHovered();

    /* Blueprint implements the design state for the disabled apperance. */
    //UFUNCTION(BlueprintImplementableEvent)
        //void AppearDisabled();

    //Navigation

    virtual void NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) override;

    virtual void NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) override;

    virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

    // Delegates

    /* Tracks when the HID changes so we can simulate a hover style on focused widgets when using a HID other than a mouse. */
    UFUNCTION()
        virtual void ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex);

public:

    // Input

    /* Updated when a mouse enters this widget, or when NotifyMouseEntered/Left(). */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        bool GetIsMouseEntered() const;

    // Focus

    /* Updated when this widget is on a changing focus path, or when NotifyOn/LeftFocusPath). */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Focus")
        bool GetIsOnFocusPath() const;

    // Appearance

    virtual void SetIsEnabled(bool bInIsEnabled) override;

    /* Returns true if the widget's owner's current input device is not a mouse (if we don't point and hover a widget with cursor, navigation is done by focus.). */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
        bool GetIsUsingHoverAppearanceForFocus() const;

    /* Gets if the desired appearance is hovered by default. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
        bool GetAppearHoveredByDefault() const;

    /* Sets if the desired appearance is hovered by default. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
        void SetAppearHoveredByDefault(bool bInAppearHoveredByDefault);

    /* Notifies of an actual or pretend event to match appearance to. Useful when another widget needs to update this widget to a matching event. */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
        void NotifyAppearanceOnFocusPath();

    /* Notifies of an actual or pretend event to match appearance to. Useful when another widget needs to update this widget to a matching event. */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
        void NotifyAppearanceLeftFocusPath();

    /* Notifies of an actual or pretend event to match appearance to. Useful when another widget needs to update this widget to a matching event. */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
        void NotifyAppearanceMouseEntered();

    /* Notifies of an actual or pretend event to match appearance to. Useful when another widget needs to update this widget to a matching event. */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
        void NotifyAppearanceMouseLeave();

};