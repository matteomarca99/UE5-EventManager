#include "UltimateEventManagerEditor.h"
#include "SEventDebuggerWidget.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FUltimateEventManagerEditorModule"

void FUltimateEventManagerEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		"EventDebugger",
		FOnSpawnTab::CreateRaw(this, &FUltimateEventManagerEditorModule::SpawnEventDebuggerTab)
	)
	.SetDisplayName(FText::FromString("Event Debugger"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);
	
	UToolMenus::RegisterStartupCallback(
	FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUltimateEventManagerEditorModule::RegisterMenus)
);
}

void FUltimateEventManagerEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("EventDebugger");
}

TSharedRef<SDockTab> FUltimateEventManagerEditorModule::SpawnEventDebuggerTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SEventDebuggerWidget)
		];
}

void FUltimateEventManagerEditorModule::RegisterMenus()
{
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");

	FToolMenuSection& Section = Menu->AddSection("EventManager", FText::FromString("Event Manager"));

	Section.AddMenuEntry(
		"OpenEventDebugger",
		FText::FromString("Event Debugger"),
		FText::FromString("Open the Event Debugger window"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FName("EventDebugger"));
		}))
	);
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUltimateEventManagerEditorModule, UltimateEventManagerEditor)