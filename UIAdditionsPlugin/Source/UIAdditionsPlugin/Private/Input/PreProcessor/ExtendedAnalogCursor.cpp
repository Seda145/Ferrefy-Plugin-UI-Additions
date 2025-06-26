/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "ExtendedAnalogCursor.h"
#include "LogUIAdditionsPlugin.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "SlateUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "UnrealClient.h"
#include "Layout/ArrangedChildren.h"


FExtendedAnalogCursor::FExtendedAnalogCursor(FLocalPlayerContext InLocalPlayerContext)
	: FAnalogCursor() {
	LocalPlayerContext = InLocalPlayerContext;
	SetStick(EAnalogStick::Right);
	SetDeadZone(0.2f);
}

int32 FExtendedAnalogCursor::GetOwnerUserIndex() const {
	return (LocalPlayerContext.IsValid() ? LocalPlayerContext.GetLocalPlayer()->GetControllerId() : 0);
}

bool FExtendedAnalogCursor::HandleKeyDownEvent(FSlateApplication& InSlateApp, const FKeyEvent& InKeyEvent) {
	if (!IsRelevantInput(InKeyEvent)) {
		// Return if the input event is not caused by the owner
		return false;
	}
	if (IsRelevantCursorMovementKey(InKeyEvent.GetKey())) {
		// If the key is relevant to the analog cursor movement, consume the input
		return true;
	}

	// TODO would like direct bindings to input, not this.
	// Possible TODO: I had this cursor read player input instead of UInputSettings with keybindings in mind, 
	// But decided that slate navigation keys should be blacklisted from keybinding. (Slate) FNavigationConfig also registers once and not per player.
	if (!InKeyEvent.IsRepeat()) {
		if (GetProcessNavSelectEvent()) {
			if (USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavLeft)
				// We could say if a keyboard key is pressed. Example: pressing Enter for select would never act as click on the analog mouse position.
				|| !InKeyEvent.GetKey().IsGamepadKey()
				// And we could do the same when navigating with "arrows" on a gamepad etc.
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavRight)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavUp)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavDown)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavNext)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavPrevious)
				) {
				/**
				* Thoughts, possible todo. There is overlap in the cursor implementation depending on what HID we use. 
				* If the button "select" is pressed on an input device such as gamepad, the expected result without a cursor is that a focused widget such as a button is pressed.
				* With a cursor, that the item hovered over is "clicked".
				* We can switch between both behaviors, the "mouse click" when the cursor is moved, the focused press when navigation keys are used.
				*/
				SetProcessNavSelectEvent(false);
			}
		}
	}

	if (GetProcessNavSelectEvent()) {
		if (USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavSelect)) {
			if (!InKeyEvent.IsRepeat()) {
				if (TSharedPtr<FSlateUser> SlateUser = InSlateApp.GetUser(InKeyEvent)) {
					const bool bIsPrimaryUser = FSlateApplication::CursorUserIndex == SlateUser->GetUserIndex();

					FPointerEvent MouseEvent(
						SlateUser->GetUserIndex(),
						FSlateApplication::CursorPointerIndex,
						SlateUser->GetCursorPosition(),
						SlateUser->GetPreviousCursorPosition(),
						bIsPrimaryUser ? InSlateApp.GetPressedMouseButtons() : TSet<FKey>(),
						// See how Super inserts the left mouse button here, which is just odd when using a non cursor input device. Why? Overlap again, possible todo.
						// It could lead to having the wrong HID detected in DetectCurrentInputDevicePreProcessor.
						EKeys::LeftMouseButton,
						0,
						bIsPrimaryUser ? InSlateApp.GetModifierKeys() : FModifierKeysState()
					);

					UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Slate User: %d, Key: %s, Processing NavSelect as click (key down)."), SlateUser->GetUserIndex(), *InKeyEvent.GetKey().ToString());
					TSharedPtr<FGenericWindow> GenWindow;
					return InSlateApp.ProcessMouseButtonDownEvent(GenWindow, MouseEvent);
				}

			}

			return true;
		}
	}

	return false;
}

