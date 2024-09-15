// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/Public/Multiplayer/ServerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Multiplayer/ServerUserSubsystem.h"
#include "Online/OnlineSessionNames.h"
#include "OnlineSubsystemUtils.h"
#include "Multiplayer/Data/ServerSubsytemData.h"

UServerSessionsSubsystem::UServerSessionsSubsystem()
{
	
}

void UServerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const auto* Subsystem = Online::GetSubsystem(GetWorld());
	checkf(Subsystem != nullptr, TEXT("Unable to get Online Subsystem."));

	SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	bCreateSessionOnDestroy = false;
}

void UServerSessionsSubsystem::Deinitialize()
{
	Super::Deinitialize();
	SessionInterface.Reset();
}

void UServerSessionsSubsystem::CreateSession(const int32 NumConnections, FString MatchType, const FString& SessionName,
	const bool bIsDedicatedServer, const bool bIsPrivate, const bool bIsLANMatch)
{
	//IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	//IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	// Save session params
	LastNumPublicConnections = NumConnections;
	LastSessionName = SessionName;
	bLastSessionIsPrivate = bIsPrivate;
	bLastSessionIsLAN = bIsLANMatch;
	DesiredMatchType = MatchType;
	if (!SessionInterface.IsValid())
	{
		OnCreateSessionComplete.Broadcast(false);
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(ServerGameSession);
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumConnections;
		LastMatchType = MatchType;

		DestroySession();
	}

	// Store the delegate in a FDelegateHandle so we can later remove it from the delegate list
	//CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionComplete::FDelegate::CreateUObject(this, &UServerSessionsSubsystem::HandleCreateSessionComplete));
	CreateSessionCompleteDelegateHandle = SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &ThisClass::HandleCreateSessionComplete);
	
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	LastSessionSettings->bIsLANMatch = bIsLANMatch;
	LastSessionSettings->NumPublicConnections = NumConnections;
	LastSessionSettings->bAllowJoinInProgress = true;
	LastSessionSettings->bAllowJoinViaPresence = true;
	LastSessionSettings->bShouldAdvertise = true;
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(FName("IsPrivate"), bIsPrivate, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(FName("SessionName"), SessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(FName("SEARCHKEYWORDS"), FString("RandomHi"), EOnlineDataAdvertisementType::ViaOnlineService);
	LastSessionSettings->bUseLobbiesIfAvailable = true;
#if !UE_BUILD_SHIPPING
	//Enforce a specific Build ID in not shipping so we can
	//easily test session creation
	//LastSessionSettings->BuildUniqueId = 1;
#endif
	
	//Create the session
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	const bool success = SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), ServerGameSession, *LastSessionSettings);
	if (!success)
	{
		//We failed to create the session simply remove the delegate for completion
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		CreateSessionCompleteDelegateHandle.Reset();
		OnCreateSessionComplete.Broadcast(false);
	}
}

void UServerSessionsSubsystem::FindSessions(const int32 MaxSearchResults, const bool bIsLANMatch)
{
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());
	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	LastSessionSearch->bIsLanQuery = bIsLANMatch;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	//LastSessionSearch->QuerySettings.Set(FName("MatchType"), LastMatchType, EOnlineComparisonOp::Equals);

	//Register the delegate for when the find session complete and store its handle for later removal
	//FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsComplete::FDelegate::CreateUObject(this, &UServerSessionsSubsystem::HandleFindSessionsComplete, LastSessionSearch.ToSharedRef()));
	FindSessionsCompleteDelegateHandle = SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &ThisClass::HandleFindSessionsComplete);
	
	//Get the player's Unique Net ID from our user subsystem
	//auto* GameInstance = GetGameInstance();
	//auto* UserSubsystem = GameInstance->GetSubsystem<UServerUserSubsystem>();
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//const bool success = SessionInterface->FindSessions(*UserSubsystem->GetNetID().GetUniqueNetId(), LastSessionSearch.ToSharedRef());
	const bool success =SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef());
	if (!success)
	{
		//We failed to find sessions simply remove the delegate for completion
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		FindSessionsCompleteDelegateHandle.Reset();
		
		OnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UServerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;
	
	//Register the delegate for when the join session complete and store its handle for later removal
	//JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionComplete::FDelegate::CreateUObject(this, &UServerSessionsSubsystem::HandleJoinSessionComplete));
	JoinSessionCompleteDelegateHandle = SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UServerSessionsSubsystem::HandleJoinSessionComplete);
	
	//Get the player's Unique Net ID from our user subsystem
	//auto* GameInstance = GetGameInstance();
	//auto* UserSubsystem = GameInstance->GetSubsystem<UServerUserSubsystem>();
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	//const bool success = SessionInterface->JoinSession(*UserSubsystem->GetNetID().GetUniqueNetId(), ServerGameSession, SessionResult);
	const bool success = SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), ServerGameSession, SessionResult);
	if (!success)
	{
		//We failed to join session simply remove the delegate for completion
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		JoinSessionCompleteDelegateHandle.Reset();

		OnJoinSessionComplete.Broadcast(false, EEOSJoinSessionResultType::UnknownError, "");
	}
}

