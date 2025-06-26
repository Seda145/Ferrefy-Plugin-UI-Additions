/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "UIAdditionsCharacter.h"
#include "WorldCursorModifierComponent.h"
#include "HUDCorePlayerControllerComponent.h"
#include "CorePlayerInput.h"
#include "SlateUtils.h"
#include "GameFramework/PlayerController.h"
#include "Components/InputComponent.h"


// Setup

AUIAdditionsCharacter::AUIAdditionsCharacter() {
	// Create the components.
	WorldCursorModifierComponent = CreateDefaultSubobject<UWorldCursorModifierComponent>(TEXT("WorldCursorModifierComponent"));
}

// Components

UWorldCursorModifierComponent* AUIAdditionsCharacter::GetWorldCursorModifierComponent() const {
	return WorldCursorModifierComponent;
}

// Input

void AUIAdditionsCharacter::SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent) {
	Super::SetupPlayerInputComponent(InPlayerInputComponent);

	check(IsValid(InPlayerInputComponent));

	// Axis mappings
	// I prefer avoiding 1 binding for two directions (left / right) and more complex bindings such as 2D axis.
	// Reason is that they are impossible to keybind in-game by end users (using Slate key selector), and difficult to manage separately on such menus.
	// Take mouse movement for example.
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisForward, this, &AUIAdditionsCharacter::ActOnInputAxisForward);
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisBackward, this, &AUIAdditionsCharacter::ActOnInputAxisForward);
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisRight, this, &AUIAdditionsCharacter::ActOnInputAxisRight);
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisLeft, this, &AUIAdditionsCharacter::ActOnInputAxisRight);
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisYawPos, this, &AUIAdditionsCharacter::ActOnInputAxisYaw);
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisYawNeg, this, &AUIAdditionsCharacter::ActOnInputAxisYaw);
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisPitchPos, this, &AUIAdditionsCharacter::ActOnInputAxisPitch);
	InPlayerInputComponent->BindAxis(USlateUtils::InputAxisPitchNeg, this, &AUIAdditionsCharacter::ActOnInputAxisPitch);
}

// Delegates | Input | Axis

void AUIAdditionsCharacter::ActOnInputAxisForward(float InAxisValue) {
	float ModifiedValue = InAxisValue;
	APlayerController* PC = Cast<APlayerController>(GetController());
	UCorePlayerInput* Pi = IsValid(PC) ? Cast<UCorePlayerInput>(PC->PlayerInput) : nullptr;
	if (IsValid(Pi)) {
		// Here it is assumed the setting registerd the same multipliers both forward and backward, so just use one.
		Pi->GetMultipliedInputAxisValue(Pi->GetCurrentInputDevice(), USlateUtils::InputAxisForward, ModifiedValue);
	}


	AddMovementInput(RootComponent->GetForwardVector(), InAxisValue, false);
}

void AUIAdditionsCharacter::ActOnInputAxisRight(float InAxisValue) {
	float ModifiedValue = InAxisValue;
	APlayerController* PC = Cast<APlayerController>(GetController());
	UCorePlayerInput* Pi = IsValid(PC) ? Cast<UCorePlayerInput>(PC->PlayerInput) : nullptr;
	if (IsValid(Pi)) {
		// Here it is assumed the setting registerd the same multipliers both left and right, so just use one.
		Pi->GetMultipliedInputAxisValue(Pi->GetCurrentInputDevice(), USlateUtils::InputAxisRight, ModifiedValue);
	}

	AddMovementInput(RootComponent->GetRightVector(), InAxisValue, false);
}

void AUIAdditionsCharacter::ActOnInputAxisYaw(float InAxisValue) {
	if (IsValid(GetWorldCursorModifierComponent()) && !GetWorldCursorModifierComponent()->GetDesiresCenteredWorldCursor()) {
		// Do not move the camera when the cursor is not desired to be centered (which would mean we are doing something with the cursor.).
		return;
	}
	const APlayerController* PC = Cast<APlayerController>(GetController());
	const UHUDCorePlayerControllerComponent* HUDCorePCComponent = IsValid(PC) 
		? PC->FindComponentByClass<UHUDCorePlayerControllerComponent>() 
		: nullptr;
	if (HUDCorePCComponent != nullptr && HUDCorePCComponent->GetInputMode() != E_PlayerControllerInputModes::Game) {
		// Do not move the camera in UI / UI Game mode, because we are using a cursor.
		return;
	}

	float ModifiedValue = InAxisValue;
	const UCorePlayerInput* Pi = IsValid(PC) ? Cast<UCorePlayerInput>(PC->PlayerInput) : nullptr;
	if (IsValid(Pi)) {
		// Here it is assumed the setting registerd the same multipliers both pos and neg, so just use one.
		Pi->GetMultipliedInputAxisValue(Pi->GetCurrentInputDevice(), USlateUtils::InputAxisYawPos, ModifiedValue);
	}

	AddControllerYawInput(ModifiedValue);
}

void AUIAdditionsCharacter::ActOnInputAxisPitch(float InAxisValue) {
	if (IsValid(GetWorldCursorModifierComponent()) && !GetWorldCursorModifierComponent()->GetDesiresCenteredWorldCursor()) {
		// Do not move the camera when the cursor is not desired to be centered (which would mean we are doing something with the cursor.).
		return;
	}
	const APlayerController* PC = Cast<APlayerController>(GetController());
	const UHUDCorePlayerControllerComponent* HUDCorePCComponent = IsValid(PC) 
		? PC->FindComponentByClass<UHUDCorePlayerControllerComponent>() 
		: nullptr;
	if (HUDCorePCComponent != nullptr && HUDCorePCComponent->GetInputMode() != E_PlayerControllerInputModes::Game) {
		// Do not move the camera in UI / UI Game mode, because we are using a cursor.
		return;
	}

	float ModifiedValue = InAxisValue;
	const UCorePlayerInput* Pi = IsValid(PC) ? Cast<UCorePlayerInput>(PC->PlayerInput) : nullptr;
	if (IsValid(Pi)) {
		// Here it is assumed the setting registerd the same multipliers both pos and neg, so just use one.
		Pi->GetMultipliedInputAxisValue(Pi->GetCurrentInputDevice(), USlateUtils::InputAxisPitchPos, ModifiedValue);
	}

	AddControllerPitchInput(ModifiedValue);
}