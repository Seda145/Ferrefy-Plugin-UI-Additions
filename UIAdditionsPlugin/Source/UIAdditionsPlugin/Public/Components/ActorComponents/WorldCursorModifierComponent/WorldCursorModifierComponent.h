/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "WorldCursorModifierComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDesiresCenteredWorldCursorChanged, bool, InDesiresCenteredWorldCursor);


/*
* Component used with a Pawn to influence the behavior of a World Cursor. 
* Used to register if a world cursor is currently desired to be centered or not by a pawn.
*/
UCLASS(ClassGroup = CommonLogic, editinlinenew, meta = (BlueprintSpawnableComponent))
class UIADDITIONSPLUGIN_API UWorldCursorModifierComponent : public UActorComponent {
	GENERATED_BODY()

private:

	// Cursor

	UPROPERTY()
		bool bDesiresCenteredWorldCursor = true;

protected:

public:

	// Delegates

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
		FOnDesiresCenteredWorldCursorChanged OnDesiresCenteredWorldCursorChanged;

private:

protected:

public:

	// Cursor

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Character | ControlModes")
		bool GetDesiresCenteredWorldCursor() const;

	UFUNCTION(BlueprintCallable, Category = "Character | ControlModes")
		void SetDesiresCenteredWorldCursor(bool bInDesiresCenteredWorldCursor);

};
