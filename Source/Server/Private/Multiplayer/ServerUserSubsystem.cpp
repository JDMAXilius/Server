// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/Public/Multiplayer/ServerUserSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineStatsInterface.h"

void UServerUserSubsystem::Login()
{
	
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	
	//Save delegate for EOS login step
	this->LoginDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(0, FOnLoginComplete::FDelegate::CreateUObject(this, &UServerUserSubsystem::HandleEOSLogin));
	
	//Actually do login if the user is not
	if (Identity->GetLoginStatus(0) != ELoginStatus::LoggedIn)
	{
		if (!Identity->AutoLogin(0))
		{
			FOnlineAccountCredentials Creds;
			Creds.Id = FString("127.0.0.1:6300");
			Creds.Token = FString("ShibCreative");
			Creds.Type = FString( "Developer");
			Identity->Login(0, Creds);
		}
	}
}

void UServerUserSubsystem::Logout()
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	checkf(Identity != nullptr, TEXT("Unable to get the Identity interface."));

	LogoutCompleteDelegateHandle = Identity->AddOnLogoutCompleteDelegate_Handle(0, FOnLogoutComplete::FDelegate::CreateUObject(this, &UServerUserSubsystem::HandleLogoutComplete));

	Identity->Logout(0);
}

FUniqueNetIdRepl UServerUserSubsystem::GetControllerUniqueNetId(APlayerController* InPlayerController) const
{
	if (!IsValid(InPlayerController))
	{
		return nullptr;
	}

	if (InPlayerController->IsLocalPlayerController())
	{
		ULocalPlayer *LocalPlayer = InPlayerController->GetLocalPlayer();
		if (IsValid(LocalPlayer))
		{
			return LocalPlayer->GetPreferredUniqueNetId().GetUniqueNetId();
		}
	}

	UNetConnection *RemoteNetConnection = Cast<UNetConnection>(InPlayerController->Player);
	if (IsValid(RemoteNetConnection))
	{
		return RemoteNetConnection->PlayerId.GetUniqueNetId();
	}

	UE_LOG(LogTemp, Error, TEXT("Player controller does not have a valid remote network connection"));
	return nullptr;
}

void UServerUserSubsystem::HandleEOSLogin(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
	const FString& Error)
{
	// Deregister the event handler.
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, this->LoginDelegateHandle);
	this->LoginDelegateHandle.Reset();
	
	UE_LOG(LogTemp, Log, TEXT( "Using the subsystem: %s"), *Subsystem->GetSubsystemName().ToString());
	
	if (bWasSuccessful)
	{
		TSharedPtr<FUserOnlineAccount> OnlineAccount = Identity->GetUserAccount(UserId);
		PlayerName = Identity->GetPlayerNickname(UserId);
		OnlineAccount->GetUserAttribute("externalAccount.epic.id", EpicAccountID);
		OnlineAccount->GetUserAttribute("productUserId", EpicProductUserID);
		OnlineAccount->GetAuthAttribute("authenticatedWith", AuthenticationSource);
		EOSConnectIDToken = Identity->GetAuthToken(LocalUserNum);
		NetID = FUniqueNetIdRepl(UserId);

		UE_LOG(LogTemp, Log, TEXT( "logged in successfully Player Name: %s"), *PlayerName);
		UE_LOG(LogTemp, Log, TEXT( "Epic Account ID is: %s"), *EpicAccountID);
		UE_LOG(LogTemp, Log, TEXT( "Product User ID is: %s"), *EpicProductUserID);
		UE_LOG(LogTemp, Log, TEXT( "was authenticated with: %s"), *AuthenticationSource);
		UE_LOG(LogTemp, Log, TEXT( "EOS Connect ID is: %s"), *EOSConnectIDToken);
		
		//Login to Playfab
		//PlayfabLogin(EpicAccountID);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT( "Failed to complete EOS Login with an error of: %s"), *Error);

		if (Error.Contains("EOS_InvalidAuth"))
		{
			UE_LOG(LogTemp, Warning, TEXT( "Potentially failed because it attempted to login a user with persistent authentication when they aren't logged in yet"));
			Login();
		}
	}

	OnEpicOnlineServicesLoginStep.Broadcast(bWasSuccessful);
	OnLoginComplete.Broadcast(bWasSuccessful, PlayerName);
}

void UServerUserSubsystem::HandleLogoutComplete(int LocalUserNum, bool bWasSuccessful)
{
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();

	Identity->ClearOnLogoutCompleteDelegate_Handle(0, LogoutCompleteDelegateHandle);
	LogoutCompleteDelegateHandle.Reset();

	OnLogoutComplete.Broadcast(bWasSuccessful);
}

bool UServerUserSubsystem::IsPlayerLoggedIn()
{
	
	IOnlineSubsystem* Subsystem = Online::GetSubsystem(this->GetWorld());
	IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
	
	return (Identity->GetLoginStatus(0) == ELoginStatus::Type::LoggedIn);
}
