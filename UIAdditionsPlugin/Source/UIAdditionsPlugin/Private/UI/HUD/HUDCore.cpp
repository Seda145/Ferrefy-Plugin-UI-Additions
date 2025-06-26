/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "HUDCore.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "SubHUDWidget.h"
#include "Engine/GameInstance.h"
#include "CursorWidget.h"
#include "DisabledCursorWidget.h"
#include "GameFramework/Controller.h"
#include "Kismet/GameplayStatics.h"
#include "WorldCursorModifierComponent.h"
#include "Framework/Application/SlateApplication.h"
#include "ExtendedAnalogCursor.h"
#include "SlateUtils.h"
#include "LogUIAdditionsPlugin.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Widgets/SWidget.h"
#include "Layout/WidgetPath.h"
#include "PlayerControllerInputModes.h"
#include "HUDCorePlayerControllerComponent.h"
#include "GameFramework/PlayerController.h"
#include "UnrealClient.h"


// Setup
	
void AHUDCore::BeginPlay() {
	Super::BeginPlay();
	
	// Be notified when a pawn's controller changes so we can create a PawnHUD.
	UGameInstance* GI = UGameplayStatics::GetGameInstance(this);
	check(IsValid(GI));
	GI->GetOnPawnControllerChanged().AddDynamic(this, &AHUDCore::ActOnPawnControllerChanged);

	// We can bind to a change in controller on the Pawn.
	APlayerController* PC = GetOwningPlayerController();
	if (IsValid(PC)) {
		// TODO. Be aware of this engine bug:
		// https://forums.unrealengine.com/t/ue5-2-1-bug-report-controller-possess-broken-if-pawn-auto-possess-ai/1218849
		PC->GetOnNewPawnNotifier().AddUObject(this, &AHUDCore::ActOnControllerPawnChanged);
		ActOnControllerPawnChanged(PC->GetPawn());
	}
	else {
		// This happens on simulation or any other situation without playercontroller.
		UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("PlayerController is invalid. Can't configure delegates to be notified of a change in possession."));
	}

	// PlayerScreenHUD
	if (IsValid(PlayerScreenHUDWidgetClass.LoadSynchronous())) {
		PlayerScreenHUD = CreateWidget<USubHUDWidget>(GetOwningPlayerController(), PlayerScreenHUDWidgetClass.LoadSynchronous());
		check(IsValid(GetPlayerScreenHUD()));
		GetPlayerScreenHUD()->AddToPlayerScreen(ZIndexPlayerScreenHUD);
		GetPlayerScreenHUD()->OnMenuVisibilityChanged.AddDynamic(this, &AHUDCore::ActOnPlayerScreenHUDVisibilityChanged);
		// Note that arguments are not used, so we can just directly call this to get updated state.
		ActOnPlayerScreenHUDVisibilityChanged(nullptr, false);
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("PlayerScreenHUDWidgetClass not configured. Ignore if not required."));
	}

	// PlayerViewportHUD
	if (IsValid(PlayerViewportHUDWidgetClass.LoadSynchronous())) {
		PlayerViewportHUD = CreateWidget<USubHUDWidget>(GetOwningPlayerController(), PlayerViewportHUDWidgetClass.LoadSynchronous());
		check(IsValid(GetPlayerViewportHUD()));
		GetPlayerViewportHUD()->AddToViewport(ZIndexPlayerViewportHUD);
		GetPlayerViewportHUD()->OnMenuVisibilityChanged.AddDynamic(this, &AHUDCore::ActOnPlayerViewportHUDVisibilityChanged);
		// Note that arguments are not used, so we can just directly call this to get updated state.
		ActOnPlayerViewportHUDVisibilityChanged(nullptr, false);
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("PlayerViewportHUDWidgetClass not configured. Ignore if not required."));
	}

	// Slate delegates
	FSlateApplication::Get().OnFocusChanging().AddUObject(this, &AHUDCore::ActOnFocusChanging);
}

void AHUDCore::BeginDestroy() {
	// Make sure to unregister the analog cursor from slate or it could remain there.
	DisableAnalogCursor();

	Super::BeginDestroy();
}

// Tick

void AHUDCore::Tick(float InDeltaSeconds) {
	Super::Tick(InDeltaSeconds);

	TickUpdateMouseLocation();
}

