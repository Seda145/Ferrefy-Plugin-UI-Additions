/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "InputState.h"
#include "Framework/Application/IInputProcessor.h"

class FSlateApplication;
struct FAnalogInputEvent;
struct FKeyEvent;
struct FPointerEvent;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInputDeviceChanged, EInputDevices, int32 /* SlateUserIndex */);

class UIADDITIONSPLUGIN_API FDetectCurrentInputDevicePreProcessor : public IInputProcessor {

private:
	
	TMap<int32, EInputDevices> CurrentInputDevices;

	// ? A big enough deadzone should not result in any false positive. 
	// Const because it is not used for anything else than detecting HID and should (probably) not be configurable.
	const float AnalogDeadZone = 0.15f;

protected:

public:
	
	FOnInputDeviceChanged OnInputDeviceChanged;

private:

protected:

public:

	// Tick

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;

	// Input

	EInputDevices GetCurrentInputDevice(int32 SlateUserIndex) const;
	
	void SetCurrentInputDevice(EInputDevices InputDevice, int32 SlateUserIndex);

	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override;
	
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override;
	
	//virtual bool HandleMouseButtonDownEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

};
