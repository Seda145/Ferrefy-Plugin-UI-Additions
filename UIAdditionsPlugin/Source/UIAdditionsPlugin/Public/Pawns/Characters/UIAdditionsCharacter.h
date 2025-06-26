/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "UIAdditionsCharacter.generated.h"


class UWorldCursorModifierComponent;


UCLASS(BlueprintType, Blueprintable)
class AUIAdditionsCharacter : public ACharacter {
    GENERATED_BODY()

private:

protected:

	// Components
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		UWorldCursorModifierComponent* WorldCursorModifierComponent = nullptr;	

public:

private:

protected:

	// Delegates | Input

	UFUNCTION()
		virtual void ActOnInputAxisForward(float InAxisValue);

	UFUNCTION()
		virtual void ActOnInputAxisRight(float InAxisValue);

	UFUNCTION()
		virtual void ActOnInputAxisYaw(float InAxisValue);

	UFUNCTION()
		virtual void ActOnInputAxisPitch(float InAxisValue);

	// Components

	UWorldCursorModifierComponent* GetWorldCursorModifierComponent() const;

public:

	// Setup

	AUIAdditionsCharacter();

	// Input

	virtual void SetupPlayerInputComponent(UInputComponent* InPlayerInputComponent) override;

};
