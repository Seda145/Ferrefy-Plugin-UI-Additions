/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "SlateUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "Components/SlateWrapperTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Slate/SGameLayerManager.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Engine/GameViewportClient.h"
#include "LogUIAdditionsPlugin.h"
#include "Engine/World.h"
#include "GameFramework/InputSettings.h"
#include "Framework/Application/SlateUser.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Blueprint/UserWidget.h"
#include "UIAdditionsPlugin.h"
#include "Modules/ModuleManager.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Blueprint.h"
#include "Engine/GameInstance.h"


// Input | Slate | Navigation Config

const FName USlateUtils::InputActionNavBack				= TEXT("NavBack");
const FName USlateUtils::InputActionNavDown				= TEXT("NavDown");
const FName USlateUtils::InputActionNavLeft				= TEXT("NavLeft");
const FName USlateUtils::InputActionNavNext				= TEXT("NavNext");
const FName USlateUtils::InputActionNavPrevious			= TEXT("NavPrevious");
const FName USlateUtils::InputActionNavRight			= TEXT("NavRight");
const FName USlateUtils::InputActionNavSelect			= TEXT("NavSelect");
const FName USlateUtils::InputActionNavUp				= TEXT("NavUp");

// Input | Slate | Custom										

const FName USlateUtils::InputActionNavExtra			= TEXT("NavExtra");
const FName USlateUtils::InputActionNavMainMenu			= TEXT("NavMainMenu");
const FName USlateUtils::InputActionNavReset			= TEXT("NavReset");

// Input | Custom

const FName USlateUtils::InputAxisForward				= TEXT("Forward");
const FName USlateUtils::InputAxisBackward				= TEXT("Backward");
const FName USlateUtils::InputAxisLeft					= TEXT("Left");
const FName USlateUtils::InputAxisRight					= TEXT("Right");
const FName USlateUtils::InputAxisUp					= TEXT("Up");
const FName USlateUtils::InputAxisDown					= TEXT("Down");
const FName USlateUtils::InputAxisYawPos				= TEXT("YawPos");
const FName USlateUtils::InputAxisYawNeg				= TEXT("YawNeg");
const FName USlateUtils::InputAxisPitchPos				= TEXT("PitchPos");
const FName USlateUtils::InputAxisPitchNeg				= TEXT("PitchNeg");

// UI Routing													

const FName USlateUtils::UIRouteMainMenu				= TEXT("MainMenu");
const FName USlateUtils::UIRouteQuit					= TEXT("Quit");


// Player Controller

int32 USlateUtils::GetSlateUserIndexForPlayerController(APlayerController* InPlayerController) {
	return GetSlateUserIndexForPlayerControllerID(UGameplayStatics::GetPlayerControllerID(InPlayerController));
}

int32 USlateUtils::GetSlateUserIndexForPlayerControllerID(int32 InControllerID) {
	return (FSlateApplication::IsInitialized()
		? FSlateApplication::Get().GetUserIndexForController(InControllerID)
		: INDEX_NONE
	);
}

TSharedPtr<FSlateUser> USlateUtils::GetSlateUserForPlayerController(APlayerController* InPlayerController) {
	return (FSlateApplication::IsInitialized()
		? FSlateApplication::Get().GetUserFromControllerId(GetSlateUserIndexForPlayerControllerID(UGameplayStatics::GetPlayerControllerID(InPlayerController)))
		: nullptr
	);
}

bool USlateUtils::IsFirstLocalPlayerController(APlayerController* InPlayerController) {
	if (!IsValid(InPlayerController)) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("InPlayerController is invalid, returning false."));
		return false;
	}
	const UGameInstance* GI = UGameplayStatics::GetGameInstance(InPlayerController);
	return IsValid(GI) ? GI->GetFirstLocalPlayerController() == InPlayerController : false;
}

UPlayerInput* USlateUtils::GetPlayerInput(APlayerController* InPlayerController) {
	return IsValid(InPlayerController) ? InPlayerController->PlayerInput : nullptr;
}

// Widget Geometry

