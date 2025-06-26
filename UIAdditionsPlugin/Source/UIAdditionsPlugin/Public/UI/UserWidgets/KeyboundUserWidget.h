/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "KeyboundUserWidget.generated.h"


DECLARE_DYNAMIC_DELEGATE(FOnRequestWidgetInputAction);


UENUM(BlueprintType)
enum class E_AdditionalInputActionReplyModes : uint8 {
    HandleSpecified,
    HandleAllExceptSpecified
};


USTRUCT()
struct UIADDITIONSPLUGIN_API FS_WidgetInputActionListener {
	GENERATED_BODY()

	UPROPERTY()
		FOnRequestWidgetInputAction Delegate;

	UPROPERTY()
		bool bOnKeyDown;

	UPROPERTY()
		bool bOnKeyUp;

	// Initialize
	FS_WidgetInputActionListener() 
		: Delegate ()
		, bOnKeyDown (true)
		, bOnKeyUp (false)
	{}
    FS_WidgetInputActionListener(const FOnRequestWidgetInputAction& InDelegate, bool bInOnKeyDown, bool bInOnKeyUp) 
        : Delegate (InDelegate)
        , bOnKeyDown (bInOnKeyDown)
        , bOnKeyUp (bInOnKeyUp)
    {}
};


/*
* This class lives to provide a clean way to bind input actions to widget functions.
* Due to how input is routed, the only proper way to process input is through the OnKeyDown and other relevant methods.
* The input component present on UserWidgets does not respect that routing process: https://forums.unrealengine.com/t/widget-input-it-smells/784112/4
* This class compares and then processes input as input actions, registered through ListenForRoutedInputAction.
* Use it for common user created UI input actions, such as "sort a panel, increase the slider".
* Processed actions will handle the FReply on relevant methods.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UKeyboundUserWidget : public UUserWidget {
    GENERATED_BODY()

private:

    UPROPERTY()
        TMap<FName, FS_WidgetInputActionListener> ListenForRoutedInputActions;

protected:

public:

    UPROPERTY(EditAnywhere, Category = "Input")
        TSet<FName> AdditionalInputActionReplies;

    UPROPERTY(EditAnywhere, Category = "Input")
        E_AdditionalInputActionReplyModes AdditionalInputActionReplyMode = E_AdditionalInputActionReplyModes::HandleSpecified;

private:

    /* Processing of user input happens through this single method, instead of separately through OnKey... OnMouse... etc.  */
    FReply FindAndRequestInputAction(const FInputChord& InInputChord, bool bInIsKeyDown);

    /* Attempts to find a listener in "ListenForRoutedInputActions" by the arguments, to execute the delegate of. Not public or blueprint exposed to avoid confusion, because this method does not simulate routed user input. */
    FReply ProcessInputActionByName(const FName& InActionName, bool bInIsKeyDown);

protected:

    // Input

    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
   
    virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
  
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
   
    virtual FReply NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    
    /**
    * Utility method to create + bind a delegate and pass it + remaining params to ListenForRoutedInputAction. Binds a UObject method address on this UObject instance to this delegate.
    *
    * @param	InObject		UObject instance
    * @param	InFunc			    Member function address pointer
    * @param	InFunctionName		Name of member function, without class name
    *
    * NOTE:  Do not call this function directly.  Instead, call BindRoutedInputAction() which is a macro proxy function that
    *        automatically sets the function name string for the caller.
    */
    template<typename UserClass>
    void __Internal_BindRoutedInputAction(const FName& InActionName, bool bInOnKeyDown, bool bInOnKeyUp, UserClass* InObject, typename TMemFunPtrType<false, UserClass, void()>::Type InFunc, const FName& InFuncName) {
        FOnRequestWidgetInputAction Callback;
        Callback.__Internal_BindDynamic(InObject, InFunc, InFuncName);
        ListenForRoutedInputAction(InActionName, bInOnKeyDown, bInOnKeyUp, Callback);
    }

    /**
    * Utility method to create + bind a delegate and pass it + remaining params to ListenForRoutedInputAction. Helper macro to bind a member UFUNCTION on this UObject instance to a dynamic delegate. 
    *
    * @param	InObject	UObject instance
    * @param	InFunc		Function pointer to member UFUNCTION, usually in form &UClassName::FunctionName
    */
#define BindRoutedInputAction(InActionName, bInOnKeyDown, bInOnKeyUp, InObject, InFunc) __Internal_BindRoutedInputAction(InActionName, bInOnKeyDown, bInOnKeyUp, InObject, InFunc, STATIC_FUNCTION_FNAME(TEXT(#InFunc)))

    /**
    * Listen for an input action, processed when input is routed through this widget OnKeyDown, OnMouse etc. 
    * Generally you want to process "navigation" or "continuous" actions when a key is pressed / held (down), and actions once when the key is up.
    */
    UFUNCTION(BlueprintCallable, Category = "Input")
        void ListenForRoutedInputAction(const FName& InActionName, bool bInOnKeyDown, bool bInOnKeyUp, const FOnRequestWidgetInputAction& InDelegate);

    UFUNCTION(BlueprintCallable, Category = "Input")
        void StopListenForRoutedInputAction(const FName& InActionName);

public:

};