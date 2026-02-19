// Fill out your copyright notice in the Description page of Project Settings.


#include "K2Node_BroadcastEvent.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "K2Node_CallFunction.h"
#include "K2Node_GetSubsystem.h"
#include "KismetCompiler.h"
#include "Data/GameEventData.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/GameEventSubsystem.h"

void UK2Node_BroadcastEvent::AllocateDefaultPins()
{
	const UEdGraphSchema_K2* Schema = GetDefault<UEdGraphSchema_K2>();

	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);
	CreatePin(EGPD_Input,Schema->PC_Object,UGameEventData::StaticClass(),TEXT("GameEventData"));

	if (EventData != nullptr)
	{
		const UClass* PayloadClass = EventData->GetPayloadClass();

		// Create property pins for each payload property
		for (TFieldIterator<FProperty> It(PayloadClass); It; ++It)
		{
			const FProperty* Property = *It;

			if (!Property->HasAnyPropertyFlags(CPF_BlueprintVisible))
				continue;

			FEdGraphPinType PinType;
			GetDefault<UEdGraphSchema_K2>()->ConvertPropertyToPinType(Property, PinType);
			
			CreatePin(
				EGPD_Input,
				PinType.PinCategory,
				PinType.PinSubCategoryObject.Get(),
				Property->GetFName()
			);
		}
	}
	Super::AllocateDefaultPins();
}

void UK2Node_BroadcastEvent::ExpandNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
    Super::ExpandNode(CompilerContext, SourceGraph);

    const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	// EventDataPin reference
    UEdGraphPin* EventDataPin = FindPinChecked(TEXT("GameEventData"));
	
	if (!EventData)
	{
		CompilerContext.MessageLog.Error(TEXT("@@ GameEventData not set"), this);
		BreakAllNodeLinks();
		return;
	}

    UClass* PayloadClass = EventData->GetPayloadClass();
    if (!PayloadClass || PayloadClass->HasAnyClassFlags(CLASS_Abstract))
    {
        CompilerContext.MessageLog.Error(TEXT("@@ Invalid or abstract PayloadClass"), this);
        BreakAllNodeLinks();
        return;
    }
	
	// Spawn Object node (payload) 
    UK2Node_CallFunction* SpawnNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	SpawnNode->FunctionReference.SetExternalMember(GET_FUNCTION_NAME_CHECKED(UGameplayStatics, SpawnObject), UGameplayStatics::StaticClass());
    SpawnNode->AllocateDefaultPins();
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(SpawnNode, this);
	
	// Set Spawn Object node Class
	UEdGraphPin* ClassPin = SpawnNode->FindPinChecked(TEXT("ObjectClass"));
	ClassPin->DefaultObject = PayloadClass;
	
	// Set Spawn Object node return pin type
	UEdGraphPin* SpawnResultPin = SpawnNode->GetReturnValuePin();
	SpawnResultPin->PinType.PinCategory = Schema->PC_Object;
	SpawnResultPin->PinType.PinSubCategoryObject = PayloadClass;
	
	// Link Exec → Spawn Exec
	CompilerContext.MovePinLinksToIntermediate(*GetExecPin(),*SpawnNode->GetExecPin());
	
	// Create assignment nodes for each payload property
    UEdGraphPin* LastThen = FKismetCompilerUtilities::GenerateAssignmentNodes(CompilerContext,SourceGraph,SpawnNode,this,SpawnResultPin,PayloadClass);

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
	
	// BroadcastEvent node
    UK2Node_CallFunction* BroadcastNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	BroadcastNode->SetFromFunction(UGameEventSubsystem::StaticClass()->FindFunctionByName(GET_FUNCTION_NAME_CHECKED(UGameEventSubsystem, BroadcastEvent)));
    BroadcastNode->AllocateDefaultPins();
    CompilerContext.MessageLog.NotifyIntermediateObjectCreation(BroadcastNode, this);
	
	//Link GetUGameEventSubsystem node -> SubscriberNode self pin 
	GetSubsystemNode->FindPinChecked(UEdGraphSchema_K2::PN_ReturnValue)->MakeLinkTo(BroadcastNode->FindPinChecked(UEdGraphSchema_K2::PN_Self));

	// Link EventDataPin -> GetEventIdNode self pin
	EventDataPin->MakeLinkTo(GetEventIdNode->FindPinChecked(UEdGraphSchema_K2::PN_Self));
	
	// Link GetEventIdNode return pin -> SubscriberNode EventId pin
	GetEventIdNode->GetReturnValuePin()->MakeLinkTo(BroadcastNode->FindPinChecked(TEXT("EventId")));

    // Link SpawnedPayload pin -> BroadcastNode payload pin
    SpawnResultPin->MakeLinkTo(BroadcastNode->FindPinChecked(TEXT("Payload")));
	
	// Link last property assignment node -> BroadcastNode Exec pin
	LastThen->MakeLinkTo(BroadcastNode->GetExecPin());

    // Link Then -> BroadcastNode Then
    CompilerContext.MovePinLinksToIntermediate(*GetThenPin(),*BroadcastNode->GetThenPin());

    BreakAllNodeLinks();
}

void UK2Node_BroadcastEvent::ReallocatePinsDuringReconstruction(TArray<UEdGraphPin*>& OldPins)
{
	AllocateDefaultPins();
	RestoreSplitPins(OldPins);
}

void UK2Node_BroadcastEvent::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	const UClass* ActionKey = GetClass();

	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* Spawner = UBlueprintNodeSpawner::Create(GetClass());
		ActionRegistrar.AddBlueprintAction(ActionKey, Spawner);
	}
}

FText UK2Node_BroadcastEvent::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return FText::FromString(TEXT("Broadcast Game Event"));
}

FString UK2Node_BroadcastEvent::GetDesc()
{
	return "Broadcast global game event through the EventManager.";
}

FText UK2Node_BroadcastEvent::GetMenuCategory() const
{
	return FText::FromString(TEXT("Game Events"));
}

FLinearColor UK2Node_BroadcastEvent::GetNodeTitleColor() const
{
	return FLinearColor(0.2f, 0.0f, 0.5f);
}

FLinearColor UK2Node_BroadcastEvent::GetNodeBodyTintColor() const
{
	return FLinearColor(0.46f, 0.18f, 1.00f);
}

void UK2Node_BroadcastEvent::PinDefaultValueChanged(UEdGraphPin* Pin)
{
	UE_LOG(LogTemp, Warning, TEXT("Refresh node"));
	
	if (Pin->PinName == TEXT("GameEventData"))
	{
		EventData = Cast<UGameEventData>(Pin->DefaultObject);
		ReconstructNode();
	}
}

// Utility

UEdGraphPin* UK2Node_BroadcastEvent::GetExecPin() const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Execute);
}

UEdGraphPin* UK2Node_BroadcastEvent::GetThenPin() const
{
	return FindPinChecked(UEdGraphSchema_K2::PN_Then);
}

UEdGraphPin* UK2Node_BroadcastEvent::GetPayloadClassPin() const
{
	return FindPinChecked(TEXT("Payload"));
}
