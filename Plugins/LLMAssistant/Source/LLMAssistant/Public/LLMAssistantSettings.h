#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LLMAssistantSettings.generated.h"

UENUM(BlueprintType)
enum class ELLMProvider : uint8
{
    OpenAI UMETA(DisplayName = "OpenAI"),
    Anthropic UMETA(DisplayName = "Anthropic"),
    Custom UMETA(DisplayName = "Custom")
};

UCLASS(config = Editor, defaultconfig)
class LLMASSISTANT_API ULLMAssistantSettings : public UObject
{
    GENERATED_BODY()

public:
    ULLMAssistantSettings();

    UPROPERTY(config, EditAnywhere, Category = "LLM Settings")
    ELLMProvider Provider;

    UPROPERTY(config, EditAnywhere, Category = "LLM Settings")
    FString APIKey;

    UPROPERTY(config, EditAnywhere, Category = "LLM Settings")
    FString ModelName;

    UPROPERTY(config, EditAnywhere, Category = "LLM Settings", meta = (EditCondition = "Provider == ELLMProvider::Custom"))
    FString CustomEndpointURL;

    UPROPERTY(config, EditAnywhere, Category = "LLM Settings", meta = (ClampMin = "100", ClampMax = "100000"))
    int32 MaxTokens;

    UPROPERTY(config, EditAnywhere, Category = "LLM Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Temperature;
};