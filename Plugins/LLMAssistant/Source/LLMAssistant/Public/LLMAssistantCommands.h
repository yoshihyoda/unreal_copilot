#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "LLMAssistantStyle.h"

class FLLMAssistantCommands : public TCommands<FLLMAssistantCommands>
{
public:
    FLLMAssistantCommands()
        : TCommands<FLLMAssistantCommands>(TEXT("LLMAssistant"),
            NSLOCTEXT("Contexts", "LLMAssistant", "LLM Assistant Plugin"),
            NAME_None,
            FLLMAssistantStyle::GetStyleSetName())
    {
    }

    // TCommands<> interface
    virtual void RegisterCommands() override;

public:
    TSharedPtr<FUICommandInfo> OpenPluginWindow;
};