#include "UltimateEventManagerEditor.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "SEventDebuggerWidget.h"
#include "ToolMenus.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Data/GameEventData.h"
#include "Kismet2/KismetEditorUtilities.h"
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

void FUltimateEventManagerEditorModule::CreateGameEvent()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	FString PackagePath = "/Game";
	FString CurrentPath = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get().GetCurrentPath().GetInternalPathString();
	
	if (!CurrentPath.IsEmpty())
		PackagePath = CurrentPath;
	
	FString AssetName;
	FString UniquePackageName;
	AssetTools.CreateUniqueAssetName(PackagePath + "/NewGameEvent", "", UniquePackageName, AssetName);
	
	UObject* NewAsset = AssetTools.CreateAsset(
		AssetName,
		PackagePath,
		UGameEventData::StaticClass(),
		nullptr
	);
	
	NotifyAssetCreated(NewAsset);
}

void FUltimateEventManagerEditorModule::CreateEventPayload()
{
	FString PackagePath = "/Game";
	FString CurrentPath = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get().GetCurrentPath().GetInternalPathString();
	if (!CurrentPath.IsEmpty())
		PackagePath = CurrentPath;

	FString AssetName;
	FString UniquePackageName;
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(PackagePath + "/NewEventPayload", "", UniquePackageName, AssetName);

	UPackage* Package = CreatePackage(*UniquePackageName);

	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
		UGameEventPayload::StaticClass(),
		Package,
		FName(*AssetName),
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass()
	);
	
	NotifyAssetCreated(Blueprint);
}

void FUltimateEventManagerEditorModule::RegisterMenus()
{
	/**
	 * Debugger Menu
	 */
	UToolMenu* ToolsMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");

	FToolMenuSection& Section =
		ToolsMenu->AddSection("EventManager", FText::FromString("Event Manager"));

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

	/**
	 * Content Browser Menu
	 */
	UToolMenu* ContentMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AddNewContextMenu");

	FToolMenuSection& ContentSection =
		ContentMenu->AddSection("EventManagerAssets",FText::FromString("Event Manager"));

	ContentSection.AddMenuEntry(
		"CreateGameEvent",
		FText::FromString("Game Event"),
		FText::FromString("Create a new Game Event Data Asset"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(
			this,
			&FUltimateEventManagerEditorModule::CreateGameEvent
		))
	);

	ContentSection.AddMenuEntry(
		"CreateEventPayload",
		FText::FromString("Event Payload"),
		FText::FromString("Create a new Event Payload Blueprint"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(
			this,
			&FUltimateEventManagerEditorModule::CreateEventPayload
		))
	);
}

void FUltimateEventManagerEditorModule::NotifyAssetCreated(UObject* NewAsset)
{
	if (NewAsset)
	{
		FAssetRegistryModule::AssetCreated(NewAsset);
		NewAsset->MarkPackageDirty();
		
		TArray<UObject*> Objects;
		Objects.Add(NewAsset);
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
		IContentBrowserSingleton& ContentBrowserSingleton = ContentBrowserModule.Get();
		ContentBrowserSingleton.SyncBrowserToAssets(Objects);
	}
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FUltimateEventManagerEditorModule, UltimateEventManagerEditor)