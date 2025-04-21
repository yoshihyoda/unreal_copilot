#include "LLMAssistantTab.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"
#include "EditorStyleSet.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "Engine/Level.h"
#include "Editor.h"
#include "EditorLevelUtils.h"
#include "LevelEditor.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Materials/Material.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Sound/SoundWave.h"
#include "EngineUtils.h"
#include "EdGraph/EdGraph.h"
#include "BlueprintEditor.h"
#include "EdGraphNode_Comment.h"

#define LOCTEXT_NAMESPACE "LLMAssistantTab"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SLLMAssistantTab::Construct(const FArguments &InArgs)
{
    LLMService = MakeShareable(new FLLMService());
    LLMService->OnResponseReceived.BindRaw(this, &SLLMAssistantTab::HandleLLMResponse);
    LLMService->OnRequestFailed.BindRaw(this, &SLLMAssistantTab::HandleLLMError);

    // Initialize asset registry
    FAssetRegistryModule &AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    AssetRegistry = &AssetRegistryModule.Get();

    // Define asset type options
    AssetTypeOptions.Add(MakeShareable(new FString("Blueprint")));
    AssetTypeOptions.Add(MakeShareable(new FString("Material")));
    AssetTypeOptions.Add(MakeShareable(new FString("Texture")));
    AssetTypeOptions.Add(MakeShareable(new FString("StaticMesh")));
    AssetTypeOptions.Add(MakeShareable(new FString("Sound")));

    SelectedAssetType = AssetTypeOptions[0];

    ChildSlot
        [SNew(SVerticalBox)

         // Title
         + SVerticalBox::Slot()
               .AutoHeight()
               .Padding(10, 5)
                   [SNew(STextBlock)
                        .Text(LOCTEXT("LLMAssistantTitle", "LLM Assistant for Unreal Engine"))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))]

         // Response Area
         + SVerticalBox::Slot()
               .FillHeight(1.0f)
               .Padding(10)
                   [SNew(SBorder)
                        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
                        .Padding(5)
                            [SNew(SScrollBox) + SScrollBox::Slot()
                                                    [SAssignNew(ResponseTextBox, SMultiLineEditableTextBox)
                                                         .IsReadOnly(true)
                                                         .AutoWrapText(true)
                                                         .Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))]]]

         // Input Area
         + SVerticalBox::Slot()
               .AutoHeight()
               .Padding(10, 0, 10, 10)
                   [SNew(SBorder)
                        .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
                        .Padding(5)
                            [SNew(SVerticalBox)

                             // Query Input
                             + SVerticalBox::Slot()
                                   .AutoHeight()
                                   .Padding(0, 0, 0, 5)
                                       [SAssignNew(QueryTextBox, SEditableTextBox)
                                            .HintText(LOCTEXT("QueryHint", "Type your Unreal Engine question here..."))
                                            .OnTextCommitted(this, &SLLMAssistantTab::OnQueryTextCommitted)]

                             // Buttons Row
                             + SVerticalBox::Slot()
                                   .AutoHeight()
                                       [SNew(SHorizontalBox)

                                        // Submit Button
                                        + SHorizontalBox::Slot()
                                              .AutoWidth()
                                              .Padding(0, 0, 5, 0)
                                                  [SNew(SButton)
                                                       .Text(LOCTEXT("SubmitButton", "Submit"))
                                                       .OnClicked(this, &SLLMAssistantTab::OnSubmitButtonClicked)]

                                        // Clear Button
                                        + SHorizontalBox::Slot()
                                              .AutoWidth()
                                              .Padding(0, 0, 5, 0)
                                                  [SNew(SButton)
                                                       .Text(LOCTEXT("ClearButton", "Clear Conversation"))
                                                       .OnClicked(this, &SLLMAssistantTab::OnClearButtonClicked)]

                                        // Index Assets Button
                                        + SHorizontalBox::Slot()
                                              .AutoWidth()
                                                  [SNew(SButton)
                                                       .Text(LOCTEXT("IndexAssetsButton", "Index Assets"))
                                                       .OnClicked(this, &SLLMAssistantTab::OnIndexAssetsButtonClicked)]]

                             // Asset Generation Section
                             + SVerticalBox::Slot()
                                   .AutoHeight()
                                   .Padding(0, 10, 0, 0)
                                       [SNew(SVerticalBox)

                                        // Section Header
                                        + SVerticalBox::Slot()
                                              .AutoHeight()
                                              .Padding(0, 0, 0, 5)
                                                  [SNew(STextBlock)
                                                       .Text(LOCTEXT("AssetGenerationHeader", "Asset Generation"))
                                                       .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))]

                                        // Asset Type and Name Row
                                        + SVerticalBox::Slot()
                                              .AutoHeight()
                                              .Padding(0, 0, 0, 5)
                                                  [SNew(SHorizontalBox)

                                                   // Asset Type Combo Box
                                                   + SHorizontalBox::Slot()
                                                         .FillWidth(0.4f)
                                                         .Padding(0, 0, 5, 0)
                                                             [SAssignNew(AssetTypeComboBox, SComboBox<TSharedPtr<FString>>)
                                                                  .OptionsSource(&AssetTypeOptions)
                                                                  .OnSelectionChanged(this, &SLLMAssistantTab::OnAssetTypeSelectionChanged)
                                                                  .OnGenerateWidget(this, &SLLMAssistantTab::MakeAssetTypeItemWidget)
                                                                  .InitiallySelectedItem(SelectedAssetType)
                                                                  .Content()
                                                                      [SNew(STextBlock)
                                                                           .Text_Lambda([this]()
                                                                                        { return SelectedAssetType.IsValid() ? FText::FromString(*SelectedAssetType) : LOCTEXT("SelectAssetType", "Select Asset Type"); })]]

                                                   // Asset Name Input
                                                   + SHorizontalBox::Slot()
                                                         .FillWidth(0.6f)
                                                             [SAssignNew(AssetNameTextBox, SEditableTextBox)
                                                                  .HintText(LOCTEXT("AssetNameHint", "Asset Name"))]]

                                        // Generate Button
                                        + SVerticalBox::Slot()
                                              .AutoHeight()
                                                  [SNew(SButton)
                                                       .Text(LOCTEXT("GenerateAssetButton", "Generate Asset"))
                                                       .OnClicked(this, &SLLMAssistantTab::OnGenerateAssetButtonClicked)]]]]];
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

