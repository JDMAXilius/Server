// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ServerGameinstance.generated.h"

class UServerSessionsSubsystem;
class UServerUserSubsystem;

UCLASS()
class SERVER_API UServerGameinstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;

	virtual void Shutdown() override;
	
	UFUNCTION(BlueprintCallable)
	void TravelToSession(APlayerController* Controller, FString Address);

	UPROPERTY()
	TObjectPtr<UServerSessionsSubsystem> SessionsSubsystem;

	UPROPERTY()
	TObjectPtr<UServerUserSubsystem> UserSubsystem;
	
	UPROPERTY(BlueprintReadOnly)
	int32 NumConnectedControllers = 0;

protected:

	//Flag indicating that a game error has occurred
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	bool bIsGameError;

	//Error msg associated with any game error 
	UPROPERTY(BlueprintReadOnly, Category = "Network")
	FString GameErrorMsg;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 DefaultNumConnections = 5;

private:
	

	//Error function handler that can occur during a session. This is to keep in memory the error and show it to the user after. 
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	void HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType, const FString& ErrorString);
	
};
