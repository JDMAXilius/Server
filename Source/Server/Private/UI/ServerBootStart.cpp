// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ServerBootStart.h"
#include "Kismet/GameplayStatics.h"
#include "Multiplayer/ServerUserSubsystem.h"

void UServerBootStart::NativeConstruct()
{
	Super::NativeConstruct();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		ServerUserSubsystem = GameInstance->GetSubsystem<UServerUserSubsystem>();
	}
	if (ServerUserSubsystem)
	{
		if (!ServerUserSubsystem->IsPlayerLoggedIn())
		{
			ServerUserSubsystem->OnLoginComplete.AddDynamic(this, &ThisClass::OnLogin);
			ServerUserSubsystem->Login();
		}
		else
		{
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			FString WelcomeMessage = FString::Printf(TEXT("User already logged in. Welcome player name: %s"), *LocalPlayer->GetNickname());
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WelcomeMessage);
			UE_LOG(LogTemp, Log, TEXT("User already logged in. Welcome player name: %s"), *LocalPlayer->GetNickname());
			// User is already logged in, proceed to main menu or desired state
			UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
		}
	}
}

/*bool UServerBootStart::Initialize()
{
	return Super::Initialize();
}*/

void UServerBootStart::OnLogin(bool bWasSuccessful, const FString User)
{
	// Unbind the delegate to avoid repeated triggers
	/*if (IdentityInterface.IsValid())
	{
		IdentityInterface->ClearOnLoginCompleteDelegates(0, this);
	}*/

	if (bWasSuccessful)
	{
		// Successful login, log message and proceed
		UE_LOG(LogTemp, Log, TEXT("EOS Login successful. Welcome player name: %s"), *User);
		FString WelcomeMessage = FString::Printf(TEXT("EOS Login successful. Welcome, UserID: %s"), *User);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WelcomeMessage);
	}
	else
	{
		// Login failed, display error message
		UE_LOG(LogTemp, Error, TEXT("EOS Login failed!"));
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Error: EOS Login failed!")));
	}
}
