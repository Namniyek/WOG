// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class WOGEditorTarget : TargetRules
{
	public WOGEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("WOG");
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
    }
}
