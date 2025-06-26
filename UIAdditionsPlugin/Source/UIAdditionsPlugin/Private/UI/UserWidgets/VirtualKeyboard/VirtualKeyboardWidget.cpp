/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "VirtualKeyboardWidget.h"
#include "VirtualKeyboardKeyWidget.h"
#include "VirtualKeyboardTextKeyWidget.h"
#include "LogUIAdditionsPlugin.h"
#include "ButtonWidget.h"
#include "SlateUtils.h"
#include "Modules/ModuleManager.h"
#include "UIAdditionsPlugin.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "Framework/Application/SlateApplication.h"


// Setup

void UVirtualKeyboardWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	if (IsValid(GetVirtualKeyboardCapsButton())) {
		GetVirtualKeyboardCapsButton()->OnButtonReleased.AddDynamic(this, &UVirtualKeyboardWidget::ActOnKeyboardCapsButtonReleased);
	}
}

// Widgets 

UButtonWidget* UVirtualKeyboardWidget::GetVirtualKeyboardCapsButton() const {
	return VirtualKeyboardCapsButton;
}

void UVirtualKeyboardWidget::RegisterKeyboardKeyWidget(UVirtualKeyboardKeyWidget* InWidget) {
	if (!IsValid(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("UVirtualKeyboardKeyWidget InWidget is invalid."));
		return;
	}
	if (VirtualKeyboardKeyWidgets.Contains(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Widget already registered. Ignoring request."));
		return;
	}

	VirtualKeyboardKeyWidgets.Add(InWidget);
	// Up to preference if you do this on press or release.
	// Doing this on press has a side effect that the button's press style doesn't show, 
	// likely because we have to temporarily focus a target widget to simulate input there.
	if (InWidget->GetTriggerOnKeyPressed()) {
		InWidget->OnButtonPressed.AddDynamic(this, &UVirtualKeyboardWidget::ActOnKeyboardKeyWidgetButtonPressed);
	}
	if (InWidget->GetTriggerOnKeyReleased()) {
		InWidget->OnButtonReleased.AddDynamic(this, &UVirtualKeyboardWidget::ActOnKeyboardKeyWidgetButtonReleased);
	}
}

void UVirtualKeyboardWidget::RegisterKeyboardTextKeyWidget(UVirtualKeyboardTextKeyWidget* InWidget) {
	if (!IsValid(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("UVirtualKeyboardTextKeyWidget InWidget is invalid."));
		return;
	}
	if (VirtualKeyboardTextKeyWidgets.Contains(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Widget already registered. Ignoring request."));
		return;
	}

	VirtualKeyboardTextKeyWidgets.Add(InWidget);
	// Up to preference if you do this on press or release.
	// Doing this on press has a side effect that the button's press style doesn't show, 
	// likely because we have to temporarily focus a target widget to simulate input there.
	if (InWidget->GetTriggerOnKeyPressed()) {
		InWidget->OnButtonPressed.AddDynamic(this, &UVirtualKeyboardWidget::ActOnKeyboardTextKeyWidgetButtonPressed);
	}
	if (InWidget->GetTriggerOnKeyReleased()) {
		InWidget->OnButtonReleased.AddDynamic(this, &UVirtualKeyboardWidget::ActOnKeyboardTextKeyWidgetButtonReleased);
	}
}

UWidget* UVirtualKeyboardWidget::GetWidgetToReceiveInput() const {
	return WidgetToReceiveInput;
}

void UVirtualKeyboardWidget::BindWidgetWhichReceivesInput(UWidget* InWidget) {
	if (!IsValid(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InWidget is invalid."));
		return;
	}
	WidgetToReceiveInput = InWidget;
}

// Appearance

bool UVirtualKeyboardWidget::GetIsCapsModifierEnabled() const {
	return bIsCapsModifierEnabled;
}

void UVirtualKeyboardWidget::SetIsCapsModifierEnabled(bool bInIsCapsModifierEnabled) {
	bIsCapsModifierEnabled = bInIsCapsModifierEnabled;
	if (IsValid(GetVirtualKeyboardCapsButton())) {
		GetVirtualKeyboardCapsButton()->SetAppearPressed(GetIsCapsModifierEnabled(), true);
	}

	for (UVirtualKeyboardTextKeyWidget* WidgetX : VirtualKeyboardTextKeyWidgets) {
		if (!IsValid(WidgetX)) {
			continue;
		}
		WidgetX->SetUseInputTextShift(GetIsCapsModifierEnabled());
	}
}

// Input

bool UVirtualKeyboardWidget::IsSimulatingInput() const {
	return bIsSimulatingInput;
}

FReply UVirtualKeyboardWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	FReply Reply = Super::NativeOnKeyDown(InGeometry, InKeyEvent);

	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Gamepad_FaceButton_Left) {
		// Up to preference, but this can be a default for gamepad usage to delete text through a simulated key press.
		ProcessInputKey(EKeys::BackSpace, true);
		Reply = FReply::Handled();
	}

	return Reply;
}

FReply UVirtualKeyboardWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	FReply Reply = Super::NativeOnKeyUp(InGeometry, InKeyEvent);

	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Gamepad_LeftTrigger) {
		// Up to preference, but this can be a default for gamepad usage to control the simulated caps / shift modifier.
		SetIsCapsModifierEnabled(!GetIsCapsModifierEnabled());
		Reply = FReply::Handled();
	}

	return Reply;
}

