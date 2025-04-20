#include "LLMAssistant.h"
#include "LLMAssistantStyle.h"
#include "LLMAssistantCommands.h"
#include "LLMAssistantTab.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "ISettingsModule.h"
#include "LLMAssistantSettings.h"

static const FName LLMAssistantTabName("LLMAssistant");

#define LOCTEXT_NAMESPACE "FLLMAssistantModule"

void FLLMAssistantModule::StartupModule()
{
    // Initialize styles and commands
    FLLMAssistantStyle::Initialize();
    FLLMAssistantStyle::ReloadTextures();
    FLLMAssistantCommands::Register();

    PluginCommands = MakeShareable(new FUICommandList);
    PluginCommands->MapAction(
        FLLMAssistantCommands::Get().OpenPluginWindow,
        FExecuteAction::CreateRaw(this, &FLLMAssistantModule::PluginButtonClicked),
        FCanExecuteAction());

    RegisterMenus();

    // Register tab spawner
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(LLMAssistantTabName,
        FOnSpawnTab::CreateRaw(this, &FLLMAssistantModule::SpawnLLMTab))
        .SetDisplayName(LOCTEXT("LLMAssistantTabTitle", "LLM Assistant"))
        .SetMenuType(ETabSpawnerMenuType::Enabled);  // Changed from Hidden to Enabled

    // Register settings
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        TSharedPtr<ISettingsSection> SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "LLM Assistant",
            LOCTEXT("LLMAssistantSettingsName", "LLM Assistant"),
            LOCTEXT("LLMAssistantSettingsDescription", "Configure the LLM Assistant plugin."),
            GetMutableDefault<ULLMAssistantSettings>()
        );
    }
}

void FLLMAssistantModule::ShutdownModule()
{
    FLLMAssistantStyle::Shutdown();
    FLLMAssistantCommands::Unregister();
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(LLMAssistantTabName);

    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "LLM Assistant");
    }
}

TSharedRef<SDockTab> FLLMAssistantModule::SpawnLLMTab(const FSpawnTabArgs& TabSpawnArgs)
{
    TSharedRef<SLLMAssistantTab> AssistantTabWidget = SNew(SLLMAssistantTab);
    LLMAssistantTab = AssistantTabWidget;

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            AssistantTabWidget
        ];
}

void FLLMAssistantModule::PluginButtonClicked()
{
    FGlobalTabmanager::Get()->TryInvokeTab(LLMAssistantTabName);
}

void FLLMAssistantModule::RegisterMenus()
{
    // Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
    FToolMenuOwnerScoped OwnerScoped(this);

    // Add toolbar button
    UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
    FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
    {
        FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(
            FLLMAssistantCommands::Get().OpenPluginWindow,
            FText::GetEmpty(),
            FText::GetEmpty(),
            FSlateIcon(FLLMAssistantStyle::GetStyleSetName(), "LLMAssistant.OpenPluginWindow")
        ));
        Entry.SetCommandList(PluginCommands);
    }

    // Add menu item under Window
    UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
    FToolMenuSection& MenuSection = Menu->FindOrAddSection("WindowLayout");
    {
        MenuSection.AddEntry(FToolMenuEntry::InitMenuEntry(
            FLLMAssistantCommands::Get().OpenPluginWindow,
            FText::GetEmpty(),
            FText::GetEmpty(),
            FSlateIcon(FLLMAssistantStyle::GetStyleSetName(), "LLMAssistant.OpenPluginWindow")
        )).SetCommandList(PluginCommands);
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FLLMAssistantModule, LLMAssistant)