void UServerSessionsSubsystem::StartSession()
{
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionComplete::FDelegate::CreateUObject(this, &UServerSessionsSubsystem::HandleStartSessionComplete));

	if (!SessionInterface->StartSession(ServerGameSession))
	{
		//We failed to join session simply remove the delegate for completion
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		StartSessionCompleteDelegateHandle.Reset();
		OnStartSessionComplete.Broadcast(false);
	}
}

void UServerSessionsSubsystem::EndSession()
{
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	//Register the delegate for when the destroy session complete and store its handle for later removal
	//EndSessionCompleteDelegateHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionComplete::FDelegate::CreateUObject(this, &UServerSessionsSubsystem::HandleEndSessionComplete));
	EndSessionCompleteDelegateHandle = SessionInterface->OnEndSessionCompleteDelegates.AddUObject(this, &ThisClass::HandleEndSessionComplete);
	
	//End existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ServerGameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->EndSession(ServerGameSession);
	}
	else //No session found
		{
		//Remove the end session completion delegate
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
		EndSessionCompleteDelegateHandle.Reset();

		OnEndSessionComplete.Broadcast(false);
		}
}

void UServerSessionsSubsystem::DestroySession()
{
	if (!ensureMsgf(SessionInterface.IsValid(), TEXT("Unable to get the Session Interface"))) return;

	//Register the delegate for when the destroy session complete and store its handle for later removal
	//DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionComplete::FDelegate::CreateUObject(this, &UServerSessionsSubsystem::HandleDestroySessionComplete));
	DestroySessionCompleteDelegateHandle = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UServerSessionsSubsystem::HandleDestroySessionComplete);
	
	//Destroy existing session if it exists
	const auto ExistingSession = SessionInterface->GetNamedSession(ServerGameSession);
	if (ExistingSession != nullptr)
	{
		SessionInterface->DestroySession(ServerGameSession);
	}
	else //No session found
		{
		//Remove the destroy session completion delegate
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();

		OnDestroySessionComplete.Broadcast(false);
		}
}

bool UServerSessionsSubsystem::RegisterPlayer(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	//IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	// Register the player with the session name; this name should match the name you provided in CreateSession.
	return SessionInterface->RegisterPlayer(ServerGameSession, *UniqueNetId, false);
}

bool UServerSessionsSubsystem::RegisterExistingPlayers()
{
	for (auto It = this->GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PlayerController = It->Get();

		// if (PlayerController->HasAuthority()) continue;

		// Register players, stop execution and return false if one of them has not successfully registered.
		if (!RegisterPlayer(PlayerController)) return false;
	}

	return true;
}

bool UServerSessionsSubsystem::UnregisterPlayer(APlayerController* InPlayerController)
{
	check(IsValid(InPlayerController));

	// This code handles logins for both the local player (listen server) and remote players (net connection).
	FUniqueNetIdRepl UniqueNetIdRepl;
	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer* LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			UniqueNetIdRepl = LocalPlayer->GetPreferredUniqueNetId();
		}
		else
		{
			UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
			check(IsValid(RemoteNetConnection));
			UniqueNetIdRepl = RemoteNetConnection->PlayerId;
		}
	}
	else
	{
		UNetConnection* RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
		check(IsValid(RemoteNetConnection));
		UniqueNetIdRepl = RemoteNetConnection->PlayerId;
	}

	// Get the unique player ID.
	TSharedPtr<const FUniqueNetId> UniqueNetId = UniqueNetIdRepl.GetUniqueNetId();
	check(UniqueNetId != nullptr);

	// Get the online session interface.
	//IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
