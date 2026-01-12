#include "SARKITCurveMappingWindow.h"

#include "ARKitKeys.h"

#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"

#include "HAL/PlatformApplicationMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/MessageDialog.h"
#include "Internationalization/Regex.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

static FString JoinTargetsCSV(const TArray<FString>& Targets)
{
	TArray<FString> Clean;
	Clean.Reserve(Targets.Num());
	for (FString T : Targets)
	{
		T.TrimStartAndEndInline();
		if (!T.IsEmpty())
		{
			Clean.Add(T);
		}
	}
	return FString::Join(Clean, TEXT(","));
}

// Supports parsing these common inputs:
// 1) Flat pairs: (("jawOpen","Jaw_Open"),("jawOpen","Jaw_Open_Extra"), ...)
// 2) One pair per line: ("jawOpen","Jaw_Open")
// 3) Simple colon format: jawOpen: Jaw_Open,Jaw_Open_Extra
static void ParseMappingTextToMultiMap(const FString& InText, TMultiMap<FString, FString>& Out)
{
	Out.Reset();

	// Colon format
	{
		TArray<FString> Lines;
		InText.ParseIntoArrayLines(Lines, true);
		for (FString Line : Lines)
		{
			Line.TrimStartAndEndInline();
			if (Line.IsEmpty()) continue;

			int32 ColonIdx = INDEX_NONE;
			if (Line.FindChar(TEXT(':'), ColonIdx))
			{
				const FString Key = Line.Left(ColonIdx).TrimStartAndEnd();
				FString Values = Line.Mid(ColonIdx + 1);
				Values.TrimStartAndEndInline();

				if (!Key.IsEmpty() && !Values.IsEmpty())
				{
					TArray<FString> Targets;
					Values.ParseIntoArray(Targets, TEXT(","), true);
					for (FString& T : Targets)
					{
						T.TrimStartAndEndInline();
						if (!T.IsEmpty())
						{
							Out.Add(Key, T);
						}
					}
				}
			}
		}
		if (Out.Num() > 0) return;
	}

	// Pair format: ("key","value") repeated
	{
		// Matches ("something","somethingElse") with optional whitespace
		const FRegexPattern Pattern(TEXT("\\(\\s*\"([^\"]+)\"\\s*,\\s*\"([^\"]+)\"\\s*\\)"));
		FRegexMatcher Matcher(Pattern, InText);

		while (Matcher.FindNext())
		{
			const FString Key = Matcher.GetCaptureGroup(1);
			const FString Val = Matcher.GetCaptureGroup(2);
			if (!Key.IsEmpty() && !Val.IsEmpty())
			{
				Out.Add(Key, Val);
			}
		}
	}
}

class SARKITCurveMappingRowWidget : public SMultiColumnTableRow<TSharedPtr<FARKitMapRow>>
{
public:
	SLATE_BEGIN_ARGS(SARKITCurveMappingRowWidget) {}
		SLATE_ARGUMENT(TSharedPtr<FARKitMapRow>, RowItem)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& OwnerTable)
	{
		RowItem = InArgs._RowItem;
		SMultiColumnTableRow::Construct(FSuperRowType::FArguments(), OwnerTable);
	}

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		if (ColumnName == "ARKit")
		{
			return SNew(STextBlock).Text(FText::FromName(RowItem->ArkitName));
		}

		if (ColumnName == "Targets")
		{
			return SNew(SEditableTextBox)
				.Text_Lambda([R = RowItem]()
					{
						return FText::FromString(R->TargetsCSV);
					})
				.OnTextCommitted_Lambda([R = RowItem](const FText& NewText, ETextCommit::Type)
					{
						R->TargetsCSV = NewText.ToString();
					});
		}

		return SNew(STextBlock);
	}

private:
	TSharedPtr<FARKitMapRow> RowItem;
};

