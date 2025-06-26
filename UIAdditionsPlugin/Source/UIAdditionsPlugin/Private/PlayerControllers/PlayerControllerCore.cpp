/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "PlayerControllerCore.h"
#include "HUDCorePlayerControllerComponent.h"
#include "Engine/EngineBaseTypes.h"
#include "LogUIAdditionsPlugin.h"
#include "GameFramework/PlayerController.h"
#include "PlayerControllerInputModes.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerInput.h"


// Setup

APlayerControllerCore::APlayerControllerCore() {
	HUDCorePlayerControllerComponent = CreateDefaultSubobject<UHUDCorePlayerControllerComponent>(TEXT("HUDCorePlayerControllerComponent"));
}

void APlayerControllerCore::ReceivedPlayer() {
	Super::ReceivedPlayer();
	OnReceivedPlayer.Broadcast();
}
