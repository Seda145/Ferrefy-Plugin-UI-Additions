/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/AnalogCursor.h"
#include "Engine/LocalPlayer.h"
#include "InputCoreTypes.h"
#include "Math/Vector2D.h"
#include "Templates/SharedPointer.h" 
#include "GenericPlatform/ICursor.h"
#include "CursorScreenSpace.h"


class UIADDITIONSPLUGIN_API FExtendedAnalogCursor : public FAnalogCursor {

private:

	FLocalPlayerContext LocalPlayerContext = FLocalPlayerContext();

	EAnalogStick AnalogStick = EAnalogStick::Right;

	FKey AnalogStickKeyIdentifier = EKeys::Gamepad_RightThumbstick;

	TArray<FKey> AnalogStickKeys;

	// Stored Slate cursor position for quick access
	FVector2D CurrentPosition = FVector2D(0, 0);

	// If true, the cursor will be frozen to the center of the player screen during its tick.
	bool bFreezeCursorToCenterOfScreen = false;

	E_CursorScreenSpace CursorScreenSpace = E_CursorScreenSpace::PlayerScreen;

	/* If we should process a NavSelect button as a cursor click or not. This should be true after using cursor movement and false after navigating by Slate navigation keys. */
	bool bProcessNavSelectEvent = false;

protected:

public:

private:

	/* If we should process the NavSelect action as a cursor click. This is not desired while we use navigation keys instead of the cursor movement. */
	bool GetProcessNavSelectEvent() const;

	/* If we should process the NavSelect action as a cursor click. This is not desired while we use navigation keys instead of the cursor movement. */
	void SetProcessNavSelectEvent(bool bInProcessNavSelectEvent);

protected:

	/* Return if the input key is relevant to the analog cursor movement. */
	virtual bool IsRelevantCursorMovementKey(const FKey InKey) const;

	/* Handles updating the cursor position and processing a Mouse Move Event */
	virtual void UpdateCursorPosition(FSlateApplication& InSlateApp, TSharedRef<FSlateUser> InSlateUser, const FVector2D& InNewPosition, bool bInForce = false) override;

public:

	FExtendedAnalogCursor(FLocalPlayerContext InLocalPlayerContext);

	virtual ~FExtendedAnalogCursor() {}

	virtual int32 GetOwnerUserIndex() const override;

	// Handle analog input of the gamepad

	virtual bool HandleAnalogInputEvent(FSlateApplication& InSlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;

	// These key / mouse handling methods conflict with UserWidget and Slate which already handle input properly, so override them so they won't make a mess.

	virtual bool HandleKeyDownEvent(FSlateApplication& InSlateApp, const FKeyEvent& InKeyEvent) override;

	virtual bool HandleKeyUpEvent(FSlateApplication& InSlateApp, const FKeyEvent& InKeyEvent) override;

	virtual void Tick(const float InDeltaTime, FSlateApplication& InSlateApp, TSharedRef<ICursor> InCursor) override;

	FVector2D GetCurrentPosition();

	bool GetFreezeCursorToCenterOfScreen() const;

	void SetFreezeCursorToCenterOfScreen(bool bInFreezeCursorToCenterOfScreen);

	E_CursorScreenSpace GetCursorScreenSpace() const;

	void SetCursorScreenSpace(E_CursorScreenSpace InCursorScreenSpace);

	void SetStick(EAnalogStick InNewAnalogStick);

};
