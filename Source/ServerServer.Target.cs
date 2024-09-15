using UnrealBuildTool;
using System.Collections.Generic;


[SupportedPlatforms(UnrealPlatformClass.Server)]
public class ServerServerTarget : TargetRules
{
	public ServerServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		CppStandard = CppStandardVersion.Cpp20;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("Server");
	}
}