FReply SLLMAssistantTab::OnIndexAssetsButtonClicked()
{
    // Update UI to show indexing process has started
    AppendToResponse(TEXT("\n\nIndexing assets, blueprints, and actors..."));

    // Perform the indexing in stages
    IndexProjectAssets();
    IndexBlueprintClasses();
    IndexSceneActors();

    // Update UI with results
    FString ResultMessage = FString::Printf(TEXT("\nIndexing complete! Found:\n- %d Assets\n- %d Blueprint Classes\n- %d Scene Actors"),
                                            IndexedAssets.Num(),
                                            IndexedBlueprintClasses.Num(),
                                            IndexedActors.Num());

    AppendToResponse(ResultMessage);

    return FReply::Handled();
}

FReply SLLMAssistantTab::OnGenerateAssetButtonClicked()
{
    FString AssetName = AssetNameTextBox->GetText().ToString();
    FString AssetType = SelectedAssetType.IsValid() ? *SelectedAssetType : TEXT("Blueprint");

    if (AssetName.IsEmpty())
    {
        AppendToResponse(TEXT("\n\nError: Please enter an asset name."));
        return FReply::Handled();
    }

    // Prepare a description based on the conversation
    FString Description;
    if (ConversationHistory.Num() > 0)
    {
        // Use the last query as a description
        FString LastQuery = ConversationHistory.Last();
        // Strip "User: " prefix if present
        if (LastQuery.StartsWith(TEXT("User: ")))
        {
            LastQuery = LastQuery.RightChop(6);
        }
        Description = LastQuery;
    }
    else
    {
        Description = FString::Printf(TEXT("Generate a %s named %s"), *AssetType, *AssetName);
    }

    // Generate the asset
    AppendToResponse(FString::Printf(TEXT("\n\nGenerating %s: %s..."), *AssetType, *AssetName));

    if (AssetType == TEXT("Blueprint"))
    {
        GenerateBlueprint(AssetName, Description);
    }
    else
    {
        GenerateAsset(AssetName, AssetType, Description);
    }

    return FReply::Handled();
}

