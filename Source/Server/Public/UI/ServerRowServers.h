// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "Multiplayer/Data/ServerSubsytemData.h"
#include "ServerRowServers.generated.h"

class UButton;
class UTextBlock;
class UServerSessionsSubsystem;
//struct FOnlineSessionSearchResult;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServerRowClicked, UServerRowServers*, ClickedRow);

UCLASS()
class SERVER_API UServerRowServers : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	//UPROPERTY(BlueprintAssignable, Category = "ServerRow")
	//FOnServerRowClicked OnServerRowClicked;
	virtual void NativeOnListItemObjectSet(UObject* ListIteamObject) override;
	// Set the server data from the session search result
	void SetServerData(const FOnlineSessionSearchResult& SearchResult);
	// Text blocks for displaying server details
	
	// Get the last session settings
	const FOnlineSessionSearchResult* GetSearchResult() { return CurrentSessionResult; }
	int32 SelectedIndex;
	// Button to select this server row
	UPROPERTY(BlueprintReadOnly, Category = "Server Menu", meta = (BindWidget))
	UButton* ServerRowButton;
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Server Menu", meta = (BindWidget))
	UTextBlock* ServerNameText;
	UPROPERTY(BlueprintReadWrite, Category = "Server Menu", meta = (BindWidget))
	UTextBlock* PlayerCountText;
	UPROPERTY(BlueprintReadWrite, Category = "Server Menu", meta = (BindWidget))
	UTextBlock* PingText;
	
	const FOnlineSessionSearchResult* CurrentSessionResult;
	// Click event handler for when the row is clicked
	UFUNCTION()
	void OnRowClicked();
	

private:
	// The subsystem designed to handle all online session functionality
	UServerSessionsSubsystem* ServerSessionsSubsystem;

};
