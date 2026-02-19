using UnrealBuildTool;

public class UltimateEventManagerEditor : ModuleRules
{
    public UltimateEventManagerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore", 
                "InputCore",
                "EditorSubsystem",
                "UnrealEd",
                "LevelEditor",
                "ApplicationCore",
                "ToolMenus",
                "UltimateEventManager"
            }
        );
    }
}