void UVirtualKeyboardWidget::ProcessKeyboardKeyWidget(UVirtualKeyboardKeyWidget* InKeyboardKeyWidget, bool bInAsKeyDown) {
	if (IsValid(InKeyboardKeyWidget)) {
		ProcessInputKey(InKeyboardKeyWidget->GetInputKey(), bInAsKeyDown);
	}
}

void UVirtualKeyboardWidget::ProcessInputKey(const FKey& InKey, bool bInAsKeyDown) {
	if (!IsValid(GetWidgetToReceiveInput())) {
		// Not yet registered, or we have nothing to send input to.
		return;
	}
	if (!GetWidgetToReceiveInput()->TakeWidget()->SupportsKeyboardFocus()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InWidget: %s does not support keyboard focus."), *GetWidgetToReceiveInput()->GetName());
		return;
	}

	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Processing input key: %s as %s"), *InKey.ToString(), *FString(bInAsKeyDown ? "pressed" : "released"));
	const FUIAdditionsPluginModule& UIAdditionsPluginModule = FModuleManager::GetModuleChecked<FUIAdditionsPluginModule>(TEXT("UIAdditionsPlugin"));
	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& Processor = UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor();
	if (!Processor.IsValid()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid FDetectCurrentInputDevicePreProcessor."));
		return;
	}

	const int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	const EInputDevices CurInputDevice = Processor->GetCurrentInputDevice(UserIndex);

	// Focus to widget which we want to receive input.
	if (FSlateApplication::Get().SetUserFocus(UserIndex, GetWidgetToReceiveInput()->TakeWidget())) {
		// Process input.
		bIsSimulatingInput = true;
		USlateUtils::ProcessKeyEvent(UserIndex, InKey, bInAsKeyDown, false);
		bIsSimulatingInput = false;
		// Focus back to virtual keyboard.
		if (!FSlateApplication::Get().SetUserFocus(UserIndex, GetPreferredFocusEntryWidget().Pin())) {
			UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Could not restore focus to virtual keyboard."));
		}
		// Restore original input device, since we just sent a key of any device.
		Processor->SetCurrentInputDevice(CurInputDevice, UserIndex);
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Could not focus widget which the virtual keyboard wants to send input to."));
	}
}

void UVirtualKeyboardWidget::ProcessKeyboardTextKeyWidget(UVirtualKeyboardTextKeyWidget* InKeyboardTextKeyWidget) {
	if (IsValid(InKeyboardTextKeyWidget)) {
		ProcessInputText(InKeyboardTextKeyWidget->GetUseInputTextShift() 
			? InKeyboardTextKeyWidget->GetInputTextShift() 
			: InKeyboardTextKeyWidget->GetInputText()
		);
	}
}