FVector2D USlateUtils::GetCenterOfPlayerScreen(ULocalPlayer* InLocalPlayer) {
	FVector2D Position = FVector2D::ZeroVector;
	if (!IsValid(InLocalPlayer)) {
		return Position;
	}
	const UWorld* World = InLocalPlayer->GetWorld();
	const UGameViewportClient* GameViewportClient = IsValid(World) ? World->GetGameViewport() : nullptr;

	return USlateUtils::GetCenterOfPlayerScreen(GameViewportClient, InLocalPlayer);
}

FVector2D USlateUtils::GetCenterOfPlayerScreen(const UGameViewportClient* InGameViewportClient, ULocalPlayer* InLocalPlayer) {
	FVector2D Position = FVector2D::ZeroVector;
	if (!IsValid(InGameViewportClient)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid GameViewportClient."));
		return Position;
	}
	const TSharedPtr<IGameLayerManager> GameLayerManager = InGameViewportClient->GetGameLayerManager();
	if (!GameLayerManager.IsValid()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid GameLayerManager."));
		return Position;
	}

	Position = GameLayerManager->GetPlayerWidgetHostGeometry(InLocalPlayer).GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f));
	return Position;
}

FVector2D USlateUtils::ClampAbsolutePositionToGeometry(const FGeometry& InGeometry, const FVector2D& InPosition) {
	return InGeometry.LocalToAbsolute(ClampLocalPositionToGeometry(InGeometry, InGeometry.AbsoluteToLocal(InPosition)));
}

FVector2D USlateUtils::ClampLocalPositionToGeometry(const FGeometry& InGeometry, const FVector2D& InLocalPosition) {
	FVector2D NewPosition = InLocalPosition;
	NewPosition.X = FMath::Clamp(NewPosition.X, 1.f, InGeometry.GetLocalSize().X - 1.f);
	NewPosition.Y = FMath::Clamp(NewPosition.Y, 1.f, InGeometry.GetLocalSize().Y - 1.f);
	return NewPosition;
}

// Widget

bool USlateUtils::IsVisible(ESlateVisibility InSlateVisibility) {
	return (InSlateVisibility == ESlateVisibility::Visible
		|| InSlateVisibility == ESlateVisibility::HitTestInvisible
		|| InSlateVisibility == ESlateVisibility::SelfHitTestInvisible
	);
}

UWidget* USlateUtils::FindAncestorWidgetByClass(const UWidget* InWidget, const TSubclassOf<UWidget> InAncestorClass, bool bInMatchSubclass, bool bInLookOutsideUserWidget) {
	if (!IsValid(InWidget) || !IsValid(InAncestorClass)) {
		return nullptr;
	}

	UWidget* WidgetX = InWidget->GetParent();

	// If a parent is not valid, we get the outer UUserWidget. From there we can also get a new parent.
	if (!IsValid(WidgetX)) {
		WidgetX = InWidget->GetTypedOuter<UUserWidget>();
		if (!IsValid(WidgetX)) {
			return nullptr;
		}
	}

	// If the parent is the target, return it.
	if (bInMatchSubclass) {
		if (WidgetX->IsA(InAncestorClass)) {
			return WidgetX;
		}			
	}
	else if (WidgetX->GetClass() == InAncestorClass) {
		return WidgetX;
	}

	// Else we walk on.
	return FindAncestorWidgetByClass(WidgetX, InAncestorClass, bInMatchSubclass, bInLookOutsideUserWidget);
}

bool USlateUtils::FindAncestorWidget(const UWidget* InWidget, const UWidget* InAncestorWidget, bool bInLookOutsideUserWidget) {
	if (!IsValid(InWidget) || !IsValid(InAncestorWidget)) {
		return false;
	}

	// Outside the loop, to keep the const.

	UWidget* WidgetX = InWidget->GetParent();

	// If a parent is not valid, we get the outer UUserWidget. From there we can also get a new parent.

	if (!IsValid(WidgetX)) {
		WidgetX = InWidget->GetTypedOuter<UUserWidget>();
		if (!IsValid(WidgetX)) {
			return false;
		}
	}

	// If the parent is the target, return true.

	if (WidgetX == InAncestorWidget) {
		return true;
	}			

	// Else we walk on.

	return FindAncestorWidget(WidgetX, InAncestorWidget, bInLookOutsideUserWidget);
}

