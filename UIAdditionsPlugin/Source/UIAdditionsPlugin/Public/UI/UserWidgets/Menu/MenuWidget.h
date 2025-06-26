/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"
#include "KeyboundUserWidget.h"
#include "Templates/SharedPointer.h"

#include "MenuWidget.generated.h"

class UMenuNavigationButtonWidget;
class UButtonWidget;
class SWidget;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRequestUINavigation, const FName&, InRoute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMenuVisibilityChanged, UMenuWidget*, InMenu, bool, bIsVisible);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMenuRegistrationChanged, const FName&, InRoute);


/*
* A menu which manages navigation inside itself, through a request (usually to a parent menu), or to a menu registered to a route (submenu).
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UMenuWidget : public UKeyboundUserWidget {
    GENERATED_BODY()

private:

    // Setup | Navigation

    // Navigation

    /* All Menus registered to a route. */
    UPROPERTY(Transient)
        TMap<FName, UMenuWidget*> Menus;

    UPROPERTY(Transient)
        TArray<UMenuNavigationButtonWidget*> NavigationButtons;

    /* The current route to an inner menu this menu navigated to. */
    UPROPERTY(Transient)
        FName ActiveRoute = NAME_None;

	/* The widget we should attempt to focus when RestoreFocus is called. */
    //UPROPERTY()
        TWeakPtr<SWidget> PreferredFocusEntryWidget = nullptr;

    /* The parent menu we can request navigation to. */
    UPROPERTY(Transient)
        UMenuWidget* RegisteredParentMenu = nullptr;

protected:

    /* Auto register this widget to a Menu ancestor during OnInitialized and Construct. */
    UPROPERTY(EditAnywhere, Category = "Setup")
        bool bAutoRegisterToParentMenu = true;

    /* Route to automatically register to on an ancestor menu if bAutoRegisterToParentMenu == true. */
    UPROPERTY(EditAnywhere, Category = "Setup")
        FName AutoRegisterRoute = NAME_None;

public:

    // Delegates

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
        FOnRequestUINavigation OnRequestUINavigation;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
        FOnMenuVisibilityChanged OnMenuVisibilityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
        FOnMenuRegistrationChanged OnRegisteredMenu;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
        FOnMenuRegistrationChanged OnUnRegisteredMenu;

private:

    // Navigation

    /* Private method which sets a descendant menu's parent menu during registration on an ancestor. */
    void SetRegisteredParentMenu(UMenuWidget* InParentMenu);

    // Delegates | Navigation

    /* The response to a navigation request of a UMenuWidget. */
    UFUNCTION()
        virtual void ActOnMenuRequestedUINavigation(const FName& InRoute);

protected:

    // Setup

    virtual void NativeOnInitialized() override;

    virtual void NativePreConstruct() override;

    virtual void NativeConstruct() override;

    // Navigation

    virtual void NativeOnFocusChanging(const FWeakWidgetPath& InPreviousFocusPath, const FWidgetPath& InNewWidgetPath, const FFocusEvent& InFocusEvent) override;

    /* Request navigation on a parent menu, if we registered to one. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        virtual void RequestUINavigation(const FName& InRoute);

    /* Implements non BP overridable or c++ exclusive logic during Show(). */
    virtual void NativeShow();

    /* Additional blueprint implementation at the end of Show(). */
    UFUNCTION(BlueprintImplementableEvent, Category = "Navigation")
        void OnShow();

    /* Implements non BP overridable or c++ exclusive logic during Hide(). */
    virtual void NativeHide();

    /* Additional blueprint implementation at the end of Hide(). */
    UFUNCTION(BlueprintImplementableEvent, Category = "Navigation")
        void OnHide();

    // Delegates

    UFUNCTION(BlueprintCallable, Category = "Delegates|Widgets")
        virtual void ActOnNavigationButtonRequestedNavigation(const FName& InRoute);

    /* Blueprint implementable event called after NavigateToRoute. Useful for the widget designer. */
    UFUNCTION(BlueprintImplementableEvent, Category = "Navigation")
        void AfterNavigateToRoute(const FName& InOldRoute, const FName& InRoute);

    // Delegates | Input

    UFUNCTION()
        virtual void ActOnNavBack();

