/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputState.h"
#include "CursorScreenSpace.h"
#include "Layout/Geometry.h"

#include "CursorWidget.generated.h"


class UDeferPaintWidget;
class UCanvasPanel;


/*
* 
*/
UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = Cursor)
class UIADDITIONSPLUGIN_API UCursorWidget : public UUserWidget {
    GENERATED_BODY()

private:

	UPROPERTY()
		TSet<FName> CursorContexts;

	//UPROPERTY(Transient)
		//E_CursorScreenSpace CursorScreenSpace = E_CursorScreenSpace::PlayerScreen;

	UPROPERTY(Transient)
		bool bFreezeCursorToCenterOfScreen = false;

	UPROPERTY(Transient)
		FVector2D Position = FVector2D::ZeroVector;

protected:

	// Setup
	
	/* The canvas serves to offset cursors by a half when required (crosshair centering vs corner pointer etc.) */
	UPROPERTY(BlueprintReadOnly, Category = "Widgets", meta = (BindWidget))
		UCanvasPanel* CursorContainer = nullptr;

public:

private:

protected:

	// Setup

	virtual void NativeOnInitialized() override;

	// Tick

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void TickUpdateCursorPosition();

	// Appearance

	/* Implement in blueprints to change the cursor appearance based on cursor contexts. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Appearance")
		void OnCursorContextsChanged();

	// Delegates

	UFUNCTION()
		void ActOnInputDeviceChanged(EInputDevices InInputDevice, int32 InSlateUserIndex);

public:

	// Appearance

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void Show();

	UFUNCTION(BlueprintCallable, Category = "Appearance")
		void Hide();

	/* Returns the absolute position of this widget on the viewport. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Cursor")
		FVector2D GetPosition() const;

	/* Get if the widget freezes to the center of the player screen during TickUpdateCursorPosition. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Cursor")
		bool GetFreezeCursorToCenterOfScreen() const;

	/* Set if the widget freezes to the center of the player screen during TickUpdateCursorPosition. */
	UFUNCTION(BlueprintCallable, Category = "Cursor")
		void SetFreezeCursorToCenterOfScreen(bool bInFreezeCursorToCenterOfScreen);

	/* The cursor can be modified based on context, such as opening a certain menu or desiring a crosshair. */
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Cursor")
		const TSet<FName>& GetCursorContexts() const;

	/* The cursor can be modified based on context, such as opening a certain menu or desiring a crosshair. */
	UFUNCTION(BlueprintCallable, Category = "Cursor")
		void AddCursorContext(const FName& InContext);

	/* The cursor can be modified based on context, such as opening a certain menu or desiring a crosshair. */
	UFUNCTION(BlueprintCallable, Category = "Cursor")
		void RemoveCursorContext(const FName& InContext);

	/* The cursor can be modified based on context, such as opening a certain menu or desiring a crosshair. Utility function which allows you to pass in a condition used to decide to add or remove a context, common use when collecting contexts. */
	UFUNCTION(BlueprintCallable, Category = "Cursor")
		void AddOrRemoveCursorContext(const FName& InContext, bool bInAdd);

};
