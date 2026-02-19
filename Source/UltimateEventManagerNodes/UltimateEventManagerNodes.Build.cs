using UnrealBuildTool;

public class UltimateEventManagerNodes : ModuleRules
{
    public UltimateEventManagerNodes(ReadOnlyTargetRules Target) : base(Target)
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
                "UnrealEd",
                "BlueprintGraph",
                "KismetCompiler",
                "StructUtilsEngine",
                "StructUtils",
                "UltimateEventManager"
            }
        );
    }
}