// Conversion

void USlateUtils::GetKeyAndCharCodes(const FKey& InKey, bool& bOutHasKeyCode, uint32& OutKeyCode, bool& bOutHasCharCode, uint32& OutCharCode) {
	const uint32* KeyCodePtr;
	const uint32* CharCodePtr;
	FInputKeyManager::Get().GetCodesFromKey(InKey, KeyCodePtr, CharCodePtr);

	bOutHasKeyCode = KeyCodePtr ? true : false;
	bOutHasCharCode = CharCodePtr ? true : false;

	OutKeyCode = KeyCodePtr ? *KeyCodePtr : 0;
	OutCharCode = CharCodePtr ? *CharCodePtr : 0;

	// These special keys are not handled by the platform layer, and while not printable
	// have character mappings that several widgets look for, since the hardware sends them.
	if (CharCodePtr == nullptr) {
		if (InKey == EKeys::Tab) {
			OutCharCode = '\t';
			bOutHasCharCode = true;
		}
		else if (InKey == EKeys::BackSpace) {
			OutCharCode = '\b';
			bOutHasCharCode = true;
		}
		else if (InKey == EKeys::Enter) {
			OutCharCode = '\n';
			bOutHasCharCode = true;
		}
	}
}

FInputChord USlateUtils::GetInputChordFromKeyEvent(const FKeyEvent& KeyEvent) {
	FInputChord InputChord;
	InputChord.Key = KeyEvent.GetKey();
	InputChord.bShift = KeyEvent.IsShiftDown();
	InputChord.bCtrl = KeyEvent.IsControlDown();
	InputChord.bAlt = KeyEvent.IsAltDown();
	InputChord.bCmd = KeyEvent.IsCommandDown();
	return InputChord;
}

FInputChord USlateUtils::GetInputChordFromPointerEvent(const FPointerEvent& PointerEvent, bool bInIsMouseWheelScroll) {
	FInputChord InputChord;
	InputChord.Key = PointerEvent.GetEffectingButton();
	InputChord.bShift = PointerEvent.IsShiftDown();
	InputChord.bCtrl = PointerEvent.IsControlDown();
	InputChord.bAlt = PointerEvent.IsAltDown();
	InputChord.bCmd = PointerEvent.IsCommandDown();

	if (bInIsMouseWheelScroll) {
		// Possible TODO: UE5.2: Note that the key returned by the FPointerEvent is actually invalid when scrolling a mouse wheel. What the? I assume it broken, and fix it up here. 
		if (!InputChord.Key.IsValid()) {
			// FSlateApplication::RouteMouseWheelOrGestureEvent just runs a mouse wheel event if the wheel delta != 0. So what about deadzones?
			if (PointerEvent.GetWheelDelta() > 0) {
				InputChord.Key = EKeys::MouseScrollUp;
			}
			else {
				InputChord.Key = EKeys::MouseScrollDown;
			}
			//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("UE5.2 sends invalid instead of MouseScrollUp/Down as key during NativeOnMouseWheel. Fixed that. Scroll: %s"), *InputChord.Key.ToString());
		}
		else {
			UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Engine now sends a mouse scroll key instead of an invalid one. Inspect if all is as expected. Scroll: %s"), *InputChord.Key.ToString());
		}
	}

	return InputChord;
}

FInputChord USlateUtils::GetInputChordFromInputActionKeyMapping(const FInputActionKeyMapping& InInputActionKeyMapping) {
	return FInputChord(InInputActionKeyMapping.Key, InInputActionKeyMapping.bShift, InInputActionKeyMapping.bCtrl, InInputActionKeyMapping.bAlt, InInputActionKeyMapping.bCmd);
}

FInputChord USlateUtils::GetInputChordFromInputAxisKeyMapping(const FInputAxisKeyMapping& InInputAxisKeyMapping) {
	return FInputChord(InInputAxisKeyMapping.Key);
}