void AHUDCore::TickUpdateMouseLocation_Implementation() {
	/** 
	* If we desire to freeze the cursor to the center of the screen but the analog cursor is not being used
	* , we could still freeze the hardware cursor here for consistency.
	* However, if you are showing the hardware cursor then know that it will look choppy.
	*/
	//if (!GetFreezeCursorToCenterOfScreen() || GetAnalogCursor().IsValid()) {
	if (!GetFreezeCursorToCenterOfScreen()) {
		return;
	}

	UWorld* World = GetWorld();
	const UGameViewportClient* GameViewportClient = IsValid(World) ? World->GetGameViewport() : nullptr;
	if (!IsValid(GameViewportClient)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid GameViewportClient."));
		return;
	}

	// Do a check if the app is running in the foreground, else the mouse will be hijacked outside of the app.
	if (!GameViewportClient->Viewport || !GameViewportClient->Viewport->IsForegroundWindow()) {
		// UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("App is not running in the foreground, canceled centering."));
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	ULocalPlayer* LocalPlayer = IsValid(PC) ? PC->GetLocalPlayer() : nullptr;
	if (!IsValid(LocalPlayer)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid LocalPlayer."));
		return;
	}

	const FVector2D NewPosition = USlateUtils::GetCenterOfPlayerScreen(GameViewportClient, LocalPlayer);
	PC->SetMouseLocation(NewPosition.X, NewPosition.Y);
}

// Panels

bool AHUDCore::IsAnySubHUDVisible() const {
	return GetIsAnyPlayerScreenHUDMenuVisible() || GetIsAnyPlayerViewportHUDMenuVisible() || GetIsAnyPawnHUDMenuVisible();
}

bool AHUDCore::GetIsAnyPlayerScreenHUDMenuVisible() const {
	return bIsAnyPlayerScreenHUDMenuVisible;
}

bool AHUDCore::GetIsAnyPlayerViewportHUDMenuVisible() const {
	return bIsAnyPlayerViewportHUDMenuVisible;
}

bool AHUDCore::GetIsAnyPawnHUDMenuVisible() const {
	return bIsAnyPawnHUDMenuVisible;
}

USubHUDWidget* AHUDCore::GetPlayerScreenHUD() const {
	return PlayerScreenHUD;
}

USubHUDWidget* AHUDCore::GetPlayerViewportHUD() const {
	return PlayerViewportHUD;
}

USubHUDWidget* AHUDCore::FindPawnHUD(APawn* InPawn) const {
	if (!IsValid(InPawn)) {
		return nullptr;
	}
	USubHUDWidget* const* DataPtr = PawnHUDs.Find(InPawn);
	return (DataPtr ? *DataPtr : nullptr);
}

USubHUDWidget* AHUDCore::FindOrCreatePawnHUD(APawn* InPawn) {
	if (!IsValid(InPawn)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid InPawn."));
		return nullptr;
	}
	if (!IsValid(GetOwningPlayerController())) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid OwningPlayerController."));
		return nullptr;
	}
	
	USubHUDWidget* PawnHUD = FindPawnHUD(InPawn);
	if (IsValid(PawnHUD)) {
		// PawnHUD already exists
		return PawnHUD;
	}
	// If it does not exist yet, create a PawnHUD.

	if (IsValid(PawnHUDWidgetClass.LoadSynchronous())) {
		PawnHUD = CreateWidget<USubHUDWidget>(GetOwningPlayerController(), PawnHUDWidgetClass.LoadSynchronous());
		if (!IsValid(PawnHUD)) {
			UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid PawnHUD."));
			return nullptr;
		}
		//PawnHUD->SetupOnCall(WidgetComponentRoot, InPawn);
		PawnHUDs.Add(InPawn, PawnHUD);
		PawnHUD->AddToPlayerScreen(ZIndexPawnHUD);
		// Note that delegate bindings are not modified here, since they are only relevant based on possession.
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("PawnHUDWidgetClass not configured. Ignore if not required."));
	}

	InPawn->OnDestroyed.AddDynamic(this, &AHUDCore::ActOnPawnDestroyed);
	return PawnHUD;
}

