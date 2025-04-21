#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "LLMService.h"
#include "AssetRegistry/IAssetRegistry.h"

class SEditableTextBox;
class SMultiLineEditableTextBox;
template <typename OptionType>
class SComboBox;

class LLMASSISTANT_API SLLMAssistantTab : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SLLMAssistantTab) {}
    SLATE_END_ARGS()

    void Construct(const FArguments &InArgs);

private:
    TSharedPtr<SEditableTextBox> QueryTextBox;
    TSharedPtr<SMultiLineEditableTextBox> ResponseTextBox;
    TSharedPtr<FLLMService> LLMService;
    TArray<FString> ConversationHistory;

    // Asset indexing and generation
    IAssetRegistry *AssetRegistry;
    TArray<FAssetData> IndexedAssets;
    TArray<UClass *> IndexedBlueprintClasses;
    TArray<AActor *> IndexedActors;

    TSharedPtr<SComboBox<TSharedPtr<FString>>> AssetTypeComboBox;
    TArray<TSharedPtr<FString>> AssetTypeOptions;
    TSharedPtr<FString> SelectedAssetType;
    TSharedPtr<SEditableTextBox> AssetNameTextBox;

    FReply OnSubmitButtonClicked();
    FReply OnClearButtonClicked();
    FReply OnGenerateAssetButtonClicked();
    FReply OnIndexAssetsButtonClicked();

    void OnQueryTextCommitted(const FText &Text, ETextCommit::Type CommitType);
    void OnAssetTypeSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo);
    void HandleLLMResponse(const FString &Response);
    void HandleLLMError(const FString &ErrorMessage);

    // Asset indexing and generation methods
    void IndexBlueprintClasses();
    void IndexSceneActors();
    void IndexProjectAssets();
    void GenerateBlueprint(const FString &BlueprintName, const FString &Description);
    void GenerateAsset(const FString &AssetName, const FString &AssetType, const FString &Description);

    // Helper methods
    FString GetConversationText() const;
    void AppendToResponse(const FString &Text);
    TSharedRef<SWidget> MakeAssetTypeItemWidget(TSharedPtr<FString> InItem);
};