E_BPInputDevices USlateUtils::ConvertEInputDevicesToBP(EInputDevices InInputDevice) {
	switch (InInputDevice) {
	case(EInputDevices::None):
		return E_BPInputDevices::None;
	case(EInputDevices::Keyboard):
		return E_BPInputDevices::Keyboard;
	case(EInputDevices::Mouse):
		return E_BPInputDevices::Mouse;
	case(EInputDevices::Gamepad):
		return E_BPInputDevices::Gamepad;
	case(EInputDevices::OculusTouch):
		return E_BPInputDevices::OculusTouch;
	case(EInputDevices::HTCViveWands):
		return E_BPInputDevices::HTCViveWands;
	case(EInputDevices::AnySpatialDevice):
		return E_BPInputDevices::AnySpatialDevice;
	case(EInputDevices::TabletFingers):
		return E_BPInputDevices::TabletFingers;
	default:
		unimplemented();
		return E_BPInputDevices::None;
	}
}

// Input device

EInputDevices USlateUtils::GetCurrentInputDevice(int32 InSlateUserIndex) {
	const FUIAdditionsPluginModule& UIAdditionsPluginModule = FModuleManager::GetModuleChecked<FUIAdditionsPluginModule>(TEXT("UIAdditionsPlugin"));
	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& Processor = UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor();
	return Processor.IsValid() ? Processor->GetCurrentInputDevice(InSlateUserIndex) : EInputDevices::None;
}

EInputDevices USlateUtils::GetCurrentInputDevice(APlayerController* InPlayerController) {
	return GetCurrentInputDevice(GetSlateUserIndexForPlayerController(InPlayerController));
}

E_BPInputDevices USlateUtils::GetCurrentInputDeviceBP(APlayerController* InPlayerController) {
	return ConvertEInputDevicesToBP(GetCurrentInputDevice(InPlayerController));
}

TSet<EMouseCursor::Type> USlateUtils::GetMouseCursorTypes() {
	TSet<EMouseCursor::Type> MouseCursorTypes;
	MouseCursorTypes.Add(EMouseCursor::None);
	MouseCursorTypes.Add(EMouseCursor::Default);
	MouseCursorTypes.Add(EMouseCursor::TextEditBeam);
	MouseCursorTypes.Add(EMouseCursor::ResizeLeftRight);
	MouseCursorTypes.Add(EMouseCursor::ResizeUpDown);
	MouseCursorTypes.Add(EMouseCursor::ResizeSouthEast);
	MouseCursorTypes.Add(EMouseCursor::ResizeSouthWest);
	MouseCursorTypes.Add(EMouseCursor::CardinalCross);
	MouseCursorTypes.Add(EMouseCursor::Crosshairs);
	MouseCursorTypes.Add(EMouseCursor::Hand);
	MouseCursorTypes.Add(EMouseCursor::GrabHand);
	MouseCursorTypes.Add(EMouseCursor::GrabHandClosed);
	MouseCursorTypes.Add(EMouseCursor::SlashedCircle);
	MouseCursorTypes.Add(EMouseCursor::EyeDropper);
	return MouseCursorTypes;
}

bool USlateUtils::ProcessKeyEvent(int32 InSlateUserIndex, const FKey& InKey, bool bInIsKeyDown, bool bInIsRepeat) {
	// Wanted to use FModifierKeysState as parameter on the method, but didn't compile on USlateUtils.h. FInputChord is not a 1:1 match.
	// Then found out on SEditableTextBox that it was ignored entirely anyway (no shift, no caps, character R remains R not r), so I skip it. 
	// Possible TODO if EPIC cleans up their stuff.

	if (!FSlateApplication::IsInitialized()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("SimulateInputKey failed, FSlateApplication is not initialized."));
		return false;
	}

	// The following is similar behavior to WidgetInteractionComponent.

	bool bHasKeyCode, bHasCharCode;
	uint32 KeyCode, CharCode;
	GetKeyAndCharCodes(InKey, bHasKeyCode, KeyCode, bHasCharCode, CharCode);
	const FKeyEvent KeyEvent = FKeyEvent(InKey, FModifierKeysState(), InSlateUserIndex, bInIsRepeat, KeyCode, CharCode);
	const bool bIsProcessKeyEventHandled = bInIsKeyDown ? FSlateApplication::Get().ProcessKeyDownEvent(KeyEvent) : FSlateApplication::Get().ProcessKeyUpEvent(KeyEvent);
	
	bool bIsCharEventHandled = true;
	if (bHasCharCode) {
		const FCharacterEvent CharacterEvent(CharCode, FModifierKeysState(), InSlateUserIndex, bInIsRepeat);
		bIsCharEventHandled = FSlateApplication::Get().ProcessKeyCharEvent(CharacterEvent);
		// UE5.2:
		// On the OS level a physical key press would result in getting the correct character.
		// When simulating a character event, the engine lacks some functionality:
		// Modifier keys (shift, caps etc.) can't be simulated through this method properly. 
		// Sending "R, no shift, no caps" to an editable text box widget results in "R" (see FSlateEditableTextLayout::HandleTypeChar),
		// ignoring FCharacterEvent. It's a Slate bug because it seems this key processing has to be finished up at the OS level, and Slate allows wrong results.
		// 
		// While this method is still useful to simulate an input action, it lacks in functionality when used for actual text input (such as virtual keyboard (only capital text, missing symbols etc.)). For such purposes, an alternative must be implemented.
	}

	return (bIsProcessKeyEventHandled || bIsCharEventHandled);
}

