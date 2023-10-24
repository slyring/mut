// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MergerUtility : ModuleRules
{
	public MergerUtility(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(
			new string[]
			{
				// ... add public include paths required here ...
			}
		);


		PrivateIncludePaths.AddRange(
			new string[]
			{
				// ... add other private include paths required here ...
			}
		);


		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
		);

		if (Target.Type ==
		    TargetRules.TargetType.Editor) //we only want this to be included for editor builds but not packaged builds
		{
			PublicDependencyModuleNames.AddRange(new[]
			{
				"UnrealEd",
				"AssetRegistry",
				"MeshUtilities",
				"ContentBrowser",
			});
		}


		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				// ... add private dependencies that you statically link with here ...	
			}
		);


		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
		);
	}
}