void AHUDCore::DestroyPawnHUD(APawn* InPawn) {
	if (!IsValid(InPawn)) {
		return;
	}

	// Remove the pawn HUD

	USubHUDWidget** PawnHUDPtr = PawnHUDs.Find(InPawn);
	USubHUDWidget* PawnHUD = PawnHUDPtr ? *PawnHUDPtr : nullptr;
	if (IsValid(PawnHUD)) {
		PawnHUD->RemoveFromParent();
		PawnHUDs.Remove(InPawn);
		// Note that delegate bindings are not modified here, since they are only relevant based on possession.
	}
}

// Input

void AHUDCore::UpdateInputMode() {
	APlayerController* PC = GetOwningPlayerController();
	if (!IsValid(PC)) {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Can not update the current input mode without a valid owning player controller."));
		return;
	}	

	UHUDCorePlayerControllerComponent* HUDCorePCComponent = PC->FindComponentByClass<UHUDCorePlayerControllerComponent>();
	if (!IsValid(HUDCorePCComponent)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("To update the input mode, the PlayerController requires a valid component of type HUDCorePlayerControllerComponent."));
		return;
	}

	if (GetIsAnyPlayerViewportHUDMenuVisible() || GetIsAnyPlayerScreenHUDMenuVisible()) {
		HUDCorePCComponent->ActivateInputMode(E_PlayerControllerInputModes::UI);
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("PlayerScreenHUD or PlayerViewportHUD has visible menu panels and desires the cursor to be unfrozen. Input mode set to 'UI'."));
		SetFreezeCursorToCenterOfScreen(false);
	}
	else if (GetIsAnyPawnHUDMenuVisible()) {
		HUDCorePCComponent->ActivateInputMode(E_PlayerControllerInputModes::GameAndUI);
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("PawnHUD has visible menu panels and desires the cursor to be unfrozen. Input mode set to 'Game + UI'."));
		SetFreezeCursorToCenterOfScreen(false);
	}
	else {
		HUDCorePCComponent->ActivateInputMode(E_PlayerControllerInputModes::Game);
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("No Sub HUD has a visible menu, the cursor does not have to be unfrozen. Input mode set to 'Game'."));

		if (GetPawnDesiresCenteredWorldCursor()) {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("The pawn desires the cursor to be frozen to center of screen (World Cursor Modifier Component). This is allowed in input mode 'Game'."));
			SetFreezeCursorToCenterOfScreen(true);
		}
		else {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Unfreezing cursor from center of screen."));
			SetFreezeCursorToCenterOfScreen(false);
		}
	}
}

// Delegates | Panels

void AHUDCore::ActOnPlayerScreenHUDVisibilityChanged(UMenuWidget* InMenu, bool bIsVisible) {
	bIsAnyPlayerScreenHUDMenuVisible = IsValid(GetPlayerScreenHUD()) && GetPlayerScreenHUD()->IsAnyMenuVisible();
	UpdateInputMode();

	if (IsValid(GetCustomCursorWidget())) {
		GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PlayerScreenHUDMenuVisible"), GetIsAnyPlayerScreenHUDMenuVisible());
		GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("AnySubHUDMenuVisible"), IsAnySubHUDVisible());
	}
}

void AHUDCore::ActOnPlayerViewportHUDVisibilityChanged(UMenuWidget* InMenu, bool bIsVisible) {
	bIsAnyPlayerViewportHUDMenuVisible = IsValid(GetPlayerViewportHUD()) && GetPlayerViewportHUD()->IsAnyMenuVisible();;
	UpdateInputMode();

	if (IsValid(GetCustomCursorWidget())) {
		GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PlayerViewportHUDMenuVisible"), GetIsAnyPlayerViewportHUDMenuVisible());
		GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("AnySubHUDMenuVisible"), IsAnySubHUDVisible());
	}
}

void AHUDCore::ActOnPawnHUDVisibilityChanged(UMenuWidget* InMenu, bool bIsVisible) {
	APlayerController* PC = GetOwningPlayerController();
	APawn* Pawn = IsValid(PC) ? PC->GetPawn() : nullptr;
	USubHUDWidget* PawnHUD = FindPawnHUD(Pawn);
	bIsAnyPawnHUDMenuVisible = IsValid(PawnHUD) && PawnHUD->IsAnyMenuVisible();
	UpdateInputMode();

	if (IsValid(GetCustomCursorWidget())) {
		GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PawnHUDMenuVisible"), GetIsAnyPawnHUDMenuVisible());
		GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("AnySubHUDMenuVisible"), IsAnySubHUDVisible());
	}
}