bool FExtendedAnalogCursor::HandleKeyUpEvent(FSlateApplication& InSlateApp, const FKeyEvent& InKeyEvent) {
	if (!IsRelevantInput(InKeyEvent)) {
		// Return if the input event is not caused by the owner
		return false;
	}
	if (IsRelevantCursorMovementKey(InKeyEvent.GetKey())) {
		// If the key is relevant to the analog cursor movement, consume the input
		return true;
	}

	if (!InKeyEvent.IsRepeat()) {
		if (GetProcessNavSelectEvent()) {
			if (USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavLeft)
				// We could say if a keyboard key is pressed. Example: pressing Enter for select would never act as click on the analog mouse position.
				|| !InKeyEvent.GetKey().IsGamepadKey()
				// And we could do the same when navigating with "arrows" on a gamepad etc.
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavRight)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavUp)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavDown)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavNext)
				|| USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavPrevious)

				) {
				/**
				* Thoughts, possible todo. There is overlap in the cursor implementation depending on what HID we use. 
				* If the button "select" is pressed on an input device such as gamepad, the expected result without a cursor is that a focused widget such as a button is pressed.
				* With a cursor, that the item hovered over is "clicked".
				* We can switch between both behaviors, the "mouse click" when the cursor is moved, the focused press when navigation keys are used.
				*/
				SetProcessNavSelectEvent(false);
			}
		}

		if (GetProcessNavSelectEvent()) {
			if (USlateUtils::FindActionMappingByKeyEventAndName(InKeyEvent, USlateUtils::InputActionNavSelect)) {
				if (TSharedPtr<FSlateUser> SlateUser = InSlateApp.GetUser(InKeyEvent)) {
					const bool bIsPrimaryUser = FSlateApplication::CursorUserIndex == SlateUser->GetUserIndex();

					TSet<FKey> EmptySet;
					FPointerEvent MouseEvent(
						SlateUser->GetUserIndex(),
						FSlateApplication::CursorPointerIndex,
						SlateUser->GetCursorPosition(),
						SlateUser->GetPreviousCursorPosition(),
						bIsPrimaryUser ? InSlateApp.GetPressedMouseButtons() : EmptySet,
						// See how Super inserts the left mouse button here, which is just odd when using a non cursor input device. Why? Overlap again, possible todo.
						// It could lead to having the wrong HID detected in DetectCurrentInputDevicePreProcessor.
						EKeys::LeftMouseButton,
						0,
						bIsPrimaryUser ? InSlateApp.GetModifierKeys() : FModifierKeysState()
					);

					UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Slate User: %d, Key: %s, Processing NavSelect as click (key up)."), SlateUser->GetUserIndex(), *InKeyEvent.GetKey().ToString());
					return InSlateApp.ProcessMouseButtonUpEvent(MouseEvent);
				}
			}
		}
	}

	return false;
}

bool FExtendedAnalogCursor::HandleAnalogInputEvent(FSlateApplication& InSlateApp, const FAnalogInputEvent& InAnalogInputEvent) {
	// When freezing the cursor we don't need to process analog input.
	if (GetFreezeCursorToCenterOfScreen()) {
		return false;
	}
	// Return if the input event is not caused by the owner
	if (!IsRelevantInput(InAnalogInputEvent)) {
		return false;
	}
	// Return if the event's input key is not relevant to the analog cursor
	if (!IsRelevantCursorMovementKey(InAnalogInputEvent.GetKey())) {
		return false;
	}

	// If we are moving the cursor, we can assume we want to click with the cursor.
	// Todo, how to deal with deadzones? Without current hardcoded one a PS3 gamepad processed the nav select event often after having used Slate nav keys.
	if (FMath::Abs(InAnalogInputEvent.GetAnalogValue()) > DeadZone) {
		SetProcessNavSelectEvent(true);
	}

	// Call super to handle the event.
	const bool bHandled = FAnalogCursor::HandleAnalogInputEvent(InSlateApp, InAnalogInputEvent);
	//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Slate User: %d, Analog: %s, State: Handled"), InAnalogInputEvent.GetUserIndex(), *InAnalogInputEvent.GetKey().ToString());
	return bHandled;
}

