/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "MenuWidget.h"
#include "SubHUDWidget.h"
#include "MenuNavigationButtonWidget.h"
#include "Blueprint/UserWidget.h"
#include "Components/PanelWidget.h"
#include "ButtonWidget.h"
#include "LogUIAdditionsPlugin.h"
#include "Widgets/SWidget.h"
#include "SlateUtils.h"
#include "Slate/SObjectWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Layout/WidgetPath.h"
#include "Framework/Application/SlateApplication.h"
#include "UnrealClient.h"


// Setup

UMenuWidget::UMenuWidget(const FObjectInitializer& InObjectInitializer)
	: Super(InObjectInitializer) {

	// Not visible by default, since a menu is expected to be navigated to.
	SetVisibility(ESlateVisibility::Collapsed);
}

void UMenuWidget::NativeOnInitialized() {
	// Bind default input actions before Super call, so BP designers can alter behavior OnInitialized if required.
	BindRoutedInputAction(USlateUtils::InputActionNavBack, false, true, this, &UMenuWidget::ActOnNavBack);
	
	Super::NativeOnInitialized();

	if (GetAutoRegisterToParentMenu()) {
		RegisterToParentMenu(GetAutoRegisterRoute());
	}
}

void UMenuWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	/* We don't want to navigate out of the menu using navigation keys. I don't see any reason why currently.
	* If we'd allow it Slate would not respect the order of menus, it would go straight from one widget to another anywhere on the screen regardless of layout.
	*/

	UWidget* RootWidget = GetRootWidget();
	if (IsValid(RootWidget)) {
		RootWidget->SetAllNavigationRules(EUINavigationRule::Stop, NAME_None);
	}
}

void UMenuWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (GetAutoRegisterToParentMenu()) {
		RegisterToParentMenu(GetAutoRegisterRoute());
	}
}

// Setup | Navigation

const TArray<UMenuNavigationButtonWidget*>& UMenuWidget::GetNavigationButtons() const {
	return NavigationButtons;
}

void UMenuWidget::RegisterNavigationButtons(TArray<UMenuNavigationButtonWidget*> InNavigationButtons) {
    for (UMenuNavigationButtonWidget* NavigationButtonX : InNavigationButtons) {
		RegisterNavigationButton(NavigationButtonX);
    }
}

void UMenuWidget::RegisterNavigationButton(UMenuNavigationButtonWidget* InNavigationButton) {
	if (!IsValid(InNavigationButton)) {
		return;
	}

	// Register the button.
	InNavigationButton->OnRequestUINavigation.AddUniqueDynamic(this, &UMenuWidget::ActOnNavigationButtonRequestedNavigation);
	NavigationButtons.AddUnique(InNavigationButton);
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Registered button: %s"), *InNavigationButton->GetName());
}

void UMenuWidget::UnRegisterNavigationButtons(TArray<UMenuNavigationButtonWidget*> InNavigationButtons) {
	for (UMenuNavigationButtonWidget* NavigationButtonX : InNavigationButtons) {
		UnRegisterNavigationButton(NavigationButtonX);
	}
}

void UMenuWidget::UnRegisterNavigationButton(UMenuNavigationButtonWidget* InNavigationButton) {
	if (!IsValid(InNavigationButton)) {
		return;
	}

	InNavigationButton->OnRequestUINavigation.RemoveDynamic(this, &UMenuWidget::ActOnNavigationButtonRequestedNavigation);
	NavigationButtons.Remove(InNavigationButton);
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("UnRegistered button: %s"), *InNavigationButton->GetName());
}

// Navigation

void UMenuWidget::Show() {
	// Perform c++ exclusive and crucial logic.
	NativeShow();
	// Lastly give BP a chance.
	OnShow();
}

void UMenuWidget::NativeShow() {
	// Hardcode the visibility change, it is required for external processes (menu, HUD, input).
	// The "Visible" mode is a hit test mode, which is not only used for cursor click registrations but also for Slate's focus navigation.
	SetVisibility(ESlateVisibility::Visible);
	OnMenuVisibilityChanged.Broadcast(this, true);	
}

