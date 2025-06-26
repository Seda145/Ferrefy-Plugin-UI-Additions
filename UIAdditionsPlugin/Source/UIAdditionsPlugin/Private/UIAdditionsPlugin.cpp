/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "UIAdditionsPlugin.h"
#include "Modules/ModuleManager.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "LogUIAdditionsPlugin.h"
#include "UIAdditionsPluginInstaller.h"
#include "Templates/SharedPointer.h"
#include "Framework/Application/SlateApplication.h"


void FUIAdditionsPluginModule::StartupModule() {
	FUIAdditionsPluginInstaller Installer = FUIAdditionsPluginInstaller();
	Installer.RunAutomatedInstaller();

	if (FSlateApplication::IsInitialized()) {
		// Register input preprocessor
		DetectCurrentInputDevicePreProcessor = MakeShared<FDetectCurrentInputDevicePreProcessor>();
		FSlateApplication::Get().RegisterInputPreProcessor(DetectCurrentInputDevicePreProcessor);
	}
	//else {
	//	UE_LOG(LogUIAdditionsPlugin, Error, TEXT("SlateApplication is not initialized."));
	//}
}

void FUIAdditionsPluginModule::ShutdownModule() {
	// Unregister input preprocessor
	// TODO doesn't seem Required? At this point FSlateApplication is not initialized (anymore).
	//if (FSlateApplication::IsInitialized()) {
	//	if (DetectCurrentInputDevicePreProcessor.IsValid()) {
	//		FSlateApplication::Get().UnregisterInputPreProcessor(DetectCurrentInputDevicePreProcessor);
	//	}
	//}
	//else {
	//	UE_LOG(LogUIAdditionsPlugin, Error, TEXT("SlateApplication is not initialized."));
	//}
}

const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& FUIAdditionsPluginModule::GetDetectCurrentInputDevicePreProcessor() const {
	return DetectCurrentInputDevicePreProcessor;
}


IMPLEMENT_MODULE(FUIAdditionsPluginModule, UIAdditionsPlugin)