void SARKITCurveMappingWindow::Construct(const FArguments& InArgs)
{
	Rows.Empty();

	for (const FName Key : ARKitKeys::Get())
	{
		TSharedPtr<FARKitMapRow> Row = MakeShared<FARKitMapRow>();
		Row->ArkitName = Key;
		Row->TargetsCSV = TEXT("");
		Rows.Add(Row);
	}

	ChildSlot
		[
			SNew(SBorder)
				.Padding(8)
				[
					SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.f, 0.f, 0.f, 8.f))
						[
							BuildTopBar()
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.f)
						[
							BuildList()
						]
				]
		];
}

TSharedRef<SWidget> SARKITCurveMappingWindow::BuildTopBar()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(4.f, 0.f))
		[
			SNew(SButton)
				.Text(FText::FromString(TEXT("Load")))
				.OnClicked(this, &SARKITCurveMappingWindow::OnLoad)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(4.f, 0.f))
		[
			SNew(SButton)
				.Text(FText::FromString(TEXT("Export")))
				.OnClicked(this, &SARKITCurveMappingWindow::OnExport)
		];
}

TSharedRef<SWidget> SARKITCurveMappingWindow::BuildList()
{
	ListView = SNew(SListView<TSharedPtr<FARKitMapRow>>)
		.ListItemsSource(&Rows)
		.OnGenerateRow(this, &SARKITCurveMappingWindow::OnGenerateRow)
		.SelectionMode(ESelectionMode::Single)
		.HeaderRow(
			SNew(SHeaderRow)
			+ SHeaderRow::Column("ARKit")
			.DefaultLabel(FText::FromString(TEXT("ARKit")))
			.FillWidth(0.25f)
			+ SHeaderRow::Column("Targets")
			.DefaultLabel(FText::FromString(TEXT("Targets (comma separated)")))
			.FillWidth(0.75f)
		);

	return SNew(SScrollBox)
		+ SScrollBox::Slot()
		[
			ListView.ToSharedRef()
		];
}

TSharedRef<ITableRow> SARKITCurveMappingWindow::OnGenerateRow(TSharedPtr<FARKitMapRow> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SARKITCurveMappingRowWidget, OwnerTable)
		.RowItem(Item);
}

void SARKITCurveMappingWindow::RefreshList()
{
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}

static FString BuildFlatPairsFromRows(const TArray<TSharedPtr<FARKitMapRow>>& Rows)
{
	TArray<FString> Pairs;

	for (const TSharedPtr<FARKitMapRow>& Row : Rows)
	{
		if (!Row.IsValid()) continue;

		TArray<FString> Targets;
		Row->TargetsCSV.ParseIntoArray(Targets, TEXT(","), true);

		const FString K = Row->ArkitName.ToString();
		for (FString& T : Targets)
		{
			T.TrimStartAndEndInline();
			if (T.IsEmpty()) continue;

			Pairs.Add(FString::Printf(TEXT("(\"%s\",\"%s\")"), *K, *T));
		}
	}

	return FString::Printf(TEXT("(%s)"), *FString::Join(Pairs, TEXT(",")));
}

FReply SARKITCurveMappingWindow::OnExport()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("DesktopPlatform not available.")));
		return FReply::Handled();
	}

	const FString DefaultPath = FPaths::ProjectDir();
	const FString DefaultFile = TEXT("ARKitMapping.csv");

	TArray<FString> OutFiles;
	const bool bSaved = DesktopPlatform->SaveFileDialog(
		nullptr,
		TEXT("Save ARKit Mapping CSV"),
		DefaultPath,
		DefaultFile,
		TEXT("CSV (*.csv)|*.csv|All Files (*.*)|*.*"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (!bSaved || OutFiles.Num() == 0)
	{
		return FReply::Handled();
	}

	FString SavePath = OutFiles[0];
	if (!SavePath.EndsWith(TEXT(".csv")))
	{
		SavePath += TEXT(".csv");
	}

	// CSV header
	TArray<FString> Lines;
	Lines.Reserve(Rows.Num() + 1);
	Lines.Add(TEXT("Name,ARKit,Targets"));

	for (const TSharedPtr<FARKitMapRow>& Row : Rows)
	{
		if (!Row.IsValid()) continue;

		TArray<FString> Targets;
		Row->TargetsCSV.ParseIntoArray(Targets, TEXT(","), true);
		for (FString& T : Targets) { T.TrimStartAndEndInline(); }
		Targets.RemoveAll([](const FString& S) { return S.IsEmpty(); });

		const FString Key = Row->ArkitName.ToString();
		const FString TargetsPipe = FString::Join(Targets, TEXT("|"));

		Lines.Add(FString::Printf(TEXT("%s,%s,%s"), *Key, *Key, *TargetsPipe));
	}

	const FString OutText = FString::Join(Lines, TEXT("\n"));

	if (!FFileHelper::SaveStringToFile(OutText, *SavePath))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to save CSV.")));
	}

	return FReply::Handled();
}

