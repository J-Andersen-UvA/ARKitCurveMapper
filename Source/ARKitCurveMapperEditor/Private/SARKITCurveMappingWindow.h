#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

#include "ARKitCurveMappingAsset.h"

struct FARKitMapRow
{
	FName ArkitName;

	// Comma-separated: Jaw_Open,Jaw_Open_Extra
	FString TargetsCSV;
};

class SARKITCurveMappingWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SARKITCurveMappingWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	void RefreshList();
	void BuildRowsFromAsset();
	void ApplyRowToAsset(const TSharedPtr<FARKitMapRow>& Row);

private:
	TSharedRef<SWidget> BuildTopBar();
	TSharedRef<SWidget> BuildList();

	TSharedRef<class ITableRow> OnGenerateRow(TSharedPtr<FARKitMapRow> Item, const TSharedRef<STableViewBase>& OwnerTable);

	FReply OnCreateNewAsset();
	FReply OnLoad();
	FReply OnExport();

private:
	TArray<TSharedPtr<FARKitMapRow>> Rows;
	TSharedPtr<SListView<TSharedPtr<FARKitMapRow>>> ListView;

	// Currently loaded mapping asset
	TWeakObjectPtr<UARKitCurveMappingAsset> MappingAsset;

	// Asset picker selection
	TWeakObjectPtr<UObject> PickedObject;
};
