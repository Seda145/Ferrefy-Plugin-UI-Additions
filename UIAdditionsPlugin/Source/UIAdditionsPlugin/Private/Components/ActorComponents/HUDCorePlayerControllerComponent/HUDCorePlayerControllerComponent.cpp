/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "HUDCorePlayerControllerComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "LogUIAdditionsPlugin.h"
#include "GameFramework/PlayerController.h"


// Input Modes

void UHUDCorePlayerControllerComponent::ActivateInputMode_Implementation(E_PlayerControllerInputModes InInputMode) {
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!IsValid(PC)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can't activate the input mode, because this component's owner is not a valid APlayerController."));
		return;
	}

	switch (InInputMode) {
	case (E_PlayerControllerInputModes::Game): {
		//UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
		FInputModeGameOnly InputModeStruct;
		PC->SetInputMode(InputModeStruct);
		PC->SetShowMouseCursor(false);
		UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Activated input mode: Game, SetShowMouseCursor: false"));
		break;
	}
	case (E_PlayerControllerInputModes::GameAndUI): {
		//UWidgetBlueprintLibrary::SetInputMode_GameAndUIEx(this, nullptr, EMouseLockMode::LockAlways, false);
		// Prevent engine behavior that prevents widgets from responding to single click events: https://answers.unrealengine.com/questions/420047/an-lmb-event-is-only-triggered-with-a-double-click.html
		//UGameplayStatics::SetViewportMouseCaptureMode(this, EMouseCaptureMode::NoCapture);

		/** Workaround:
		* bShowMouseCursor does not, like the name suggest, just alter 'cursor visibility'. It influences input behavior and misbehaves when set true combined with "Input Mode Game / Game + UI" (as configured in UWidgetBlueprintLibrary).
		* The SceneViewPort releases focus if you release a mouse button in mode "GameAndUI" when bShowMouseCursor == true.This results in loss of mouse input axis data or loss of controls if not countered.Why that is implemented that way is unclear. As a workaround calling ActivateInputModeGameAndUI after mouse button release should be enough to counter that problem.
		*/
		FInputModeGameAndUI InputModeStruct;
		InputModeStruct.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		InputModeStruct.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputModeStruct);
		PC->SetShowMouseCursor(true);
		UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Activated input mode: GameAndUI, SetShowMouseCursor: true"));
		break;
	}
	case (E_PlayerControllerInputModes::UI): {
		//UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this, nullptr, EMouseLockMode::LockAlways);
		FInputModeUIOnly InputModeStruct;
		InputModeStruct.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
		PC->SetInputMode(InputModeStruct);
		PC->SetShowMouseCursor(true);
		UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Activated input mode: UI, SetShowMouseCursor: true"));
		break;
	}
	}

	InputMode = InInputMode;
}

E_PlayerControllerInputModes UHUDCorePlayerControllerComponent::GetInputMode() const {
	return InputMode;
}

