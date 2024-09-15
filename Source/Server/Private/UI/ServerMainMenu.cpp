// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ServerMainMenu.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Multiplayer/Data/ServerSubsytemData.h"
#include "Server/Public/Multiplayer/ServerSessionsSubsystem.h"
#include "UI/ServerRowServers.h"

void UServerMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	PathToLevel = FString::Printf(TEXT("%s?listen"), *PathToLevel);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		ServerSessionsSubsystem = GameInstance->GetSubsystem<UServerSessionsSubsystem>();
	}

	if (ServerSessionsSubsystem)
	{
		ServerSessionsSubsystem->OnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		ServerSessionsSubsystem->OnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		ServerSessionsSubsystem->OnJoinSessionComplete.AddDynamic(this, &ThisClass::OnJoinSession);
		ServerSessionsSubsystem->OnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		ServerSessionsSubsystem->OnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}

	// Initialize buttons
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (FindButton)
	{
		FindButton->OnClicked.AddDynamic(this, &ThisClass::FindButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &ThisClass::QuitButtonClicked);
	}
	
	// Clear selected row initially
	//SelectedServerRow = nullptr;
}

void UServerMainMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		MenuTearDown();
		FString WelcomeMessage = FString::Printf(TEXT("Success to create session!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WelcomeMessage);
		UWorld* World = GetWorld();
		if (World)
		{
			//ServerSessionsSubsystem->StartSession();
			World->ServerTravel(PathToLevel, true);
			//UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen), true, TEXT("%s?listen");
		}
	}
	else
	{
		FString WelcomeMessage = FString::Printf(TEXT("Failed to create session!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WelcomeMessage);
		HostButton->SetIsEnabled(true);
	}
}

void UServerMainMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (bWasSuccessful && ServerListView)
	{
		ServerSessionsSubsystem->JoinSession(SessionResults[0]);
		//UpdateServerList(SessionResults);
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		FString WelcomeMessage = FString::Printf(TEXT("Fail to Join Session!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, WelcomeMessage);
		JoinButton->SetIsEnabled(true);
	}
}

void UServerMainMenu::OnJoinSession(bool bWasSuccessful, EEOSJoinSessionResultType Type, const FString Address)
{          
	if (bWasSuccessful)
	{
		FString WelcomeMessage = FString::Printf(TEXT("Session Aviable!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WelcomeMessage);
		
		MenuTearDown();

		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
	else
	{
		FString WelcomeMessage = FString::Printf(TEXT("No Session Aviable!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, WelcomeMessage);
	}
	
}

void UServerMainMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UServerMainMenu::OnStartSession(bool bWasSuccessful)
{
}

void UServerMainMenu::UpdateServerList(const TArray<FOnlineSessionSearchResult>& SearchResults)
{
	
	if (ServerListView)
	{
		ServerListView->ClearListItems();
		ServerRowMap.Empty(); // Clear the map when updating the list

		for (const auto& SearchResult : SearchResults)
		{
			UServerRowServers* ServerRow = CreateWidget<UServerRowServers>(this, UServerRowServers::StaticClass());
			if (ServerRow)
			{
				ServerRow->SetServerData(SearchResult);
				CurentIndex = CurentIndex + ServerRow->SelectedIndex + 1;
				ServerRow->SelectedIndex = CurentIndex;
				ServerRow->ServerRowButton->OnClicked.AddDynamic(this, &UServerMainMenu::OnRowClicked);
				//ServerRow->ServerRowButton->OnClicked.AddDynamic(this, &UServerMainMenu::JoinButtonClicked);
				// Add the row to the list and map
				ServerListView->AddItem(ServerRow);
				ServerRowMap.Add(ServerRow, SearchResult);
				SelectedServerRows.Add(ServerRow);
			}
		}
	}
}
void UServerMainMenu::JoinButtonClicked()
{
	
	for (const auto& SelectedSRow : SelectedServerRows)
	{
		if (ServerRowMap.Contains(SelectedSRow) && ServerSessionsSubsystem)
		{
			// Get the selected session data and attempt to join
			const FOnlineSessionSearchResult& SelectedSession = ServerRowMap[SelectedSRow];
			ServerSessionsSubsystem->JoinSession(SelectedSession);
		}
		else
		{
			// No row selected, or session not valid
			JoinButton->SetIsEnabled(true);
		}
	}
}
void UServerMainMenu::OnRowClicked()
{
	// Handle row selection
	if (UServerRowServers* ClickedRow = Cast<UServerRowServers>(ServerListView->GetSelectedItem()))
	{
		if (ServerRowMap.Contains(ClickedRow))
		{
			const FOnlineSessionSearchResult& SelectedSession = ServerRowMap[ClickedRow];
			ServerSessionsSubsystem->JoinSession(SelectedSession);
		}
	}
}
void UServerMainMenu::HostButtonClicked()
{
	if (ServerSessionsSubsystem)
	{
		HostButton->SetIsEnabled(false);
		ServerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType, ServerGameSession.ToString(), false, false, LANCheckBox->IsChecked() );
	}
}

void UServerMainMenu::FindButtonClicked()
{
	//JoinButton->SetIsEnabled(false);
	if (ServerSessionsSubsystem)
	{
		ServerSessionsSubsystem->FindSessions(200'000, LANCheckBox->IsChecked());
	}
}

void UServerMainMenu::QuitButtonClicked()
{
	// Get the player controller to handle quitting
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		// If running in standalone or packaged game, use Console Command to quit
		PlayerController->ConsoleCommand("quit");
	}
	else
	{
		// If running in editor, you might want to just exit the current level
		UE_LOG(LogTemp, Warning, TEXT("Quit button clicked - Exiting Game"));
        
		// If in editor, just close the PIE session
		if (GEngine)
		{
			GEngine->Exec(GetWorld(), TEXT("QUIT_EDITOR"));
		}
	}
}

void UServerMainMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World) 
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