// Delegates | Pawn

void AHUDCore::ActOnPawnControllerChanged(APawn* InPawn, AController* InController) {
	if (!IsValid(InPawn)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid InPawn."));
		return;
	}

	// If the controller is nullptr the pawn has been unpossessed.
	if (!InController) {
		// Unbind UpdateInputMode

		UWorldCursorModifierComponent* ACWorldCursorModifier = InPawn->FindComponentByClass<UWorldCursorModifierComponent>();
		// Not asserting if world cursor modifier is null, because without a UI it can be defaulted to freeze.
		// If that is not what you want, you should just add this component to your pawn.
		if (ACWorldCursorModifier != nullptr) {
			// We must respond to when a InPawn desires its world cursor to be frozen or unfrozen
			// , and let the HUD update its input mode to pick the correct one.
			ACWorldCursorModifier->OnDesiresCenteredWorldCursorChanged.RemoveDynamic(this, &AHUDCore::ActOnPawnDesiresCenteredWorldCursorChanged);	
		}
		ActOnPawnDesiresCenteredWorldCursorChanged(true);

		// Unbind the PawnHUD

		USubHUDWidget* PawnHUD = FindPawnHUD(InPawn);
		if (IsValid(PawnHUD)) {
			// Only hide the PawnHUD so it can be restored later. 
			// No need to destroy it here.
			PawnHUD->SetVisibility(ESlateVisibility::Collapsed);
			// UnBind the delegate here, which is only relevant during possession.
			PawnHUD->OnMenuVisibilityChanged.RemoveDynamic(this, &AHUDCore::ActOnPawnHUDVisibilityChanged);
			// Note that arguments are not used, so we can just directly call this to get updated state.
			ActOnPawnHUDVisibilityChanged(nullptr, false);
		}
	}
}

void AHUDCore::ActOnControllerPawnChanged(APawn* InPawn) {
	// This method is bound to a PlayerController. We can use it to create the PawnHUD.
	if (!IsValid(InPawn)) {
		return;
	}
	APlayerController* PC = Cast<APlayerController>(InPawn->GetController());
	// Test if the Pawn is controlled by a player.
	if (!IsValid(PC)) {
		return;
	}

	UWorldCursorModifierComponent* ACWorldCursorModifier = InPawn->FindComponentByClass<UWorldCursorModifierComponent>();
	// Not asserting if world cursor modifier is null, because without a UI it can be defaulted to freeze.
	// If that is not what you want, you should just add this component to your pawn.
	if (ACWorldCursorModifier != nullptr) {
		ACWorldCursorModifier->OnDesiresCenteredWorldCursorChanged.AddDynamic(this, &AHUDCore::ActOnPawnDesiresCenteredWorldCursorChanged);
		ActOnPawnDesiresCenteredWorldCursorChanged(ACWorldCursorModifier->GetDesiresCenteredWorldCursor());
	}
	else {
		ActOnPawnDesiresCenteredWorldCursorChanged(true);
	}

	USubHUDWidget* PawnHUD = FindOrCreatePawnHUD(InPawn);
	if (IsValid(PawnHUD)) {
		// Bind the delegate here, which is only relevant during possession.
		PawnHUD->OnMenuVisibilityChanged.AddDynamic(this, &AHUDCore::ActOnPawnHUDVisibilityChanged);
		// Show the PawnHUD
		PawnHUD->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		// Note that arguments are not used, so we can just directly call this to get updated state.
		ActOnPawnHUDVisibilityChanged(nullptr, false);
	}
}

void AHUDCore::ActOnPawnDestroyed(AActor* InDestroyedActor) {
	APawn* Pawn = Cast<APawn>(InDestroyedActor);
	if (Pawn) {
		DestroyPawnHUD(Pawn);
	}
}

void AHUDCore::ActOnPawnDesiresCenteredWorldCursorChanged(bool InPawnDesiresCenteredWorldCursor) {
	bPawnDesiresCenteredWorldCursor = InPawnDesiresCenteredWorldCursor;

	if (IsValid(GetCustomCursorWidget())) {
		GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PawnDesiresCenteredWorldCursor"), GetPawnDesiresCenteredWorldCursor());
		UpdateInputMode();
	}
}

