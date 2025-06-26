/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "CursorWidget.h"
#include "DetectCurrentInputDevicePreProcessor.h"
#include "Engine/LocalPlayer.h"
#include "LogUIAdditionsPlugin.h"
#include "Modules/ModuleManager.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Framework/Application/SlateUser.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UIAdditionsPlugin.h"
#include "SlateUtils.h"
#include "ExtendedAnalogCursor.h"
#include "InputState.h"


// Setup

void UCursorWidget::NativeOnInitialized() {
	Super::NativeOnInitialized();
	
	// Bind to when input device changes
	const FUIAdditionsPluginModule& UIAdditionsPluginModule = FModuleManager::GetModuleChecked<FUIAdditionsPluginModule>(TEXT("UIAdditionsPlugin"));
	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& Processor = UIAdditionsPluginModule.GetDetectCurrentInputDevicePreProcessor();
	if (Processor.IsValid()) {
		Processor->OnInputDeviceChanged.AddUObject(this, &UCursorWidget::ActOnInputDeviceChanged);
		const int32 SlateUserIndex = USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer());
		ActOnInputDeviceChanged(Processor->GetCurrentInputDevice(SlateUserIndex), SlateUserIndex);		
	}
	
	// Process the initial state.
	OnCursorContextsChanged();
}

// Tick

void UCursorWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);

	TickUpdateCursorPosition();
}

void UCursorWidget::TickUpdateCursorPosition() {
	TSharedPtr<FSlateUser> SlateUser = USlateUtils::GetSlateUserForPlayerController(GetOwningPlayer());
	if (SlateUser == nullptr) {
		return;
	}
	UCanvasPanelSlot* CanvasSlot = IsValid(CursorContainer) ? Cast<UCanvasPanelSlot>(CursorContainer->Slot) : nullptr;
	if (!IsValid(CanvasSlot)) {
		return;
	}

	// Get absolute coordinates into NewPosition.
	FVector2D UnProcessedPosition = FVector2D::ZeroVector;
	if (GetFreezeCursorToCenterOfScreen()) {
		UnProcessedPosition = USlateUtils::GetCenterOfPlayerScreen(GetOwningLocalPlayer());
	}
	else {
		UnProcessedPosition = SlateUser->GetCursorPosition();
	}

	/* 
	* UE5.1.1 SetPositionInViewport seems broken, so we need to use a canvas slot to position the cursors. 
	* Another note about that method is that if you feed coordinates to SetPositionInViewport it doesn't like (outside of resolution?), it breaks tick for some reason.
	*/

	/* 
	* Possible TODO. I want to clamp the cursor widget to the relevant space properly (Player Screen / Viewport) based on what Sub HUD is used.
	* Challenge seems to be that if we convert absolute cursor space to Player Screen that the hardware cursor is not aligned with the new coordinates. 
	* There is some offset which seems to be the distance between the viewport and game screen based on screen shape?
	* For now just clamp to viewport. The canvas of the cursors should also be taking the entire viewport.
	*/

	//const FGeometry Geometry = (GetCursorScreenSpace() == E_CursorScreenSpace::PlayerScreen
	//	? UWidgetLayoutLibrary::GetPlayerScreenWidgetGeometry(GetOwningPlayer())
	//	: UWidgetLayoutLibrary::GetViewportWidgetGeometry(GetOwningPlayer())
	//);

	FGeometry ViewportGeometry = UWidgetLayoutLibrary::GetViewportWidgetGeometry(GetOwningPlayer());
	FVector2D LocalPosition = USlateUtils::ClampLocalPositionToGeometry(ViewportGeometry, ViewportGeometry.AbsoluteToLocal(UnProcessedPosition));

	// Store the absolute position to be available on request.
	Position = ViewportGeometry.LocalToAbsolute(LocalPosition);

	// Use the local position for the canvas slot.
	CanvasSlot->SetPosition(LocalPosition);
	//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Position: %s"), *LocalPosition.ToString());
}

// Appearance

void UCursorWidget::Show() {
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UCursorWidget::Hide() {
	SetVisibility(ESlateVisibility::Collapsed);
}

FVector2D UCursorWidget::GetPosition() const {
	return Position;
}

bool UCursorWidget::GetFreezeCursorToCenterOfScreen() const {
	return bFreezeCursorToCenterOfScreen;
}

void UCursorWidget::SetFreezeCursorToCenterOfScreen(bool bInFreezeCursorToCenterOfScreen) {
	bFreezeCursorToCenterOfScreen = bInFreezeCursorToCenterOfScreen;
}

const TSet<FName>& UCursorWidget::GetCursorContexts() const {
	return CursorContexts;
}

void UCursorWidget::AddCursorContext(const FName& InContext) {
	CursorContexts.Add(InContext);
	OnCursorContextsChanged();
}

void UCursorWidget::RemoveCursorContext(const FName& InContext) {
	CursorContexts.Remove(InContext);
	OnCursorContextsChanged();
}

void UCursorWidget::AddOrRemoveCursorContext(const FName& InContext, bool bInAdd) {
	if (bInAdd) {
		AddCursorContext(InContext);
	}
	else {
		RemoveCursorContext(InContext);
	}
}

// Delegates

void UCursorWidget::ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex) {
	if (USlateUtils::GetSlateUserIndexForPlayerController(GetOwningPlayer()) != InSlateUserIndex) {
		// Return if this event was triggered by another player.
		return;
	}
	OnCursorContextsChanged();
}
