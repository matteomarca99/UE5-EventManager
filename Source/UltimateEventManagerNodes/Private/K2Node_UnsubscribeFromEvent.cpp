// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_UnsubscribeFromEvent.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_GetSubsystem.h"
#include "KismetCompiler.h"
#include "Data/GameEventData.h"
#include "Subsystem/GameEventSubsystem.h"

void UK2Node_UnsubscribeFromEvent::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	CreatePin(EGPD_Input,Schema->PC_Object,UGameEventData::StaticClass(),TEXT("GameEventData"));

	if (EventData != nullptr)
	{
		// Get Callback signature
		const UFunction* SubscribeFunc = UGameEventSubsystem::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGameEventSubsystem, Subscribe));
		FProperty* CallbackProp = SubscribeFunc->FindPropertyByName(TEXT("Callback"));
		const FDelegateProperty* DelegateProp = CastField<FDelegateProperty>(CallbackProp);
		
		FCreatePinParams PinParams;
		PinParams.bIsConst = true;
		PinParams.bIsReference = true;
		
		if(SubscribeFunc)
		{
			// EventData valid -> create event callback pin with correct signature
			UEdGraphPin* DelegatePin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Delegate, TEXT("Callback"), PinParams);
			FMemberReference::FillSimpleMemberReference<UFunction>(DelegateProp->SignatureFunction, DelegatePin->PinType.PinSubCategoryMemberReference);
		}
	}
	Super::AllocateDefaultPins();
}

void UK2Node_UnsubscribeFromEvent::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);

	if (!EventData)
	{
		CompilerContext.MessageLog.Error(TEXT("@@ GameEventData not set"), this);
		BreakAllNodeLinks();
		return;
	}

	// Pins reference
	UEdGraphPin* EventDataPin = FindPinChecked(TEXT("GameEventData"));
	UEdGraphPin* CallbackPin = FindPinChecked(TEXT("Callback"));

	// GetUGameEventSubsystem node
    UK2Node_GetSubsystem* GetSubsystemNode = CompilerContext.SpawnIntermediateNode<UK2Node_GetSubsystem>(this, SourceGraph);
    GetSubsystemNode->Initialize(UGameEventSubsystem::StaticClass());
    GetSubsystemNode->AllocateDefaultPins();
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(GetSubsystemNode, this);
	
	// GetEventId node
    UK2Node_CallFunction* GetEventIdNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
    GetEventIdNode->SetFromFunction(UGameEventData::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGameEventData, GetEventId)));
    GetEventIdNode->AllocateDefaultPins();
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(GetEventIdNode, this);

    // Link EventData node -> GetEventId node
    CompilerContext.MovePinLinksToIntermediate(*EventDataPin,*GetEventIdNode->FindPinChecked(UEdGraphSchema_K2::PN_Self));
	
	// UnsubscribeFromEvent node
    UK2Node_CallFunction* UnsubscribeNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	UnsubscribeNode->SetFromFunction(UGameEventSubsystem::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGameEventSubsystem, Unsubscribe)));
    UnsubscribeNode->AllocateDefaultPins();
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(UnsubscribeNode, this);
	
	// Link CallBack pin -> UnsubscribeNode Callback pin
	CompilerContext.MovePinLinksToIntermediate(*CallbackPin,*UnsubscribeNode->FindPinChecked(TEXT("Callback")));
	
	// Link Exec pin → UnsubscribeNode Exec pin
	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(),*UnsubscribeNode->GetExecPin());
	
	// Link GetUGameEventSubsystem node -> UnsubscribeNode self pin 
	GetSubsystemNode->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue)->MakeLinkTo(UnsubscribeNode->FindPinChecked(UEdGraphSchema_K2::PN_Self));

    // Link EventDataPin -> GetEventIdNode self pin
	EventDataPin->MakeLinkTo(GetEventIdNode->FindPinChecked(UEdGraphSchema_K2::PN_Self));
	
	// Link GetEventIdNode return pin -> UnsubscribeNode EventId pin
	GetEventIdNode->GetReturnValuePin()->MakeLinkTo(UnsubscribeNode->FindPinChecked(TEXT("EventId")));

    // Link Then -> UnsubscribeNode Then
    CompilerContext.MovePinLinksToIntermediate(*GetThenPin(),*UnsubscribeNode->GetThenPin());

	BreakAllNodeLinks();
}

void UK2Node_UnsubscribeFromEvent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
	}
}

FText UK2Node_UnsubscribeFromEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (EventData)
	{
		return FText::FromString(FString::Printf(TEXT("Unsubscribe From %s Event"), *EventData->GetName()));
	}
	return FText::FromString("Unsubscribe From Game Event");
}

FString UK2Node_UnsubscribeFromEvent::GetDesc()
{
	return "Unsubscribe from global game event through the EventManager.";
}

FText UK2Node_UnsubscribeFromEvent::GetMenuCategory() const
{
	return FText::FromString(TEXT("Game Events"));
}

FLinearColor UK2Node_UnsubscribeFromEvent::GetNodeTitleColor() const
{
	return FLinearColor(0.2f, 0.0f, 0.5f);
}

FLinearColor UK2Node_UnsubscribeFromEvent::GetNodeBodyTintColor() const
{
	return FLinearColor(0.46f, 0.18f, 1.00f);
}

void UK2Node_UnsubscribeFromEvent::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	if (Pin->PinName == TEXT("GameEventData"))
	{
		EventData = Cast<UGameEventData>(Pin->DefaultObject);
		ReconstructNode();
	}
}

void UK2Node_UnsubscribeFromEvent::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	RestoreSplitPins(OldPins);
}

// Utility

UEdGraphPin* UK2Node_UnsubscribeFromEvent::GetExecPin() const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Execute);
}

UEdGraphPin* UK2Node_UnsubscribeFromEvent::GetThenPin() const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Then);
}
