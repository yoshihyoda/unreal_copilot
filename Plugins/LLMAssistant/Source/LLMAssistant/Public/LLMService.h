#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "Dom/JsonObject.h"
#include "LLMAssistantSettings.h"

DECLARE_DELEGATE_OneParam(FOnLLMResponseReceived, const FString&);
DECLARE_DELEGATE_OneParam(FOnLLMRequestFailed, const FString&);

class LLMASSISTANT_API FLLMService
{
public:
    FLLMService();
    ~FLLMService();

    void SendQuery(const FString& Query);

    FOnLLMResponseReceived OnResponseReceived;
    FOnLLMRequestFailed OnRequestFailed;

private:
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateOpenAIRequest(const FString& Query);
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateAnthropicRequest(const FString& Query);
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> CreateCustomRequest(const FString& Query);
    void OnResponseReceived_Internal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    FString ParseOpenAIResponse(const TSharedPtr<FJsonObject>& JsonObject);
    FString ParseAnthropicResponse(const TSharedPtr<FJsonObject>& JsonObject);
    FString ParseCustomResponse(const TSharedPtr<FJsonObject>& JsonObject);

    FHttpModule* HttpModule;
    ULLMAssistantSettings* Settings;
};