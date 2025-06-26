/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#include "UIAdditionsPluginEditorInstaller.h"
#include "AssetUtils.h"
#include "Engine/DataTable.h"
#include "Internationalization/StringTable.h"


// Setup

void FUIAdditionsPluginEditorInstaller::RunAutomatedInstaller() {
	UAssetUtils::AddDirectoryToScanAndCook(TEXT("/UIAdditionsPlugin/Localization/Text"), TEXT("StringTable"), UStringTable::StaticClass());
	UAssetUtils::AddDirectoryToScanAndCook(TEXT("/UIAdditionsPlugin/Data/DataTables"), TEXT("DataTable"), UDataTable::StaticClass());
}