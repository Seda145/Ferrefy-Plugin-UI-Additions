/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "VirtualKeyboardTextKeyWidget.h"
#include "VirtualKeyboardWidget.h"
#include "SlateUtils.h"
#include "LogUIAdditionsPlugin.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"


// Setup

void UVirtualKeyboardTextKeyWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	RegisterToAncestorVirtualKeyboardWidget();
}

void UVirtualKeyboardTextKeyWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	UpdateInputTextTexture();
}

void UVirtualKeyboardTextKeyWidget::RegisterToAncestorVirtualKeyboardWidget() {
	UVirtualKeyboardWidget* TheKeyboardWidget = Cast<UVirtualKeyboardWidget>(USlateUtils::FindAncestorWidgetByClass<UVirtualKeyboardWidget>(this));
	if (!IsValid(TheKeyboardWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("UVirtualKeyboardTextKeyWidget using key: %s has not found a valid ancestor UVirtualKeyboardWidget"), *GetInputText());
		return;
	}

	// This is a one time setup to set up delegate bindings.
	TheKeyboardWidget->RegisterKeyboardTextKeyWidget(this);
}

// Input

FString UVirtualKeyboardTextKeyWidget::GetInputText() const {
	return InputText;
}

void UVirtualKeyboardTextKeyWidget::SetInputText(const FString& InInputText) {
	InputText = InInputText;
}

bool UVirtualKeyboardTextKeyWidget::GetTriggerOnKeyPressed() const {
	return bTriggerOnKeyPressed;
}

bool UVirtualKeyboardTextKeyWidget::GetTriggerOnKeyReleased() const {
	return bTriggerOnKeyReleased;
}

FString UVirtualKeyboardTextKeyWidget::GetInputTextShift() const {
	return InputTextShift;
}

void UVirtualKeyboardTextKeyWidget::SetInputTextShift(const FString& InInputTextShift) {
	InputTextShift = InInputTextShift;
}

bool UVirtualKeyboardTextKeyWidget::GetUseInputTextShift() const {
	return bUseInputTextShift;
}

void UVirtualKeyboardTextKeyWidget::SetUseInputTextShift(bool bInUseInputTextShift) {
	bUseInputTextShift = bInUseInputTextShift;
	UpdateInputTextTexture();
}

// Widgets 

UImage* UVirtualKeyboardTextKeyWidget::GetInputTextImageWidget() const {
	return InputTextImageWidget;
}

// Appearance

void UVirtualKeyboardTextKeyWidget::UpdateInputTextTexture() {
	if (!IsValid(GetInputTextImageWidget())) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("GetInputTextImageWidget() is invalid"));
		return;
	}

	UTexture2D* TexPtr = GetUseInputTextShift() ? GetInputKeyShiftTexture() : GetInputKeyTexture();
	if (TexPtr != nullptr) {
		GetInputTextImageWidget()->SetBrushFromTexture(TexPtr, true);
		GetInputTextImageWidget()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else {
		// If we found nothing at all, better null the texture and hide the widget. Otherwise we can end up with an old key texture.
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("A texture has not been configured for input text: %s"), *FString(GetUseInputTextShift() ? GetInputTextShift() : GetInputText()));
		GetInputTextImageWidget()->SetBrushFromTexture(nullptr);
		GetInputTextImageWidget()->SetVisibility(ESlateVisibility::Collapsed);
	}
}

UTexture2D* UVirtualKeyboardTextKeyWidget::GetInputKeyTexture() const {
	return InputKeyTexture;
}

void UVirtualKeyboardTextKeyWidget::SetInputKeyTexture(UTexture2D* InInputKeyTexture) {
	InputKeyTexture = InInputKeyTexture;
	UpdateInputTextTexture();
}

UTexture2D* UVirtualKeyboardTextKeyWidget::GetInputKeyShiftTexture() const {
	return InputKeyShiftTexture;
}

void UVirtualKeyboardTextKeyWidget::SetInputKeyShiftTexture(UTexture2D* InInputKeyShiftTexture) {
	InputKeyShiftTexture = InInputKeyShiftTexture;
	UpdateInputTextTexture();
}