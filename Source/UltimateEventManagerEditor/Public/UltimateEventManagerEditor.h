#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FUltimateEventManagerEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    TSharedRef<SDockTab> SpawnEventDebuggerTab(const FSpawnTabArgs& Args);
    void CreateGameEvent();
    void CreateEventPayload();
    void RegisterMenus();
    void NotifyAssetCreated(UObject* Asset);
};