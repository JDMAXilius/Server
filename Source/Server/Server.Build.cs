// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Server : ModuleRules
{
	public Server(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core","UMG", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "OnlineSubsystem",/* "OnlineSubsystemSteam",*/ "OnlineSubsystemUtils"/*, "OnlineSubsystemEOS" , "OnlineSubsystem", "OnlineSubsystemUtils", "OnlineSubsystemBlueprints","Networking", "GameplayTasks", "PlayFab", "PlayFabCommon", "PlayFabCpp"*/ });
	}
}
