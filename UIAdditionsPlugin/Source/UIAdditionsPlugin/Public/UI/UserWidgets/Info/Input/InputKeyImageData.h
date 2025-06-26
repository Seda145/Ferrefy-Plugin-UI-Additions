/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Engine/Texture2D.h"
#include "UObject/SoftObjectPtr.h"
#include "Engine/DataTable.h" 

#include "InputKeyImageData.generated.h"


USTRUCT(BlueprintType)
struct UIADDITIONSPLUGIN_API FS_InputKeyImageData : public FTableRowBase {
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, EditAnywhere)
        TMap<FKey, TSoftObjectPtr<UTexture2D>> KeyTextures;

    // Initialize
    FS_InputKeyImageData() 
        : KeyTextures (TMap<FKey, TSoftObjectPtr<UTexture2D>>())
    {}
};