void UMenuWidget::Hide() {
	// Perform c++ exclusive and crucial logic.
	NativeHide();
	// Lastly give BP a chance.
	OnHide();
}

void UMenuWidget::NativeHide() {
	// Hardcode the visibility change, it is required for external processes (menu, HUD, input).
	SetVisibility(ESlateVisibility::Collapsed);
	OnMenuVisibilityChanged.Broadcast(this, false);	
}

bool UMenuWidget::GetAutoRegisterToParentMenu() const {
	return bAutoRegisterToParentMenu;
}

void UMenuWidget::SetAutoRegisterToParentMenu(bool bInAutoRegisterToParentMenu) {
	bAutoRegisterToParentMenu = bInAutoRegisterToParentMenu;
}

void UMenuWidget::RegisterToParentMenu(const FName& InRoute) {
	if (InRoute.IsNone()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Menu %s can't auto register when route is None."), *GetName());
		return;
	}

	// When a widget is added to the viewport, or to a parent widget, we want to update our registration to a parent menu.

	UMenuWidget* NewParentMenuWidget = USlateUtils::FindAncestorWidgetByClass<UMenuWidget>(this);

	// Do nothing if already registered.
	if (IsValid(NewParentMenuWidget) && NewParentMenuWidget == GetRegisteredParentMenu()) {
		return;
	}

	// Remove any old registration.
	if (IsValid(GetRegisteredParentMenu())) {
		GetRegisteredParentMenu()->UnRegisterMenu(InRoute);
	}

	// Make the new registration.
	if (IsValid(NewParentMenuWidget)) {
		NewParentMenuWidget->RegisterMenu(this, InRoute);
		return;
	}
}

FName UMenuWidget::GetAutoRegisterRoute() const {
	return AutoRegisterRoute;
}

void UMenuWidget::SetAutoRegisterRoute(const FName& InAutoRegisterRoute) {
	AutoRegisterRoute = InAutoRegisterRoute;
}

UMenuWidget* UMenuWidget::GetRegisteredParentMenu() const {
	return RegisteredParentMenu;
}

void UMenuWidget::SetRegisteredParentMenu(UMenuWidget* InParentMenu) {
	RegisteredParentMenu = InParentMenu;
}

void UMenuWidget::UnStuckFocusNavigation_Implementation(EUINavigation InNavigation) {
    RestoreFocus();
}

void UMenuWidget::RestoreFocus() {
	UMenuWidget* ActiveMenu = GetMenuByNavigationRoute(GetActiveRoute());
	if (IsValid(ActiveMenu) && USlateUtils::IsVisible(ActiveMenu->GetVisibility())) {
		// If we have an active menu, which is visible, attempt to restore focus on that.
		ActiveMenu->RestoreFocus();
		return;
	}
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: This menu was found as the active menu, through recursive search."), *GetName());
	if (!IsValid(GetOwningLocalPlayer())) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("%s: Got no valid owning local player."), *GetName());
		return;
	}
	TSharedPtr<SWidget> PrefWidget = GetPreferredFocusEntryWidget().Pin();
	if (!PrefWidget.IsValid()) {
		// The only situation where this is acceptable is when a sub HUD closes its active menu into nothing.
		// If it were to happen from normal menu to menu navigation, the wrong widget receives input.
		if (Cast<USubHUDWidget>(this)) {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: GetPreferredFocusEntryWidget returns an invalid SWidget, but this widget is a sub HUD with no active menu. Aborting RestoreFocus."), *GetName());
		}
		else {
			UE_LOG(LogUIAdditionsPlugin, Error, TEXT("%s: GetPreferredFocusEntryWidget returns an invalid SWidget. Can't focus correctly!"), *GetName());
		}
		return;
	}
	if (!PrefWidget->GetVisibility().IsVisible()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("%s: GetPreferredFocusEntryWidget returns a non 'Visible' SWidget. Can't focus correctly!"), *GetName());
		return;
	}

	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Restoring focus to: %s"), *GetName(), *PrefWidget->GetTypeAsString());

	int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	FReply& DelayedSlateOperations = GetOwningLocalPlayer()->GetSlateOperations();
	if (FSlateApplication::Get().SetUserFocus(UserIndex, PrefWidget)) {
		DelayedSlateOperations.CancelFocusRequest();
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Could not set focus this frame, delaying a frame."), *GetName());
		DelayedSlateOperations.SetUserFocus(PrefWidget.ToSharedRef());
	}
}