// Delegates

void AHUDCore::ActOnFocusChanging(const FFocusEvent& InFocusEvent, const FWeakWidgetPath& InOldFocusedWidgetPath, const TSharedPtr<SWidget>& InOldFocusedWidget, const FWidgetPath& InNewFocusedWidgetPath, const TSharedPtr<SWidget>& InNewFocusedWidget) {
	/**
	* This method attempts to restore generally undesired focus loss:
	* This makes it less likely to lose focus accidentally, which is especially useful when using non cursor devices.
	* 
	* TODO doesn't appear to restore focus when a button (or wrapping parent) goes invisible. Deal with that?
	*/

	UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("HUD detects focus change. Reason: %s, New path: %s"), *UEnum::GetValueAsString(InFocusEvent.GetCause()), (InNewFocusedWidget.IsValid() ? *InNewFocusedWidget->ToString() : TEXT("null")));

	// Guard against re entry, otherwise SomeMenu->RestoreFocus is going to be trouble.
	static bool bIsEntered = false;
	if (bIsEntered) {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Hit re entry guard."));
		return;
	}

	USubHUDWidget* ViewportHUD = GetPlayerViewportHUD();
	USubHUDWidget* ScreenHUD = GetPlayerScreenHUD();
	USubHUDWidget* PawnHUD = FindPawnHUD(GetOwningPawn());

	bool bNeedRestore = false;

	// TODO found that with below condition alt tabbing from editor runtime to notepad and back to editor runtime caused wrong menus to focus?! Engine needs to be treated more strictly.
	//if (InFocusEvent.GetCause() != EFocusCause::SetDirectly && !InNewFocusedWidgetPath.IsValid()) {
	if (
		(
			InFocusEvent.GetCause() != EFocusCause::SetDirectly 
			&& InFocusEvent.GetCause() != EFocusCause::Navigation
		) 
		|| !InNewFocusedWidgetPath.IsValid()
		) {
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Undesired focus loss detected. (not set directly OR not by Slate navigation OR new focus path invalid.)"));
		bNeedRestore = true;
	}

#if WITH_EDITOR
	UWorld* World = GetWorld();
	const UGameViewportClient* GameViewportClient = IsValid(World) ? World->GetGameViewport() : nullptr;
	// So... If the editor runs along PIE and we click a button outside of PIE on the editor, we don't want to hijack focus from the editor back into PIE?
	if (!IsValid(GameViewportClient) || !GameViewportClient->Viewport || !GameViewportClient->Viewport->IsForegroundWindow()) {
		 UE_LOG(LogUIAdditionsPlugin, Verbose, TEXT("Not restoring focus. App is not running in the foreground, we're WITH_EDITOR and don't possibly want to hijack editor focus."));
		return;
	}
