// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameEventSubsystem.generated.h"

class UGameEventPayload;

/** 
 * Exposed delegate for game events. 
 * Use this to bind callbacks from Blueprints or external systems.
 */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnGameEventReceived, const UGameEventPayload*, Payload);

/** 
 * Internal, faster multicast delegate used by Subscribers.
 * Not meant for Blueprint binding; optimized for internal event dispatching.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEventReceived_Internal, const UGameEventPayload*, Payload);

/**
 * System that manages the routing of game events.
 */
UCLASS()
class ULTIMATEEVENTMANAGER_API UGameEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/**
	 * Broadcasts a game event via the GameEventSubsystem.
	 * @param EventId The GameplayTag identifier of the event to broadcast
	 * @param Payload The payload data associated with the event.
	 */
	UFUNCTION(BlueprintCallable, Category = "Game Events")
	void BroadcastEvent(const FGameplayTag EventId, const UGameEventPayload* Payload);
	
	/**
	 * Subscribes a callback to a specific game event.
	 * @param EventId  The GameplayTag identifier of the event to listen for.
	 * @param Callback The delegate to invoke when the event occurs.
	 */
	UFUNCTION(BlueprintCallable,  Category = "Game Events")
	void Subscribe(const FGameplayTag EventId, const FOnGameEventReceived& Callback);
	
	/**
	 * Unsubscribes a previously bound callback from a specific game event.
	 * @param EventId  The GameplayTag identifier of the event to stop listening for.
	 * @param Callback The delegate that was previously bound and should be removed.
	 */
	UFUNCTION(BlueprintCallable,  Category = "Game Events")
	void Unsubscribe(const FGameplayTag EventId, const FOnGameEventReceived& Callback);

	/**
	 * Removes the given subscriber from each event's subscriber map.
	 * @param Subscriber The UObject that should be removed from all event subscriptions.
	 */
	UFUNCTION(BlueprintCallable,  Category = "Game Events")
	void UnsubscribeFromAll(UObject* Subscriber);
	
private:
	/** 
	 * Maps each event to a set of subscribers. 
	 * Key = EventId.
	 * Value = Map from subscriber UObject (weak pointer) to internal multicast delegate.
	 */
	TMap<FGameplayTag, TMap<TWeakObjectPtr<const UObject>, FOnGameEventReceived_Internal>> Subscribers;

public:
	TMap<FGameplayTag, TMap<TWeakObjectPtr<const UObject>, FOnGameEventReceived_Internal>> GetSubscribers() const;
};