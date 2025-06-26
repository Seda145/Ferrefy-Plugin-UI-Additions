/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "ButtonWidget.h"
#include "CentralButton.h"
#include "Framework/Application/SlateApplication.h"
#include "SlateUtils.h"
#include "UIAdditionsPlugin.h"
#include "Modules/ModuleManager.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "LogUIAdditionsPlugin.h"
#include "HUDCore.h"


// Setup

void UButtonWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	check(IsValid(ButtonWidget));
	// OnPressed is always broadcasted. The OnClicked seems to be an additional delegate and Touch doesn't have a delegate. Just use OnPressed for everything.
	// The SButton widget holds a property which decides when the OnPressed is activated (down / up etc.).
	GetButtonWidget()->OnPressed.AddDynamic(this, &UButtonWidget::ActOnButtonPressed);
	GetButtonWidget()->OnReleased.AddDynamic(this, &UButtonWidget::ActOnButtonReleased);
	GetButtonWidget()->OnHovered.AddDynamic(this, &UButtonWidget::ActOnButtonHovered);
	GetButtonWidget()->OnUnhovered.AddDynamic(this, &UButtonWidget::ActOnButtonUnhovered);

	const FUIAdditionsPluginModule& UIAdditionsPluginModule = FModuleManager::GetModuleChecked<FUIAdditionsPluginModule>(TEXT("UIAdditionsPlugin"));
	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& Processor = UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor();
	check(Processor.IsValid());
	Processor->OnInputDeviceChanged.AddUObject(this, &UButtonWidget::ActOnInputDeviceChanged);
	const int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	ActOnInputDeviceChanged(USlateUtils::GetCurrentInputDevice(UserIndex), UserIndex);
}

// Navigation

void UButtonWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	
	if (GetFocusButtonWidgetOnMouseEnter() && IsValid(GetButtonWidget()) && GetButtonWidget()->GetIsFocusable()) {
		const int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
		const EInputDevices CurDevice = USlateUtils::GetCurrentInputDevice(UserIndex);

		const AHUDCore* HUD = IsValid(GetOwningPlayer()) ? Cast<AHUDCore>(GetOwningPlayer()->GetHUD()) : nullptr;
		const bool bIsUsingCursor = CurDevice == EInputDevices::Mouse || (IsValid(HUD) && HUD->IsAnalogCursorValid());
		// Checking if currently using a cursor prevents cursor hijacking focus when mouse enters a widget because a keyboard key is scrolling a list.
		if (bIsUsingCursor) {
			// If this is not done, engine behavior is that a setting widget other than the one hovered on can remain focused.
			// At times this is desired (a clickable / hoverable widget is not always then desired to be focused by gamepad etc.).
			// 
			// With the average menu navigation buttons I find it extremely likely that focus on hover behavior is desired
			// , else turn off FocusButtonWidgetOnMouseEnter on the instance / class / editor panel.
			// 
			// This provides a more seamless experience when switching input device.
			// Setting focus on hover solves the following situations:
			// - A UI key navigation navigates from a widget other than the one hovered.
			// - NativeOnMouseEnter not triggering when steps: 1. hovering A, 2. navigating with keys to widget B. 3. moving the mouse while still hovering A.
			GetButtonWidget()->SetFocus();
		}
	}
}

// Widgets

UCentralButton* UButtonWidget::GetButtonWidget() const {
	return ButtonWidget;
}

// Appearance

bool UButtonWidget::GetAppearHovered() const {
	return bAppearHovered;
}
    
void UButtonWidget::SetAppearHovered(bool bInAppearHovered, bool bInPlaySound) {
	if (GetAppearHovered() == bInAppearHovered) {
		return;
	}

	bAppearHovered = bInAppearHovered;
	UpdateButtonInteractionStateAppearance(bInPlaySound);
}

bool UButtonWidget::GetAppearPressed() const {
	return bAppearPressed;
}

void UButtonWidget::SetAppearPressed(bool bInAppearPressed, bool bInPlaySound) {
	if (GetAppearPressed() == bInAppearPressed) {
		return;
	}
	
	bAppearPressed = bInAppearPressed;
	UpdateButtonInteractionStateAppearance(bInPlaySound);
}

