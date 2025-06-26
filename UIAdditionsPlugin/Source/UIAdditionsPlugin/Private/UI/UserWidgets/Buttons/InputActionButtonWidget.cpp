/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "InputActionButtonWidget.h"
#include "UIAdditionsPlugin.h"
#include "LogUIAdditionsPlugin.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"
#include "GameFramework/InputSettings.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetInputLibrary.h"
#include "Engine/LocalPlayer.h"
#include "SlateUtils.h"
#include "LocalizationUtils.h"
#include "Modules/ModuleManager.h"
#include "InputCoreTypes.h"
#include "Internationalization/StringTable.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "CorePlayerInput.h"


// Setup

UInputActionButtonWidget::UInputActionButtonWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	
	// People are familiar with having this type of widget in a bottom corner of the screen, 
	// a place which is often clickable but not easily (or not chosen to be) navigatable by non cursor devices.
	// This is why I turn this off.
	bFocusButtonWidgetOnMouseEnter = false;

	{
		ConstructorHelpers::FObjectFinderOptional<UDataTable> Obj(TEXT("/UIAdditionsPlugin/Data/DataTables/UI/Input/DT_InputActionButtonKeyImages.DT_InputActionButtonKeyImages"));
		if (Obj.Succeeded()) {
			KeyBindingImageDT.DataTable = Obj.Get();
			KeyBindingImageDT.RowName = TEXT("Default");
		}
		else {
			UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Could not set a default datatable for KeyBindingImageDT."));
		}
	}

	{
		ConstructorHelpers::FObjectFinderOptional<UStringTable> Obj(TEXT("/UIAdditionsPlugin/Localization/Text/ST_HID.ST_HID"));
		if (Obj.Succeeded()) {
			ActionMappingLocalizationStringTable = Obj.Get();
		}
		else {
			UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Could not set a default string table for ActionMappingLocalizationStringTable."));
		}
	}
}

void UInputActionButtonWidget::NativePreConstruct() {
	Super::NativePreConstruct();

	UpdateText();
	UpdateKeyBindingTexture(USlateUtils::GetCurrentInputDevice(GetOwningPlayer()));
}

// Appearance

const FDataTableRowHandle& UInputActionButtonWidget::GetKeyBindingImageDT() const {
	return KeyBindingImageDT;
}

UStringTable* UInputActionButtonWidget::GetActionMappingLocalizationStringTable() const {
	return ActionMappingLocalizationStringTable;
}

void UInputActionButtonWidget::SetActionMappingLocalizationStringTable(UStringTable* InStringTable) {
	ActionMappingLocalizationStringTable = InStringTable;
	UpdateText();
	UObject* Test = nullptr;
	UStringTable* TestB = Cast<UStringTable>(Test);
}

