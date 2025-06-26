/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
using System.IO;
using UnrealBuildTool;

public class UIAdditionsPlugin : ModuleRules {
	public UIAdditionsPlugin(ReadOnlyTargetRules Target) : base(Target) {
		PrivatePCHHeaderFile = "Public/UIAdditionsPlugin.h";
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;
		IWYUSupport = IWYUSupport.Full;

		// While testing if all headers are included properly, set the following:
		// PCHUsage = PCHUsageMode.NoSharedPCHs;
		// bUseUnity = false;

		IncludeSubDirectoriesRecursive(ModuleDirectory + "\\Private", true);
		IncludeSubDirectoriesRecursive(ModuleDirectory + "\\Public", false);
		
		PrivateDependencyModuleNames.AddRange(new string[] {
			"MovieScene"
			, "MovieSceneTracks"
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core"
			, "CoreUObject"
			, "Engine"
			, "InputCore"
			, "InteractiveToolsFramework"
			, "UMG"
			, "Slate"
			, "SlateCore"
			, "DeveloperSettings"
			, "AdvancedWidgets"
			// Plugins
			, "UtilityPlugin"
			, "CentralUIPlugin"
		});
	}

	private void IncludeSubDirectoriesRecursive(string DirectoryPathToSearch, bool bIsPrivate) {
		foreach (string DirectoryPath in Directory.GetDirectories(DirectoryPathToSearch)) {
			if (bIsPrivate) {
				PrivateIncludePaths.Add(DirectoryPath);
			}
			else {
				PublicIncludePaths.Add(DirectoryPath);
			}
			IncludeSubDirectoriesRecursive(DirectoryPath, bIsPrivate);
		}
	}
}
