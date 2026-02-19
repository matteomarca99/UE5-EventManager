// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_SubscribeToEvent.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_GetSubsystem.h"
#include "KismetCompiler.h"
#include "Data/GameEventData.h"
#include "Subsystem/GameEventSubsystem.h"

void UK2Node_SubscribeToEvent::AllocateDefaultPins()
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

void UK2Node_SubscribeToEvent::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
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
	
	// SubscribeToEvent node
    UK2Node_CallFunction* SubscribeNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	SubscribeNode->SetFromFunction(UGameEventSubsystem::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGameEventSubsystem, Subscribe)));
    SubscribeNode->AllocateDefaultPins();
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(SubscribeNode, this);
	
	// Link CallBack pin -> SubscriberNode Callback pin
	CompilerContext.MovePinLinksToIntermediate(*CallbackPin,*SubscribeNode->FindPinChecked(TEXT("Callback")));
	
	// Link Exec pin → SubscribeNode Exec pin
	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(),*SubscribeNode->GetExecPin());
	
	// Link GetUGameEventSubsystem node -> SubscriberNode self pin 
	GetSubsystemNode->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue)->MakeLinkTo(SubscribeNode->FindPinChecked(UEdGraphSchema_K2::PN_Self));

    // Link EventDataPin -> GetEventIdNode self pin
	EventDataPin->MakeLinkTo(GetEventIdNode->FindPinChecked(UEdGraphSchema_K2::PN_Self));
	
	// Link GetEventIdNode return pin -> SubscriberNode EventId pin
	GetEventIdNode->GetReturnValuePin()->MakeLinkTo(SubscribeNode->FindPinChecked(TEXT("EventId")));

    // Link Then -> SubscribeNode Then
    CompilerContext.MovePinLinksToIntermediate(*GetThenPin(),*SubscribeNode->GetThenPin());

	BreakAllNodeLinks();
}

void UK2Node_SubscribeToEvent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
	}
}

FText UK2Node_SubscribeToEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (EventData)
	{
		return FText::FromString(FString::Printf(TEXT("Subscribe To %s Event"), *EventData->GetName()));
	}
	return FText::FromString("Subscribe To Game Event");
}

FString UK2Node_SubscribeToEvent::GetDesc()
{
	return "Subscribe to global game event through the EventManager.";
}

FText UK2Node_SubscribeToEvent::GetMenuCategory() const
{
	return FText::FromString(TEXT("Game Events"));
}

FLinearColor UK2Node_SubscribeToEvent::GetNodeTitleColor() const
{
	return FLinearColor(0.2f, 0.0f, 0.5f);
}

FLinearColor UK2Node_SubscribeToEvent::GetNodeBodyTintColor() const
{
	return FLinearColor(0.46f, 0.18f, 1.00f);
}

void UK2Node_SubscribeToEvent::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	UE_LOG(LogTemp, Warning, TEXT("Refresh node"));
	
	if (Pin->PinName == TEXT("GameEventData"))
	{
		EventData = Cast<UGameEventData>(Pin->DefaultObject);
		ReconstructNode();
	}
}

void UK2Node_SubscribeToEvent::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	RestoreSplitPins(OldPins);
}

// Utility

UEdGraphPin* UK2Node_SubscribeToEvent::GetExecPin() const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Execute);
}

UEdGraphPin* UK2Node_SubscribeToEvent::GetThenPin() const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Then);
}