#endif // WITH_EDITOR

	if (!bNeedRestore) {
		bool bFoundValidNewPath = (
			(IsValid(ViewportHUD) && InNewFocusedWidgetPath.ContainsWidget(&ViewportHUD->TakeWidget().Get()))
			|| (IsValid(ScreenHUD) && InNewFocusedWidgetPath.ContainsWidget(&ScreenHUD->TakeWidget().Get()))
			|| (IsValid(PawnHUD) && InNewFocusedWidgetPath.ContainsWidget(&PawnHUD->TakeWidget().Get()))
			);
		if (!bFoundValidNewPath) {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Undesired focus loss (new path != on a sub HUD)."));
			bNeedRestore = true;
		}
	}

	if (!bNeedRestore) {
		return;
	}

	USubHUDWidget* RestoreToWidget = nullptr;
	if (InOldFocusedWidgetPath.IsValid()) {
		if (IsValid(ViewportHUD) && InOldFocusedWidgetPath.ContainsWidget(&ViewportHUD->TakeWidget().Get())) {
			RestoreToWidget = ViewportHUD;
		}
		else if (IsValid(ScreenHUD) && InOldFocusedWidgetPath.ContainsWidget(&ScreenHUD->TakeWidget().Get())) {
			RestoreToWidget = ScreenHUD;
		}
		else if (IsValid(PawnHUD) && InOldFocusedWidgetPath.ContainsWidget(&PawnHUD->TakeWidget().Get())) {
			RestoreToWidget = ScreenHUD;
		}
	}

	// If the sub HUD has no visible menu, it has nothing for the user to interact with and we can assume that the focus changed from a submenu to nothing. Ignore?
	if (IsValid(RestoreToWidget)) {
		if (!RestoreToWidget->IsAnyMenuVisible()) {
			UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("The sub HUD desired to restore to (%s) has no visible menu, so it's no longer desired."), *RestoreToWidget->GetName());
			RestoreToWidget = nullptr;
		}
	}

	if (!IsValid(RestoreToWidget)) {
		/**
		* We don't want to lose focus to nothing or to anything outside of the Sub HUDs.
		* Clicking anywhere on the screen could for example focus the viewport, which is not helpful if we want to focus only specific widgets with a gamepad.
		* If we couldn't retrieve an old focus path to a sub HUD, focus any sub HUD based on Z-Order?
		*/

		if (GetIsAnyPlayerViewportHUDMenuVisible()) {
			RestoreToWidget = ViewportHUD;
		}
		else if (GetIsAnyPlayerScreenHUDMenuVisible()) {
			RestoreToWidget = ScreenHUD;
		}
		else if (GetIsAnyPawnHUDMenuVisible()) {
			RestoreToWidget = PawnHUD;
		}
	}

	if (IsValid(RestoreToWidget)) {
		bIsEntered = true;
		////
		UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Attempts to restore focus to a visible sub HUD: %s."), *RestoreToWidget->GetName());
		RestoreToWidget->RestoreFocus();
		////
		bIsEntered = false;
	}
}

// Cursor

TSharedPtr<FExtendedAnalogCursor> AHUDCore::GetAnalogCursor() const { 
	return AnalogCursor; 
}

void AHUDCore::EnableAnalogCursor() {
	if (IsAnalogCursorValid()) {
		//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Can not enable the analog cursor, it is already enabled."));
		return;
	}
	if (!FSlateApplication::IsInitialized()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can not enable the analog cursor, Slate is not initialized."));
		return;
	}
	APlayerController* PC = GetOwningPlayerController();
	ULocalPlayer* LocalPlayer = IsValid(PC) ? PC->GetLocalPlayer() : nullptr;
	if (!IsValid(LocalPlayer)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can not enable the analog cursor, LocalPlayer is not valid."));
		return;
	}
	UWorld* World = GetWorld();
	if (!IsValid(World)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can not enable the analog cursor, World is not valid."));
		return;
	}

	AnalogCursor = MakeShareable(new FExtendedAnalogCursor(FLocalPlayerContext(LocalPlayer, World)));
	if (!GetAnalogCursor().IsValid()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("analog cursor is invalid."));
		return;
	}

	FSlateApplication::Get().RegisterInputPreProcessor(GetAnalogCursor());
	// This freezes the actual virtual cursor to a position.
	GetAnalogCursor()->SetFreezeCursorToCenterOfScreen(GetFreezeCursorToCenterOfScreen());
}

void AHUDCore::DisableAnalogCursor() {
	if (!IsAnalogCursorValid()) {
		//UE_LOG(LogUIAdditionsPlugin, VeryVerbose, TEXT("Can not disable the analog cursor, it is already disabled."));
		return;
	}
	if (!FSlateApplication::IsInitialized()) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can not disable the analog cursor, Slate is not initialized."));
		return;
	}

	FSlateApplication::Get().UnregisterInputPreProcessor(GetAnalogCursor());
	AnalogCursor.Reset();
}

bool AHUDCore::IsAnalogCursorValid() const {
	return GetAnalogCursor().IsValid();
}

bool AHUDCore::GetPawnDesiresCenteredWorldCursor() const {
	return bPawnDesiresCenteredWorldCursor;
}

bool AHUDCore::GetFreezeCursorToCenterOfScreen() const {
	return bFreezeCursorToCenterOfScreen;
}