void FExtendedAnalogCursor::Tick(const float InDeltaTime, FSlateApplication& InSlateApp, TSharedRef<ICursor> InCursor) {
	const TSharedPtr<FSlateUser> SlateUser = InSlateApp.GetUser(GetOwnerUserIndex());
	if (!SlateUser) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("ExtendedAnalogCursor << invalid SlateUser."));
		return;
	}

	if (GetFreezeCursorToCenterOfScreen()) {
		if (!GEngine || !LocalPlayerContext.IsValid()) {
			// UE_LOG(LogUIAdditionsPlugin, Error, TEXT("ExtendedAnalogCursor << invalid GEngine or PlayerContext."));
			return; 
		}

		ClearAnalogValues();

		// Update the Slate cursor position with the center position.
		const FVector2D NewPosition = USlateUtils::GetCenterOfPlayerScreen(LocalPlayerContext.GetLocalPlayer());
		UpdateCursorPosition(InSlateApp, SlateUser.ToSharedRef(), NewPosition, true);

		// No need to do anything beyond here while centering the cursor.
		return;
	}
	else {
		const FVector2D OldPosition = SlateUser->GetCursorPosition();

		float SpeedMult = 1.0f; // Used to do a speed multiplication before adding the delta to the position to make widgets sticky
		FVector2D AdjAnalogVals = GetAnalogValues(AnalogStick); // A copy of the analog values so I can modify them based being over a widget

		// Adjust analog values according to dead zone
		const float AnalogValsSize = AdjAnalogVals.Size();

		if (AnalogValsSize > 0.0f) {
			const float TargetSize = FMath::Max(AnalogValsSize - DeadZone, 0.0f) / (1.0f - DeadZone);
			AdjAnalogVals /= AnalogValsSize;
			AdjAnalogVals *= TargetSize;
		}

		// Check if there is a sticky widget beneath the cursor
		if (USlateUtils::GetCurrentInputDevice(SlateUser->GetUserIndex()) != EInputDevices::Mouse) {
			FWidgetPath WidgetPath = InSlateApp.LocateWindowUnderMouse(OldPosition, InSlateApp.GetInteractiveTopLevelWindows(), false, SlateUser->GetUserIndex());
			if (WidgetPath.IsValid()) {
				const FArrangedChildren::FArrangedWidgetArray& AllArrangedWidgets = WidgetPath.Widgets.GetInternalArray();
				for (const FArrangedWidget& ArrangedWidget : AllArrangedWidgets) {
					const TSharedRef<SWidget> Widget = ArrangedWidget.Widget;
					if (Widget->IsInteractable()) {
						SpeedMult = StickySlowdown;
						//FVector2D Adjustment = WidgetsAndCursors.Last().Geometry.Position - OldPosition; // example of calculating distance from cursor to widget center
						break;
					}
				}
			}
		}

		float CurrentMinSpeedX = 0.0f;
		float CurrentMaxSpeedX = 0.0f;
		float CurrentMinSpeedY = 0.0f;
		float CurrentMaxSpeedY = 0.0f;
		FVector2D ExpAcceleration = FVector2D::ZeroVector;

		switch (Mode) {
		case (AnalogCursorMode::Accelerated):
			// Generate Min and Max for X to clamp the speed, this gives us instant direction change when crossing the axis
			if (AdjAnalogVals.X > 0.0f) {
				CurrentMaxSpeedX = AdjAnalogVals.X * MaxSpeed;
			}
			else {
				CurrentMinSpeedX = AdjAnalogVals.X * MaxSpeed;
			}

			// Generate Min and Max for Y to clamp the speed, this gives us instant direction change when crossing the axis
			if (AdjAnalogVals.Y > 0.0f) {
				CurrentMaxSpeedY = AdjAnalogVals.Y * MaxSpeed;
			}
			else {
				CurrentMinSpeedY = AdjAnalogVals.Y * MaxSpeed;
			}

			// Cubic acceleration curve
			ExpAcceleration = AdjAnalogVals * AdjAnalogVals * AdjAnalogVals * Acceleration;
			// Preserve direction (if we use a squared equation above)
			//ExpAcceleration.X *= FMath::Sign(AnalogValues.X);
			//ExpAcceleration.Y *= FMath::Sign(AnalogValues.Y);

			CurrentSpeed += ExpAcceleration * InDeltaTime;
			CurrentSpeed.X = FMath::Clamp(CurrentSpeed.X, CurrentMinSpeedX, CurrentMaxSpeedX);
			CurrentSpeed.Y = FMath::Clamp(CurrentSpeed.Y, CurrentMinSpeedY, CurrentMaxSpeedY);
			break;
		case (AnalogCursorMode::Direct):
			CurrentSpeed = AdjAnalogVals * MaxSpeed;
			break;
		}

		CurrentOffset += CurrentSpeed * InDeltaTime * SpeedMult;
		const FVector2D NewPosition = OldPosition + CurrentOffset;

		// save the remaining sub-pixel offset 
		CurrentOffset.X = FGenericPlatformMath::Frac(NewPosition.X);
		CurrentOffset.Y = FGenericPlatformMath::Frac(NewPosition.Y);

		// update the cursor position
		UpdateCursorPosition(InSlateApp, SlateUser.ToSharedRef(), NewPosition, true);
	}
}

