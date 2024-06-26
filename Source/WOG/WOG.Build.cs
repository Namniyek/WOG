// Copyright Epic Games, Inc. All Rights Reserved.

using System.Diagnostics.CodeAnalysis;
using UnrealBuildTool;

[SuppressMessage("ReSharper", "InconsistentNaming")]
public class WOG : ModuleRules
{
	public WOG(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "OnlineSubsystem",
            "EnhancedInput",
            "AnimGraphRuntime",
            "Niagara",
            "AGRPRO",
            "TargetSystem",
            "MotionWarping",
            "AutoSettingsCore",
            "AutoSettings",
            "AutoSettingsInput",
            "CPathfinding",
            "CommonUI",
            "MetasoundEngine",
            "GeometryCollectionEngine",
            "AIModule"
        });

        PublicIncludePaths.Add("WOG/");

        PrivateDependencyModuleNames.AddRange(new string[] {
            "Slate",
            "SlateCore",
            "GameplayAbilities",
            "GameplayTags",
            "GameplayTasks"
        });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
