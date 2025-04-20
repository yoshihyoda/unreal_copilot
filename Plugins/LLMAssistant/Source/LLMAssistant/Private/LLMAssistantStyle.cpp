#include "LLMAssistantStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"

TSharedPtr<FSlateStyleSet> FLLMAssistantStyle::StyleInstance = nullptr;

void FLLMAssistantStyle::Initialize()
{
    if (!StyleInstance.IsValid())
    {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
    }
}

void FLLMAssistantStyle::Shutdown()
{
    FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
    ensure(StyleInstance.IsUnique());
    StyleInstance.Reset();
}

FName FLLMAssistantStyle::GetStyleSetName()
{
    static FName StyleSetName(TEXT("LLMAssistantStyle"));
    return StyleSetName;
}

void FLLMAssistantStyle::ReloadTextures()
{
    if (FSlateApplication::IsInitialized())
    {
        FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
    }
}

const ISlateStyle& FLLMAssistantStyle::Get()
{
    return *StyleInstance;
}

TSharedRef<FSlateStyleSet> FLLMAssistantStyle::Create()
{
    TSharedRef<FSlateStyleSet> StyleSet = MakeShareable(new FSlateStyleSet(GetStyleSetName()));
    StyleSet->SetContentRoot(IPluginManager::Get().FindPlugin("LLMAssistant")->GetBaseDir() / TEXT("Resources"));

    // Define icon size
    const FVector2D Icon40x40(40.0f, 40.0f);

    // Set the icon without using IMAGE_BRUSH macro
    FString IconPath = StyleSet->RootToContentDir(TEXT("ButtonIcon_40x"), TEXT(".png"));
    StyleSet->Set("LLMAssistant.OpenPluginWindow", new FSlateImageBrush(IconPath, Icon40x40));

    return StyleSet;
}