void SLLMAssistantTab::OnQueryTextCommitted(const FText &Text, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter)
    {
        OnSubmitButtonClicked();
    }
}

void SLLMAssistantTab::OnAssetTypeSelectionChanged(TSharedPtr<FString> NewSelection, ESelectInfo::Type SelectInfo)
{
    if (NewSelection.IsValid())
    {
        SelectedAssetType = NewSelection;
    }
}

void SLLMAssistantTab::HandleLLMResponse(const FString &Response)
{
    // Add to conversation history
    ConversationHistory.Add(FString::Printf(TEXT("Assistant: %s"), *Response));

    // Update text display
    ResponseTextBox->SetText(FText::FromString(GetConversationText()));
}

void SLLMAssistantTab::HandleLLMError(const FString &ErrorMessage)
{
    // Add error to conversation history
    ConversationHistory.Add(FString::Printf(TEXT("Error: %s"), *ErrorMessage));

    // Update text display
    ResponseTextBox->SetText(FText::FromString(GetConversationText()));
}

void SLLMAssistantTab::IndexProjectAssets()
{
    // Clear previous results
    IndexedAssets.Empty();

    // Get all assets in the project
    TArray<FAssetData> AllAssets;
    AssetRegistry->GetAllAssets(AllAssets, true);

    // Filter to relevant asset types if needed
    for (const FAssetData &Asset : AllAssets)
    {
        IndexedAssets.Add(Asset);
    }
}

void SLLMAssistantTab::IndexBlueprintClasses()
{
    // Clear previous results
    IndexedBlueprintClasses.Empty();

    // Get all blueprint classes
    TArray<UClass *> BlueprintClasses;
    for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
    {
        UClass *Class = *ClassIt;

        // Check if it's a blueprint class
        if (Class->IsChildOf(AActor::StaticClass()) && Class->HasAnyClassFlags(CLASS_CompiledFromBlueprint))
        {
            IndexedBlueprintClasses.Add(Class);
        }
    }
}

void SLLMAssistantTab::IndexSceneActors()
{
    // Clear previous results
    IndexedActors.Empty();

    // Get all actors in the current level
    UWorld *EditorWorld = GEditor->GetEditorWorldContext().World();
    if (!EditorWorld)
    {
        return;
    }

    for (TActorIterator<AActor> ActorIt(EditorWorld); ActorIt; ++ActorIt)
    {
        AActor *Actor = *ActorIt;
        if (Actor && !Actor->IsActorBeingDestroyed() && !Actor->IsTemplate())
        {
            IndexedActors.Add(Actor);
        }
    }
}

void SLLMAssistantTab::GenerateBlueprint(const FString &BlueprintName, const FString &Description)
{
    // Create the blueprint package
    FString PackagePath = TEXT("/Game/Blueprints/Generated/");
    FString PackageName = PackagePath + BlueprintName;

    // Create package and blueprint
    UPackage *Package = CreatePackage(*PackageName);
    UBlueprint *Blueprint = nullptr;

    // Generate a blueprint based on Actor
    Blueprint = FKismetEditorUtilities::CreateBlueprint(AActor::StaticClass(), Package, *BlueprintName,
                                                        BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());

    if (Blueprint)
    {
        // Add a description as a comment
        if (Blueprint->UbergraphPages.Num() > 0)
        {
            UEdGraph *Graph = Blueprint->UbergraphPages[0];
            UEdGraphNode_Comment *CommentNode = NewObject<UEdGraphNode_Comment>(Graph);
            if (CommentNode)
            {
                CommentNode->NodePosX = 0;
                CommentNode->NodePosY = 0;
                CommentNode->NodeWidth = 400;
                CommentNode->NodeHeight = 100;
                CommentNode->NodeComment = Description;
                CommentNode->CommentColor = FLinearColor(1.0f, 0.7f, 0.0f);
                Graph->AddNode(CommentNode);
            }
        }

        // Mark package dirty and save it
        Package->MarkPackageDirty();
        FAssetRegistryModule::AssetCreated(Blueprint);

        // Now let's use the LLM to populate the blueprint
        FString LLMQuery = FString::Printf(TEXT("Generate Unreal Engine blueprint logic for: %s. The blueprint should: %s"),
                                           *BlueprintName, *Description);

        // Add to conversation history
        ConversationHistory.Add(FString::Printf(TEXT("User: %s"), *LLMQuery));

        // Update response text to show thinking state
        FString CurrentConversation = GetConversationText();
        CurrentConversation += TEXT("\n\nThinking about blueprint implementation...");
        ResponseTextBox->SetText(FText::FromString(CurrentConversation));

        // Send query to LLM
        LLMService->SendQuery(LLMQuery);

        AppendToResponse(FString::Printf(TEXT("\nBlueprint created at: %s"), *PackageName));
    }
    else
    {
        AppendToResponse(TEXT("\nError: Failed to create blueprint."));
    }
}

void SLLMAssistantTab::GenerateAsset(const FString &AssetName, const FString &AssetType, const FString &Description)
{
    // Get the asset tools module
    FAssetToolsModule &AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    IAssetTools &AssetTools = AssetToolsModule.Get();

    // Define target package path
    FString PackagePath = TEXT("/Game/Generated/");
    FString AssetPath = PackagePath + AssetName;

    // Define asset class based on type
    UClass *AssetClass = nullptr;
    if (AssetType == TEXT("Material"))
    {
        AssetClass = UMaterial::StaticClass();
    }
    else if (AssetType == TEXT("Texture"))
    {
        AssetClass = UTexture2D::StaticClass();
    }
    else if (AssetType == TEXT("StaticMesh"))
    {
        AssetClass = UStaticMesh::StaticClass();
    }
    else if (AssetType == TEXT("Sound"))
    {
        AssetClass = USoundWave::StaticClass();
    }

    if (AssetClass)
    {
        // Create the asset
        UObject *NewAsset = AssetTools.CreateAsset(AssetName, PackagePath, AssetClass, nullptr);

        if (NewAsset)
        {
            // Mark package dirty
            NewAsset->MarkPackageDirty();

            // Now let's use the LLM to get details about how to configure this asset
            FString LLMQuery = FString::Printf(TEXT("Generate instructions for configuring an Unreal Engine %s named %s. The asset should: %s"),
                                               *AssetType, *AssetName, *Description);

            // Add to conversation history
            ConversationHistory.Add(FString::Printf(TEXT("User: %s"), *LLMQuery));

            // Update response text to show thinking state
            FString CurrentConversation = GetConversationText();
            CurrentConversation += TEXT("\n\nThinking about asset configuration...");
            ResponseTextBox->SetText(FText::FromString(CurrentConversation));

            // Send query to LLM
            LLMService->SendQuery(LLMQuery);

            AppendToResponse(FString::Printf(TEXT("\nAsset created at: %s"), *AssetPath));
        }
        else
        {
            AppendToResponse(FString::Printf(TEXT("\nError: Failed to create %s asset."), *AssetType));
        }
    }
    else
    {
        AppendToResponse(FString::Printf(TEXT("\nError: Unsupported asset type: %s"), *AssetType));
    }
}

FString SLLMAssistantTab::GetConversationText() const
{
    FString Result;

    for (int32 i = 0; i < ConversationHistory.Num(); i++)
    {
        const FString &Entry = ConversationHistory[i];

        if (!Result.IsEmpty())
        {
            Result += TEXT("\n\n");
        }

        Result += Entry;
    }

    return Result;
}

void SLLMAssistantTab::AppendToResponse(const FString &Text)
{
    FString CurrentText = ResponseTextBox->GetText().ToString();
    ResponseTextBox->SetText(FText::FromString(CurrentText + Text));
}

TSharedRef<SWidget> SLLMAssistantTab::MakeAssetTypeItemWidget(TSharedPtr<FString> InItem)
{
    return SNew(STextBlock)
        .Text(FText::FromString(InItem.IsValid() ? *InItem : TEXT("Unknown")));
}

#undef LOCTEXT_NAMESPACE