void UButtonWidget::UpdateButtonInteractionStateAppearance_Implementation(bool bInPlaySound) {
	if (!IsValid(GetButtonWidget())) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("%s: Invalid ButtonWidget, can't update the style."), *GetName());
		return;
	}
	const FS_CentralButtonStyle* const DTPtr = GetButtonWidget()->GetCentralStyle().GetRow<FS_CentralButtonStyle>(CUR_LOG_CONTEXT);
	if (DTPtr == nullptr) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("%s: Invalid DT pointer, can't update style."), *GetName());
		return;
	}
	
	/* 
	* Slate has no out of the box way to show a button as "hovered" when focused by a device such as gamepad, leaving us blind,
	* There is no nice way to do it with the SButton or UButton without reimplementing the entire thing, which I don't for compatibility reasons.
	* We can inject the hovered style as the normal style when not using the cursor as a hack. 
	* Note that we don't have to check if the button is disabled or enabled. This method simply swaps styles to the normal channel.
	* If the button is disabled it will already use the "disabled" channel.
	*/

	//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Appear hovered?: %s"), *GetName(), (InAppearHovered ? TEXT("True") : TEXT("False")));

	FButtonStyle StyleX = DTPtr->WidgetStyle;

	if (GetAppearPressed() || GetIsButtonPressed()) {
		StyleX.SetNormal(DTPtr->WidgetStyle.Pressed);
		StyleX.SetNormalForeground(DTPtr->WidgetStyle.PressedForeground);
		StyleX.SetNormalPadding(DTPtr->WidgetStyle.PressedPadding);
		// Let it synchronize with Slate...
		GetButtonWidget()->SetStyle(StyleX);
		if (bInPlaySound) {
			FSlateApplication::Get().PlaySound(DTPtr->WidgetStyle.PressedSlateSound);
		}
		OnAppearPressed();
	}
	else if (GetAppearHovered() 
		|| (!GetDesiresHoverAppearanceForFocus() && GetIsButtonHovered()) 
		|| (GetIsOnFocusPath() && GetDesiresHoverAppearanceForFocus())
		) {
		StyleX.SetNormal(DTPtr->WidgetStyle.Hovered);
		StyleX.SetNormalForeground(DTPtr->WidgetStyle.HoveredForeground);
		// Has no hovered padding.
		// Let it synchronize with Slate...
		GetButtonWidget()->SetStyle(StyleX);
		if (bInPlaySound) {
			FSlateApplication::Get().PlaySound(DTPtr->WidgetStyle.HoveredSlateSound);
		}
		OnAppearHovered();
	}
	else {
		GetButtonWidget()->SetStyle(DTPtr->WidgetStyle);
		OnAppearNormal();
	}
}

// Navigation

bool UButtonWidget::GetIsOnFocusPath() const {
	return bIsOnFocusPath;
}

bool UButtonWidget::GetIsButtonHovered() const {
	return bIsButtonHovered;
}

bool UButtonWidget::GetIsButtonPressed() const {
	return bIsButtonPressed;
}

bool UButtonWidget::GetDesiresHoverAppearanceForFocus() const {
	return bDesiresHoverAppearanceForFocus;
}

bool UButtonWidget::GetFocusButtonWidgetOnMouseEnter() const {
	return bFocusButtonWidgetOnMouseEnter;
}

void UButtonWidget::SetFocusButtonWidgetOnMouseEnter(bool bInFocusButtonWidgetOnMouseEnter) {
	bFocusButtonWidgetOnMouseEnter = bInFocusButtonWidgetOnMouseEnter;
}

void UButtonWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) {
	Super::NativeOnAddedToFocusPath(InFocusEvent);

	bIsOnFocusPath = true;
	UpdateButtonInteractionStateAppearance(true);
}

void UButtonWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) {
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);

	bIsOnFocusPath = false;
	UpdateButtonInteractionStateAppearance(true);
}

void UButtonWidget::SetIsEnabled(bool bInIsEnabled) {
	Super::SetIsEnabled(bInIsEnabled);

	if (!IsValid(GetButtonWidget())) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("%s: Invalid GetButtonWidget(), can't update its enabled state."), *GetName());
		return;
	}

	GetButtonWidget()->SetIsEnabled(GetIsEnabled());
}

// Delegates

void UButtonWidget::ActOnButtonPressed() {
	bIsButtonPressed = true;
	// Sound is already handled by the inner button.
	UpdateButtonInteractionStateAppearance(false);
	OnButtonPressed.Broadcast(this);
}

void UButtonWidget::ActOnButtonReleased() {
	bIsButtonPressed = false;
	// Sound is already handled by the inner button.
	UpdateButtonInteractionStateAppearance(false);
	OnButtonReleased.Broadcast(this);
}

void UButtonWidget::ActOnButtonHovered() {
	bIsButtonHovered = true;
	// Sound is already handled by the inner button.
	UpdateButtonInteractionStateAppearance(false);
	OnButtonHovered.Broadcast(this);
}

void UButtonWidget::ActOnButtonUnhovered() {
	bIsButtonHovered = false;
	// Sound is already handled by the inner button.
	UpdateButtonInteractionStateAppearance(false);
	OnButtonUnhovered.Broadcast(this);
}

void UButtonWidget::ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex) {
	const int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	if (UserIndex == INDEX_NONE || UserIndex != InSlateUserIndex) {
		// Invalid or another player.
		return;
	}

	bDesiresHoverAppearanceForFocus = USlateUtils::GetCurrentInputDevice(UserIndex) != EInputDevices::Mouse;
	UpdateButtonInteractionStateAppearance(true);

	if (GetFocusButtonWidgetOnMouseEnter()) {
		if (InInputDevice == EInputDevices::Mouse && IsHovered()) {
			if (IsValid(GetButtonWidget()) && GetButtonWidget()->GetIsFocusable()) {
				// See why SetFocus is done on NativeOnMouseEnter.
				// Reason I also call this on ActOnInputDeviceChanged is that it is possible to switch to a currently hovering cursor device without triggering MouseEnter.
				GetButtonWidget()->SetFocus();
			}
		}
	}
}
