/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "SlateUtils.h"
#include "GameFramework/PlayerInput.h"
#include "InputKeyMode.h"

#include "CorePlayerInput.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInputDeviceChangedBP, E_BPInputDevices, InInputDevice, int32, InSlateUserIndex);


/**
* Core Player Input is a Player Input class, which registers Custom Slate Navigation Config to Slate (UI). 
* This makes the UI navigation keys configurable from the project settings. 
* It also tracks desired input modifiers (axis multipliers, input key modes (hold / toggle)) 
* and tracks the currently used input device. 
* 
* Tracked values are intended to be implemented elsewhere (Pawn / Controller) where required.
* In-game input modifiers can be added and removed for various reasons. 
* A setting inverting the pitch input mapping on a gamepad, can do exactly that by registering an input axis multiplier for it.
* 
* The benefit of having multiple input modifiers, is that the same system is merged with gameplay functionality. 
* For example, the pitch inversion setting adds up to other game features:
* - (gamepad) Base sensitivity + (gamepad) inversion setting + vehicle sensitivity + vehicle damage + vehicle inversion
* - (Mouse) Base sensitivity + (mouse) inversion setting + rifle weight + is aiming? + is shooting?
*/
UCLASS(BlueprintType)
class UIADDITIONSPLUGIN_API UCorePlayerInput : public UPlayerInput {
	GENERATED_BODY()

private:

	// Input 

	UPROPERTY(Transient)
		EInputDevices CurrentInputDevice = EInputDevices::None;

	TMap<EInputDevices, TMap<FName, E_InputKeyModes>> InputActionKeyModes;

	TMap<EInputDevices, TMap<FName, TMap<FName, float>>> InputAxisMultipliers;

protected:

public:

	// Delegates

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnInputDeviceChangedBP OnInputDeviceChangedBP;

private:

	// Delegates

	UFUNCTION()
		void ActOnInputDeviceChanged(EInputDevices InNewInputDevice, int32 InSlateUserIndex);

protected:

	// Input
	
	void RegisterSlateNavigationConfig();

	// Delegates

	UFUNCTION()
		virtual void ActOnWorldBeginPlay();

public:

	// Setup

	virtual void PostInitProperties() override;

	// Input

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
		EInputDevices GetCurrentInputDevice() const;

    /* Returns InputActionKeyModes, registered as InputDevice > InputName > InputKeyMode. */
    const TMap<EInputDevices, TMap<FName, E_InputKeyModes>>& GetInputActionKeyModes() const;

    /* Adds an item to InputActionKeyModes. */
	UFUNCTION(BlueprintCallable, Category = "Input")
		void AddInputActionKeyMode(EInputDevices InInputDevice, const FName& InInputName, E_InputKeyModes InInputKeyMode);

    /* Finds an input action key mode, or returns "toggle" mode if not found. Implementation of return value is intended elsewhere (Pawn / Controller). */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
		E_InputKeyModes GetInputActionKeyMode(EInputDevices InInputDevice, const FName& InInputName) const;

    /* Removes an item from InputActionKeyModes. */
	UFUNCTION(BlueprintCallable, Category = "Input")
		void RemoveInputActionKeyMode(EInputDevices InInputDevice, const FName& InInputName);

    /* Returns InputAxisMultipliers, registered as InputDevice > InputName > Id > Multiplier. */
    const TMap<EInputDevices, TMap<FName, TMap<FName, float>>>& GetInputAxisMultipliers() const;

	UFUNCTION(BlueprintCallable, Category = "Input")
		void AddInputAxisMultiplier(EInputDevices InInputDevice, const FName& InInputName, const FName& InId, float InMultiplier);

    /**
    * Modifies the referenced float value by all to input name registered float multipliers. Implementation of the value is intended elsewhere (Pawn / Controller). 
    * This can be used when complexity is required, when a single axis input mapping scale or single input device usage is just not enough.
    * For example, a list of multipliers could be: (gamepad) Base sensitivity + vehicle sensitivity + vehicle damage + vehicle inversion.
    * or: (Mouse) Base sensitivity + rifle weight + scope aim or hip fire? + currently shooting?
    */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
		void GetMultipliedInputAxisValue(EInputDevices InInputDevice, const FName& InInputName, float& RefValue) const;

	UFUNCTION(BlueprintCallable, Category = "Input")
		void RemoveInputAxisMultiplier(EInputDevices InInputDevice, const FName& InInputName, const FName& InId);

};

