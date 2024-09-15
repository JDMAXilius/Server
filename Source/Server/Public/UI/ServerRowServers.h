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

UCLASS()
class SERVER_API UServerRowServers : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	// Set the server data from the session search result
	void SetServerData(const FOnlineSessionSearchResult& SearchResult);

	// Get the last session settings
	const FOnlineSessionSearchResult* GetSearchResult() const { return CurrentSessionResult; }

	UPROPERTY(BlueprintReadOnly, Category = "Server Menu", meta = (BindWidget))
	UButton* ServerRowButton;

	int32 SelectedIndex;
	
protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Server Menu", meta = (BindWidget))
	UTextBlock* ServerNameText;

	UPROPERTY(BlueprintReadWrite, Category = "Server Menu", meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(BlueprintReadWrite, Category = "Server Menu", meta = (BindWidget))
	UTextBlock* PingText;

	const FOnlineSessionSearchResult* CurrentSessionResult;

	UFUNCTION()
	void OnRowClicked();

private:
	// The subsystem designed to handle all online session functionality
	UServerSessionsSubsystem* ServerSessionsSubsystem;
};