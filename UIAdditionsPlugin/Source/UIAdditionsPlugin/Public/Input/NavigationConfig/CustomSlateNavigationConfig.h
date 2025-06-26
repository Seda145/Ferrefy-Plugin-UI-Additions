/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/NavigationConfig.h"
#include "Types/SlateEnums.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerInput.h"

class UGameInstance;


class UIADDITIONSPLUGIN_API FCustomSlateNavigationConfig : public FNavigationConfig {

private:

protected:

public:

	TMap<FKey, EUINavigationAction> KeyActionEventRules;

private:

	// Input

	// Reads Slate Nav* input mappings from UInputSettings and writes them to the Slate FNavigationConfig.
	bool InstallNavInputMappingsToSlate();

protected:

	// Get the navigation direction from the analog internally.
	virtual EUINavigation GetNavigationDirectionFromAnalogInternal(const FAnalogInputEvent& InAnalogEvent) override;

public:

	// Setup

	FCustomSlateNavigationConfig();

	virtual void OnRegister();

	// Input

	// Get the navigation direction from a given key event.
	virtual EUINavigation GetNavigationDirectionFromKey(const FKeyEvent& InKeyEvent) const override;

	// Get the navigation direction from a given analog event.
	virtual EUINavigation GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent) override;

	UE_DEPRECATED(4.24, "GetNavigationActionForKey doesn't handle multiple users properly, use GetNavigationActionFromKey instead")
		virtual EUINavigationAction GetNavigationActionForKey(const FKey& InKey) const override;

};