void AHUDCore::SetFreezeCursorToCenterOfScreen(bool bInFreezeCursorToCenterOfScreen) {
	bFreezeCursorToCenterOfScreen = bInFreezeCursorToCenterOfScreen;
	if (GetAnalogCursor().IsValid()) {
		// This freezes the actual virtual cursor to a position.
		GetAnalogCursor()->SetFreezeCursorToCenterOfScreen(GetFreezeCursorToCenterOfScreen());
	}

	if (IsValid(GetCustomCursorWidget())) {
		// This will simply set the widget position to center, avoiding pulling actual position from the cursor device / virtual cursor.
		GetCustomCursorWidget()->SetFreezeCursorToCenterOfScreen(GetFreezeCursorToCenterOfScreen());
	}

	// If freezing, the hardware cursor would have to be synchronized on tick, as it can not be locked.
	TickUpdateMouseLocation();
}

FVector2D AHUDCore::GetCursorPosition() const {
	if (IsAnalogCursorValid()) {
		return GetAnalogCursor()->GetCurrentPosition();
	}
	float OutX = 0.f;
	float OutY = 0.f;
	GetOwningPlayerController()->GetMousePosition(OutX, OutY);
	return FVector2D(OutX, OutY);
}

UCursorWidget* AHUDCore::GetCustomCursorWidget() const {
	return CustomCursorWidget;
}

void AHUDCore::MakeCustomCursorWidget(TSoftClassPtr<UCursorWidget> InCursorWidgetClass) {
	if (!IsValid(InCursorWidgetClass.LoadSynchronous())) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can not enable the analog cursor, CustomCursorWidgetClass is invalid."));
		return;
	}
	const UWorld* World = GetWorld();
	UGameViewportClient* const GameViewportClient = IsValid(World) ? World->GetGameViewport() : nullptr;
	if (!IsValid(GameViewportClient)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Can not enable the analog cursor, the GameViewportClient is invalid."));
		return;
	}

	if (IsValid(GetCustomCursorWidget())) {
		// Remove an existing widget. Not calling RemoveCustomCursorWidget to avoid rebuilding the original software pointers.
		GetCustomCursorWidget()->RemoveFromParent();
	}

	CustomCursorWidget = CreateWidget<UCursorWidget>(GetOwningPlayerController(), InCursorWidgetClass.LoadSynchronous());
	check(IsValid(GetCustomCursorWidget()));
	GetCustomCursorWidget()->AddToViewport(ZIndexCursor);
	GetCustomCursorWidget()->SetFreezeCursorToCenterOfScreen(GetFreezeCursorToCenterOfScreen());
	// I intentionally did not move this context logic to the cursor itself (as observer). 
	// It is only aware of its own context list and any implementation is up to the cursor.
	GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PawnDesiresCenteredWorldCursor"), GetPawnDesiresCenteredWorldCursor());
	GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PlayerScreenHUDMenuVisible"), GetIsAnyPlayerScreenHUDMenuVisible());
	GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PlayerViewportHUDMenuVisible"), GetIsAnyPlayerViewportHUDMenuVisible());
	GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("PawnHUDMenuVisible"), GetIsAnyPawnHUDMenuVisible());
	GetCustomCursorWidget()->AddOrRemoveCursorContext(TEXT("AnySubHUDMenuVisible"), IsAnySubHUDVisible());

	/** 
	* Set dummies to the software cursors in the viewport (same Software Cursors as in Project Settings > User Interface).
	* This will allow us to use custom cursors through the HUDCore.
	* The original software cursors had the issue that they were not associated with a player and shared across all players visually
	*/
	for (EMouseCursor::Type CursorTypeX : USlateUtils::GetMouseCursorTypes()) {
		// Set dummy.
		GameViewportClient->SetSoftwareCursorWidget(CursorTypeX, CreateWidget(GetGameInstance(), UDisabledCursorWidget::StaticClass()));
	}
}

void AHUDCore::RemoveCustomCursorWidget() {
	if (IsValid(GetCustomCursorWidget())) {
		GetCustomCursorWidget()->RemoveFromParent();
	}

	const UWorld* World = GetWorld();
	UGameViewportClient* const GameViewportClient = IsValid(World) ? World->GetGameViewport() : nullptr;
	if (IsValid(GameViewportClient)) {
		/**
		* Restore originals to the software cursors in the viewport.
		*/
		for (EMouseCursor::Type CursorTypeX : USlateUtils::GetMouseCursorTypes()) {
			GameViewportClient->SetSoftwareCursorWidget(CursorTypeX, nullptr);
		}
		GameViewportClient->RebuildCursors();
	}
	else {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("Invalid GameViewportClient."));
	}
}
