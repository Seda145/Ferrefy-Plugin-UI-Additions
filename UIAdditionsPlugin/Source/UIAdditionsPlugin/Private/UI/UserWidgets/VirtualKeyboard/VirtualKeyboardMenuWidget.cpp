/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "VirtualKeyboardMenuWidget.h"
#include "VirtualKeyboardWidget.h"
#include "LogUIAdditionsPlugin.h"


// Input

bool UVirtualKeyboardMenuWidget::IsAnyVirtualKeyboardSimulatingInput() const {
	for (UVirtualKeyboardWidget* WidgetX : GetVirtualKeyboardWidgets()) {
		if (!IsValid(WidgetX)) {
			continue;
		}
		if (WidgetX->IsSimulatingInput()) {
			return true;
		}
	}
	
	return false;
}

void UVirtualKeyboardMenuWidget::BindWidgetWhichReceivesInput(UWidget* InWidget) {
	if (!IsValid(InWidget)) {
		UE_LOG(LogUIAdditionsPlugin, Error, TEXT("InWidget is invalid."));
		return;
	}
	
	
	for (UVirtualKeyboardWidget* WidgetX : GetVirtualKeyboardWidgets()) {
		if (!IsValid(WidgetX)) {
			continue;
		}
		WidgetX->BindWidgetWhichReceivesInput(InWidget);
	}
}

