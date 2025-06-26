/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InputCoreTypes.h"
#include "Layout/Geometry.h"
#include "Components/SlateWrapperTypes.h"
#include "Framework/Commands/InputChord.h"
#include "GenericPlatform/ICursor.h"
#include "Templates/SharedPointer.h"
#include "InputState.h"
#include "Components/Widget.h"
#include "Templates/SubclassOf.h"
#include "GameFramework/PlayerInput.h"

#include "SlateUtils.generated.h"

class FSlateUser;
class UWidget;
class UUserWidget;
class APlayerController;


UENUM(BlueprintType)
enum class E_BPInputDevices : uint8 {
	None,
	Keyboard,
	Mouse,
	Gamepad,
	OculusTouch,
	HTCViveWands,
	AnySpatialDevice,
	TabletFingers
};


UCLASS()
class UIADDITIONSPLUGIN_API USlateUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

private:

protected:

public:

	// Input | Slate | Navigation Config

	/**
	* These are Slate (UI) reserved navigation / input action mappings.
	* "NavNext" is used similar to tab key usage in a web browser.
	* "NavPrevious" is left unassigned, Shift + NavNext is hardcoded to act as "NavPrevious" (Slate compatibility)
	*/

	static const FName InputActionNavBack;
	static const FName InputActionNavDown;
	static const FName InputActionNavLeft;
	static const FName InputActionNavNext;
	static const FName InputActionNavPrevious;
	static const FName InputActionNavRight;
	static const FName InputActionNavSelect;
	static const FName InputActionNavUp;

	// Input | Slate | Custom

	/**
	* The Nav mappings below are not present in the engine's implementation of Slate navigation (FNavigationConfig) and require custom implementation in the UI.
	* These are common UI actions that need to be consistent (have the same input implementation everywhere) for good UX.
	*/

	static const FName InputActionNavExtra;
	static const FName InputActionNavMainMenu;
	static const FName InputActionNavReset;

	// Input | Custom

	/**
	* Mappings below require custom implementation. These are just extremely common input mappings to move pawns.
	*/

	static const FName InputAxisForward;
	static const FName InputAxisBackward;
	static const FName InputAxisLeft;
	static const FName InputAxisRight;
	static const FName InputAxisUp;
	static const FName InputAxisDown;
	static const FName InputAxisYawPos;
	static const FName InputAxisYawNeg;
	static const FName InputAxisPitchPos;
	static const FName InputAxisPitchNeg;

	// UI Routing

	/**
	* These are paths used with UMenuWidget navigation.
	*/

	static const FName UIRouteMainMenu;
	static const FName UIRouteQuit;

private:

protected:

public:

	// PlayerController

	/* Get Slate user index by controller ID. Returns -1 if failed. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|PlayerController", meta = (CallableWithoutWorldContext))
		static int32 GetSlateUserIndexForPlayerController(APlayerController* InPlayerController);

	/* Get Slate user index by controller ID. Returns -1 if failed. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|PlayerController", meta = (CallableWithoutWorldContext))
		static int32 GetSlateUserIndexForPlayerControllerID(int32 InControllerID);

	/* Get Slate user. Returns nullptr if failed. Not available to BP. */
	static TSharedPtr<FSlateUser> GetSlateUserForPlayerController(APlayerController* InPlayerController);
	
	/* Returns true if InPlayerController is the first local player controller, by using UGameInstance::GetFirstLocalPlayerController which uses its local player order to test against. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|PlayerController", meta = (CallableWithoutWorldContext))
		static bool IsFirstLocalPlayerController(APlayerController* InPlayerController);

	/* Getter was missing from PlayerController for blueprint users, so this is the getter. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|PlayerController", meta = (CallableWithoutWorldContext))
		static UPlayerInput* GetPlayerInput(APlayerController* InPlayerController);

	// Widget Geometry

	/* Returns the position of the center of a player's screen. If not possible, returns FVector2D::ZeroVector. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|WidgetGeometry", meta = (CallableWithoutWorldContext))
		static FVector2D GetCenterOfPlayerScreen(ULocalPlayer* InLocalPlayer);

	/* Returns the position of the center of a player's screen. If not possible, returns FVector2D::ZeroVector. */
	static FVector2D GetCenterOfPlayerScreen(const UGameViewportClient* InGameViewportClient, ULocalPlayer* InLocalPlayer);

	/* Clamps any absolute coordinate InGeometry, returning a new absolute coordinate. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|WidgetGeometry")
		static FVector2D ClampAbsolutePositionToGeometry(const FGeometry& InGeometry, const FVector2D& InPosition);

	/* Clamps InLocalPosition local to InGeometry to its bounds, returning a new local coordinate. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|WidgetGeometry")
		static FVector2D ClampLocalPositionToGeometry(const FGeometry& InGeometry, const FVector2D& InLocalPosition);

	// Widget

	/**
	* Returns true if the input visibility is Visible, HitTestInvisible or SelfHitTestInvisible.
	* This util is useful if you just want to know what a UWidget's Visibility property is set to: "USlateUtils::IsVisible(Widget->GetVisibility))".
	* 
	* The engine offers similar methods (example: UWidget::IsVisible()) which give different results and at times are not desired for their implementation.
	*/
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|Widget", meta = (CallableWithoutWorldContext))
		static bool IsVisible(ESlateVisibility InSlateVisibility);

	template<class T>
	static T* FindAncestorWidgetByClass(const UWidget* InWidget, bool bInMatchSubclass = true, bool bInLookOutsideUserWidget = true) {
		static_assert(TPointerIsConvertibleFromTo<T, const UWidget>::Value, "'T' Template parameter to FindAncestorWidgetByClass must be derived from UWidget");
		return (T*)FindAncestorWidgetByClass(InWidget, T::StaticClass(), bInMatchSubclass, bInLookOutsideUserWidget);
	}

	/* Find the first ancestor widget of InWidget of a class. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|Widget", meta = (CallableWithoutWorldContext))
		static UWidget* FindAncestorWidgetByClass(const UWidget* InWidget, const TSubclassOf<UWidget> InAncestorClass, bool bInMatchSubclass = true, bool bInLookOutsideUserWidget = true);

	/* Find the ancestor widget of InWidget. Return true if it was found. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|Widget", meta = (CallableWithoutWorldContext))
		static bool FindAncestorWidget(const UWidget* InWidget, const UWidget* InAncestorWidget, bool bInLookOutsideUserWidget = true);

	// Conversion

	/* 
	* Gets the key and char codes for sending keys for the platform.
	* This is partly borrowed from the WidgetInteractionComponent.
	*/
	static void GetKeyAndCharCodes(const FKey& InKey, bool& bOutHasKeyCode, uint32& OutKeyCode, bool& bOutHasCharCode, uint32& OutCharCode);

	/* Create an FInputChord from a FKeyEvent */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|Conversion", meta = (CallableWithoutWorldContext))
		static FInputChord GetInputChordFromKeyEvent(const FKeyEvent& KeyEvent);

	/* Create an FInputChord from a FPointerEvent. Use bInIsMouseWheelScroll when the event is caused by a scrolling mouse wheel. UE5.2 Slate doesn't send a valid key for a mouse wheel scroll which is fixed up here. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|Conversion", meta = (CallableWithoutWorldContext))
		static FInputChord GetInputChordFromPointerEvent(const FPointerEvent& PointerEvent, bool bInIsMouseWheelScroll);

	/* Create an FInputChord from a FInputActionKeyMapping. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|Conversion", meta = (CallableWithoutWorldContext))
		static FInputChord GetInputChordFromInputActionKeyMapping(const FInputActionKeyMapping& InInputActionKeyMapping);

	/* Create an FInputChord from a FInputAxisKeyMapping. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|Conversion", meta = (CallableWithoutWorldContext))
		static FInputChord GetInputChordFromInputAxisKeyMapping(const FInputAxisKeyMapping& InInputAxisKeyMapping);

	/* Conversion method to get an enum Blueprint can work with out of EInputDevices */
	static E_BPInputDevices ConvertEInputDevicesToBP(EInputDevices InInputDevice);

	// Input device

	static EInputDevices GetCurrentInputDevice(int32 InSlateUserIndex);

	static EInputDevices GetCurrentInputDevice(APlayerController* InPlayerController);

	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputDevice", meta = (CallableWithoutWorldContext))
		static E_BPInputDevices GetCurrentInputDeviceBP(APlayerController* InPlayerController);

	static TSet<EMouseCursor::Type> GetMouseCursorTypes();

	/**
	* Process a key event through Slate. If possible, also processes a key char event. Returns true if no events are left unhandled. 
	*
	* UE5.2: Works very well to simulate input actions for overall UI control, does not work well to simulate text editing.
	* Explanation: 
	* 	UE5.2: On the OS level a physical key press would result in getting the correct character.
	*	When simulating a character event, the engine lacks some functionality:
	*	Modifier keys (shift, caps etc.) can't be simulated through this method properly. 
	*	Sending "R, no shift, no caps" to an editable text box widget results in "R" (see FSlateEditableTextLayout::HandleTypeChar),
	*	ignoring FCharacterEvent. It's a Slate bug because it seems this key processing has to be finished up at the OS level, and Slate allows wrong results.
	*/
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputDevice")
		static bool ProcessKeyEvent(int32 InSlateUserIndex, const FKey& InKey, bool bInIsKeyDown, bool bInIsRepeat);

	// Input Mappings

	/* Get the names of all standard slate actions (directional navigation, select, back) */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static TArray<FName> GetStandardSlateNavInputNames();

	/* Get a default value for a standard Slate action. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static TArray<FInputActionKeyMapping> GetDefaultStandardSlateNavInputMapping(const FName& InName);

	/* Configures the project settings (input) if an input mapping is missing. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static bool ValidateStandardSlateNavInputMappings();

	/* Get the names of all extended slate actions (These are common implementable UI actions not registered in Slate.) */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static TArray<FName> GetExtendedSlateNavInputNames();

	/* Get a default value for an extended Slate action. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static TArray<FInputActionKeyMapping> GetDefaultExtendedSlateNavInputMapping(const FName& InName);

	/* Configures the project settings (input) if an input mapping is missing. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static bool ValidateExtendedSlateNavInputMappings();

	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static TArray<FName> GetCustomInputAxisNames();

	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static TArray<FInputAxisKeyMapping> GetDefaultCustomInputAxisMapping(const FName& InName);

	UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
		static bool ValidateCustomInputAxisMappings();

	/* Returns true if an input action mapping exist in UInputSettings matching exactly both the name and an FInputChord created from provided FKeyEvent. */
    UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
        static bool FindActionMappingByKeyEventAndName(FKeyEvent InKeyEvent, const FName& InName);

    /* Returns true if an input action mapping exist in UInputSettings matching exactly both the name and an FInputChord created from provided FPointerEvent. Mouse wheel scroll key is fixed up if bInIsMouseWheelScroll when UE5.2 sends an invalid key in the PointerEvent. */
    UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
        static bool FindActionMappingByPointerEventAndName(FPointerEvent InPointerEvent, const FName& InName, bool bInIsMouseWheelScroll);

    /* Returns true if an input action mapping exist in UInputSettings matching exactly both the name and FInputChord. */
    UFUNCTION(BlueprintCallable, Category = "BPFL|SlateUtils|InputMappings", meta = (CallableWithoutWorldContext))
        static bool FindActionMappingByInputChordAndName(FInputChord InInputChord, const FName& InName);


	// For the blueprint users, let's expose the constants (methods are quick to generate with notepad++ multi line editing):


	/* Input | Slate | Navigation Config */
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavBack();
				
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavDown();
				
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavLeft();
				
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavNext();
				
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavPrevious();
				
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavRight();
				
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavSelect();
				
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavUp();

	/* Input | Slate | Custom */
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavExtra();
		
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavMainMenu();
		
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputActionNavReset();

	/* Input | Custom */

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisForward();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisBackward();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisLeft();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisRight();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisUp();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisDown();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisYawPos();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisYawNeg();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisPitchPos();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantInputAxisPitchNeg();

	/* UI Routing */

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantUIRouteMainMenu();
		
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "BPFL|SlateUtils|Constants", meta = (CallableWithoutWorldContext))
		static const FName& ConstantUIRouteQuit();


};