public:

    // Setup

    UMenuWidget(const FObjectInitializer& InObjectInitializer);

    // Navigation

    /* 1. Updates visibility. 2. Calls c++ crucial / exclusive logic. 3. Calls the blueprint event. Do not call your intention is NavigateToRoute on the parent widget. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void Show();

    /* 1. Updates visibility. 2. Calls c++ crucial / exclusive logic. 3. Calls the blueprint event. Do not call your intention is NavigateToRoute on the parent widget. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void Hide();

	/* Restore focus to the preferred focus entry widget, or recursively through an active submenu. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void RestoreFocus();

    /* Bind to a Slate boundary navigation rule to restore focus. This is useful when you navigate with the cursor to a place from which you can not navigate out of with a gamepad. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Navigation")
        void UnStuckFocusNavigation(EUINavigation InNavigation);

    void UnStuckFocusNavigation_Implementation(EUINavigation InNavigation);

    /* Get all registered navigation buttons. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        const TArray<UMenuNavigationButtonWidget*>& GetNavigationButtons() const;

    /* Register menu navigation buttons for use with this menu. */
    UFUNCTION(BlueprintCallable, Category = "Setup|Navigation")
        void RegisterNavigationButtons(TArray<UMenuNavigationButtonWidget*> InNavigationButtons);

    /* Register menu navigation buttons for use with this menu. */
    UFUNCTION(BlueprintCallable, Category = "Setup|Navigation")
        void RegisterNavigationButton(UMenuNavigationButtonWidget* InNavigationButton);

    UFUNCTION(BlueprintCallable, Category = "Setup|Navigation")
        void UnRegisterNavigationButtons(TArray<UMenuNavigationButtonWidget*> InNavigationButtons);

    UFUNCTION(BlueprintCallable, Category = "Setup|Navigation")
        void UnRegisterNavigationButton(UMenuNavigationButtonWidget* InNavigationButton);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        bool GetAutoRegisterToParentMenu() const;

    /* Configure if this widget should automatically find and register to a parent MenuWidget. The value is processed on OnInitialized and NativeConstruct. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void SetAutoRegisterToParentMenu(bool bInAutoRegisterToParentMenu);

    /* RegisterToParentMenu runs on NativeConstruct / NativeOnInitialized if GetAutoRegisterToParentMenu(). Would still need to be called manually if not early enough. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void RegisterToParentMenu(const FName& InRoute);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        FName GetAutoRegisterRoute() const;

    /* Configure the route to use for auto registration. The value is processed on Construct. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void SetAutoRegisterRoute(const FName& InAutoRegisterRoute);

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        UMenuWidget* GetRegisteredParentMenu() const;

    /**
    * The default widget you must be able to focus to during RestoreFocus. 
    * Failing to do so leads to incorrect focus restoration. 
    * This widget is commonly a navigation button on a menu which you want to focus on the moment the menu opens by default.
    */
    UFUNCTION(BlueprintImplementableEvent, BlueprintPure = false, Category = "Navigation")
        UWidget* GetDefaultPreferredFocusEntryWidget() const;

    /**
    * The default widget you must be able to focus to during RestoreFocus. 
    * Failing to do so leads to incorrect focus restoration. 
    * This widget is commonly a navigation button on a menu which you want to focus on the moment the menu opens by default.
    */
    TWeakPtr<SWidget> GetPreferredFocusEntryWidget();

	/* 
    * The PreferredFocusEntryWidget is the widget you want to attempt to restore focus to during RestoreFocus. 
    * From InWidget the SWidget is retrieved, be it from a UserWidget or any base widget.
    * Make sure to use the exact focusable widget.
    * This preference will automatically update when navigating focus.
    */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        void SetPreferredFocusEntryWidget(UWidget* InWidget);

    /* 
    * The PreferredFocusEntryWidget is the widget you want to attempt to restore focus to during RestoreFocus. 
    * From InWidget the SWidget is retrieved, be it from a UserWidget or any base widget.
    * Make sure to use the exact focusable widget.
    * This preference will automatically update when navigating focus.
    */
    void SetPreferredFocusEntryWidget(const TWeakPtr<SWidget>& InWidget);

    /* Get all registered menu panels. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        TMap<FName, UMenuWidget*> GetMenus() const;

    /* Get a menu panel registered to this route, or nullptr if there is none. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        UMenuWidget* GetMenuByNavigationRoute(const FName& InRoute) const;

    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        bool IsAnyMenuVisible() const;

    /* Get a route to a menu panel if it is registered to one. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        FName GetNavigationRouteByMenu(UMenuWidget* InMenuWidget) const;

    /* Returns the last route set by NavigateToRoute. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        FName GetActiveRoute() const;

    /* Attempts to find a route from the registered menus, which matches InWidget. */
    UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Navigation")
        FName GetMenuRoute(UMenuWidget* InWidget) const;

    /* Attempts to NavigateToRoute with a route retrieved by GetMenuRoute. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        UMenuWidget* NavigateTo(UMenuWidget* InWidget);

    /* Returns a pointer to what was navigated to, or null. Menu must be a registered descendant. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        UMenuWidget* NavigateToRoute(const FName& InRoute);

    /* Register a menu to a route so it can be navigated to using NavigateToRoute. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        virtual bool RegisterMenu(UMenuWidget* InMenuWidget, const FName& InRoute);

    /* Unregister any menu registered to this route. */
    UFUNCTION(BlueprintCallable, Category = "Navigation")
        virtual bool UnRegisterMenu(const FName& InRoute);

};