/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "SubHUDWidget.h"
#include "SlateUtils.h"


// Setup

USubHUDWidget::USubHUDWidget(const FObjectInitializer& InObjectInitializer)
	: Super(InObjectInitializer) {

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	// There is nothing to register to because this should be at the top level of the widget hierarchy.
	SetAutoRegisterToParentMenu(false);
}

void USubHUDWidget::NativeOnInitialized() {
    Super::NativeOnInitialized();

	// Stop listening to the default action MenuWidget implements. 
	// The Sub HUD is meant to be implemented at the "top" managed by the AHUD, so there is nothing to go back to.
	StopListenForRoutedInputAction(USlateUtils::InputActionNavBack);
}

// Navigation

bool USubHUDWidget::RegisterMenu(UMenuWidget* InMenuWidget, const FName& InRoute) {
	bool bSuccess = Super::RegisterMenu(InMenuWidget, InRoute);
	
	if (bSuccess) {
		// Add the binding. We want to know when a registered menu changes visibility so we can broadcast up to the HUD.
		UMenuWidget* MW = GetMenuByNavigationRoute(InRoute);
		if (IsValid(MW)) {
			MW->OnMenuVisibilityChanged.AddDynamic(this, &USubHUDWidget::ActOnMenuVisibilityChanged);
			ActOnMenuVisibilityChanged(MW, USlateUtils::IsVisible(MW->GetVisibility()));
		}
	}

	return bSuccess;
}

bool USubHUDWidget::UnRegisterMenu(const FName& InRoute) {
	bool bSuccess = Super::UnRegisterMenu(InRoute);
	
	if (bSuccess) {
		// Remove the binding.
		UMenuWidget* MW = GetMenuByNavigationRoute(InRoute);
		if (IsValid(MW)) {
			MW->OnMenuVisibilityChanged.RemoveDynamic(this, &USubHUDWidget::ActOnMenuVisibilityChanged);
			// One last broadcast up to the HUD.
			ActOnMenuVisibilityChanged(MW, false);
		}
	}

	return bSuccess;
}

// Delegates

/* Respond to visibility change in a registered UMenuWidget. */
void USubHUDWidget::ActOnMenuVisibilityChanged(UMenuWidget* InMenuWidget, bool bInIsVisible) {
	// Let the HUD know a widget changed visibility. When one is removed, it should broadcast as "invisible" one last time.
	// In the current implementation of the HUD it is fine to just broadcast a change in our own visibility + the change in visibility of a directly registered submenu through the same delegate. The HUD knows enough at that point.
	OnMenuVisibilityChanged.Broadcast(InMenuWidget, bInIsVisible);
}