void UVirtualKeyboardWidget::ProcessInputText(const FString& InText) {
	if (!IsValid(GetWidgetToReceiveInput()) || InText.IsEmpty()) {
		// Not yet registered, or we have nothing to send input to.
		return;
	}

	// For sending input text directly to a target

	// Note to self: I tested a few approaches which failed.
	// 1. Use simulate input key by FKey: Because a physical key is not pressed, some OS key processing was bypassed. 
	//		This caused a key R to remain R even if caps / shift was not on. Slate does not deal with this and doesn't use FCharacterEvent on editable text.
	//		Some keys missing on FKey entirely: < > # ? etc. and string injections not possible.
	// 2. Copy over a preconfigured text to UEditableTextBox: Broken on EPIC's part on all sides. Doesn't check input validity. Doesn't show text on widget. etc.
	// https://forums.unrealengine.com/t/bugs-bugs-bugs-slate-widget-synchronization/738172
	// https://forums.unrealengine.com/t/ue5-2-1-bug-report-editabletextbox-broke-further-gettext-not-returning-set-value/1213966
	// https://forums.unrealengine.com/t/ue5-1-1-ueditabletextbox-settext-reverts-to-previous-text-cant-set-text/1180739
	//
	// What does work is to configure a FString on a virtual keyboard button that we want to inject on Editable Text, 
	// then focus the editable text, then simulate a char press, char by char for the FString.

	const FUIAdditionsPluginModule& UIAdditionsPluginModule = FModuleManager::GetModuleChecked<FUIAdditionsPluginModule>(TEXT("UIAdditionsPlugin"));
	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& Processor = UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor();
	if (!Processor.IsValid()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid FDetectCurrentInputDevicePreProcessor."));
		return;
	}

	const int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	const EInputDevices CurInputDevice = Processor->GetCurrentInputDevice(UserIndex);

	for (TCHAR CharX : InText) {
		FCharacterEvent CharEvent = FCharacterEvent(CharX, FModifierKeysState(), UserIndex, false);

		// Focus to widget which we want to receive input.
		if (FSlateApplication::Get().SetUserFocus(UserIndex, GetWidgetToReceiveInput()->TakeWidget())) {
			// Process input.
			bIsSimulatingInput = true;
			FSlateApplication::Get().ProcessKeyCharEvent(CharEvent);
			bIsSimulatingInput = false;
			// Focus back to virtual keyboard.
			if (!FSlateApplication::Get().SetUserFocus(UserIndex, GetPreferredFocusEntryWidget().Pin())) {
				UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Could not restore focus to virtual keyboard."));
			}
			// Restore original input device, since we just sent a key of any device.
			Processor->SetCurrentInputDevice(CurInputDevice, UserIndex);
		}
		else {
			UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Could not focus widget which the virtual keyboard wants to send input to."));
		}
	}
}

// Delegates

void UVirtualKeyboardWidget::ActOnKeyboardCapsButtonReleased(UButtonWidget* InButtonWidget) {
	// Toggle caps modifier.
	SetIsCapsModifierEnabled(!GetIsCapsModifierEnabled());
}

void UVirtualKeyboardWidget::ActOnKeyboardKeyWidgetButtonPressed(UButtonWidget* InButtonWidget) {
	ProcessKeyboardKeyWidget(Cast<UVirtualKeyboardKeyWidget>(InButtonWidget), true);
}

void UVirtualKeyboardWidget::ActOnKeyboardKeyWidgetButtonReleased(UButtonWidget* InButtonWidget) {
	ProcessKeyboardKeyWidget(Cast<UVirtualKeyboardKeyWidget>(InButtonWidget), false);
}

void UVirtualKeyboardWidget::ActOnKeyboardTextKeyWidgetButtonPressed(UButtonWidget* InButtonWidget) {
	ProcessKeyboardTextKeyWidget(Cast<UVirtualKeyboardTextKeyWidget>(InButtonWidget));
}

void UVirtualKeyboardWidget::ActOnKeyboardTextKeyWidgetButtonReleased(UButtonWidget* InButtonWidget) {
	ProcessKeyboardTextKeyWidget(Cast<UVirtualKeyboardTextKeyWidget>(InButtonWidget));
}
