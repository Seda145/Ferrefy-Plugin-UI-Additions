/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "ButtonWidget.h"
#include "Engine/DataTable.h"
#include "InputKeyImageData.h"
#include "UObject/SoftObjectPtr.h"

#include "VirtualKeyboardKeyWidget.generated.h"


class UImage;


/*
* Widget which can be quickly set up visually to match an input key.
* It is intended to be used to simulate input actions in external systems, much like the InputActionButtonWidget.
* If your intention is to input text by using this widget as a virtual input key button, you should use VirtualKeyboardTextKeyWidget instead for better support.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UVirtualKeyboardKeyWidget : public UButtonWidget {
    GENERATED_BODY()

private:

protected:

    // Appearance

    UPROPERTY(EditAnywhere, Category = "Appearance", meta = (RowType = "/Script/UIAdditionsPlugin.S_InputKeyImageData"))
        FDataTableRowHandle KeyBindingImageDT = FDataTableRowHandle();

    // Input

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        FKey InputKey = EKeys::Invalid;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        bool bTriggerOnKeyPressed = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        bool bTriggerOnKeyReleased = true;

    // Widgets

    /* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UImage* InputKeyImageWidget = nullptr;

public:

    // Setup

    UVirtualKeyboardKeyWidget(const FObjectInitializer& ObjectInitializer);

private:

    // Setup

    void RegisterToAncestorVirtualKeyboardWidget();

    // Appearance

    const FDataTableRowHandle& GetKeyBindingImageDT() const;

    void UpdateInputKeyTexture();

protected:

    // Setup

    virtual void NativeOnInitialized() override;

    virtual void NativePreConstruct() override;

    // Widgets

    UImage* GetInputKeyImageWidget() const;

public:

    // Input

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        FKey GetInputKey() const;

    UFUNCTION(BlueprintCallable, Category = "Input")
        void SetInputKey(const FKey& InInputKey);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        bool GetTriggerOnKeyPressed() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        bool GetTriggerOnKeyReleased() const;

};