// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ServerRowServers.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Multiplayer/ServerSessionsSubsystem.h"


void UServerRowServers::NativeOnListItemObjectSet(UObject* ListIteamObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListIteamObject);
	
	CurrentSessionResult = Cast<FOnlineSessionSearchResult>(ListIteamObject);
	if (CurrentSessionResult != nullptr)
	{
		// Update text fields with session data
		ServerNameText->SetText(FText::FromString(CurrentSessionResult->Session.OwningUserName));
		PlayerCountText->SetText(FText::Format(FText::FromString("{0}/{1}"), 
		CurrentSessionResult->Session.SessionSettings.NumPublicConnections - CurrentSessionResult->Session.NumOpenPublicConnections, 
		CurrentSessionResult->Session.SessionSettings.NumPublicConnections));
		PingText->SetText(FText::AsNumber(CurrentSessionResult->PingInMs));
	}
	else
	{
		ServerNameText->SetText(FText::FromString("Server User Name"));
		PlayerCountText->SetText(FText::FromString("{0}/{1}"));
		PingText->SetText(FText::AsNumber(1));
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		ServerSessionsSubsystem = GameInstance->GetSubsystem<UServerSessionsSubsystem>();
	}
	
	// Bind button click to row click handler
	//if (ServerRowButton)
	{
		//ServerRowButton->OnClicked.AddDynamic(this, &UServerRowServers::OnRowClicked);
	}
}

void UServerRowServers::SetServerData(const FOnlineSessionSearchResult& SearchResult)
{
	CurrentSessionResult = &SearchResult;

	if (CurrentSessionResult->IsValid())
	{
		// Update text fields with session data
		if (ServerNameText)
		{
			ServerNameText->SetText(FText::FromString(CurrentSessionResult->Session.OwningUserName));
		}
		if (PlayerCountText)
		{
			int32 CurrentPlayers = CurrentSessionResult->Session.SessionSettings.NumPublicConnections - 
								   CurrentSessionResult->Session.NumOpenPublicConnections;
			int32 MaxPlayers = CurrentSessionResult->Session.SessionSettings.NumPublicConnections;
			PlayerCountText->SetText(FText::Format(FText::FromString("{0}/{1}"), FText::AsNumber(CurrentPlayers), FText::AsNumber(MaxPlayers)));
		}
		if (PingText)
		{
			PingText->SetText(FText::AsNumber(CurrentSessionResult->PingInMs));
		}
	}
}

void UServerRowServers::OnRowClicked()
{
	// Notify the main menu of the clicked row
	//OnServerRowClicked.Broadcast(this);
	 /*if (ServerSessionsSubsystem && CurrentSessionResult.IsValid())
    {
	 	ServerRowButton->SetIsEnabled(false);
	 	FString WelcomeMessage = FString::Printf(TEXT("Success to Join Session!"));
	 	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WelcomeMessage);
		const FOnlineSessionSearchResult& SelectedSession = CurrentSessionResult;
        ServerSessionsSubsystem->JoinSession(SelectedSession);
    }
	else
	{
		ServerRowButton->SetIsEnabled(true);
		FString WelcomeMessage = FString::Printf(TEXT("Fail to Join Session!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, WelcomeMessage);
	}*/
}
