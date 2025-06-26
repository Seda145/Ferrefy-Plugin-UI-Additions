/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "VirtualKeyboardKeyWidget.h"
#include "VirtualKeyboardWidget.h"
#include "SlateUtils.h"
#include "LogUIAdditionsPlugin.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPtr.h"
#include "UObject/ConstructorHelpers.h"


// Setup

UVirtualKeyboardKeyWidget::UVirtualKeyboardKeyWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {


	ConstructorHelpers::FObjectFinderOptional<UDataTable> Obj(TEXT("/UIAdditionsPlugin/Data/DataTables/UI/Input/DT_InputActionButtonKeyImages.DT_InputActionButtonKeyImages"));
	if (Obj.Succeeded()) {
		KeyBindingImageDT.DataTable = Obj.Get();
		KeyBindingImageDT.RowName = TEXT("Default");
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Could not set a default datatable for KeyBindingImageDT."));
	}
}

void UVirtualKeyboardKeyWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();

	RegisterToAncestorVirtualKeyboardWidget();
}

void UVirtualKeyboardKeyWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	UpdateInputKeyTexture();
}

void UVirtualKeyboardKeyWidget::RegisterToAncestorVirtualKeyboardWidget() {
	UVirtualKeyboardWidget* TheKeyboardWidget = Cast<UVirtualKeyboardWidget>(USlateUtils::FindAncestorWidgetByClass<UVirtualKeyboardWidget>(this));
	if (!IsValid(TheKeyboardWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("UVirtualKeyboardKeyWidget using key: %s has not found a valid ancestor UVirtualKeyboardWidget"), *GetInputKey().ToString());
		return;
	}

	// This is a one time setup to set up delegate bindings.
	TheKeyboardWidget->RegisterKeyboardKeyWidget(this);
}

// Input

FKey UVirtualKeyboardKeyWidget::GetInputKey() const {
	return InputKey;
}

void UVirtualKeyboardKeyWidget::SetInputKey(const FKey& InInputKey) {
	InputKey = InInputKey;
	UpdateInputKeyTexture();
}

bool UVirtualKeyboardKeyWidget::GetTriggerOnKeyPressed() const {
	return bTriggerOnKeyPressed;
}

bool UVirtualKeyboardKeyWidget::GetTriggerOnKeyReleased() const {
	return bTriggerOnKeyReleased;
}

// Widgets 

UImage* UVirtualKeyboardKeyWidget::GetInputKeyImageWidget() const {
	return InputKeyImageWidget;
}

// Appearance

const FDataTableRowHandle& UVirtualKeyboardKeyWidget::GetKeyBindingImageDT() const {
	return KeyBindingImageDT;
}

void UVirtualKeyboardKeyWidget::UpdateInputKeyTexture() {
	if (!IsValid(GetInputKeyImageWidget())) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("GetInputKeyImageWidget() is invalid"));
		return;
	}
	const FS_InputKeyImageData* DataPtr = GetKeyBindingImageDT().GetRow<FS_InputKeyImageData>(CUR_LOG_CONTEXT);
	if (DataPtr == nullptr) {
		// The point of this widget is to display an available action with a key image. If we can't, show an error.
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("KeyBindingImageDT is invalid"));
		return;
	}

	const TSoftObjectPtr<UTexture2D>* TexPtr = DataPtr->KeyTextures.Find(GetInputKey());
	if (TexPtr != nullptr && !TexPtr->IsNull()) {
		GetInputKeyImageWidget()->SetBrushFromSoftTexture(*TexPtr, true);
		GetInputKeyImageWidget()->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else {
		// If we found nothing at all, better null the texture and hide the widget. Otherwise we can end up with an old key texture.
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Can't find a texture for key: %s"), *GetInputKey().ToString());
		GetInputKeyImageWidget()->SetBrushFromTexture(nullptr);
		GetInputKeyImageWidget()->SetVisibility(ESlateVisibility::Collapsed);
	}
}