#pragma once

#include "Widgets/SCompoundWidget.h"

/*
 * Represents a single entry in the dictionary: display key, value, and its display position in the UI.
 */
struct FValueField
{
	FValueField(const FString& DisplayKey, const FString& Value, const int DisplayPosition)
		: DisplayKey(DisplayKey),
		  Value(Value),
		  DisplayPosition(DisplayPosition)
	{
	}

	FString DisplayKey;
	FString Value;
	int DisplayPosition;
};

class SGrailDictionaryWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGrailDictionaryWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	
private:
	// Handle clicks
	FReply OnAddClicked();
	FReply OnRemoveElementClicked(const FString& Key);
	FReply OnSaveDataClicked();

	// Handle building the dictionary list
	void RebuildDictionarySlots();
	void RebuildDictionarySlot(TTuple<FString, FValueField>& Entry);
	TSharedRef<SEditableTextBox> BuildKeySlot(const FString& Key);
	TSharedRef<SEditableTextBox> BuildValueSlot(const FString& Key);
	TSharedRef<SButton> BuildRemoveSlot(const FString& Key);

	void UpdateInvalidEntries();

	// Handle saving
	bool TrySaveToJson(const FString& SavePath);
	static bool TryGetSavePath(FString& OutPath);
	
	TMap<FString, FValueField> InternalMap;
	TSet<FString> ErrorKeys;
	TSharedPtr<SVerticalBox> DictionarySlotsContainer;
	static int NewEntryIndex;
};