bool FExtendedAnalogCursor::IsRelevantCursorMovementKey(const FKey Key) const {
	return AnalogStickKeys.Contains(Key);
}

void FExtendedAnalogCursor::UpdateCursorPosition(FSlateApplication& InSlateApp, TSharedRef<FSlateUser> InSlateUser, const FVector2D& InNewPosition, bool bInForce) {
	const FVector2D OldPosition = InSlateUser->GetCursorPosition();
	if (!LocalPlayerContext.IsValid()) {
		return;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(LocalPlayerContext.GetLocalPlayer(), EGetWorldErrorMode::LogAndReturnNull);
	const UGameViewportClient* GameViewportClient = IsValid(World) ? World->GetGameViewport() : nullptr;
	if (!IsValid(GameViewportClient)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("ExtendedAnalogCursor << invalid GameViewportClient."));
		return;
	}
	// Do a check if the app is running in the foreground, else the mouse will be hijacked outside of the app.
	if (!GameViewportClient->Viewport || !GameViewportClient->Viewport->IsForegroundWindow()) {
		// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("App is not running in the foreground, canceled centering."));
		return;
	}

	// Return if the cursor position has not changed and is not forced
	int32 NewIntPosX = InNewPosition.X;
	int32 NewIntPosY = InNewPosition.Y;
	int32 OldIntPosX = OldPosition.X;
	int32 OldIntPosY = OldPosition.Y;
	if (!bInForce) {
		if (OldIntPosX == NewIntPosX && OldIntPosY == NewIntPosY) {
			return;
		}
	}

	// Clamp the new position to within relevant space. 
	// Setting things out of bounds (SetCursorPosition, SetPositionInViewport) seems to break things (such as tick on widget.).
	// TODO check if this is enough for split screen so that we don't move onto another screen.
	const FGeometry Geometry = (GetCursorScreenSpace() == E_CursorScreenSpace::PlayerScreen
		? UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(LocalPlayerContext.GetPlayerController())
		: UWidgetLayoutLibrary::GetViewportWidgetGeometry(LocalPlayerContext.GetPlayerController())
	);

	const FVector2D ClampedNewPosition = USlateUtils::ClampAbsolutePositionToGeometry(Geometry, InNewPosition);
	
	// Set the Slate cursor position
	InSlateUser->SetCursorPosition(ClampedNewPosition);
	//UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Set cursor position to: %s"), *ClampedNewPosition.ToString());

	// Get the updated position, which might have been locked or clamped
	const FVector2D UpdatedPosition = InSlateUser->GetCursorPosition();

	const bool bIsPrimaryUser = FSlateApplication::CursorUserIndex == InSlateUser->GetUserIndex();
	// Engine default for mouse event:
	FKey EffectingKey = EKeys::Invalid;

	if (GetFreezeCursorToCenterOfScreen()) {
		// Hack on EffectingKey, EKeys::Insert is used to identify the event in other input processors as non mouse and non gamepad
		// , because they would otherwise receive a EKeys::Invalid on a mouse event which would make identification impossible.
		// Currently this is required to prevent a detection of HID change in DetectCurrentInputDevicePreProcessor.
		EffectingKey = EKeys::Insert;

		// When "freezing" the cursor, we need to send the new position as the previous position in the mouse event.
		// If we don't, the resulting input events on the mouse will report changing axis values, affecting character controls.
		FPointerEvent MouseEvent(
			InSlateUser->GetUserIndex(),
			UpdatedPosition,
			UpdatedPosition,
			bIsPrimaryUser ? InSlateApp.GetPressedMouseButtons() : TSet<FKey>(),
			EffectingKey,
			0,
			bIsPrimaryUser ? InSlateApp.GetModifierKeys() : FModifierKeysState()
		);

		InSlateApp.ProcessMouseMoveEvent(MouseEvent);
	}
	else {
		// By sending the analog stick as key to the mouse event, we can figure out what device caused the OnMouseMove method later.
		if (GetAnalogValues(AnalogStick).Size() > DeadZone) {
			EffectingKey = AnalogStickKeyIdentifier;
		}

		FPointerEvent MouseEvent(
			InSlateUser->GetUserIndex(),
			UpdatedPosition,
			OldPosition,
			bIsPrimaryUser ? InSlateApp.GetPressedMouseButtons() : TSet<FKey>(),
			EffectingKey,
			0,
			bIsPrimaryUser ? InSlateApp.GetModifierKeys() : FModifierKeysState()
		);

		InSlateApp.ProcessMouseMoveEvent(MouseEvent);
	}

	//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Slate User: %d, Position: %s"), SlateUser->GetUserIndex(), *UpdatedPosition.ToString());
	// Store for quick access
	CurrentPosition = UpdatedPosition;
}

