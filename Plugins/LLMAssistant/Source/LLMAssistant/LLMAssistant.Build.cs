using UnrealBuildTool;

public class LLMAssistant : ModuleRules
{
    public LLMAssistant(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "HTTP",
            "Json",
            "JsonUtilities",
            "Slate",
            "SlateCore"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "Projects",
                "ToolMenus",
                "UnrealEd",
                "LevelEditor",
                "EditorStyle",
                "ApplicationCore",
                "DesktopPlatform",
                "Settings"
            });
        }
    }
}
