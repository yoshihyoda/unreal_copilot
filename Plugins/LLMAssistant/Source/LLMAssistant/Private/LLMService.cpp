#include "LLMService.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FLLMService::FLLMService()
{
    HttpModule = &FHttpModule::Get();
    Settings = GetMutableDefault<ULLMAssistantSettings>();
}

FLLMService::~FLLMService()
{
    // Nothing to clean up
}

void FLLMService::SendQuery(const FString &Query)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();

    switch (Settings->Provider)
    {
    case ELLMProvider::OpenAI:
        Request = CreateOpenAIRequest(Query);
        break;
    case ELLMProvider::Anthropic:
        Request = CreateAnthropicRequest(Query);
        break;
    case ELLMProvider::Custom:
        Request = CreateCustomRequest(Query);
        break;
    }

    Request->OnProcessRequestComplete().BindRaw(this, &FLLMService::OnResponseReceived_Internal);
    Request->ProcessRequest();
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FLLMService::CreateOpenAIRequest(const FString &Query)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetURL(TEXT("https://api.openai.com/v1/chat/completions"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings->APIKey));

    // Create request body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), Settings->ModelName);
    JsonObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);
    JsonObject->SetNumberField(TEXT("temperature"), Settings->Temperature);

    TArray<TSharedPtr<FJsonValue>> MessagesArray;

    // System message
    TSharedPtr<FJsonObject> SystemMsgObj = MakeShareable(new FJsonObject);
    SystemMsgObj->SetStringField(TEXT("role"), TEXT("system"));
    SystemMsgObj->SetStringField(TEXT("content"), TEXT("You are a helpful assistant for Unreal Engine development. Provide concise and accurate information."));
    MessagesArray.Add(MakeShareable(new FJsonValueObject(SystemMsgObj)));

    // User message
    TSharedPtr<FJsonObject> UserMsgObj = MakeShareable(new FJsonObject);
    UserMsgObj->SetStringField(TEXT("role"), TEXT("user"));
    UserMsgObj->SetStringField(TEXT("content"), Query);
    MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMsgObj)));

    JsonObject->SetArrayField(TEXT("messages"), MessagesArray);

    // Convert to string
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(RequestBody);

    return Request;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FLLMService::CreateAnthropicRequest(const FString &Query)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetURL(TEXT("https://api.anthropic.com/v1/messages"));
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("x-api-key"), Settings->APIKey);
    Request->SetHeader(TEXT("anthropic-version"), TEXT("2023-06-01"));

    // Create request body
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), Settings->ModelName);
    JsonObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);

    // Create messages array
    TArray<TSharedPtr<FJsonValue>> MessagesArray;

    // System message is now a separate field in the root object
    JsonObject->SetStringField(TEXT("system"), TEXT("You are a helpful assistant for Unreal Engine development. Provide concise and accurate information."));

    // User message
    TSharedPtr<FJsonObject> UserMsgObj = MakeShareable(new FJsonObject);
    UserMsgObj->SetStringField(TEXT("role"), TEXT("user"));

    // Create content array for the message
    TArray<TSharedPtr<FJsonValue>> ContentArray;

    // Add text content
    TSharedPtr<FJsonObject> TextContent = MakeShareable(new FJsonObject);
    TextContent->SetStringField(TEXT("type"), TEXT("text"));
    TextContent->SetStringField(TEXT("text"), Query);
    ContentArray.Add(MakeShareable(new FJsonValueObject(TextContent)));

    // Set content array to the message
    UserMsgObj->SetArrayField(TEXT("content"), ContentArray);

    // Add user message to messages array
    MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMsgObj)));

    // Set messages array to the request
    JsonObject->SetArrayField(TEXT("messages"), MessagesArray);

    // Convert to string
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(RequestBody);

    return Request;
}

