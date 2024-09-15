// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Multiplayer/Data/ServerSubsytemData.h"
#include "ServerListServers.generated.h"


class UButton;
class UTextBlock;
class UListView;
class UServerRowServers;
class UServerSessionsSubsystem;

UCLASS()
class SERVER_API UServerListServers : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;

	// Function to refresh the server list
	UFUNCTION(BlueprintCallable, Category = "Server List")
	void RefreshServerList();

	// Function to join the selected server
	UFUNCTION(BlueprintCallable, Category = "Server List")
	void JoinSelectedServer();

protected:
	// Callback function for when server search completes
	void OnFindSessionsComplete(bool bWasSuccessful);

	// Callback function for join session complete
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	// Function to update server rows in the list
	void UpdateServerList(const TArray<FOnlineSessionSearchResult>& SearchResults);
	void HandleServerRowSelected(const FOnlineSessionSearchResult& SessionResult);
	void OnRowClicked();

	// Function to create custom server options (e.g., server name, max players, LAN settings)
	void SetCustomServerSettings();

private:
	// Bindings from the widget Blueprint
	UPROPERTY(meta = (BindWidget))
	UButton* RefreshButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UPROPERTY(meta = (BindWidget))
	UListView* ServerListView;
	
	// Reference to the session interface
	IOnlineSessionPtr SessionInterface;

	// Handle for the Find Sessions complete delegate
	FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
	FDelegateHandle OnFindSessionsCompleteDelegateHandle;

	// Handle for the Join Sessions complete delegate
	FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;

	// Session search object
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// Selected server result for joining
	FOnlineSessionSearchResult SelectedSessionResult;

	// The subsystem designed to handle all online session functionality
	UServerSessionsSubsystem* ServerSessionsSubsystem;
};