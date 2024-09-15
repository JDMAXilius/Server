﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ServerRowServers.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Multiplayer/Data/ServerSubsytemData.h"
#include "ServerMainMenu.generated.h"

class UButton;
class UCheckBox;
class UListView;
class UServerSessionsSubsystem;
//struct FOnlineSessionSearchResult;

UCLASS()
class SERVER_API UServerMainMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Server Menu", meta = (BindWidget))
	UButton* HostButton;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Server Menu", meta = (BindWidget))
	UButton* FindButton;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Server Menu", meta = (BindWidget))
	UButton* JoinButton;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Server Menu", meta = (BindWidget))
	UButton* QuitButton;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Server Menu", meta = (BindWidget))
	UCheckBox* LANCheckBox;

	UPROPERTY(BlueprintReadOnly, Category = "Server Menu", meta = (BindWidget))
	UListView* ServerListView;
	int32 CurentIndex;
protected:
	// Map to store server rows and their corresponding session data
	TMap<UServerRowServers*, FOnlineSessionSearchResult> ServerRowMap;
	// Selected server row
	TArray<UServerRowServers*> SelectedServerRow;
	
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	UFUNCTION()
	void OnJoinSession(bool bWasSuccessful, EEOSJoinSessionResultType Type, const FString Address);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
	void UpdateServerList(const TArray<FOnlineSessionSearchResult>& SearchResults);
	void OnServerRowSelected(UServerRowServers* ClickedRow);
	void OnRowClicked();

private:
	
	UFUNCTION()
	void HostButtonClicked();
	
	UFUNCTION()
	void FindButtonClicked();
	
	UFUNCTION()
	void JoinButtonClicked();
	UFUNCTION()
	void SelectedJoinButtonClicked();

	UFUNCTION()
	void QuitButtonClicked();

	void MenuTearDown();

	// The subsystem designed to handle all online session functionality
	UServerSessionsSubsystem* ServerSessionsSubsystem;
	
	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "Server Menu",  meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{4};

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "Server Menu",  meta = (AllowPrivateAccess = "true"))
	FString MatchType{TEXT("DeathMatch")};

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "Server Menu",  meta = (AllowPrivateAccess = "true"))
	bool IsLanMatch{true};

	UPROPERTY(BlueprintReadWrite,EditDefaultsOnly, Category = "Server Menu",  meta = (AllowPrivateAccess = "true"))
	FString PathToLevel{TEXT("/Game/Framework/Map/ClientMap")};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Menu",  meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> LevelToOpen;
	
};
