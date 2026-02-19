// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameEventPayload.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "GameEventData.generated.h"

/**
 * Data asset that contains information about an event.
 */
UCLASS(Blueprintable, BlueprintType)
class ULTIMATEEVENTMANAGER_API UGameEventData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FGameplayTag EventId;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Event")
	FText Description;
	
	UPROPERTY(EditAnywhere, Category="Payload") 
	TSubclassOf<UGameEventPayload> Payload;
	
	UFUNCTION(BlueprintPure,  Category = "Event Data")
	FGameplayTag GetEventId() const { return EventId; }

	UFUNCTION(BlueprintPure,  Category = "Event Data")
	UClass* GetPayloadClass() const { return Payload; }
};
