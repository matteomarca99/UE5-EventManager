// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/GameEventSubsystem.h"

void UGameEventSubsystem::BroadcastEvent(const FGameplayTag EventId, const UGameEventPayload* Payload)
{
	if (auto* SubscriberMap = Subscribers.Find(EventId))
	{
		// Iterate over all subscriber objects
		for (auto It = SubscriberMap->CreateIterator(); It; ++It)
		{
			const TWeakObjectPtr<const UObject>& SubscriberPtr = It.Key();
			FOnGameEventReceived_Internal& Delegate = It.Value();

			if (SubscriberPtr.IsValid())
			{
				Delegate.Broadcast(Payload);
			}
			else
			{
				// Clean up destroyed subscriber
				It.RemoveCurrent();
			}
		}
	}
}

void UGameEventSubsystem::Subscribe(const FGameplayTag EventId, const FOnGameEventReceived& Callback)
{
	if (!Callback.IsBound() || !Callback.GetUObject())
	{
		return;
	}

	// Weak pointer to UObject to track subscriber safely
	TWeakObjectPtr<const UObject> SubscriberPtr = Callback.GetUObject();
	FOnGameEventReceived_Internal& InternalDelegate = Subscribers.FindOrAdd(EventId).FindOrAdd(SubscriberPtr);
	InternalDelegate.Add(Callback);
}

void UGameEventSubsystem::Unsubscribe(const FGameplayTag EventId, const FOnGameEventReceived& Callback)
{
	if (!Callback.IsBound() || !Callback.GetUObject())
	{
		return;
	}

	if (auto* SubscriberMap = Subscribers.Find(EventId))
	{
		TWeakObjectPtr<const UObject> SubscriberPtr = Callback.GetUObject();

		if (FOnGameEventReceived_Internal* Delegate = SubscriberMap->Find(SubscriberPtr))
		{
			Delegate->Remove(Callback);

			// Remove subscriber entry if no more delegates
			if (!Delegate->IsBound())
			{
				SubscriberMap->Remove(SubscriberPtr);
			}
		}

		// Clean up map if empty
		if (SubscriberMap->IsEmpty())
		{
			Subscribers.Remove(EventId);
		}
	}
}

void UGameEventSubsystem::UnsubscribeFromAll(UObject* Subscriber)
{
	if (!Subscriber)
		return;

	for (auto It = Subscribers.CreateIterator(); It; ++It)
	{
		auto& InnerMap = It.Value();
		InnerMap.Remove(Subscriber);

		if (InnerMap.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}

TMap<FGameplayTag, TMap<TWeakObjectPtr<const UObject>, FOnGameEventReceived_Internal>> UGameEventSubsystem::GetSubscribers() const
{
	return Subscribers;
}