// Input Mappings

TArray<FName> USlateUtils::GetStandardSlateNavInputNames() {
	TArray<FName> Names;
	Names.Add(USlateUtils::InputActionNavBack);
	Names.Add(USlateUtils::InputActionNavDown);
	Names.Add(USlateUtils::InputActionNavLeft);
	Names.Add(USlateUtils::InputActionNavNext);
	Names.Add(USlateUtils::InputActionNavPrevious);
	Names.Add(USlateUtils::InputActionNavRight);
	Names.Add(USlateUtils::InputActionNavSelect);
	Names.Add(USlateUtils::InputActionNavUp);
	return Names;
}

TArray<FInputActionKeyMapping> USlateUtils::GetDefaultStandardSlateNavInputMapping(const FName& InName) {
	TArray<FInputActionKeyMapping> Mappings;

	if (InName.IsEqual(USlateUtils::InputActionNavBack)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavBack, EKeys::Escape));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavBack, EKeys::Gamepad_FaceButton_Right));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavDown)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavDown, EKeys::Down));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavDown, EKeys::Gamepad_DPad_Down));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavLeft)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavLeft, EKeys::Left));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavLeft, EKeys::Gamepad_DPad_Left));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavNext)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavNext, EKeys::Tab));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavPrevious)) {
		// Left unassigned for slate compatibility.
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavPrevious, EKeys::Invalid));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavRight)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavRight, EKeys::Right));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavRight, EKeys::Gamepad_DPad_Right));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavSelect)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavSelect, EKeys::Enter));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavSelect, EKeys::SpaceBar));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavSelect, EKeys::Gamepad_FaceButton_Bottom));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavUp)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavUp, EKeys::Up));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavUp, EKeys::Gamepad_DPad_Up));
	}

	if (Mappings.Num() == 0) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Attempted to get a default action key mapping for an unsupported action."));
	}

	return Mappings;
}

bool USlateUtils::ValidateStandardSlateNavInputMappings() {
	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	if (!IsValid(InputSettings)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InputSettings is not valid."));
		return false;
	}

	for (const FName& NameX : USlateUtils::GetStandardSlateNavInputNames()) {
		if (InputSettings->DoesActionExist(NameX)) {
			continue;
		}

		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Writing default input Nav* action mapping for missing: %s."), *NameX.ToString());
		for (const FInputActionKeyMapping& MappingX : GetDefaultStandardSlateNavInputMapping(NameX)) {
			InputSettings->AddActionMapping(MappingX, false);
		}
	}

	InputSettings->ForceRebuildKeymaps();
	InputSettings->SaveKeyMappings();

	return true;
}

TArray<FName> USlateUtils::GetExtendedSlateNavInputNames() {
	return {
		USlateUtils::InputActionNavExtra,
		USlateUtils::InputActionNavMainMenu,
		USlateUtils::InputActionNavReset
	};
}

