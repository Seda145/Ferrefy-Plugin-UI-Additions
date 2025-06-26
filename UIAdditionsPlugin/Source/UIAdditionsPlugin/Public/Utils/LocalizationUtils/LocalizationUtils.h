/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Internationalization/LocalizedTextSourceTypes.h"

#include "LocalizationUtils.generated.h"


class UStringTable;


/**
* This class provides small utilities for procedural localization, useful when procedurally generating widgets.
*/
UCLASS()
class UIADDITIONSPLUGIN_API ULocalizationUtils : public UBlueprintFunctionLibrary {
	GENERATED_BODY()

private:

protected:

public:

private:

protected:

public:

	// BPFL|Utils|Localization

	/* Blueprint callable method to retrieve a string table id from a string table. This was, oddly, not yet blueprint accessible from UStringTable itself. */
	UFUNCTION(BlueprintCallable, Category = "BPFL|LocalizationUtils")
		static FName GetStringTableId(UStringTable* InStringTable);

	/* Returns the platform default locale by default. If not localized, and other localized culture names exist, return the first localized culture name found. */
	static FString FindPreferredLocalizedDefaultLocale(ELocalizationLoadFlags InLocalizationFlags = ELocalizationLoadFlags::Game);

	/* Attempts to localize procedurally from a string table. If you wish to gather entries for localization on non procedural content, you should not be using this. InStringTableId is formatted as "/UIAdditionsPlugin/Localization/Text/ST_HID.ST_HID" */
	UFUNCTION(BlueprintCallable, Category = "BPFL|LocalizationUtils")
		static FText Localize(const FName& InStringTableId, const FString& InKey);

	// BPFL|Utils|Localization|Input

	/* Attempts to localize procedurally from a string table. If you wish to gather entries for localization on non procedural content, you should not be using this. InStringTableId is formatted as "/UIAdditionsPlugin/Localization/Text/ST_HID.ST_HID". Given the key "Forward", it will look for "InputMapping_Forward". */
	UFUNCTION(BlueprintCallable, Category = "BPFL|LocalizationUtils")
		static FText LocalizeInputMappingName(const FName& InStringTableId, const FName& InName);

	/* Attempts to localize procedurally from a string table. If you wish to gather entries for localization on non procedural content, you should not be using this. InStringTableId is formatted as "/UIAdditionsPlugin/Localization/Text/ST_HID.ST_HID". Given the key "Forward", it will look for "InputMapping_Forward_Description". */
	UFUNCTION(BlueprintCallable, Category = "BPFL|LocalizationUtils")
		static FText LocalizeInputMappingDescription(const FName& InStringTableId, const FName& InName);

	/* Attempts to localize procedurally from a string table. If you wish to gather entries for localization on non procedural content, you should not be using this. InStringTableId is formatted as "/UIAdditionsPlugin/Localization/Text/ST_HID.ST_HID". Given the key "GraphicsQuality", it will look for "Setting_GraphicsQuality". */
	UFUNCTION(BlueprintCallable, Category = "BPFL|LocalizationUtils")
		static FText LocalizeSettingTitle(const FName& InStringTableId, const FString& InString);

	/* Attempts to localize procedurally from a string table. If you wish to gather entries for localization on non procedural content, you should not be using this. InStringTableId is formatted as "/UIAdditionsPlugin/Localization/Text/ST_HID.ST_HID". Given the key "GraphicsQuality", it will look for "Setting_GraphicsQuality_Description". */
	UFUNCTION(BlueprintCallable, Category = "BPFL|LocalizationUtils")
		static FText LocalizeSettingDescription(const FName& InStringTableId, const FString& InString);

	/* Attempts to localize procedurally from a string table. If you wish to gather entries for localization on non procedural content, you should not be using this. InStringTableId is formatted as "/UIAdditionsPlugin/Localization/Text/ST_HID.ST_HID". Given the key "HighQuality", it will look for "SettingValue_HighQuality" */
	UFUNCTION(BlueprintCallable, Category = "BPFL|LocalizationUtils")
		static FText LocalizeSettingValue(const FName& InStringTableId, const FString& InString);
};