#include "LLMAssistantCommands.h"

#define LOCTEXT_NAMESPACE "FLLMAssistantModule"

void FLLMAssistantCommands::RegisterCommands()
{
    UI_COMMAND(OpenPluginWindow, "LLM Assistant", "Open the LLM Assistant window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE