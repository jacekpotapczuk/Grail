#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "SSGrailDictionaryWidget.h"
#include "Microsoft/AllowMicrosoftPlatformTypes.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"

int32 SSGrailDictionaryWidget::NewEntryIndex = 0;

void SSGrailDictionaryWidget::Construct(const FArguments& InArgs)
{
	NewEntryIndex = 0;
	ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(120.f)
				[
					SNew(SButton)
					.Text(FText::FromString("Add Element"))
					.OnClicked(this, &SSGrailDictionaryWidget::OnAddClicked)
				]
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		.Padding(5)
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				SAssignNew(DictionarySlotsContainer, SVerticalBox)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.Text(FText::FromString("Save Data"))
			.OnClicked(this, &SSGrailDictionaryWidget::OnSaveDataClicked)
		]
	];
	
	RebuildDictionarySlots();
}

FReply SSGrailDictionaryWidget::OnAddClicked()
{
	const FString NewKey = FString::Printf(TEXT("Key %d"), NewEntryIndex++);

	InternalMap.Add(NewKey, FValueField(TEXT(""), TEXT(""), NewEntryIndex));
	InternalMap.ValueSort([](const FValueField& A, const FValueField& B)
	{
		return A.DisplayPosition < B.DisplayPosition;
	});
	
	RebuildDictionarySlots();
	UpdateInvalidEntries();
	
	return FReply::Handled();
}

FReply SSGrailDictionaryWidget::OnSaveDataClicked()
{
	if (ErrorKeys.Num() > 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("All keys should be unique.")));
		return FReply::Handled();
	}

	if (InternalMap.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Add at least one entry.")));
		return FReply::Handled();
	}

	FString SavePath;

	if (!TryGetSavePath(SavePath))
	{
		// user canceled
		return FReply::Handled();
	}

	TrySaveToJson(SavePath);
	
	return FReply::Handled();
}

bool SSGrailDictionaryWidget::TryGetSavePath(FString& OutPath)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	
	if (!DesktopPlatform)
	{
		return false;
	}

	TArray<FString> OutFiles;
	const bool bFileSelected = DesktopPlatform->SaveFileDialog(
		nullptr,
		TEXT("Save Data"),
		FPaths::ProjectDir(),
		TEXT("dict.json"),
		TEXT("JSON Files (*.json)|*.json"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (!bFileSelected || OutFiles.Num() == 0)
	{
		return false;
	}

	OutPath = OutFiles[0];
	return true;
}

bool SSGrailDictionaryWidget::TrySaveToJson(const FString& SavePath)
{
	const TSharedRef<FJsonObject> JsonObject = MakeShared<FJsonObject>();
	
	for (const auto& Pair : InternalMap)
	{
		JsonObject->SetStringField(Pair.Value.DisplayKey, Pair.Value.Value);
	}

	FString OutputString;
	const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
	
	if (!FJsonSerializer::Serialize(JsonObject, Writer))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to serialize JSON"));
		return false;
	}

	if (!FFileHelper::SaveStringToFile(OutputString, *SavePath))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to save file: %s"), *SavePath);
		return false;
	}

	return true;
}

void SSGrailDictionaryWidget::RebuildDictionarySlots()
{
	if (!DictionarySlotsContainer.IsValid())
	{
		return;
	}

	DictionarySlotsContainer->ClearChildren();

	for (TTuple<FString, FValueField>& Entry : InternalMap)
	{
		RebuildDictionarySlot(Entry);
	}
}

void SSGrailDictionaryWidget::RebuildDictionarySlot(TTuple<FString, FValueField>& Entry)
{
	const FString Key = Entry.Key;
	FString& Value = Entry.Value.Value;

	DictionarySlotsContainer->AddSlot()
							.AutoHeight()
							.Padding(2)
	[
		SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(0.3f)
		[
			BuildKeySlot(Key)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		[
			BuildValueSlot(Key)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(0.05f)
		[
			BuildRemoveSlot(Key)
		]
	];
}

TSharedRef<SEditableTextBox> SSGrailDictionaryWidget::BuildKeySlot(const FString& Key)
{
	return SNew(SEditableTextBox)
		.Text(FText::FromString(InternalMap[Key].DisplayKey))
		.BackgroundColor_Lambda([this, Key]()
		{
			if (ErrorKeys.Contains(InternalMap[Key].DisplayKey))
			{
				return FSlateColor(FColor::Red);
			}

			return FSlateColor::UseForeground();
		})
		.OnTextCommitted_Lambda([this, Key](const FText& NewText, ETextCommit::Type)
		{
			FValueField Current = InternalMap[Key];
			Current.DisplayKey = NewText.ToString();
			InternalMap[Key] = Current;
			
			UpdateInvalidEntries();
		});
}

TSharedRef<SEditableTextBox> SSGrailDictionaryWidget::BuildValueSlot(const FString& Key)
{
	return SNew(SEditableTextBox)
		.Text(FText::FromString(InternalMap[Key].Value))
		.OnTextCommitted_Lambda([this, Key](const FText& NewText, ETextCommit::Type)
		{
			FValueField Current = InternalMap[Key];
			Current.Value = NewText.ToString();
			InternalMap[Key] = Current;
		});
}

TSharedRef<SButton> SSGrailDictionaryWidget::BuildRemoveSlot(const FString& Key)
{
	return SNew(SButton)
		.Text(FText::FromString("X"))
		.HAlign(HAlign_Center)
		.OnClicked_Lambda([this, Key]()
		{
			return OnRemoveElementClicked(Key);
		});
}

FReply SSGrailDictionaryWidget::OnRemoveElementClicked(const FString& Key)
{
	InternalMap.Remove(Key);
	RebuildDictionarySlots();
	UpdateInvalidEntries();
	return FReply::Handled();
}

void SSGrailDictionaryWidget::UpdateInvalidEntries()
{
	TSet<FString> Seen;
	bool bAnyErrors = false;
	ErrorKeys.Empty();
	
	for (auto& [InternalKey, ValueField] : InternalMap)
	{
		FString DisplayKey = ValueField.DisplayKey;

		if (Seen.Contains(DisplayKey))
		{
			ErrorKeys.Add(DisplayKey);
			bAnyErrors |= true;
		}
		
		Seen.Add(DisplayKey);
	}

	if (bAnyErrors)
	{
		Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
	}
}
