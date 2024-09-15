// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ServerListServers.h"
#include "Components/Button.h"
#include "Components/ListView.h"
#include "UI/ServerRowServers.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Game/ServerGameinstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Multiplayer/Data/ServerSubsytemData.h"
#include "Server/Public/Multiplayer/ServerSessionsSubsystem.h"

void UServerListServers::NativeConstruct()
{
	Super::NativeConstruct();
	 
	// Initialize the session interface
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
	}
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		ServerSessionsSubsystem = GameInstance->GetSubsystem<UServerSessionsSubsystem>();
	}

	if (ServerSessionsSubsystem)
	{
		//ServerSessionsSubsystem->OnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		//ServerSessionsSubsystem->OnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		//ServerSessionsSubsystem->OnJoinSessionComplete.AddDynamic(this, &ThisClass::OnJoinSession);
	}
	
	// Bind the Refresh button click event
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &UServerListServers::RefreshServerList);
	}

	// Bind the Join button click event
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UServerListServers::JoinSelectedServer);
	}

	// Set up the delegate for finding sessions
	//OnFindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &UServerListServers::OnFindSessionsComplete);

	// Set up the delegate for joining sessions
	//OnJoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &UServerListServers::OnJoinSessionComplete);
}

void UServerListServers::RefreshServerList()
{
	if (SessionInterface.IsValid())
	{
		// Initialize a new session search
		SessionSearch = MakeShareable(new FOnlineSessionSearch());
		SessionSearch->bIsLanQuery = false;
		SessionSearch->MaxSearchResults = 50;
		SessionSearch->PingBucketSize = 50;

		// Add the delegate to the session interface
		OnFindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegate);

		// Start searching for sessions
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UServerListServers::JoinSelectedServer()
{
	if (SessionInterface.IsValid() && SelectedSessionResult.IsValid())
	{
		OnJoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegate);
		SessionInterface->JoinSession(0, NAME_GameSession, SelectedSessionResult);
	}
}

void UServerListServers::OnFindSessionsComplete(bool bWasSuccessful)
{
	// Remove the delegate handle to clean up
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(OnFindSessionsCompleteDelegateHandle);
	}

	// If the search was successful, update the server list
	if (bWasSuccessful && SessionSearch.IsValid())
	{
		UpdateServerList(SessionSearch->SearchResults);
	}
}

void UServerListServers::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result)
{
	// Remove the delegate handle to clean up
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(OnJoinSessionCompleteDelegateHandle);
	}

	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		FString ConnectInfo;
		if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
		{
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
			if (PlayerController)
			{
				PlayerController->ClientTravel(ConnectInfo, TRAVEL_Absolute);
			}
		}
	}
	else
	{
		// Handle failed join (show message or enable buttons, etc.)
	}
}

void UServerListServers::UpdateServerList(const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	if (ServerListView)
	{
		ServerListView->ClearListItems();

		for (const auto& SearchResult : SearchResults)
		{
			UServerRowServers* ServerRow = CreateWidget<UServerRowServers>(this, UServerRowServers::StaticClass());
			if (ServerRow)
			{
				ServerRow->SetServerData(SearchResult);
				ServerListView->AddItem(ServerRow);

				// Bind the row's OnServerRowSelected delegate to the handler function in ServerListWidget
				//ServerRow->ServerRowButton->OnClicked.AddDynamic(this, &UServerListServers::OnRowClicked);
			}
		}
	}
}

void UServerListServers::HandleServerRowSelected(const FOnlineSessionSearchResult& SessionResult)
{
	// Set the selected session result for joining
	SelectedSessionResult = SessionResult;
    
	// Optionally, highlight the selected row or provide feedback to the user
}

void UServerListServers::OnRowClicked()
{
	// Broadcast the delegate to notify the parent widget
	//OnServerRowSelected.Broadcast(CurrentSessionResult);
}

void UServerListServers::SetCustomServerSettings()
{
	/*
	// Get the online subsystem (e.g., Steam, EOS, etc.)
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnlineSubsystem not found!"));
		return;
	}

	// Get the session interface from the online subsystem
	IOnlineSessionPtr SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Session interface is invalid!"));
		return;
	}
	*/

	// Create custom session settings
	FOnlineSessionSettings SessionSettings;

	// Example customization
	SessionSettings.bIsLANMatch = true; // Set to true for LAN, false for online
	SessionSettings.NumPublicConnections = 10; // Max number of players
	SessionSettings.bAllowJoinInProgress = true; // Allow players to join in progress
	SessionSettings.bShouldAdvertise = true; // Advertise the session
	SessionSettings.bUsesPresence = true; // Use presence information
	SessionSettings.bAllowJoinViaPresence = true; // Allow joining via friends/presence
	SessionSettings.bIsDedicated = false; // Set to true if this is a dedicated server

	// Set custom session properties
	FString CustomServerName = TEXT("MyCustomServer"); // Replace with your desired server name
	SessionSettings.Set(FName("SERVER_NAME_KEY"), CustomServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// Example of setting max players or other custom settings as needed
	SessionSettings.Set(FName("MAX_PLAYERS_KEY"), 10, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// Create the session with the specified settings
	FName SessionName = NAME_GameSession; // This can be changed to any session name you prefer
	SessionInterface->CreateSession(0, SessionName, SessionSettings);

	// Log the session creation attempt
	UE_LOG(LogTemp, Log, TEXT("Attempting to create session: %s"), *CustomServerName);
}
