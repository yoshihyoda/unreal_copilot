#include "LLMAssistantTab.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "LLMAssistantTab"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLLMAssistantTab::Construct(const FArguments& InArgs)
{
    LLMService = MakeShareable(new FLLMService());
    LLMService->OnResponseReceived.BindRaw(this, &SLLMAssistantTab::HandleLLMResponse);
    LLMService->OnRequestFailed.BindRaw(this, &SLLMAssistantTab::HandleLLMError);

    ChildSlot
        [
            SNew(SVerticalBox)

                // Title
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(10, 5)
                [
                    SNew(STextBlock)
                        .Text(LOCTEXT("LLMAssistantTitle", "LLM Assistant for Unreal Engine"))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
                ]

                // Response Area
                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .Padding(10)
                [
                    SNew(SBorder)
                        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
                        .Padding(5)
                        [
                            SNew(SScrollBox)
                                + SScrollBox::Slot()
                                [
                                    SAssignNew(ResponseTextBox, SMultiLineEditableTextBox)
                                        .IsReadOnly(true)
                                        .AutoWrapText(true)
                                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
                                ]
                        ]
                ]

                // Input Area
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(10, 0, 10, 10)
                [
                    SNew(SBorder)
                        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
                        .Padding(5)
                        [
                            SNew(SVerticalBox)

                                // Query Input
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 5)
                                [
                                    SAssignNew(QueryTextBox, SEditableTextBox)
                                        .HintText(LOCTEXT("QueryHint", "Type your Unreal Engine question here..."))
                                        .OnTextCommitted(this, &SLLMAssistantTab::OnQueryTextCommitted)
                                ]

                                // Buttons Row
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SNew(SHorizontalBox)

                                        // Submit Button
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        .Padding(0, 0, 5, 0)
                                        [
                                            SNew(SButton)
                                                .Text(LOCTEXT("SubmitButton", "Submit"))
                                                .OnClicked(this, &SLLMAssistantTab::OnSubmitButtonClicked)
                                        ]

                                        // Clear Button
                                        + SHorizontalBox::Slot()
                                        .AutoWidth()
                                        [
                                            SNew(SButton)
                                                .Text(LOCTEXT("ClearButton", "Clear Conversation"))
                                                .OnClicked(this, &SLLMAssistantTab::OnClearButtonClicked)
                                        ]
                                ]
                        ]
                ]
        ];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

FReply SLLMAssistantTab::OnSubmitButtonClicked()
{
    FString Query = QueryTextBox->GetText().ToString();

    if (!Query.IsEmpty())
    {
        // Add to conversation history
        ConversationHistory.Add(FString::Printf(TEXT("User: %s"), *Query));

        // Update response text to show thinking state
        FString CurrentConversation = GetConversationText();
        CurrentConversation += TEXT("\n\nThinking...");
        ResponseTextBox->SetText(FText::FromString(CurrentConversation));

        // Send query to LLM
        LLMService->SendQuery(Query);

        // Clear input
        QueryTextBox->SetText(FText::GetEmpty());
    }

    return FReply::Handled();
}

FReply SLLMAssistantTab::OnClearButtonClicked()
{
    ConversationHistory.Empty();
    ResponseTextBox->SetText(FText::GetEmpty());
    return FReply::Handled();
}

void SLLMAssistantTab::OnQueryTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter)
    {
        OnSubmitButtonClicked();
    }
}

void SLLMAssistantTab::HandleLLMResponse(const FString& Response)
{
    // Add to conversation history
    ConversationHistory.Add(FString::Printf(TEXT("Assistant: %s"), *Response));

    // Update text display
    ResponseTextBox->SetText(FText::FromString(GetConversationText()));
}

void SLLMAssistantTab::HandleLLMError(const FString& ErrorMessage)
{
    // Add error to conversation history
    ConversationHistory.Add(FString::Printf(TEXT("Error: %s"), *ErrorMessage));

    // Update text display
    ResponseTextBox->SetText(FText::FromString(GetConversationText()));
}

FString SLLMAssistantTab::GetConversationText() const
{
    FString Result;

    for (int32 i = 0; i < ConversationHistory.Num(); i++)
    {
        const FString& Entry = ConversationHistory[i];

        if (!Result.IsEmpty())
        {
            Result += TEXT("\n\n");
        }

        Result += Entry;
    }

    return Result;
}

#undef LOCTEXT_NAMESPACE