FVector2D FExtendedAnalogCursor::GetCurrentPosition() {
	return CurrentPosition;
}

bool FExtendedAnalogCursor::GetFreezeCursorToCenterOfScreen() const {
	return bFreezeCursorToCenterOfScreen;
}

void FExtendedAnalogCursor::SetFreezeCursorToCenterOfScreen(bool bInFreezeCursorToCenterOfScreen) {
	bFreezeCursorToCenterOfScreen = bInFreezeCursorToCenterOfScreen;
	UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Freeze cursor?: %s."), (GetFreezeCursorToCenterOfScreen() ? TEXT("True") : TEXT("False")));
}

E_CursorScreenSpace FExtendedAnalogCursor::GetCursorScreenSpace() const {
	return CursorScreenSpace;
}

void FExtendedAnalogCursor::SetCursorScreenSpace(E_CursorScreenSpace InCursorScreenSpace) {
	CursorScreenSpace = InCursorScreenSpace;
}

void FExtendedAnalogCursor::SetStick(EAnalogStick NewAnalogStick) {
	switch (NewAnalogStick) {
	case(EAnalogStick::Left):
		AnalogStick = EAnalogStick::Left;
		AnalogStickKeyIdentifier = EKeys::Gamepad_LeftThumbstick;
		AnalogStickKeys.Empty();
		AnalogStickKeys.Add(EKeys::Gamepad_LeftX);
		AnalogStickKeys.Add(EKeys::Gamepad_LeftY);
		AnalogStickKeys.Add(EKeys::Gamepad_LeftStick_Right);
		AnalogStickKeys.Add(EKeys::Gamepad_LeftStick_Left);
		AnalogStickKeys.Add(EKeys::Gamepad_LeftStick_Up);
		AnalogStickKeys.Add(EKeys::Gamepad_LeftStick_Down);
		break;
	case(EAnalogStick::Right):
		AnalogStick = EAnalogStick::Right;
		AnalogStickKeyIdentifier = EKeys::Gamepad_RightThumbstick;
		AnalogStickKeys.Empty();
		AnalogStickKeys.Add(EKeys::Gamepad_RightX);
		AnalogStickKeys.Add(EKeys::Gamepad_RightY);
		AnalogStickKeys.Add(EKeys::Gamepad_RightStick_Right);
		AnalogStickKeys.Add(EKeys::Gamepad_RightStick_Left);
		AnalogStickKeys.Add(EKeys::Gamepad_RightStick_Up);
		AnalogStickKeys.Add(EKeys::Gamepad_RightStick_Down);
		break;
	}
}

bool FExtendedAnalogCursor::GetProcessNavSelectEvent() const {
	return bProcessNavSelectEvent;
}

void FExtendedAnalogCursor::SetProcessNavSelectEvent(bool bInProcessNavSelectEvent) {
	if (GetProcessNavSelectEvent() != bInProcessNavSelectEvent) {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Using deadzone: %f. Set process nav select as cursor click?: %s."), DeadZone,(bInProcessNavSelectEvent ? TEXT("True") : TEXT("False")));
	}
	bProcessNavSelectEvent = bInProcessNavSelectEvent;
}