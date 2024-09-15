// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/ServerRowServers.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Multiplayer/ServerSessionsSubsystem.h"

void UServerRowServers::NativeOnListItemObjectSet(UObject* ListItemObject)
{
    // Ensure the ListItemObject is valid before using it
    if (!ListItemObject)
    {
        UE_LOG(LogTemp, Error, TEXT("ListItemObject is null in NativeOnListItemObjectSet."));
        return;
    }

   // CurrentSessionResult = Cast<const FOnlineSessionSearchResult>(ListItemObject);
    if (!CurrentSessionResult)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to cast ListItemObject to FOnlineSessionSearchResult."));
        return;
    }

    // Display the session details
    SetServerData(*CurrentSessionResult);
}

void UServerRowServers::SetServerData(const FOnlineSessionSearchResult& SearchResult)
{
    CurrentSessionResult = &SearchResult;

    if (CurrentSessionResult && CurrentSessionResult->IsValid())
    {
        /*ServerNameText->SetText(FText::FromString(CurrentSessionResult->Session.OwningUserName));
        PlayerCountText->SetText(FText::Format(
            FText::FromString("{0}/{1}"),
            CurrentSessionResult->Session.SessionSettings.NumPublicConnections - CurrentSessionResult->Session.NumOpenPublicConnections,
            CurrentSessionResult->Session.SessionSettings.NumPublicConnections
        ));
        PingText->SetText(FText::AsNumber(CurrentSessionResult->PingInMs));*/
    }
}

void UServerRowServers::OnRowClicked()
{
    if (ServerRowButton)
    {
        ServerRowButton->OnClicked.AddDynamic(this, &UServerRowServers::OnRowClicked);
        UE_LOG(LogTemp, Log, TEXT("Server Row Clicked."));
    }
}