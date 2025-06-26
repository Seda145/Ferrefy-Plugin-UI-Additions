/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "ButtonWidget.h"
#include "InputState.h"
#include "Engine/DataTable.h"
#include "InputKeyImageData.h"
#include "UObject/SoftObjectPtr.h"

#include "InputActionButtonWidget.generated.h"

class UImage;
class UTextBlock;
class UStringTable;


/**
* Widget which displays an image for an input action and simulates it on activation. 
* Optionally it displays localized text for the input action.
* This widget is best used to inform someone that an action can be performed by pressing a key associated with an input mapping.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UInputActionButtonWidget : public UButtonWidget {
    GENERATED_BODY()

private:

    // Appearance

    UPROPERTY(EditAnywhere, Category = "Appearance", meta = (RowType = "/Script/UIAdditionsPlugin.S_InputKeyImageData"))
        FDataTableRowHandle KeyBindingImageDT = FDataTableRowHandle();

    UPROPERTY(EditAnywhere, Category = "Appearance")
        FName ActionMappingName = FName();
		
	// Localization

    UPROPERTY(EditAnywhere, Category = "Localization")
        UStringTable* ActionMappingLocalizationStringTable = nullptr;

    UPROPERTY(EditAnywhere, Category = "Localization")
        FText OverrideText = FText();

protected:

    // Widgets

    /* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UImage* KeyImageWidget = nullptr;

    /* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UTextBlock* KeyTextWidget = nullptr;

public:

private:

    // Navigation
    
    void SimulateKeyEventOnPressOrRelease(bool bInPressed);

    // Appearance

    const FDataTableRowHandle& GetKeyBindingImageDT() const;

protected:

    // Setup

    virtual void NativePreConstruct() override;

    // Appearance

    void UpdateKeyBindingTexture(EInputDevices InInputDevice);

    void UpdateText();

    // Delegates

    virtual void ActOnButtonPressed() override;    
    
    virtual void ActOnButtonReleased() override;
    
    virtual void ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex) override;

public:

    // Setup

    UInputActionButtonWidget(const FObjectInitializer& ObjectInitializer);

    // Appearance

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
        UStringTable* GetActionMappingLocalizationStringTable() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Appearance")
        void SetActionMappingLocalizationStringTable(UStringTable* InStringTable);

    /* Overrides the automatically localized input action with something else. If the overriding text is empty or whitespace, it will not override anything. */
    UFUNCTION(BlueprintCallable, Category = "Appearance")
        void SetOverrideText(const FText& InText);
    
};