void UMenuWidget::NativeOnFocusChanging(const FWeakWidgetPath& InPreviousFocusPath, const FWidgetPath& InNewWidgetPath, const FFocusEvent& InFocusEvent) {
	Super::NativeOnFocusChanging(InPreviousFocusPath, InNewWidgetPath, InFocusEvent);

	if (!InNewWidgetPath.IsValid()) {
		// Don't process null.
		return;
	}
	TSharedPtr<SWidget> LastNotCastedWidget = InNewWidgetPath.GetLastWidget();
	if (LastNotCastedWidget == GetPreferredFocusEntryWidget().Pin()) {
		// this widget is already preferred. 
		return;
	}	

	bool bIsPreferred = false;
	for (int32 i = InNewWidgetPath.Widgets.Num() - 1; i >= 0; i--)	{
		TSharedPtr<SWidget> WidgetX = InNewWidgetPath.Widgets[i].Widget;
		if (!WidgetX.IsValid()) {
			continue;
		}
		/**
		* Starting from the focused widget, up through its ancestors, we want to find the first menu widget it can be on.
		* If the widget is on this menu, it is desired to store it as a focus preference.
		* If it is nested in another menu, it is not desired.
		* To know if an SWidget is a menu, we have to cast to SObjectWidget and retrieve a UserWidget from there.
		*/
		TSharedPtr<SObjectWidget> ObjWidgetX = nullptr;
		// Dynamic casting is not available. To avoid UB on a static cast do a type comparison.
		if (WidgetX->GetType() == "SObjectWidget"){
			ObjWidgetX = StaticCastSharedPtr<SObjectWidget>(WidgetX);
		}

		if (!ObjWidgetX.IsValid()) {
			// There is nothing to do.
			continue;
		}

		UUserWidget* UsrWidgetX = ObjWidgetX->GetWidgetObject();
		if (UsrWidgetX == this) {
			// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Newly focused widget is directly on this menu. Will prefer it for focus."));
			// OK, it's directly on this menu.
			bIsPreferred = true;
			break;
		}
		else if (Cast<UMenuWidget>(UsrWidgetX)) {
			// It's nested in a different menu, not preferred.
			//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Newly focused widget is nested in another menu on this menu. It is not preferred for this menu's focus."));
			break;
		}
	}

	if (bIsPreferred) {
		SetPreferredFocusEntryWidget(LastNotCastedWidget);
	}
}

void UMenuWidget::RequestUINavigation(const FName& InRoute) {
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Requesting navigation to route: %s"), *GetName(), *InRoute.ToString());
    OnRequestUINavigation.Broadcast(InRoute);
}

TWeakPtr<SWidget> UMenuWidget::GetPreferredFocusEntryWidget() {
	if (PreferredFocusEntryWidget.IsValid()) {
		return PreferredFocusEntryWidget;
	}

	UWidget* DefaultWidget = GetDefaultPreferredFocusEntryWidget();
	return IsValid(DefaultWidget) ? TWeakPtr<SWidget>(DefaultWidget->TakeWidget()) : nullptr;
}

void UMenuWidget::SetPreferredFocusEntryWidget(UWidget* InWidget) {
	SetPreferredFocusEntryWidget(IsValid(InWidget) ? TWeakPtr<SWidget>(InWidget->TakeWidget()) : nullptr);
}

