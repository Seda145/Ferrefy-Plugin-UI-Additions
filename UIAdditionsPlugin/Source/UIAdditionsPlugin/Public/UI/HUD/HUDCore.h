/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Templates/SharedPointer.h"

#include "HUDCore.generated.h"

class USubHUDWidget;
class AController;
class UCursorWidget;
class FExtendedAnalogCursor;
class SWidget;
class FWidgetPath;
class FWeakWidgetPath;
struct FFocusEvent;


/*
* HUD actor which implements "sub" HUD widgets. 
* Takes control over PlayerController input modes.
* Manages 3 types of "sub" HUD widgets:
* 1. 1 For local players using their own screen area.
* 2. 1 For local players using the entire viewport area.
* 3. PawnHUDs: 1 For every player controlled pawn, for local players using their own screen area.
*/
UCLASS(BlueprintType, Blueprintable)
class UIADDITIONSPLUGIN_API AHUDCore : public AHUD {
	GENERATED_BODY()

private:

	static const int32 ZIndexPawnHUD = 1;
	static const int32 ZIndexPlayerScreenHUD = 2;
	static const int32 ZIndexPlayerViewportHUD = 3;
	static const int32 ZIndexCursor = 4;

	// Setup

	UPROPERTY()
		USubHUDWidget* PlayerScreenHUD = nullptr;

	UPROPERTY()
		USubHUDWidget* PlayerViewportHUD = nullptr;

	UPROPERTY()
		TMap<APawn*, USubHUDWidget*> PawnHUDs;
	
	// Cursor

	//UPROPERTY()
	TSharedPtr<FExtendedAnalogCursor> AnalogCursor = nullptr;

	UPROPERTY()
		UCursorWidget* CustomCursorWidget = nullptr;

	UPROPERTY()
		bool bFreezeCursorToCenterOfScreen = false;

	UPROPERTY()
		bool bPawnDesiresCenteredWorldCursor = false;

	// Sub HUD tracking

	UPROPERTY()
		bool bIsAnyPlayerScreenHUDMenuVisible = false;

	UPROPERTY()
		bool bIsAnyPlayerViewportHUDMenuVisible = false;

	UPROPERTY()
		bool bIsAnyPawnHUDMenuVisible = false;

protected:

	// HUD

	UPROPERTY(EditAnywhere, Category = "HUD")
		TSoftClassPtr<USubHUDWidget> PlayerScreenHUDWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "HUD")
		TSoftClassPtr<USubHUDWidget> PlayerViewportHUDWidgetClass = nullptr;

	UPROPERTY(EditAnywhere, Category = "HUD")
		TSoftClassPtr<USubHUDWidget> PawnHUDWidgetClass = nullptr;

public:

private:

