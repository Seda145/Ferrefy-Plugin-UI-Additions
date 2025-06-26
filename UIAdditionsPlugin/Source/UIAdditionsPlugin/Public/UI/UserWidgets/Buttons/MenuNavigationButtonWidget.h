/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "ButtonWidget.h"

#include "MenuNavigationButtonWidget.generated.h"

class UMenuWidget;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNavigationButtonRequestUINavigation, const FName&, InRoute);


/*
* Simple widget which broadcasts a request containing a navigation route on activation. Commonly used in menu navigation.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UMenuNavigationButtonWidget : public UButtonWidget {
    GENERATED_BODY()

private:

protected:

	// Setup
    
    /* The navigation route to pass to a registered parent menu. */
    UPROPERTY(EditAnywhere, Category = "Setup")
        FName NavigationRoute = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Setup")
        bool bAutoRegisterToParentMenu = true;

    UPROPERTY()
        UMenuWidget* RegisteredParentMenu = nullptr;

public:

	// Delegates
	
    UPROPERTY(BlueprintAssignable, Category = "Delegates")
        FOnNavigationButtonRequestUINavigation OnRequestUINavigation;

private:

protected:

    // Setup

    virtual void NativeOnInitialized() override;

    virtual void NativeConstruct() override;

    // Delegates

    virtual void ActOnButtonReleased() override;

public:
    
    // Navigation

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        bool GetAutoRegisterToParentMenu() const;

    /* Configure if this widget should automatically find and register to a parent MenuWidget. The value is processed on OnInitialized and NativeConstruct. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void SetAutoRegisterToParentMenu(bool bInAutoRegisterToParentMenu);

    /* RegisterToParentMenu runs on OnInitialized and NativeConstruct if GetAutoRegisterToParentMenu(). Call this manually when you need it to register earlier, for example directly after adding this widget as content to a widget panel. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void RegisterToParentMenu();

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        FName GetNavigationRoute() const;

    /* This is the route another menu is registered to on a parent menu of this button. A route passing FName::IsNone (empty / NAME_None) is used as a "go back" route. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void SetNavigationRoute(const FName& InRoute);

};