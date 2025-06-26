/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "CorePlayerInput.h"
#include "Framework/Application/SlateApplication.h"
#include "Templates/SharedPointer.h"
#include "Framework/Application/NavigationConfig.h"
#include "CustomSlateNavigationConfig.h"
#include "LogUIAdditionsPlugin.h"
#include "Modules/ModuleManager.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "UIAdditionsPlugin.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateUser.h"
#include "Engine/World.h"


// Setup

void UCorePlayerInput::PostInitProperties() {
	Super::PostInitProperties();

	// Runs once when the editor loads and once when the game runs. Check if a world exists, then we should be in game.
	UWorld* World = GetWorld();
	if (!IsValid(World)) {
		return;
	}

	RegisterSlateNavigationConfig();

	World->OnWorldBeginPlay.AddUObject(this, &UCorePlayerInput::ActOnWorldBeginPlay);
	if (World->HasBegunPlay()) {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Detected world has begun play."));
		ActOnWorldBeginPlay();
	}
}

// Input

void UCorePlayerInput::RegisterSlateNavigationConfig() {
	// Install the custom Slate navigation config, which will read and install the standard Slate mappings.
	// Set the global nav config. Note that this sets the global navconfig but that there is also a GetUserNavigationConfig()?
	if (FSlateApplication::IsInitialized()) {
		UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Installing FCustomSlateNavigationConfig to manage custom Slate navigation keys."));
		// Install slate navigation / action keys as configured from project settings (UInputSettings).
		// Slate navigation keys should be blacklisted for runtime keybinding (they should not be modified past this point).
		FSlateApplication::Get().SetNavigationConfig(TSharedRef<FCustomSlateNavigationConfig>(new FCustomSlateNavigationConfig()));
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Could not install custom slate navigation. Slate is not initialized."));
	}
}

EInputDevices UCorePlayerInput::GetCurrentInputDevice() const {
	return CurrentInputDevice;
}

const TMap<EInputDevices, TMap<FName, E_InputKeyModes>>& UCorePlayerInput::GetInputActionKeyModes() const {
	return InputActionKeyModes;
}

void UCorePlayerInput::AddInputActionKeyMode(EInputDevices InInputDevice, const FName& InInputName, E_InputKeyModes InInputKeyMode) {
	TMap<FName, E_InputKeyModes>& Regist = InputActionKeyModes.FindOrAdd(InInputDevice);
	Regist.Add(InInputName, InInputKeyMode);
}

E_InputKeyModes UCorePlayerInput::GetInputActionKeyMode(EInputDevices InInputDevice, const FName& InInputName) const {
	const TMap<FName, E_InputKeyModes>* Regist = GetInputActionKeyModes().Find(InInputDevice);
	const E_InputKeyModes* ModePtr = Regist != nullptr ? Regist->Find(InInputName) : nullptr;
	if (ModePtr != nullptr) {
		return *ModePtr;
	}
	// Default to a value, up to preference but toggle could be more common.
	return E_InputKeyModes::Toggle;
}

void UCorePlayerInput::RemoveInputActionKeyMode(EInputDevices InInputDevice, const FName& InInputName) {
	TMap<FName, E_InputKeyModes>* Regist = InputActionKeyModes.Find(InInputDevice);
	if (Regist != nullptr) {
		Regist->Remove(InInputName);
	}
}

const TMap<EInputDevices, TMap<FName, TMap<FName, float>>>& UCorePlayerInput::GetInputAxisMultipliers() const {
	return InputAxisMultipliers;
}

void UCorePlayerInput::AddInputAxisMultiplier(EInputDevices InInputDevice, const FName& InInputName, const FName& InId, float InMultiplier) {
	TMap<FName, TMap<FName, float>>& Regist = InputAxisMultipliers.FindOrAdd(InInputDevice);
	TMap<FName, float>& RegistX = Regist.FindOrAdd(InInputName);
	RegistX.Add(InId, InMultiplier);
}

void UCorePlayerInput::GetMultipliedInputAxisValue(EInputDevices InInputDevice, const FName& InInputName, float& RefValue) const {
	const TMap<FName, TMap<FName, float>>* RegistPtr = GetInputAxisMultipliers().Find(InInputDevice);
	const TMap<FName, float>* RegistX = RegistPtr != nullptr ? RegistPtr->Find(InInputName) : nullptr;
	if (RegistX != nullptr) {
		for (TPair<FName, float> MultX : *RegistX) {
			RefValue *= MultX.Value;
		}
	}
}

void UCorePlayerInput::RemoveInputAxisMultiplier(EInputDevices InInputDevice, const FName& InInputName, const FName& InId) {
	TMap<FName, TMap<FName, float>>* RegistPtr = InputAxisMultipliers.Find(InInputDevice);
	TMap<FName, float>* RegistX = RegistPtr != nullptr ? RegistPtr->Find(InInputName) : nullptr;
	if (RegistX != nullptr) {
		RegistX->Remove(InId);
	}
}

// Delegates

void UCorePlayerInput::ActOnWorldBeginPlay() {
	// Added some super verbose log because World->OnWorldBeginPlay did not work on a subsystem earlier on standalone.
	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Responding to WorldBeginPlay."));
	const ULocalPlayer* PL = IsValid(GetOuterAPlayerController()) ? GetOuterAPlayerController()->GetLocalPlayer() : nullptr;
	if (!IsValid(PL)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("A valid local player is expected for this player input at this point. Did you remove a player on BeginPlay? This could lead to this error."));
		check(false);
	}

	// Bind to when the current input device changes
	const FUIAdditionsPluginModule& UIAdditionsPluginModule = FModuleManager::GetModuleChecked<FUIAdditionsPluginModule>(TEXT("UIAdditionsPlugin"));
	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& Processor = UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor();
	if (Processor.IsValid()) {
		Processor->OnInputDeviceChanged.AddUObject(this, &UCorePlayerInput::ActOnInputDeviceChanged);
		const TSharedPtr<const FSlateUser> SlateUserX = PL->GetSlateUser();
		if (SlateUserX.IsValid()) {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Slate User is detected and ActOnInputDeviceChanged is called right away."));
			ActOnInputDeviceChanged(Processor->GetCurrentInputDevice(SlateUserX->GetUserIndex()), SlateUserX->GetUserIndex());
		}
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor() not valid, can't respond to a change in input device."));
	}
}

void UCorePlayerInput::ActOnInputDeviceChanged(EInputDevices InNewInputDevice, int32 InSlateUserIndex) {
	const APlayerController* PC = GetOuterAPlayerController();
	const ULocalPlayer* PL = IsValid(PC) ? PC->GetLocalPlayer() : nullptr;
	const TSharedPtr<const FSlateUser> SlateUserX = IsValid(PL) ? PL->GetSlateUser() : nullptr;
	// Check if the change is relevant for this component.
	if (SlateUserX.IsValid() && SlateUserX->GetUserIndex() == InSlateUserIndex) {
		CurrentInputDevice = InNewInputDevice;
		// For blueprint users. C++ users can just get the FDetectCurrentInputDevicePreProcessor.
		OnInputDeviceChangedBP.Broadcast(USlateUtils::ConvertEInputDevicesToBP(GetCurrentInputDevice()), SlateUserX->GetUserIndex());
	}
}