void UMenuWidget::SetPreferredFocusEntryWidget(const TWeakPtr<SWidget>& InWidget) {
	PreferredFocusEntryWidget = InWidget;
	// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Set preferred focus entry widget to: %s"), *GetName(), (InWidget.IsValid() ? *InWidget.Pin()->GetTypeAsString() : TEXT("Null")));
}

TMap<FName, UMenuWidget*> UMenuWidget::GetMenus() const {
	return Menus;
}

UMenuWidget* UMenuWidget::GetMenuByNavigationRoute(const FName& InRoute) const {
	return Menus.FindRef(InRoute);
}

bool UMenuWidget::IsAnyMenuVisible() const {
	if (!USlateUtils::IsVisible(GetVisibility())) {
		// Return false if self is invisible.
		return false;
	}
	for (const TPair<FName, UMenuWidget*>& MenuWidgetX : GetMenus()) {
		if (IsValid(MenuWidgetX.Value) && USlateUtils::IsVisible(MenuWidgetX.Value->GetVisibility())) {
			return true;
		}
	}
	return false;
}

FName UMenuWidget::GetNavigationRouteByMenu(UMenuWidget* InMenuWidget) const {
	if (!InMenuWidget) {
		return NAME_None;
	}
	for (const TPair<FName, UMenuWidget*>& MenuX : GetMenus()) {
		if (MenuX.Value == InMenuWidget) {
			return MenuX.Key;
		}
	}

	return NAME_None;
}

FName UMenuWidget::GetActiveRoute() const {
	return ActiveRoute;
}

FName UMenuWidget::GetMenuRoute(UMenuWidget* InWidget) const {
	if (!IsValid(InWidget)) {
		return NAME_None;
	}
	for (const TPair<FName, UMenuWidget*> MenuX : GetMenus()) {
		if (MenuX.Value == InWidget) {
			return MenuX.Key;
		}
	}
	return NAME_None;
}

UMenuWidget* UMenuWidget::NavigateTo(UMenuWidget* InWidget) {
	return NavigateToRoute(GetMenuRoute(InWidget));
}

UMenuWidget* UMenuWidget::NavigateToRoute(const FName& InRoute) {
	
	auto SetAppearPressedOnButtonsByRoute = [this](const FName& InRouteX, bool bInSetAppearPressedX) {
		for (UMenuNavigationButtonWidget* ButtonX : GetNavigationButtons()) {
			if (!IsValid(ButtonX)) {
				continue;
			}
			if (InRouteX == ButtonX->GetNavigationRoute()) {
				ButtonX->SetAppearPressed(bInSetAppearPressedX, false);
			}
		}		
	};
	
	if (!GetActiveRoute().IsNone()) {
		// Remove the pressed visualization from any button on the previous navigation route.
		SetAppearPressedOnButtonsByRoute(GetActiveRoute(), false);
	}

	if (InRoute.IsNone()) {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Navigating to NAME_None. Closing menu on active route if any."), *GetName());

		// Keep the current route as the old route, then set the new route.
		const FName OldRoute = GetActiveRoute();
		ActiveRoute = NAME_None;

		// Hide any old menu.
		UMenuWidget* OldMenu = GetMenuByNavigationRoute(OldRoute);
		if (IsValid(OldMenu)) {
			OldMenu->Hide();
		}
		RestoreFocus();

		// Notify blueprints.
		AfterNavigateToRoute(OldRoute, GetActiveRoute());
		return nullptr;
	}
	else {
		/**
		* If the route is not none this means we want to navigate to a child menu of this menu.
		*/
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Attempts navigating to route: %s"), *GetName(), *InRoute.ToString());

		// Keep the current route as the old route, then set the new route.
		const FName OldRoute = GetActiveRoute();
		ActiveRoute = InRoute;
		const FName TrackRoute = GetActiveRoute();
		
		// Add the pressed visualization to any button on the new navigation route.
		// think of this as an active "tab" button on a navigation bar.
		SetAppearPressedOnButtonsByRoute(GetActiveRoute(), true);

		UMenuWidget* NewMenu = GetMenuByNavigationRoute(GetActiveRoute());
		if (IsValid(NewMenu)) {
			// Hide any old menu.
			UMenuWidget* OldMenu = GetMenuByNavigationRoute(OldRoute);
			if (IsValid(OldMenu)) {
				OldMenu->Hide();
			}
			
			// navigate to the new menu.
			NewMenu->Show();

			/**
			* If for whatever reason TrackRoute != ActiveRoute at this point then it is expected that the child menu already requested navigation to NAME_None.
			* This is awkward. We should not continue and assume NavigateToRoute was called again.
			*/ 
			if (TrackRoute != GetActiveRoute()) {
				UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("%s: ActiveRoute changed during NavigateToRoute. Likely a child menu requested NAME_None navigation the moment it was navigated to. This is awkward but hits a guard. Returning menu from new route."), *GetName());
				return GetMenuByNavigationRoute(GetActiveRoute());
			}

			NewMenu->RestoreFocus();

			// Notify blueprints.
			AfterNavigateToRoute(OldRoute, GetActiveRoute());
			return NewMenu;
		}
		else {
			// If no menu was found on this route and the route was not NAME_None, we have an invalid route.
			UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("%s: No menu was found on this route. Aborting."), *GetName());
			return nullptr;
		}
	}
}

