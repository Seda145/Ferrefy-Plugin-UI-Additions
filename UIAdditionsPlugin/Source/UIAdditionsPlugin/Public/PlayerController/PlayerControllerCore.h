/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "PlayerControllerCore.generated.h"


class UHUDCorePlayerControllerComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReceivedPlayer);


/*

*/
UCLASS(config = Game, BlueprintType, Blueprintable)
class UIADDITIONSPLUGIN_API APlayerControllerCore : public APlayerController {
	GENERATED_BODY()

private:

protected:

	// Setup | Components

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		UHUDCorePlayerControllerComponent* HUDCorePlayerControllerComponent = nullptr;

public:

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnReceivedPlayer OnReceivedPlayer;

private:

protected:

public:

	// Setup

	APlayerControllerCore();

	virtual void ReceivedPlayer() override;

};