void UInputActionButtonWidget::UpdateKeyBindingTexture(EInputDevices InInputDevice) {
	const FS_InputKeyImageData* DataPtr = GetKeyBindingImageDT().GetRow<FS_InputKeyImageData>(CUR_LOG_CONTEXT);
	if (DataPtr == nullptr) {
		// The point of this widget is to display an available action with a key image. If we can't, show an error.
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("KeyBindingImageDT is invalid"));
		return;
	}
	if (!IsValid(KeyImageWidget)) {
		return;
	}

	TArray<FInputActionKeyMapping> ActionKeyMappings;

	const APlayerController* PC = GetOwningPlayer();
	const UPlayerInput* Pi = IsValid(PC) ? PC->PlayerInput : nullptr;
	// Player input is required to test against input mappings.
	if (IsValid(Pi)) {
		ActionKeyMappings = Pi->GetKeysForAction(ActionMappingName);
	}
	else {
		// In situations like editor usage there might not be a controller / player input to get info from.
		// Could use this as alternative. Player Input was used in preparation for per player keybindings.
		const UInputSettings* InputSettings = UInputSettings::GetInputSettings();
		if (IsValid(InputSettings)) {
			InputSettings->GetActionMappingByName(ActionMappingName, ActionKeyMappings);
		}
	}

	TSet<FKey> PotentialKeys;
	for (FInputActionKeyMapping MappingX : ActionKeyMappings) {
		PotentialKeys.Add(MappingX.Key);
	}
	TMap<EInputDevices, TSoftObjectPtr<UTexture2D>> PotentialTextures;

	for (FKey KeyX : PotentialKeys) {
		const TSoftObjectPtr<UTexture2D>* TexPtr = DataPtr->KeyTextures.Find(KeyX);
		if (!TexPtr) {
			continue;
		}

		if (KeyX.IsGamepadKey()) {
			PotentialTextures.Add(EInputDevices::Gamepad, *TexPtr);
		}
		else if (KeyX.IsMouseButton()) {
			PotentialTextures.Add(EInputDevices::Mouse, *TexPtr);
		}
		else if (UKismetInputLibrary::Key_IsKeyboardKey(KeyX)) {
			PotentialTextures.Add(EInputDevices::Keyboard, *TexPtr);
		}
		else {
			UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("This device is not implemented"));
		}
	}

	// We can assume that in some cases we can use a texture for a different input device, if the actual device has none configured.
	// This makes sense when using both the keyboard and mouse but you want to display only the keyboard texture. 
	// The fallback will be the keyboard texture or nullptr if not configured. 
	const TSoftObjectPtr<UTexture2D>* TexPtr = PotentialTextures.Find(InInputDevice);
	if (TexPtr == nullptr) {
		TexPtr = PotentialTextures.Find(EInputDevices::Keyboard);
	}
	
	if (TexPtr != nullptr && !TexPtr->IsNull()) {
		KeyImageWidget->SetBrushFromSoftTexture(*TexPtr, true);
		KeyImageWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	else {
		// If we found nothing at all, better null the texture and hide the widget. Otherwise we can end up with an old key texture.
		KeyImageWidget->SetBrushFromTexture(nullptr);
		KeyImageWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInputActionButtonWidget::SetOverrideText(const FText& InText) {
	OverrideText = InText;
	UpdateText();
}

void UInputActionButtonWidget::UpdateText() {
	if (IsValid(KeyTextWidget)) {
		if (!OverrideText.IsEmptyOrWhitespace()) {
			KeyTextWidget->SetText(OverrideText);
		}
		else {
			if (IsValid(GetActionMappingLocalizationStringTable())) {
				KeyTextWidget->SetText(ULocalizationUtils::LocalizeInputMappingName(GetActionMappingLocalizationStringTable()->GetStringTableId(), ActionMappingName));
			}
			else {
				KeyTextWidget->SetText(FText());
				UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Override text is unused (empty) and the string table used for localization is not valid."));
			}
		}
	}
}

// Navigation

void UInputActionButtonWidget::SimulateKeyEventOnPressOrRelease(bool bInPressed) {
	const int32 SlateUserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
	if (SlateUserIndex == INDEX_NONE) {
		return;
	}
	const APlayerController* PC = GetOwningPlayer();
	const UPlayerInput* Pi = IsValid(PC) ? PC->PlayerInput : nullptr;
	// Player input is required to test against input mappings.
	if (!IsValid(Pi)) {
		return;
	}

	/**
	 * Outmappings will contain multiple keys for one action, so make sure only to execute once.
	 * It doesn't matter what key is executed as long as it belongs to the same input action. The shown key texture is just a visual.
	 * Try to find a key for the current input device. If not found, fallback to keyboard. If not found then, fallback to first key.
	 * This is so that we don't unintentionally cause a detection of a change in the currently used input device.
	*/
	const TArray<FInputActionKeyMapping> OutMappings = Pi->GetKeysForAction(ActionMappingName);
	const EInputDevices CurInputDevice = USlateUtils::GetCurrentInputDevice(GetOwningPlayer());
	FKey KeyToSimulate = EKeys::Invalid;
	const FInputActionKeyMapping* PossibleMapping = nullptr;
	if (OutMappings.Num() > 0) {
		// Todo there are checks for gamepad and keyboard and mouse keys, but not for other devices??
		switch (CurInputDevice) {
		case(EInputDevices::Gamepad):
			PossibleMapping = OutMappings.FindByPredicate([&CurInputDevice](const FInputActionKeyMapping& MappingX){ return MappingX.Key.IsGamepadKey(); });
			break;
		case(EInputDevices::Mouse):
			PossibleMapping = OutMappings.FindByPredicate([&CurInputDevice](const FInputActionKeyMapping& MappingX){ return MappingX.Key.IsMouseButton(); });
			break;
		}
		if (PossibleMapping == nullptr) {
			// We either are a keyboard or want to fallback to a keyboard.
			PossibleMapping = OutMappings.FindByPredicate([&CurInputDevice](const FInputActionKeyMapping& MappingX){ return UKismetInputLibrary::Key_IsKeyboardKey(MappingX.Key); });
			if (CurInputDevice != EInputDevices::Keyboard) {
				if (PossibleMapping != nullptr) {
					UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Using a keyboard key as a fallback, because no key was retrieved for this action for the current HID."));
				}
			}
		}
		if (PossibleMapping == nullptr) {
			// If all fails just get the first key possible. This will make the DetectCurrentInputDevicePreProcessor detect another HID.
			UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("The key could not be retrieved for this specific input device, so we get the first key available. This will cause a change in what HID is detected to be currently in use."));
			PossibleMapping = &OutMappings[0];
		}
	}
	if (PossibleMapping != nullptr) {
		KeyToSimulate = PossibleMapping->Key;
	}
	
	if (KeyToSimulate != EKeys::Invalid) {
		USlateUtils::ProcessKeyEvent(SlateUserIndex, KeyToSimulate, bInPressed, false);
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("The key could not be retrieved, so no key event can be processed."));
	}	
}

// Delegates

void UInputActionButtonWidget::ActOnButtonPressed() {
	Super::ActOnButtonPressed();
	
	SimulateKeyEventOnPressOrRelease(true);
}

void UInputActionButtonWidget::ActOnButtonReleased() {
	Super::ActOnButtonReleased();

	SimulateKeyEventOnPressOrRelease(false);
}

void UInputActionButtonWidget::ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex) {
	Super::ActOnInputDeviceChanged(InInputDevice, InSlateUserIndex);

	if (USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer()) != InSlateUserIndex) {
		// Ignore HID change of another player
		return;
	}

	UpdateKeyBindingTexture(InInputDevice);
}

