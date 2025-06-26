/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "KeyboundUserWidget.h"
#include "GameFramework/InputSettings.h"
#include "SlateUtils.h"
#include "LogUIAdditionsPlugin.h"
#include "Framework/Commands/InputBindingManager.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Commands/UICommandInfo.h"
#include "InputCoreTypes.h"
#include "GameFramework/PlayerController.h"
#include "CorePlayerInput.h"


// Input

FReply UKeyboundUserWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	const FReply Reply = FindAndRequestInputAction(USlateUtils::GetInputChordFromKeyEvent(InKeyEvent), true);
	return (Reply.IsEventHandled() ? Reply : Super::NativeOnKeyDown(InGeometry, InKeyEvent));
}

FReply UKeyboundUserWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) {
	const FReply Reply = FindAndRequestInputAction(USlateUtils::GetInputChordFromKeyEvent(InKeyEvent), false);
	return (Reply.IsEventHandled() ? Reply : Super::NativeOnKeyUp(InGeometry, InKeyEvent));
}

FReply UKeyboundUserWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	const FReply Reply = FindAndRequestInputAction(USlateUtils::GetInputChordFromPointerEvent(InMouseEvent, false), true);
	return (Reply.IsEventHandled() ? Reply : Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent));
}

FReply UKeyboundUserWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	const FReply Reply = FindAndRequestInputAction(USlateUtils::GetInputChordFromPointerEvent(InMouseEvent, false), false);
	return (Reply.IsEventHandled() ? Reply : Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent));
}

FReply UKeyboundUserWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	const FReply Reply = FindAndRequestInputAction(USlateUtils::GetInputChordFromPointerEvent(InMouseEvent, true), true);
	return (Reply.IsEventHandled() ? Reply : Super::NativeOnMouseWheel(InGeometry, InMouseEvent));
}

FReply UKeyboundUserWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) {
	const FReply Reply = FindAndRequestInputAction(USlateUtils::GetInputChordFromPointerEvent(InMouseEvent, false), true);
	return (Reply.IsEventHandled() ? Reply : Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent));
}

// KeyBinding

