#include "LLMAssistantSettings.h"

ULLMAssistantSettings::ULLMAssistantSettings()
{
    Provider = ELLMProvider::OpenAI;
    ModelName = TEXT("gpt-4");
    MaxTokens = 1000;
    Temperature = 0.7f;
    CustomEndpointURL = TEXT("https://api.anthropic.com/v1/messages");

}