TSharedRef<IHttpRequest, ESPMode::ThreadSafe> FLLMService::CreateCustomRequest(const FString &Query)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = HttpModule->CreateRequest();
    Request->SetURL(Settings->CustomEndpointURL);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Settings->APIKey));

    // Create request body - using OpenAI format as default, adjust as needed
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject);
    JsonObject->SetStringField(TEXT("model"), Settings->ModelName);
    JsonObject->SetNumberField(TEXT("max_tokens"), Settings->MaxTokens);
    JsonObject->SetNumberField(TEXT("temperature"), Settings->Temperature);

    TArray<TSharedPtr<FJsonValue>> MessagesArray;

    // System message
    TSharedPtr<FJsonObject> SystemMsgObj = MakeShareable(new FJsonObject);
    SystemMsgObj->SetStringField(TEXT("role"), TEXT("system"));
    SystemMsgObj->SetStringField(TEXT("content"), TEXT("You are a helpful assistant for Unreal Engine development. Provide concise and accurate information."));
    MessagesArray.Add(MakeShareable(new FJsonValueObject(SystemMsgObj)));

    // User message
    TSharedPtr<FJsonObject> UserMsgObj = MakeShareable(new FJsonObject);
    UserMsgObj->SetStringField(TEXT("role"), TEXT("user"));
    UserMsgObj->SetStringField(TEXT("content"), Query);
    MessagesArray.Add(MakeShareable(new FJsonValueObject(UserMsgObj)));

    JsonObject->SetArrayField(TEXT("messages"), MessagesArray);

    // Convert to string
    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);

    Request->SetContentAsString(RequestBody);

    return Request;
}

void FLLMService::OnResponseReceived_Internal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (!bWasSuccessful || !Response.IsValid())
    {
        FString ErrorStr = TEXT("Error: Unable to connect to the LLM service.");
        if (Response.IsValid())
        {
            ErrorStr = FString::Printf(TEXT("Error %d: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
        }

        OnRequestFailed.ExecuteIfBound(ErrorStr);
        return;
    }

    if (Response->GetResponseCode() != 200)
    {
        FString ErrorStr = FString::Printf(TEXT("HTTP Error %d: %s"), Response->GetResponseCode(), *Response->GetContentAsString());
        OnRequestFailed.ExecuteIfBound(ErrorStr);
        return;
    }

    const FString ResponseStr = Response->GetContentAsString();
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseStr);

    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        OnRequestFailed.ExecuteIfBound(TEXT("Error: Failed to parse response from LLM service."));
        return;
    }

    FString ParsedResponse;

    switch (Settings->Provider)
    {
    case ELLMProvider::OpenAI:
        ParsedResponse = ParseOpenAIResponse(JsonObject);
        break;
    case ELLMProvider::Anthropic:
        ParsedResponse = ParseAnthropicResponse(JsonObject);
        break;
    case ELLMProvider::Custom:
        ParsedResponse = ParseCustomResponse(JsonObject);
        break;
    }

    if (ParsedResponse.IsEmpty())
    {
        OnRequestFailed.ExecuteIfBound(TEXT("Error: Unable to extract response content from LLM service."));
        return;
    }

    OnResponseReceived.ExecuteIfBound(ParsedResponse);
}

FString FLLMService::ParseOpenAIResponse(const TSharedPtr<FJsonObject> &JsonObject)
{
    // Extract text from OpenAI response format
    const TArray<TSharedPtr<FJsonValue>> *ChoicesArray;
    if (JsonObject->TryGetArrayField(TEXT("choices"), ChoicesArray) && ChoicesArray->Num() > 0)
    {
        const TSharedPtr<FJsonObject> *Choice;
        if ((*ChoicesArray)[0]->TryGetObject(Choice))
        {
            const TSharedPtr<FJsonObject> *MessageObj;
            if (Choice->Get()->TryGetObjectField(TEXT("message"), MessageObj))
            {
                FString Content;
                if (MessageObj->Get()->TryGetStringField(TEXT("content"), Content))
                {
                    return Content;
                }
            }
        }
    }

    return TEXT("");
}

FString FLLMService::ParseAnthropicResponse(const TSharedPtr<FJsonObject> &JsonObject)
{
    // First check if response has valid content array
    const TArray<TSharedPtr<FJsonValue>> *ContentArray;
    if (JsonObject->TryGetArrayField(TEXT("content"), ContentArray) && ContentArray->Num() > 0)
    {
        const TSharedPtr<FJsonObject> *ContentItem;
        if ((*ContentArray)[0]->TryGetObject(ContentItem))
        {
            FString Type;
            FString Text;
            if (ContentItem->Get()->TryGetStringField(TEXT("type"), Type) &&
                Type == TEXT("text") &&
                ContentItem->Get()->TryGetStringField(TEXT("text"), Text))
            {
                return Text;
            }
        }
    }

    // For older Anthropic API versions
    FString Completion;
    if (JsonObject->TryGetStringField(TEXT("completion"), Completion))
    {
        return Completion;
    }

    return TEXT("");
}

FString FLLMService::ParseCustomResponse(const TSharedPtr<FJsonObject> &JsonObject)
{
    // This should be customized based on the API being used
    // Falling back to OpenAI format for now
    return ParseOpenAIResponse(JsonObject);
}