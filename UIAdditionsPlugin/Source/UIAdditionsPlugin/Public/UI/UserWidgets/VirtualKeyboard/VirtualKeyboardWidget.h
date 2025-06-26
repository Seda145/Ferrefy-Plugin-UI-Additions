/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"
#include "Templates/SharedPointer.h"

#include "VirtualKeyboardWidget.generated.h"


class UVirtualKeyboardKeyWidget;
class UVirtualKeyboardTextKeyWidget;
class UButtonWidget;


/*
* 
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UVirtualKeyboardWidget : public UMenuWidget {
    GENERATED_BODY()

private:

    // Widgets

    UPROPERTY(Transient)
        TArray<UVirtualKeyboardKeyWidget*> VirtualKeyboardKeyWidgets;    
    
    UPROPERTY(Transient)
        TArray<UVirtualKeyboardTextKeyWidget*> VirtualKeyboardTextKeyWidgets;

    UPROPERTY(Transient)
        UWidget* WidgetToReceiveInput = nullptr;

    // Input

    UPROPERTY(Transient)
        bool bIsSimulatingInput = false;

    // Appearance

    UPROPERTY(Transient)
        bool bIsCapsModifierEnabled = false;

protected:

    // Widgets
        
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidgetOptional))
        UButtonWidget* VirtualKeyboardCapsButton = nullptr;

public:

private:
    
    // Delegates

    UFUNCTION()
        void ActOnKeyboardKeyWidgetButtonPressed(UButtonWidget* InButtonWidget);

    UFUNCTION()
        void ActOnKeyboardKeyWidgetButtonReleased(UButtonWidget* InButtonWidget);

    UFUNCTION()
        void ActOnKeyboardTextKeyWidgetButtonReleased(UButtonWidget* InButtonWidget);

    UFUNCTION()
        void ActOnKeyboardTextKeyWidgetButtonPressed(UButtonWidget* InButtonWidget);

    UFUNCTION()
        void ActOnKeyboardCapsButtonReleased(UButtonWidget* InButtonWidget);

protected:

    // Setup

    virtual void NativeOnInitialized() override;

    // Widgets

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Widgets")
        UButtonWidget* GetVirtualKeyboardCapsButton() const;

    // Input

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

    /* Processes input a keyboard key widget represents, as if pressed or released. */
    UFUNCTION(BlueprintCallable, Category = "Input")
        void ProcessKeyboardKeyWidget(UVirtualKeyboardKeyWidget* InKeyboardKeyWidget, bool bInAsKeyDown);    
        
    /* Processes an input key as if processed through a keyboard key widget, just without a widget. */
    UFUNCTION(BlueprintCallable, Category = "Input")
        void ProcessInputKey(const FKey& InKey, bool bInAsKeyDown);    

    /* Attempts to add text to GetWidgetToReceiveInput(), retrieved from InKeyboardKeyWidget. */
    UFUNCTION(BlueprintCallable, Category = "Input")
        void ProcessKeyboardTextKeyWidget(UVirtualKeyboardTextKeyWidget* InKeyboardKeyWidget);

    /* Attempts to add text to GetWidgetToReceiveInput(). */
    UFUNCTION(BlueprintCallable, Category = "Input")
        void ProcessInputText(const FString& InText);

public:

    // Widgets 

    UFUNCTION(BlueprintCallable, Category = "Widgets")
        void RegisterKeyboardKeyWidget(UVirtualKeyboardKeyWidget* InWidget);    
    
    UFUNCTION(BlueprintCallable, Category = "Widgets")
        void RegisterKeyboardTextKeyWidget(UVirtualKeyboardTextKeyWidget* InWidget);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Widgets")
        UWidget* GetWidgetToReceiveInput() const;

    // Input

    /* Return if we are currently simulating input. 
    * This is especially useful when simulation causes a detected change in input device
    * , and you want to test if the cause is this simulation or an actual user action. 
    */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        bool IsSimulatingInput() const;

    /**
    * Sets the widget which input from the virtual keyboard needs to be sent to.
    */
    UFUNCTION(BlueprintCallable, Category = "Widgets")
        void BindWidgetWhichReceivesInput(UWidget* InWidget);

    // Appearance

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
        bool GetIsCapsModifierEnabled() const;

    UFUNCTION(BlueprintCallable, Category = "Appearance")
        void SetIsCapsModifierEnabled(bool bInIsCapsModifierEnabled);
        
};