/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "LocalizationUtils.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Internationalization/StringTable.h"
#include "LogUIAdditionsPlugin.h"


// BPFL | Utils | Localization

FName ULocalizationUtils::GetStringTableId(UStringTable* InStringTable) {
	return IsValid(InStringTable) ? InStringTable->GetStringTableId() : FName();
}

FString ULocalizationUtils::FindPreferredLocalizedDefaultLocale(ELocalizationLoadFlags InLocalizationFlags) {
	// Default the locale to the default specified by FPlatformMisc.
	const FString PlatformDefaultLocale = FPlatformMisc::GetDefaultLocale();
	FString PreferredLocale = PlatformDefaultLocale;

	const TArray<FString> Locales = FTextLocalizationManager::Get().GetLocalizedCultureNames(InLocalizationFlags);
	if (Locales.Num() > 0) {
		// Else, as safety? Test that the default locale is actually in Locales, else return the first localized culture name.
		if (!Locales.Contains(PreferredLocale)) {
			PreferredLocale = Locales[0];
		}
	}

	return PreferredLocale;
}

FText ULocalizationUtils::Localize(const FName& InStringTableId, const FString& InKey) {
	const FText Result = FText::FromStringTable(InStringTableId, InKey);
	if (Result.IsEmpty()) {
		UE_LOG(LogUIAdditionsPlugin, Warning, TEXT("Localized string empty. String table ID: %s, Key: %s"), *InStringTableId.ToString(), *InKey);
	}
	return Result;
}

// BPFL | Utils | Localization | Input

FText ULocalizationUtils::LocalizeInputMappingName(const FName& InStringTableId, const FName& InName) {
	return Localize(InStringTableId, FString("InputMapping_" + InName.ToString()));
}

FText ULocalizationUtils::LocalizeInputMappingDescription(const FName& InStringTableId, const FName& InName) {
	return Localize(InStringTableId, FString("InputMapping_" + InName.ToString()) + "_Description");
}

FText ULocalizationUtils::LocalizeSettingTitle(const FName& InStringTableId, const FString& InString) {
	return Localize(InStringTableId, FString("Setting_" + InString));
}

FText ULocalizationUtils::LocalizeSettingDescription(const FName& InStringTableId, const FString& InString) {
	return Localize(InStringTableId, FString("Setting_" + InString + "_Description"));
}

FText ULocalizationUtils::LocalizeSettingValue(const FName& InStringTableId, const FString& InString) {
	return Localize(InStringTableId, FString("SettingValue_" + InString));
}