/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerControllerInputModes.h"

#include "HUDCorePlayerControllerComponent.generated.h"


/*
* Provides a simple way to manage input modes.
* Used as an extension to the PlayerController to easily cooperate with AHudCore.
* Implements a workaround to a focus loss issue in APlayerController.
*/
UCLASS(ClassGroup = CommonLogic, editinlinenew, meta = (BlueprintSpawnableComponent))
class UIADDITIONSPLUGIN_API UHUDCorePlayerControllerComponent : public UActorComponent {
	GENERATED_BODY()

private:

	// Input Modes

	/* The input mode as set on this component using ActivateInputMode(). Does not question the PlayerController for its input mode. */
	UPROPERTY()
		E_PlayerControllerInputModes InputMode = E_PlayerControllerInputModes::Game;

protected:

public:

private:

protected:

public:

	// Input Modes

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "InputModes")
		void ActivateInputMode(E_PlayerControllerInputModes InInputMode);

	virtual void ActivateInputMode_Implementation(E_PlayerControllerInputModes InInputMode);

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "InputModes")
		E_PlayerControllerInputModes GetInputMode() const;

};