FReply UKeyboundUserWidget::FindAndRequestInputAction(const FInputChord& InInputChord, bool bInIsKeyDown) {
    FReply Reply = FReply::Unhandled();

	const APlayerController* PC = GetOwningPlayer();
	const UPlayerInput* Pi = IsValid(PC) ? PC->PlayerInput : nullptr;
	// Player input is required to test against input mappings, else we can't know for sure if to handle or not.
	check(IsValid(Pi));

//#if WITH_EDITOR
//
// // Below is a useful utility to respect editor actions at all times, but I don't use this by default anymore. Instead I respect how widgets handle things even if they block actions like "StopPlaySession".
// 
//	auto WantToDoInEditor = [&InInputChord](const FName& ContextX, const FName& CommandX) -> bool {
//		TSharedPtr<FUICommandInfo> InfoX = FInputBindingManager::Get().FindCommandInContext(ContextX, CommandX);
//		if (InfoX.IsValid() && InfoX->HasActiveChord(InInputChord)) {
//			UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Processing input %s which is also a specific editor action. Returns unhandled. UI command: %s, in context: %s"), *InInputChord.Key.ToString(), *CommandX.ToString(), *ContextX.ToString());
//			return false;
//		}
//		return true;
//	};
//
//	// Some engine weirdness in UE5.2:
//  // - Editor actions registered to a key and then also a null key.
//	// - Input events sending invalid keys such as mouse scroll (Pointer) event.
// 	// .. Just avoid anything unexplainable.
//	if (InInputChord.Key.IsValid()) {
//		// Just useful, devs must be able to stop play in the editor at any time and not be blocked by some widget.
//		if (!WantToDoInEditor(TEXT("PlayWorld"), TEXT("StopPlaySession"))) {
//			return Reply;
//		}
//	}
//
//#endif // WITH_EDITOR

	// Process the actions we are listening for.
	for (const TPair<FName, FS_WidgetInputActionListener>& PairX : ListenForRoutedInputActions) {
		const TArray<FInputActionKeyMapping> OutMappings = Pi->GetKeysForAction(PairX.Key);
		// Find the first matching input mapping matching the action name.
		for (const FInputActionKeyMapping& MappingX : OutMappings) {
			if (InInputChord == FInputChord(MappingX.Key, MappingX.bShift, MappingX.bCtrl, MappingX.bAlt, MappingX.bCmd)) {
				// Find the first action matching this coord and process the action only once.
				if (ProcessInputActionByName(PairX.Key, bInIsKeyDown).IsEventHandled()) {
					// We don't return here because multiple actions could be bound to the mapping.
					Reply = FReply::Handled();
				}
				break;
			}
		}
	}

	if (!Reply.IsEventHandled()) {
		bool bAdditionalDesiresHandled = (
			// We can default to false if we want to handle no additional input unless if it's in AdditionalInputActionReplies.
			// We can default to true if we want to handle any additional input unless if it's in AdditionalInputActionReplies.
			AdditionalInputActionReplyMode == E_AdditionalInputActionReplyModes::HandleSpecified
			? false
			: true
		);

		FString ActionNameToHandle = "";
		for (const FName& ActionX : AdditionalInputActionReplies) {
			const TArray<FInputActionKeyMapping> OutMappings = Pi->GetKeysForAction(ActionX);

			// Check if the action mappings contain ActionX by the provided input chord, so that we know if we are processing ActionX or not.
			const bool bFoundMapping = OutMappings.ContainsByPredicate([&InInputChord](const FInputActionKeyMapping& MappingX) { return InInputChord == FInputChord(MappingX.Key, MappingX.bShift, MappingX.bCtrl, MappingX.bAlt, MappingX.bCmd); });
			if (bFoundMapping) {
				if (AdditionalInputActionReplyMode == E_AdditionalInputActionReplyModes::HandleSpecified) {
					ActionNameToHandle = ActionX.ToString();
					bAdditionalDesiresHandled = true;
					break;
				}
				else if (AdditionalInputActionReplyMode == E_AdditionalInputActionReplyModes::HandleAllExceptSpecified) {
					bAdditionalDesiresHandled = false;
					break;
				}
			}
		}

		// Debug
		if (bAdditionalDesiresHandled) {
			Reply = FReply::Handled();
			switch(AdditionalInputActionReplyMode) {
			case (E_AdditionalInputActionReplyModes::HandleSpecified):
				UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Handling input action %s specified in AdditionalInputActionReplies."), *GetName(), *ActionNameToHandle);
				break;
			case (E_AdditionalInputActionReplyModes::HandleAllExceptSpecified):
				UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("%s: Handling all input actions except specified in AdditionalInputActionReplies."), *GetName());
				break;
			}
		}
	}

    return Reply;
}

FReply UKeyboundUserWidget::ProcessInputActionByName(const FName& InActionName, bool bInIsKeyDown) {
	//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("ProcessInputActionByName: %s, KeyDown?: %s"), *InActionName.ToString(), (bInIsKeyDown ? TEXT("True") : TEXT("False")));
	FS_WidgetInputActionListener* Listener = ListenForRoutedInputActions.Find(InActionName);
	if (Listener == nullptr) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("ListenForRoutedInputActions does not contain %s."), *InActionName.ToString());
		return FReply::Unhandled();
	}
	if ((bInIsKeyDown && !Listener->bOnKeyDown)
		|| (!bInIsKeyDown && !Listener->bOnKeyUp)
		) {
		return FReply::Unhandled();
	}

	if (Listener->Delegate.IsBound()) {
		Listener->Delegate.Execute();
		//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Handling key for action: %s"), *InActionName.ToString());
		return FReply::Handled();
	}
	
	// If the delegate is not bound, we have not implemented functionality for the action and should regard this action as unhandled.
	// If you don't do this, you would handle everything including the slate navigation.
	return FReply::Unhandled();
}

void UKeyboundUserWidget::ListenForRoutedInputAction(const FName& InActionName, bool bInOnKeyDown, bool bInOnKeyUp, const FOnRequestWidgetInputAction& InDelegate) {
	ListenForRoutedInputActions.Add(InActionName, FS_WidgetInputActionListener(InDelegate, bInOnKeyDown, bInOnKeyUp));
}

void UKeyboundUserWidget::StopListenForRoutedInputAction(const FName& InActionName) {
	ListenForRoutedInputActions.Remove(InActionName);
}
