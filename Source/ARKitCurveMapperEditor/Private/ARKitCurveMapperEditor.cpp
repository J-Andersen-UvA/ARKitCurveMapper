#include "ARKitCurveMapperEditor.h"

#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "SARKITCurveMappingWindow.h"

const FName FARKitCurveMapperEditorModule::TabName(TEXT("ARKitCurveMapperTab"));

void FARKitCurveMapperEditorModule::StartupModule()
{
	FTabSpawnerEntry& Spawner = FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabName,
		FOnSpawnTab::CreateRaw(this, &FARKitCurveMapperEditorModule::SpawnTab)
	);

	Spawner.SetDisplayName(FText::FromString(TEXT("ARKit Curve Mapper")));
	Spawner.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FARKitCurveMapperEditorModule::RegisterMenus)
	);
}

void FARKitCurveMapperEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TabName);
}

void FARKitCurveMapperEditorModule::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
	FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");

	Section.AddMenuEntry(
		"OpenARKitCurveMapper",
		FText::FromString("ARKit Curve Mapper"),
		FText::FromString("Open ARKit Curve Mapper"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FARKitCurveMapperEditorModule::TabName);
		}))
	);
}

TSharedRef<SDockTab> FARKitCurveMapperEditorModule::SpawnTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SARKITCurveMappingWindow)
		];
}

IMPLEMENT_MODULE(FARKitCurveMapperEditorModule, ARKitCurveMapperEditor)
