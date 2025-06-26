/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "DisabledCursorWidget.generated.h"


/* UDisabledCursorWidget:
* Used as a dummy for software cursor entries present in the project settings. This dummy makes it possible to have an invisible cursor on the default implementation.
* This allows us to take full control over what we display, which is required when you wish to make a complex UserWidget based cursor (see HUD Core).
*/
UCLASS(BlueprintType, NotBlueprintable, ClassGroup = Cursor)
class UIADDITIONSPLUGIN_API UDisabledCursorWidget : public UUserWidget {
    GENERATED_BODY()


};