//	IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

	// Unregister the player with the session name; this name should match the name you provided in CreateSession.
	return SessionInterface->UnregisterPlayer(ServerGameSession, *UniqueNetId);
}

void UServerSessionsSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		CreateSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("Session named %s created: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UServerSessionsSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Num of Sessions available: %d"), SessionInterface->GetNumSessions());
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		FindSessionsCompleteDelegateHandle.Reset();
	}
	
	UE_LOG(LogTemp, Log, TEXT("LastSearchResults, Server Counts: %d"), LastSessionSearch->SearchResults.Num());
	
	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		TArray<FOnlineSessionSearchResult> Results;
		UE_LOG(LogTemp, Log, TEXT("Sessions found: false"));
		OnFindSessionsComplete.Broadcast(Results, false);
		return;
	}

	TArray<FOnlineSessionSearchResult> Results;
	Results.Reserve(LastSessionSearch->SearchResults.Num());

	for (const auto& result : LastSessionSearch->SearchResults)
	{
		if (result.IsValid())
		{
			FOnlineSessionSearchResult r{};
			r = result;
			Results.Add(r);
		}
	}

	if (Results.Num() != 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Sessions found: true"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Sessions found: false"));
	}
	UE_LOG(LogTemp, Log, TEXT("FinalsSearchResults, Server Counts: %d"), Results.Num());
	OnFindSessionsComplete.Broadcast(Results, true);
}

void UServerSessionsSubsystem::HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	UE_LOG(LogTemp, Log, TEXT("Session named %s joinned: %s"), *SessionName.ToString(), Result == EOnJoinSessionCompleteResult::Type::Success ? TEXT("true") : TEXT("false"));

	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		JoinSessionCompleteDelegateHandle.Reset();

		// Add network error handle if the game client fails to connect to the server
		NetworkFailureDelegateHandle = GEngine->OnNetworkFailure().AddUObject(this, &UServerSessionsSubsystem::HandleNetworkFailure);
	}
	else
	{
		OnJoinSessionComplete.Broadcast(Result == EOnJoinSessionCompleteResult::Type::Success, UnknownError, "");
		return;
	}

	EEOSJoinSessionResultType ResultBP = EEOSJoinSessionResultType::UnknownError;
	switch (Result)
	{
	case EOnJoinSessionCompleteResult::Success:
		ResultBP = Success;
		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		ResultBP = SessionIsFull;
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		ResultBP = SessionDoesNotExist;
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		ResultBP = CouldNotRetrieveAddress;
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		ResultBP = AlreadyInSession;
		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		ResultBP = UnknownError;
		break;
	}

	//Get the session address to make a client travel
	FString Address;
	SessionInterface->GetResolvedConnectString(ServerGameSession, Address);
	
	//Fire our own delegate
	OnJoinSessionComplete.Broadcast(Result == EOnJoinSessionCompleteResult::Type::Success, ResultBP, Address);
}

void UServerSessionsSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		DestroySessionCompleteDelegateHandle.Reset();

		// Unregister network failure handler when we destroy the current session
		if (NetworkFailureDelegateHandle.IsValid())
		{
			GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
			NetworkFailureDelegateHandle.Reset();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Session named %s destroyed: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnDestroySessionComplete.Broadcast(bWasSuccessful);

	// Check if we need to create another session right after it got destroyed
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false; // Make sure to set this back to false to not get stuck in a loop
		//CreateSession(LastNumPublicConnections, LastSessionName, bLastSessionIsDedicatedServer, bLastSessionIsPrivate, bLastSessionIsLAN);
	}
}

void UServerSessionsSubsystem::HandleEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
		EndSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("Session named %s ended: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnEndSessionComplete.Broadcast(bWasSuccessful);
}

void UServerSessionsSubsystem::HandleStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		StartSessionCompleteDelegateHandle.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("Session named %s started: %s"), *SessionName.ToString(), bWasSuccessful ? TEXT("true") : TEXT("false"));
	OnStartSessionComplete.Broadcast(bWasSuccessful);
}

void UServerSessionsSubsystem::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->DestroySession(ServerGameSession);
		// Optional: Notify players or handle reconnection logic
	}

	// Unregister network failure handler
	GEngine->OnNetworkFailure().Remove(NetworkFailureDelegateHandle);
	NetworkFailureDelegateHandle.Reset();
}
