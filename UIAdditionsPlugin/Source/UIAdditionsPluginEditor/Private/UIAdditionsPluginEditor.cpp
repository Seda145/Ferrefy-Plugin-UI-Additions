/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "UIAdditionsPluginEditor.h"
#include "Modules/ModuleManager.h"
#include "UIAdditionsPluginEditorInstaller.h"


void FUIAdditionsPluginEditorModule::StartupModule() {
	FUIAdditionsPluginEditorInstaller().RunAutomatedInstaller();
}


IMPLEMENT_MODULE(FUIAdditionsPluginEditorModule, UIAdditionsPluginEditor)