TArray<FInputActionKeyMapping> USlateUtils::GetDefaultExtendedSlateNavInputMapping(const FName& InName) {
	TArray<FInputActionKeyMapping> Mappings;

	if (InName.IsEqual(USlateUtils::InputActionNavExtra)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavExtra, EKeys::V));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavExtra, EKeys::Gamepad_FaceButton_Left));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavMainMenu)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavMainMenu, EKeys::Escape));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavMainMenu, EKeys::Gamepad_Special_Right));
	}
	else if (InName.IsEqual(USlateUtils::InputActionNavReset)) {
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavReset, EKeys::R));
		Mappings.Add(FInputActionKeyMapping(USlateUtils::InputActionNavReset, EKeys::Gamepad_FaceButton_Top));
	}

	if (Mappings.Num() == 0) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Attempted to get a default action key mapping for an unsupported action."));
	}

	return Mappings;
}

bool USlateUtils::ValidateExtendedSlateNavInputMappings() {
	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	if (!IsValid(InputSettings)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InputSettings is not valid."));
		return false;
	}

	for (const FName& NameX : GetExtendedSlateNavInputNames()) {
		if (InputSettings->DoesActionExist(NameX)) {
			continue;
		}

		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Writing default input Nav* action mapping for missing: %s."), *NameX.ToString());
		for (const FInputActionKeyMapping& MappingX : GetDefaultExtendedSlateNavInputMapping(NameX)) {
			InputSettings->AddActionMapping(MappingX, false);
		}
	}

	InputSettings->ForceRebuildKeymaps();
	InputSettings->SaveKeyMappings();

	return true;
}

TArray<FName> USlateUtils::GetCustomInputAxisNames() {
	return {
		USlateUtils::InputAxisForward,
		USlateUtils::InputAxisBackward,
		USlateUtils::InputAxisLeft,
		USlateUtils::InputAxisRight,
		USlateUtils::InputAxisUp,
		USlateUtils::InputAxisDown,
		USlateUtils::InputAxisYawPos,
		USlateUtils::InputAxisYawNeg,
		USlateUtils::InputAxisPitchPos,
		USlateUtils::InputAxisPitchNeg,
	};
}

TArray<FInputAxisKeyMapping> USlateUtils::GetDefaultCustomInputAxisMapping(const FName& InName) {
	TArray<FInputAxisKeyMapping> Mappings;

	if (InName.IsEqual(USlateUtils::InputAxisForward)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisForward, EKeys::W, 1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisForward, EKeys::Gamepad_LeftStick_Up, 1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisBackward)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisBackward, EKeys::S, -1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisBackward, EKeys::Gamepad_LeftStick_Down, -1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisLeft)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisLeft, EKeys::A, -1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisLeft, EKeys::Gamepad_LeftStick_Left, -1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisRight)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisRight, EKeys::D, 1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisRight, EKeys::Gamepad_LeftStick_Right, 1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisUp)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisUp, EKeys::W, 1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisUp, EKeys::Gamepad_LeftStick_Up, 1.f));
	}	
	else if (InName.IsEqual(USlateUtils::InputAxisDown)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisDown, EKeys::S, -1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisDown, EKeys::Gamepad_LeftStick_Down, -1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisYawPos)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisYawPos, EKeys::MouseX, 1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisYawPos, EKeys::Gamepad_RightStick_Right, 1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisYawNeg)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisYawNeg, EKeys::Gamepad_RightStick_Left, -1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisPitchPos)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisPitchPos, EKeys::Gamepad_RightStick_Up, -1.f));
	}
	else if (InName.IsEqual(USlateUtils::InputAxisPitchNeg)) {
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisPitchNeg, EKeys::MouseY, -1.f));
		Mappings.Add(FInputAxisKeyMapping(USlateUtils::InputAxisPitchNeg, EKeys::Gamepad_RightStick_Down, 1.f));
	}

	if (Mappings.Num() == 0) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Attempted to get a default action key mapping for an unsupported action."));
	}

	return Mappings;
}

