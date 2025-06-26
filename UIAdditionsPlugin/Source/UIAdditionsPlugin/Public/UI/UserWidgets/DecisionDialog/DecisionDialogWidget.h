/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"

#include "DecisionDialogWidget.generated.h"


class UButtonWidget;
class UPanelWidget;
class UTextBlock;


UENUM(BlueprintType)
enum class E_DecisionDialogDecisions : uint8 {
    Yes,
    No,
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDecisionDialogDecided, E_DecisionDialogDecisions, InDecision);


/*
* A decision dialog is a widget which asks you to respond to a question. 
* Optionally it will make a decision for you after an amount of time.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UDecisionDialogWidget : public UMenuWidget {
    GENERATED_BODY()

private:

    // Setup | AutoDecide

    UPROPERTY(EditAnywhere, Category = "Setup|AutoDecide")
        bool bEnableAutoDecide = false;

    UPROPERTY(EditAnywhere, Category = "Setup|AutoDecide")
        E_DecisionDialogDecisions AutoDecision = E_DecisionDialogDecisions::No;

    UPROPERTY(EditAnywhere, Category = "Setup|AutoDecide", meta = (ClampMin = "2", UIMin = "2"))
        int32 AutoDecisionDuration = 10;

    UPROPERTY()
        int32 AutoDecisionRemainingDuration = AutoDecisionDuration;

    // Delegates | WidgetAnimation | AutoDecide

    UPROPERTY()
        FWidgetAnimationDynamicEvent OnWidgetAnimAutoDecideFinished;

protected:

    // Setup

    UPROPERTY(EditAnywhere, Category = "Setup")
        FText Title = FText();

    UPROPERTY(EditAnywhere, Category = "Setup")
        FText Description = FText();

	/* Procedurally generated animation used to loop a second during AutoDecide. */
    UPROPERTY(BlueprintReadOnly, Transient, Category = "Setup")
        UWidgetAnimation* AnimAutoDecide = nullptr;

    // Setup | Dependencies	

	/* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UTextBlock* TitleTextWidget = nullptr;

	/* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UTextBlock* DescriptionWidget = nullptr;

	/* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UPanelWidget* AutoDecideDescriptionPanelWidget = nullptr;

	/* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UTextBlock* AutoDecideDescriptionTextWidget = nullptr;

	/* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
        UTextBlock* AutoDecideDurationTextWidget = nullptr;

	/* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidgetOptional))
        UButtonWidget* ButtonYes = nullptr;

	/* Dependency required from a deriving class. */
    UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidgetOptional))
        UButtonWidget* ButtonNo = nullptr;

    // Setup | AutoDecide

    UPROPERTY(EditAnywhere, Category = "Setup|AutoDecide")
        FText AutoDecideDescription = FText();

public:

    UPROPERTY(BlueprintAssignable, Category = "Widgets|Menus|Settings")
        FOnDecisionDialogDecided OnDecided;

private:

protected:

	// Setup

    UDecisionDialogWidget(const FObjectInitializer& InObjectInitializer);

    virtual void NativeOnInitialized() override;

    virtual void NativePreConstruct() override;

	// AutoDecide

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "AutoDecide")
        int32 GetAutoDecisionDuration() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "AutoDecide")
        int32 GetAutoDecisionRemainingDuration() const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "AutoDecide")
        bool GetEnableAutoDecide() const;

    UFUNCTION(BlueprintNativeEvent, Category = "AutoDecide")
        void UpdateAutoDecideDurationWidget();

    virtual void UpdateAutoDecideDurationWidget_Implementation();

    // Delegates | DecisionDialogButton

    UFUNCTION()
        void ActOnButtonYesActivated(UButtonWidget* InButtonWidget);

    UFUNCTION()
        void ActOnButtonNoActivated(UButtonWidget* InButtonWidget);

    // Delegates | WidgetAnimation

    UFUNCTION()
        void ActOnWidgetAnimAutoDecideFinished();

public:

    // Navigation

    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void RestartAutoDecisionAnimation();

    virtual void NativeShow() override;

    virtual void NativeHide() override;

};