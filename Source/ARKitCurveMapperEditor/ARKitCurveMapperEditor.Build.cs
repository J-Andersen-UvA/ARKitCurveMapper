using UnrealBuildTool;

public class ARKitCurveMapperEditor : ModuleRules
{
    public ARKitCurveMapperEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "ToolMenus",
                "UnrealEd",
                "AssetTools",
                "ContentBrowser",
                "EditorStyle",
                "PropertyEditor",
                "InputCore",
                "ApplicationCore",
                "ARKitCurveMapper",
                "DesktopPlatform",
                "ApplicationCore",
            }
        );
    }
}
