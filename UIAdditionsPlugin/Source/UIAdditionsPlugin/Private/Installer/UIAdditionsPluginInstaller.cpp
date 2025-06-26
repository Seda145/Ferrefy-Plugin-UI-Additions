/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "UIAdditionsPluginInstaller.h"
#include "SlateUtils.h"
#include "CorePlayerInput.h"
#include "LogUIAdditionsPlugin.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/PlayerInput.h"


// Setup

void FUIAdditionsPluginInstaller::RunAutomatedInstaller() const {
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Running automated installer for UIAdditionsPlugin."));
	
	// We automatically install any missing input mapping to Slate's standard nav actions and our own extended actions.
	USlateUtils::ValidateStandardSlateNavInputMappings();
	USlateUtils::ValidateExtendedSlateNavInputMappings();
	USlateUtils::ValidateCustomInputAxisMappings();
}