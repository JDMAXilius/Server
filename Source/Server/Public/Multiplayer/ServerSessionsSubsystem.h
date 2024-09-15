// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Multiplayer/Data/ServerSubsytemData.h"
#include "ServerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FServerOnJoinSessionComplete, bool, bWasSuccessful, EEOSJoinSessionResultType, Type, const FString, Address);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerOnEndSessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerOnStartSessionComplete, bool, bWasSuccessful);

//Need to forward declare classes used 
class FOnlineSessionSearch;
class FOnlineSessionSearchResult;

UCLASS()
class SERVER_API UServerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	
	UServerSessionsSubsystem();
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	// Public interface to start session creation
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	void CreateSession(const int32 NumConnections, FString MatchType, const FString& SessionName, const bool bIsDedicatedServer, const bool
	                   bIsPrivate, const bool bIsLANMatch);

	// Function to initiate a session search
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	void FindSessions(const int32 MaxSearchResults, const bool bIsLANMatch);

	// Function to join a session
	//UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);

	// Function to start the session
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	void StartSession();

	// Function to end the session
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	void EndSession();
	
	// Function to destroy the session
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	void DestroySession();

	// Function to register incomming player to the session
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	bool RegisterPlayer(APlayerController* InPlayerController);

	// Function to register existing players currently in the session
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	bool RegisterExistingPlayers();

	// Function to unregister player from the session
	UFUNCTION(BlueprintCallable, Category = "Server Subsystems|Sessions")
	bool UnregisterPlayer(APlayerController* InPlayerController);

	// Get the last session settings
	TSharedPtr<FOnlineSessionSettings> GetLastSessionSettings() { return LastSessionSettings; }
	
	
	//
	// Public delegates for class to bind callbacks
	//
	UPROPERTY(BlueprintAssignable, Category = "Server Subsystems|Sessions|Callback")
	FServerOnCreateSessionComplete OnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete OnFindSessionsComplete;
	UPROPERTY(BlueprintAssignable, Category = "Server Subsystems|Sessions|Callback")
	FServerOnJoinSessionComplete OnJoinSessionComplete;
	UPROPERTY(BlueprintAssignable, Category = "Server Subsystems|Sessions|Callback")
	FServerOnDestroySessionComplete OnDestroySessionComplete;
	UPROPERTY(BlueprintAssignable, Category = "Server Subsystems|Sessions|Callback")
	FServerOnEndSessionComplete OnEndSessionComplete;
	UPROPERTY(BlueprintAssignable, Category = "Server Subsystems|Sessions|Callback")
	FServerOnStartSessionComplete OnStartSessionComplete;

	int32 DesiredNumPublicConnections{};
	FString DesiredMatchType{};
	
protected:
	//
	// Internal callbacks for the delegates.
	//
	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleEndSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleStartSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

private:

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastSessionName;
	FString LastMatchType;
	bool bLastSessionIsDedicatedServer;
	bool bLastSessionIsPrivate;
	bool bLastSessionIsLAN;

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//
	// Delegate Handles for the OnlineSubsystem session interfaces
	//
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle EndSessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FDelegateHandle NetworkFailureDelegateHandle;
};
