// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_SubscribeToEvent.generated.h"

class UGameEventData;

/**
 * Custom editor node "SubscribeToEvent"
 * Node used to subscribe to an event via the GameEventSubsystem.
 */
UCLASS()
class ULTIMATEEVENTMANAGERNODES_API UK2Node_SubscribeToEvent : public UK2Node
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	TObjectPtr<UGameEventData> EventData;
	
public:
	virtual void AllocateDefaultPins() override;
	virtual void ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FString GetDesc() override;
	virtual FText GetMenuCategory() const override;
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FLinearColor GetNodeBodyTintColor() const override;
	virtual void PinDefaultValueChanged(UEdGraphPin* Pin) override;
	virtual void ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins) override;
	
protected:
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetThenPin() const;
};
