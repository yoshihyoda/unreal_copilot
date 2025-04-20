#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "LLMService.h"

class SEditableTextBox;
class SMultiLineEditableTextBox;

class LLMASSISTANT_API SLLMAssistantTab : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLLMAssistantTab) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    TSharedPtr<SEditableTextBox> QueryTextBox;
    TSharedPtr<SMultiLineEditableTextBox> ResponseTextBox;
    TSharedPtr<FLLMService> LLMService;
    TArray<FString> ConversationHistory;

    FReply OnSubmitButtonClicked();
    FReply OnClearButtonClicked();
    void OnQueryTextCommitted(const FText& Text, ETextCommit::Type CommitType);
    void HandleLLMResponse(const FString& Response);
    void HandleLLMError(const FString& ErrorMessage);

    // Added missing method declaration
    FString GetConversationText() const;
};