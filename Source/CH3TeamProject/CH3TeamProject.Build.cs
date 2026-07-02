// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CH3TeamProject : ModuleRules
{
	public CH3TeamProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "NavigationSystem" });
	}
}
