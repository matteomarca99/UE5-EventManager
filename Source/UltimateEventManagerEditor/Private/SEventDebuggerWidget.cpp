#include "SEventDebuggerWidget.h"
#include "Subsystem/GameEventSubsystem.h"

void SEventDebuggerWidget::Construct(const FArguments& InArgs)
{
	RefreshList();

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SButton)
			.Text(FText::FromString("Refresh Subscribers"))
			.OnClicked_Lambda([this]()
			{
				RefreshList();
				return FReply::Handled();
			})
		]

		+ SVerticalBox::Slot()
		.FillHeight(1)
		[
			SAssignNew(ListView, SListView<TSharedPtr<FString>>)
			.ListItemsSource(&Items)
			.OnGenerateRow_Lambda([](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& OwnerTable)
			{
				return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
				[
					SNew(STextBlock).Text(FText::FromString(*Item))
				];
			})
		]
	];
}

void SEventDebuggerWidget::RefreshList()
{
	Items.Empty();

	if (GEditor == nullptr)
		return;

	UWorld* PIEWorld = GEditor->PlayWorld;
	if (!PIEWorld)
		return;

	UGameInstance* GI = PIEWorld->GetGameInstance();
	if (!GI)
		return;

	UGameEventSubsystem* Subsystem = GI->GetSubsystem<UGameEventSubsystem>();
	if (!Subsystem)
		return;
	
	for (const auto& Pair : Subsystem->GetSubscribers())
	{
		const FGameplayTag& EventId = Pair.Key;
		const auto& SubscriberMap = Pair.Value;

		Items.Add(MakeShared<FString>(TEXT("Event: ") + EventId.ToString()));

		for (const auto& SubPair : SubscriberMap)
		{
			const UObject* Subscriber = SubPair.Key.Get();
			FString Name = Subscriber ? Subscriber->GetName() : TEXT("INVALID - PENDING DELETE");

			Items.Add(MakeShared<FString>(TEXT("  - Subscriber: ") + Name));
		}
	}

	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}
}