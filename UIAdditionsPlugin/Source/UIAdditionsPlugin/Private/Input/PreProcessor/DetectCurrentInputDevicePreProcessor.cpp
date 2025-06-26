/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "DetectCurrentInputDevicePreProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "LogUIAdditionsPlugin.h"
#include "Input/Events.h"


// Tick

void FDetectCurrentInputDevicePreProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) {
	// Required override, but nothing to do here.
}

// Input

EInputDevices FDetectCurrentInputDevicePreProcessor::GetCurrentInputDevice(int32 SlateUserIndex) const {
	const EInputDevices* InputDevice = CurrentInputDevices.Find(SlateUserIndex);
	return (InputDevice ? *InputDevice : EInputDevices::None);
}

bool FDetectCurrentInputDevicePreProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp,	const FAnalogInputEvent& InAnalogInputEvent) {
	if ((FMath::Abs(InAnalogInputEvent.GetAnalogValue()) > AnalogDeadZone) && InAnalogInputEvent.GetKey().IsGamepadKey())	{
		// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HandleAnalogInputEvent"));
		SetCurrentInputDevice(EInputDevices::Gamepad, InAnalogInputEvent.GetUserIndex());
	}
	return false;
}

bool FDetectCurrentInputDevicePreProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) {
	// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HandleKeyDownEvent"));
	SetCurrentInputDevice(InKeyEvent.GetKey().IsGamepadKey() ? EInputDevices::Gamepad : EInputDevices::Keyboard, InKeyEvent.GetUserIndex());
	return false;
}

bool FDetectCurrentInputDevicePreProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) {
	// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HandleMouseMoveEvent"));

	if (MouseEvent.GetEffectingButton() == EKeys::Insert) {
		// Hack on EffectingKey, EKeys::Insert is used to identify "Cursor freezing" of the ExtendedAnalogCursor here as an event which should not be 
		// linked to an input device, as the event is not caused by user input.
		return false;
	}

	// TODO is a configurable deadzone required?
	if (MouseEvent.GetCursorDelta().Length() < 4.f) {
		return false;
	}

	EInputDevices InputDevice = EInputDevices::Mouse;
	if (MouseEvent.GetEffectingButton() == EKeys::Gamepad_LeftThumbstick 
		|| MouseEvent.GetEffectingButton() == EKeys::Gamepad_RightThumbstick
		) {
		// Analog cursor
		InputDevice = EInputDevices::Gamepad;
		// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HandleMouseMoveEvent HID: Gamepad"));
	}
	
	SetCurrentInputDevice(InputDevice, MouseEvent.GetUserIndex());
	return false;
}

bool FDetectCurrentInputDevicePreProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) {
	// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HandleMouseWheelOrGestureEvent"));
	SetCurrentInputDevice(EInputDevices::Mouse, (InGestureEvent ? InGestureEvent->GetUserIndex() : InWheelEvent.GetUserIndex()));
	return false;
}

//bool FDetectCurrentInputDevicePreProcessor::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) {
//	 UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HandleMouseButtonDownEvent"));

	/**
	* TODO
	* I would update the input device here, but the FAnalogCursor system hardcodes a left mouse click even when a gamepad is controlling the mouse.
	* Passing on a gamepad button into the mouse event in FAnalogCursor::HandleKeyDownEvent breaks click functionality, it will focus instead with "mouse" as focus reason.
	* Seems like a poor design choice in the FAnalogCursor which prevents us from properly detecting the input device here.
	*/

//	SetCurrentInputDevice(EInputDevices::Mouse, MouseEvent.GetUserIndex());
//	return false;
//}

void FDetectCurrentInputDevicePreProcessor::SetCurrentInputDevice(EInputDevices InputDevice, int32 SlateUserIndex) {
	if (GetCurrentInputDevice(SlateUserIndex) != InputDevice) {
		CurrentInputDevices.Add(SlateUserIndex, InputDevice);
		OnInputDeviceChanged.Broadcast(InputDevice, SlateUserIndex);
		// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HID Changed for SlateUserIndex %d: %s"), SlateUserIndex, *UEnum::GetValueAsString(InputDevice));
	}
}

