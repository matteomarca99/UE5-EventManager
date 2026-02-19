#pragma once

class SEventDebuggerWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SEventDebuggerWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedPtr<SListView<TSharedPtr<FString>>> ListView;
	TArray<TSharedPtr<FString>> Items;

	void RefreshList();
};