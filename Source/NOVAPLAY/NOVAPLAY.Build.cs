// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NOVAPLAY : ModuleRules
{
	public NOVAPLAY(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Se agregaron AIModule, GameplayTasks y NavigationSystem para soportar
		// AIController + Behavior Tree + Blackboard + NavMesh (persecucion del docente)
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "AIModule", "GameplayTasks", "NavigationSystem", "Slate", "SlateCore" });
	}
}