bool USlateUtils::ValidateCustomInputAxisMappings() {
	UInputSettings* InputSettings = UInputSettings::GetInputSettings();
	if (!IsValid(InputSettings)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InputSettings is not valid."));
		return false;
	}

	for (const FName& NameX : GetCustomInputAxisNames()) {
		if (InputSettings->DoesAxisExist(NameX)) {
			continue;
		}

		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Writing default input CustomInputAxis mapping for missing: %s."), *NameX.ToString());
		for (const FInputAxisKeyMapping& MappingX : GetDefaultCustomInputAxisMapping(NameX)) {
			InputSettings->AddAxisMapping(MappingX, false);
		}
	}

	InputSettings->ForceRebuildKeymaps();
	InputSettings->SaveKeyMappings();

	return true;
}

bool USlateUtils::FindActionMappingByKeyEventAndName(FKeyEvent InKeyEvent, const FName& InName) {
	return FindActionMappingByInputChordAndName(GetInputChordFromKeyEvent(InKeyEvent), InName);
}

bool USlateUtils::FindActionMappingByPointerEventAndName(FPointerEvent InPointerEvent, const FName& InName, bool bInIsMouseWheelScroll) {
	return FindActionMappingByInputChordAndName(GetInputChordFromPointerEvent(InPointerEvent, bInIsMouseWheelScroll), InName);
}

bool USlateUtils::FindActionMappingByInputChordAndName(FInputChord InInputChord, const FName& InName) {
	TArray<FInputActionKeyMapping> OutMappings;
	UInputSettings::GetInputSettings()->GetActionMappingByName(InName, OutMappings);
	if (OutMappings.Contains(FInputActionKeyMapping(InName, InInputChord.Key, InInputChord.bShift, InInputChord.bCtrl, InInputChord.bAlt, InInputChord.bCmd))) {
		return true;
	}
	return false;
}


// For the blueprint users, let's expose the constants (methods are quick to generate with notepad++ multi line editing):


/* Input | Slate | Navigation Config */

const FName& USlateUtils::ConstantInputActionNavBack() {
	return InputActionNavBack;
}

const FName& USlateUtils::ConstantInputActionNavDown() {
	return InputActionNavDown;
}

const FName& USlateUtils::ConstantInputActionNavLeft() {
	return InputActionNavLeft;
}

const FName& USlateUtils::ConstantInputActionNavNext() {
	return InputActionNavNext;
}

const FName& USlateUtils::ConstantInputActionNavPrevious() {
	return InputActionNavPrevious;
}

const FName& USlateUtils::ConstantInputActionNavRight() {
	return InputActionNavRight;
}

const FName& USlateUtils::ConstantInputActionNavSelect() {
	return InputActionNavSelect;
}

const FName& USlateUtils::ConstantInputActionNavUp() {
	return InputActionNavUp;
}

/* Input | Slate | Custom */

const FName& USlateUtils::ConstantInputActionNavExtra() {
	return InputActionNavExtra;
}

const FName& USlateUtils::ConstantInputActionNavMainMenu() {
	return InputActionNavMainMenu;
}

const FName& USlateUtils::ConstantInputActionNavReset() {
	return InputActionNavReset;
}

/* Input | Custom */

const FName& USlateUtils::ConstantInputAxisForward() {
	return InputAxisForward;
}

const FName& USlateUtils::ConstantInputAxisBackward() {
	return InputAxisBackward;
}

const FName& USlateUtils::ConstantInputAxisLeft() {
	return InputAxisLeft;
}

const FName& USlateUtils::ConstantInputAxisRight() {
	return InputAxisRight;
}

const FName& USlateUtils::ConstantInputAxisUp() {
	return InputAxisUp;
}

const FName& USlateUtils::ConstantInputAxisDown() {
	return InputAxisDown;
}

const FName& USlateUtils::ConstantInputAxisYawPos() {
	return InputAxisYawPos;
}

const FName& USlateUtils::ConstantInputAxisYawNeg() {
	return InputAxisYawNeg;
}

const FName& USlateUtils::ConstantInputAxisPitchPos() {
	return InputAxisPitchPos;
}

const FName& USlateUtils::ConstantInputAxisPitchNeg() {
	return InputAxisPitchNeg;
}

/* UI Routing */

const FName& USlateUtils::ConstantUIRouteMainMenu() {
	return UIRouteMainMenu;
}

const FName& USlateUtils::ConstantUIRouteQuit() {
	return UIRouteQuit;
}
