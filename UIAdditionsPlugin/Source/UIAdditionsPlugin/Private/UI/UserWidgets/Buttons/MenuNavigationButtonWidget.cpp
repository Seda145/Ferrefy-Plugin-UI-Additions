/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "MenuNavigationButtonWidget.h"
#include "Components/PanelWidget.h"
#include "MenuWidget.h"
#include "LogUIAdditionsPlugin.h"
#include "SlateUtils.h"
#include "CentralButton.h"


// Setup

void UMenuNavigationButtonWidget::NativeConstruct() {
	Super::NativeConstruct();

	if (GetAutoRegisterToParentMenu()) {
		RegisterToParentMenu();
	}
}

void UMenuNavigationButtonWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	if (GetAutoRegisterToParentMenu()) {
		RegisterToParentMenu();
	}
}

// Navigation

bool UMenuNavigationButtonWidget::GetAutoRegisterToParentMenu() const {
	return bAutoRegisterToParentMenu;
}

void UMenuNavigationButtonWidget::SetAutoRegisterToParentMenu(bool bInAutoRegisterToParentMenu) {
	bAutoRegisterToParentMenu = bInAutoRegisterToParentMenu;
}

void UMenuNavigationButtonWidget::RegisterToParentMenu() {
	// When a widget is added to the viewport, or to a parent widget, we want to update our registration to a parent menu.
	// Multiple registrations are allowed and not managed or validated on the button level.
	UMenuWidget* NewParentMenuWidget = USlateUtils::FindAncestorWidgetByClass<UMenuWidget>(this);
	if (IsValid(NewParentMenuWidget)) {
		NewParentMenuWidget->RegisterNavigationButton(this);
	}
}

FName UMenuNavigationButtonWidget::GetNavigationRoute() const {
	return NavigationRoute;
}

void UMenuNavigationButtonWidget::SetNavigationRoute(const FName& InRoute) {
	NavigationRoute = InRoute;
}

// Delegates

void UMenuNavigationButtonWidget::ActOnButtonReleased() {
	Super::ActOnButtonReleased();
	
	// Assumption we want to do this when released.
	// For action events this generally responds more nicely because a released button does not re-execute like when holding a key.
	// Note that this is also perfect if we want to modify the style of a button post interaction (showing it as pressed while having a related menu open etc.).
	OnRequestUINavigation.Broadcast(GetNavigationRoute());	
}