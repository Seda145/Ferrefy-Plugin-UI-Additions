/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"

#include "SubHUDWidget.generated.h"


/*
* Managed by the HUD. A "Sub" HUD is the modern widget version of a HUD meant to take over visual tasks.
* The parent class is a menu widget, used to navigate through descendant menus.
*/
UCLASS(Abstract, BlueprintType, Blueprintable)
class UIADDITIONSPLUGIN_API USubHUDWidget : public UMenuWidget {
	GENERATED_BODY()

private:

protected:

public:

private:

protected:

	// Setup
	
	virtual void NativeOnInitialized() override;
	
	// Delegates

	/* Respond to visibility change in a registered UMenuWidget. */
	UFUNCTION()
		void ActOnMenuVisibilityChanged(UMenuWidget* InMenuWidget, bool bInIsVisible);

public:

	// Setup

	USubHUDWidget(const FObjectInitializer& InObjectInitializer);
	
	// Navigation
	
	virtual bool RegisterMenu(UMenuWidget* InMenuWidget, const FName& InRoute) override;

	virtual bool UnRegisterMenu(const FName& InRoute) override;

};