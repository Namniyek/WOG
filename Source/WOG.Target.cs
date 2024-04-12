// Copyright Epic Games, Inc. All Rights Reserved.

using System.Diagnostics.CodeAnalysis;
using UnrealBuildTool;

[SuppressMessage("ReSharper", "InconsistentNaming")]
public class WOGTarget : TargetRules
{
	public WOGTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.Add("WOG");
	}
}