FReply SARKITCurveMappingWindow::OnLoad()
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("DesktopPlatform not available.")));
		return FReply::Handled();
	}

	TArray<FString> OutFiles;
	const FString DefaultPath = FPaths::ProjectDir();

	const bool bOpened = DesktopPlatform->OpenFileDialog(
		nullptr,
		TEXT("Load ARKit Mapping CSV"),
		DefaultPath,
		TEXT(""),
		TEXT("CSV (*.csv)|*.csv|All Files (*.*)|*.*"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (!bOpened || OutFiles.Num() == 0)
	{
		return FReply::Handled();
	}

	FString Text;
	if (!FFileHelper::LoadFileToString(Text, *OutFiles[0]))
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("Failed to read CSV.")));
		return FReply::Handled();
	}

	TArray<FString> Lines;
	Text.ParseIntoArrayLines(Lines, true);
	if (Lines.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("CSV is empty.")));
		return FReply::Handled();
	}

	// Detect format by header
	bool bHasNameColumn = false;
	{
		FString Header = Lines[0];
		Header.TrimStartAndEndInline();
		bHasNameColumn = Header.StartsWith(TEXT("Name,"), ESearchCase::IgnoreCase);
	}

	// Parsed: ARKitKey -> TargetsPipe
	TMap<FString, FString> Parsed;

	for (int32 i = 0; i < Lines.Num(); ++i)
	{
		FString Line = Lines[i];
		Line.TrimStartAndEndInline();
		if (Line.IsEmpty()) continue;

		// skip header
		if (i == 0 && (Line.StartsWith(TEXT("ARKit,"), ESearchCase::IgnoreCase) || Line.StartsWith(TEXT("Name,"), ESearchCase::IgnoreCase)))
		{
			continue;
		}

		if (bHasNameColumn)
		{
			// Name,ARKit,Targets
			FString Col0, Rest;
			if (!Line.Split(TEXT(","), &Col0, &Rest)) continue;

			FString Col1, Col2;
			if (!Rest.Split(TEXT(","), &Col1, &Col2)) continue;

			Col1.TrimStartAndEndInline(); // ARKit
			Col2.TrimStartAndEndInline(); // Targets
			if (!Col1.IsEmpty())
			{
				Parsed.Add(Col1, Col2);
			}
		}
		else
		{
			// ARKit,Targets
			FString Col0, Col1;
			if (!Line.Split(TEXT(","), &Col0, &Col1)) continue;

			Col0.TrimStartAndEndInline();
			Col1.TrimStartAndEndInline();
			if (!Col0.IsEmpty())
			{
				Parsed.Add(Col0, Col1);
			}
		}
	}

	if (Parsed.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("No valid rows found in CSV.")));
		return FReply::Handled();
	}

	// Apply to UI rows: pipe -> comma
	for (TSharedPtr<FARKitMapRow>& Row : Rows)
	{
		if (!Row.IsValid()) continue;

		const FString Key = Row->ArkitName.ToString();
		if (const FString* TargetsPipe = Parsed.Find(Key))
		{
			TArray<FString> Targets;
			TargetsPipe->ParseIntoArray(Targets, TEXT("|"), true);

			for (FString& T : Targets) { T.TrimStartAndEndInline(); }
			Targets.RemoveAll([](const FString& S) { return S.IsEmpty(); });

			Row->TargetsCSV = FString::Join(Targets, TEXT(","));
		}
	}

	RefreshList();
	return FReply::Handled();
}