protected:

	// Setup
	
	virtual void BeginPlay() override;
	
	// Tick

	virtual void Tick(float InDeltaSeconds) override;

	/* This is called during Tick. Default behavior is for it to set the owning playercontroller's mouse location (not analog cursor or widget) to the center of the screen if GetFreezeCursorToCenterOfScreen() && app is running in the foreground. A hardware cursor can not exactly be locked to a position, so this tick synchronizes it. An alternative you could be looking for could be using an analog cursor. */
	UFUNCTION(BlueprintNativeEvent, Category = "HUDCore|Input")
	void TickUpdateMouseLocation();

	/* This is called during Tick. Default behavior is for it to set the owning playercontroller's mouse location (not analog cursor or widget) to the center of the screen if GetFreezeCursorToCenterOfScreen() && app is running in the foreground. A hardware cursor can not exactly be locked to a position, so this tick synchronizes it. An alternative you could be looking for could be using an analog cursor. */
	virtual void TickUpdateMouseLocation_Implementation();

	// Panels

	UFUNCTION(BlueprintCallable, Category = "HUD|HUDCore|SubHUD")
		USubHUDWidget* FindOrCreatePawnHUD(APawn* InPawn);

	// Input

	/* Calculates what the current input mode should be for the playercontroller, based on tracked visibility of menus inside the Sub HUDs, then sets it (UHUDCorePlayerControllerComponent). Automatically sets SetFreezeCursorToCenterOfScreen. Explanation: In input mode UI / Game + UI you should not have a frozen cursor (because there are widgets to interact with), and in input mode Game you might but only if desired. */
	virtual void UpdateInputMode();

	// Cursor

	bool GetPawnDesiresCenteredWorldCursor() const;

	bool GetFreezeCursorToCenterOfScreen() const;

	/* Sets bFreezeCursorToCenterOfScreen, then requests freezing on the analog cursor and custom cursor. Does not lock the hardware cursor, but its position can be attempted to synchronize to a position during tick.  */
	virtual void SetFreezeCursorToCenterOfScreen(bool bInFreezeCursorToCenterOfScreen);

	// Delegates

	void ActOnFocusChanging(const FFocusEvent& InFocusEvent, const FWeakWidgetPath& InOldFocusedWidgetPath, const TSharedPtr<SWidget>& InOldFocusedWidget, const FWidgetPath& InNewFocusedWidgetPath, const TSharedPtr<SWidget>& InNewFocusedWidget);

	// Delegates | Panels

	UFUNCTION()
		void ActOnPlayerScreenHUDVisibilityChanged(UMenuWidget* InMenu, bool bIsVisible);

	UFUNCTION()
		void ActOnPlayerViewportHUDVisibilityChanged(UMenuWidget* InMenu, bool bIsVisible);

	UFUNCTION()
		void ActOnPawnHUDVisibilityChanged(UMenuWidget* InMenu, bool bIsVisible);

	// Delegates | Pawn

	UFUNCTION()
		void ActOnPawnControllerChanged(APawn* InPawn, AController* InController);

	UFUNCTION()
		void ActOnControllerPawnChanged(APawn* InPawn);

	UFUNCTION()
		void ActOnPawnDestroyed(AActor* InDestroyedActor);

	UFUNCTION()
		void ActOnPawnDesiresCenteredWorldCursorChanged(bool InPawnDesiresCenteredWorldCursor);

public:

	// Setup

	virtual void BeginDestroy();

	// Panels

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|SubHUD")
		bool IsAnySubHUDVisible() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|SubHUD")
		bool GetIsAnyPlayerScreenHUDMenuVisible() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|SubHUD")
		bool GetIsAnyPlayerViewportHUDMenuVisible() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|SubHUD")
		bool GetIsAnyPawnHUDMenuVisible() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|SubHUD")
		USubHUDWidget* GetPlayerScreenHUD() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|SubHUD")
		USubHUDWidget* GetPlayerViewportHUD() const;	
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|SubHUD")
		USubHUDWidget* FindPawnHUD(APawn* InPawn) const;

	UFUNCTION(BlueprintCallable, Category = "HUD|HUDCore|SubHUD")
		void DestroyPawnHUD(APawn* InPawn);

	// Cursor

	FORCEINLINE TSharedPtr<FExtendedAnalogCursor> GetAnalogCursor() const;

	/* Following functions serve as an interface between the cursor and blueprint users */

	/** 
	* Enable the analog cursor. This enables a software cursor with compatibility for split-screen cursor setups. 
	* The analog cursor also implements behavior for use with a gamepad thumbstick.
	*/
	UFUNCTION(BlueprintCallable, Category = "HUD|HUDCore|Cursor")
		void EnableAnalogCursor();

	/* Disable the analog cursor. */
	UFUNCTION(BlueprintCallable, Category = "HUD|HUDCore|Cursor")
		void DisableAnalogCursor();

	/* BP Utility. Returns if the analog cursor is valid, which also means it is enabled. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|Cursor")
		bool IsAnalogCursorValid() const;

	/* Returns the analog cursor position if valid, or the playercontroller mouse position. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|Cursor")
		FVector2D GetCursorPosition() const;

	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "HUD|HUDCore|Cursor")
		UCursorWidget* GetCustomCursorWidget() const;

	/** 
	* Creates a completely custom software cursor you controll through InCursorWidgetClass. 
	* To make this possible, invisible dummies are loaded instead of the software cursors specified in the project settings. 
	* Contrary to software cursors set there, this cursor can also have an associated player.
	* Any existing custom cursor widget will be removed.
	*/
	UFUNCTION(BlueprintCallable, Category = "HUD|HUDCore|Cursor")
		void MakeCustomCursorWidget(TSoftClassPtr<UCursorWidget> InCursorWidgetClass);

	UFUNCTION(BlueprintCallable, Category = "HUD|HUDCore|Cursor")
		void RemoveCustomCursorWidget();

};