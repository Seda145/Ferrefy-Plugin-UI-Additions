/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "CustomSlateNavigationConfig.h"
#include "GameFramework/InputSettings.h"
#include "LogUIAdditionsPlugin.h"
#include "SlateUtils.h"


// Setup

FCustomSlateNavigationConfig::FCustomSlateNavigationConfig() {
	bKeyNavigation = true;
	// Tab and analog would conflict with our setup of InputAction mappings. Everything is customized as KeyNavigation.
	bTabNavigation = false;
	bAnalogNavigation = false;
	AnalogHorizontalKey = FKey(NAME_None);
	AnalogVerticalKey = FKey(NAME_None);
	// To be filled from project settings instead of default slate:
	KeyEventRules.Empty();
	KeyActionEventRules.Empty();
}

void FCustomSlateNavigationConfig::OnRegister() {
	FNavigationConfig::OnRegister();

	if (!InstallNavInputMappingsToSlate()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("There was an error while installing nav inputs to Slate."));
	}
}

// Input

bool FCustomSlateNavigationConfig::InstallNavInputMappingsToSlate() {
	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	if (!IsValid(InputSettings)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InputSettings is not valid."));
		return false;
	}

	// Lambda to fill the Slate rule arrays.
	auto AddMappingToKeyRules = [this, &InputSettings](FName ActionMappingName, EUINavigation UINavigation, EUINavigationAction UINavigationAction) {
		TArray<FInputActionKeyMapping> OutArray;
		InputSettings->GetActionMappingByName(ActionMappingName, OutArray);

		for (FInputActionKeyMapping InputActionKeyMappingX : OutArray) {
			if (UINavigation != EUINavigation::Invalid) {
				if (!KeyEventRules.Contains(InputActionKeyMappingX.Key)) {
					KeyEventRules.Emplace(InputActionKeyMappingX.Key, UINavigation);
				}
				else {
					UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Input mapping '%s' overlaps a key with another Nav binding, This is not supported and will break slate navigation. In case you want to overlap Previous/Next navigation on a key like Tab: only assign the Next key. Shift + Next will then act as Previous."), *ActionMappingName.ToString());
					return false;
				}
			}
			else if (UINavigationAction != EUINavigationAction::Invalid) {
				if (!KeyActionEventRules.Contains(InputActionKeyMappingX.Key)) {
					KeyActionEventRules.Emplace(InputActionKeyMappingX.Key, UINavigationAction);
				}
				else {
					UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Input mapping '%s' overlaps a key with another Nav binding, This is not supported and will break slate navigation. In case you want to overlap Previous/Next navigation on a key like Tab: only assign the Next key. Shift + Next will then act as Previous."), *ActionMappingName.ToString());
					return false;
				}
			}
		}

		return true;
	};

	KeyEventRules.Empty();
	KeyActionEventRules.Empty();
	UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Installing nav inputs to Slate."));
	return (AddMappingToKeyRules(USlateUtils::InputActionNavUp, EUINavigation::Up, EUINavigationAction::Invalid)
		&& AddMappingToKeyRules(USlateUtils::InputActionNavDown, EUINavigation::Down, EUINavigationAction::Invalid)
		&& AddMappingToKeyRules(USlateUtils::InputActionNavLeft, EUINavigation::Left, EUINavigationAction::Invalid)
		&& AddMappingToKeyRules(USlateUtils::InputActionNavRight, EUINavigation::Right, EUINavigationAction::Invalid)
		&& AddMappingToKeyRules(USlateUtils::InputActionNavPrevious, EUINavigation::Previous, EUINavigationAction::Invalid)
		&& AddMappingToKeyRules(USlateUtils::InputActionNavNext, EUINavigation::Next, EUINavigationAction::Invalid)
		&& AddMappingToKeyRules(USlateUtils::InputActionNavSelect, EUINavigation::Invalid, EUINavigationAction::Accept)
		&& AddMappingToKeyRules(USlateUtils::InputActionNavBack, EUINavigation::Invalid, EUINavigationAction::Back)
	);
}

EUINavigation FCustomSlateNavigationConfig::GetNavigationDirectionFromKey(const FKeyEvent& InKeyEvent) const {
	if (const EUINavigation* UINavigationPtr = KeyEventRules.Find(InKeyEvent.GetKey())) {
		if (bKeyNavigation) {
			// Since keys can only be assigned once to KeyEventRules, assume that Shift + EUINavigation::Next means previous.
			if (*UINavigationPtr == EUINavigation::Next) {
				const bool bAllowEatingKeyEvents = !InKeyEvent.IsControlDown() && !InKeyEvent.IsAltDown() && !InKeyEvent.IsCommandDown();
				// Since KeyEventRules don't record InputChords, simulate keyboard previous navigation of shift + previous key?
				// This would be useful for default keyboard navigation of Shift + Tab / Tab.
				if (bAllowEatingKeyEvents) {
					return (InKeyEvent.IsShiftDown()) ? EUINavigation::Previous : EUINavigation::Next;
				}
			}
			else {
				return *UINavigationPtr;
			}
		}
	}
	return EUINavigation::Invalid;
}

EUINavigation FCustomSlateNavigationConfig::GetNavigationDirectionFromAnalog(const FAnalogInputEvent& InAnalogEvent) {
	return EUINavigation::Invalid;
}

EUINavigation FCustomSlateNavigationConfig::GetNavigationDirectionFromAnalogInternal(const FAnalogInputEvent& InAnalogEvent) {
	return EUINavigation::Invalid;
}

EUINavigationAction FCustomSlateNavigationConfig::GetNavigationActionForKey(const FKey& InKey) const {
	const EUINavigationAction* UINavigationActionPtr = KeyActionEventRules.Find(InKey);
	if (UINavigationActionPtr) {
		if (*UINavigationActionPtr == EUINavigationAction::Accept
			|| *UINavigationActionPtr == EUINavigationAction::Back
			) {
			return *UINavigationActionPtr;
		}
	}

	return EUINavigationAction::Invalid;
}

