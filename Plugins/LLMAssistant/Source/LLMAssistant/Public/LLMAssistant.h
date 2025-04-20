#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"

class FToolBarBuilder;
class FMenuBuilder;
class SLLMAssistantTab;

class FLLMAssistantModule : public IModuleInterface
{
public:
    // Remove any incorrect 'override' keywords if they exist
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    void PluginButtonClicked();

private:
    void RegisterMenus();
    TSharedRef<SDockTab> SpawnLLMTab(const FSpawnTabArgs& TabSpawnArgs);

    TSharedPtr<class FUICommandList> PluginCommands;
    TSharedPtr<SLLMAssistantTab> LLMAssistantTab;
};