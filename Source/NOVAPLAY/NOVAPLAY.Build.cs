// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class NOVAPLAY : ModuleRules
{
	public NOVAPLAY(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// AIModule, GameplayTasks y NavigationSystem: AIController + Behavior Tree + Blackboard + NavMesh
		// (persecucion del docente). UMG: todos los widgets (HUD, inventario, pausa, atrapado) heredan
		// de UUserWidget, que vive en ese modulo.
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "AIModule", "GameplayTasks", "NavigationSystem", "Slate", "SlateCore", "UMG" });
	}
}