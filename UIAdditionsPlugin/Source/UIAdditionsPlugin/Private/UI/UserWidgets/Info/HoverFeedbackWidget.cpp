/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "HoverFeedbackWidget.h"
#include "UIAdditionsPlugin.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "Modules/ModuleManager.h"
#include "Components/NamedSlot.h"
#include "LogUIAdditionsPlugin.h"


// Setup

void UHoverFeedbackWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	check(IsValid(ContentSlot));
	const FUIAdditionsPluginModule& UIAdditionsPluginModule = FModuleManager::GetModuleChecked<FUIAdditionsPluginModule>(TEXT("UIAdditionsPlugin"));
	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& Processor = UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor();
	check(Processor.IsValid());
	Processor->OnInputDeviceChanged.AddUObject(this, &UHoverFeedbackWidget::ActOnInputDeviceChanged);
	const int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	ActOnInputDeviceChanged(USlateUtils::GetCurrentInputDevice(UserIndex), UserIndex);
}

// Input

bool UHoverFeedbackWidget::GetIsMouseEntered() const {
	return bIsMouseEntered;
}

// Focus

bool UHoverFeedbackWidget::GetIsOnFocusPath() const {
	return bIsOnFocusPath;
}

// Appearance

void UHoverFeedbackWidget::SetIsEnabled(bool bInIsEnabled) {
	Super::SetIsEnabled(bInIsEnabled);

	UpdateFeedbackAppearance();
}

void UHoverFeedbackWidget::UpdateFeedbackAppearance() {
	//if (!GetIsEnabled()) {
		/**
		* I chose not to AppearDisabled here for the following reasons:
		* - The engine makes the disabled feedback widget slightly transparent when disabling already.
		* - In normal use a UserWidget wrapping around this feedback widget usually handles focus / hover / input events, and feedback can still be given even if certain functionality and inner widgets are disabled. This can be useful, as for UI navigation reliability I like that the wrapping UserWidget can still be used for navigation + we can use the feedback widget + it gets a "disabled" transparancy out of the box. If you were to lock the feedback widget to a disabled appearance, you would navigate blindly.
		*/
		//AppearDisabled();
		//return;
	//}

	if (GetAppearHoveredByDefault()
		|| (!GetIsUsingHoverAppearanceForFocus() && GetIsMouseEntered())
		|| (GetIsUsingHoverAppearanceForFocus() && GetIsOnFocusPath())
		) {
		AppearHovered();
	}
	else {
		AppearNormal();
	}

	// Not only useful to update appearance elsewhere
	// , but also useful for another to be notified when this widget is on / left the focus path.
	OnHoverFeedbackAppearanceChanged.Broadcast();
}

bool UHoverFeedbackWidget::GetIsUsingHoverAppearanceForFocus() const {
	return bIsUsingHoverAppearanceForFocus;
}

bool UHoverFeedbackWidget::GetAppearHoveredByDefault() const {
	return bAppearHoveredByDefault;
}

void UHoverFeedbackWidget::SetAppearHoveredByDefault(bool bInAppearHoveredByDefault) {
	bAppearHoveredByDefault = bInAppearHoveredByDefault;
	UpdateFeedbackAppearance();
}

// Navigation

void UHoverFeedbackWidget::NativeOnAddedToFocusPath(const FFocusEvent& InFocusEvent) {
	Super::NativeOnAddedToFocusPath(InFocusEvent);
	NotifyAppearanceOnFocusPath();
}

void UHoverFeedbackWidget::NativeOnRemovedFromFocusPath(const FFocusEvent& InFocusEvent) {
	Super::NativeOnRemovedFromFocusPath(InFocusEvent);
	NotifyAppearanceLeftFocusPath();
}

void UHoverFeedbackWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	NotifyAppearanceMouseEntered();
}

void UHoverFeedbackWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent) {
	Super::NativeOnMouseLeave(InMouseEvent);
	NotifyAppearanceMouseLeave();
}

void UHoverFeedbackWidget::NotifyAppearanceOnFocusPath() {
	bIsOnFocusPath = true;
	UpdateFeedbackAppearance();
}

void UHoverFeedbackWidget::NotifyAppearanceLeftFocusPath() {
	bIsOnFocusPath = false;
	UpdateFeedbackAppearance();
}

void UHoverFeedbackWidget::NotifyAppearanceMouseEntered() {
	bIsMouseEntered = true;
	UpdateFeedbackAppearance();
}

void UHoverFeedbackWidget::NotifyAppearanceMouseLeave() {
	bIsMouseEntered = false;
	UpdateFeedbackAppearance();
}

// Delegates

void UHoverFeedbackWidget::ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex) {
	const int32 UserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	if (UserIndex == INDEX_NONE || UserIndex != InSlateUserIndex) {
		// Invalid or another player.
		return;
	}
	// TODO does this work well with analog cursor?
	bIsUsingHoverAppearanceForFocus = USlateUtils::GetCurrentInputDevice(UserIndex) != EInputDevices::Mouse;
	UpdateFeedbackAppearance();
}