bool UMenuWidget::RegisterMenu(UMenuWidget* InMenuWidget, const FName& InRoute) {
	if (!IsValid(InMenuWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("can't register an invalid / null menu. Route:"), *InRoute.ToString());
		return false;
	}
	if (InRoute.IsNone()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Attempted to register a widget to an empty route."));
		return false;
	}
	if (GetMenuByNavigationRoute(InRoute)) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Can't register to this route, something has been registered to it already. Route: %s"), *InRoute.ToString());
		return false;
	}
	if (InMenuWidget->GetRegisteredParentMenu()) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Can't register a menu already registered to this or another menu. Route: %s"), *InRoute.ToString());
		return false;
	}
	if (!USlateUtils::FindAncestorWidget(InMenuWidget, this)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can't register a menu which is not a child of this menu. Route: %s"), *InRoute.ToString());
		return false;
	}

	InMenuWidget->OnRequestUINavigation.AddDynamic(this, &UMenuWidget::ActOnMenuRequestedUINavigation);
	InMenuWidget->SetRegisteredParentMenu(this);
	Menus.Add(InRoute, InMenuWidget);
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Registered menu: %s, on route: %s"), *InMenuWidget->GetName(), *InRoute.ToString());
	return true;
}

bool UMenuWidget::UnRegisterMenu(const FName& InRoute) {
	UMenuWidget* ExistingMenu = GetMenuByNavigationRoute(InRoute);
	if (IsValid(ExistingMenu)) {
		ExistingMenu->SetRegisteredParentMenu(nullptr);
		ExistingMenu->OnRequestUINavigation.RemoveDynamic(this, &UMenuWidget::ActOnMenuRequestedUINavigation);
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Unregistered menu: %s, on route: %s"), *ExistingMenu->GetName(), *InRoute.ToString());
		
		Menus.Remove(InRoute);
		return true;
	}
	return false;
}

// Delegates | Navigation

void UMenuWidget::ActOnNavigationButtonRequestedNavigation(const FName& InRoute) {
	if (InRoute.IsNone()) {
		RequestUINavigation(InRoute);
	}
	else {
		NavigateToRoute(InRoute);	
	}
}

void UMenuWidget::ActOnMenuRequestedUINavigation(const FName& InRoute) {
	NavigateToRoute(InRoute);
}

// Delegates | Input

void UMenuWidget::ActOnNavBack() {
    RequestUINavigation(NAME_None);
}