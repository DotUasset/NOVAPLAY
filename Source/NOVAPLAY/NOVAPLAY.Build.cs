// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NOVAPLAY : ModuleRules
{
	public NOVAPLAY(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay" });
	}
}
