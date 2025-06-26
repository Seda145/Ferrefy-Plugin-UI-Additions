/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "MenuWidget.h"

#include "VirtualKeyboardMenuWidget.generated.h"


/**
* Menu widget class which can be used to nest virtual keyboard widgets inside of it. 
* Useful when you want to manage and navigate between various types of virtual keyboard widgets
* , which is common if you want to display alpha / numeric / symbol or other types of panels separately.
*/
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DontUseGenericSpawnObject = "True"))
class UIADDITIONSPLUGIN_API UVirtualKeyboardMenuWidget : public UMenuWidget {
    GENERATED_BODY()

private:

protected:

public:

private:

protected:

public:

	// Widgets
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintPure = false, BlueprintCallable, Category = "Widgets")
		TArray<UVirtualKeyboardWidget*> GetVirtualKeyboardWidgets() const;
	
	// Input
	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Input")
		bool IsAnyVirtualKeyboardSimulatingInput() const;
	
    /**
    * Sets the widget which input from the virtual keyboard needs to be sent to.
    */
    UFUNCTION(BlueprintCallable, Category = "Widgets")
        void BindWidgetWhichReceivesInput(UWidget* InWidget);
        
};