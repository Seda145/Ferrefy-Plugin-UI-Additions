/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "ButtonWidget.h"
#include "Engine/DataTable.h"

#include "VirtualKeyboardTextKeyWidget.generated.h"


class UImage;
class UTexture2D;


/*
* Widget which can be set up visually to match an input key and it's shifted input key.
* It is intended to be used to simulate text input external systems, and offers better support for this than VirtualKeyboardKeyWidget.
* Support for this is improved because input texts matching this virtual key are manually configured and copied over, instead of simulating a key press.
* An easier to set up key press (VirtualKeyboardKeyWidget) would be lacking for text input: miss characters, miss character shifting and such logic done on the OS level code / some parts of Slate.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UVirtualKeyboardTextKeyWidget : public UButtonWidget {
    GENERATED_BODY()

private:

protected:

    // Input

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        FString InputText = "";

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        FString InputTextShift = "";

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        bool bTriggerOnKeyPressed = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
        bool bTriggerOnKeyReleased = true;

    UPROPERTY(Transient)
        bool bUseInputTextShift = false;

    // Appearance
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
        UTexture2D* InputKeyTexture = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
        UTexture2D* InputKeyShiftTexture = nullptr;

    // Widgets

    /* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UImage* InputTextImageWidget = nullptr;

public:

private:

    // Setup

    void RegisterToAncestorVirtualKeyboardWidget();

    // Appearance

    void UpdateInputTextTexture();

protected:

    // Setup

    virtual void NativeOnInitialized() override;

    virtual void NativePreConstruct() override;

    // Widgets

    UImage* GetInputTextImageWidget() const;

public:

    // Input

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        FString GetInputText() const;

    UFUNCTION(BlueprintCallable, Category = "Input")
        void SetInputText(const FString& InInputKey);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        bool GetTriggerOnKeyPressed() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        bool GetTriggerOnKeyReleased() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        FString GetInputTextShift() const;

    UFUNCTION(BlueprintCallable, Category = "Input")
        void SetInputTextShift(const FString& InInputTextShift);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        bool GetUseInputTextShift() const;
    
    UFUNCTION(BlueprintCallable, Category = "Input")
        void SetUseInputTextShift(bool bInUseInputTextShift);

    // Appearance

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        UTexture2D* GetInputKeyTexture() const;

    UFUNCTION(BlueprintCallable, Category = "Input")
        void SetInputKeyTexture(UTexture2D* InInputKeyTexture);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
        UTexture2D* GetInputKeyShiftTexture() const;

    UFUNCTION(BlueprintCallable, Category = "Input")
        void SetInputKeyShiftTexture(UTexture2D* InInputKeyShiftTexture);

};