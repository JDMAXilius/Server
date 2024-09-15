// Fill out your copyright notice in the Description page of Project Settings.


#include "Server/Public/Game/ServerGameinstance.h"
#include "Misc/NetworkVersion.h"
#include "Multiplayer/ServerSessionsSubsystem.h"
#include "Multiplayer/ServerUserSubsystem.h"

void UServerGameinstance::Init()
{
	Super::Init();
	
	SessionsSubsystem = GetSubsystem<UServerSessionsSubsystem>();
	UserSubsystem = GetSubsystem<UServerUserSubsystem>();

	GetEngine()->OnNetworkFailure().AddUObject(this, &UServerGameinstance::HandleNetworkFailure);
	GetEngine()->OnTravelFailure().AddUObject(this, &UServerGameinstance::HandleTravelFailure);
	
#if !UE_BUILD_SHIPPING
	FNetworkVersion::IsNetworkCompatibleOverride.BindLambda([](
		uint32 LocalNetworkVersion,
		uint32 RemoteNetworkVersion
	) { return true; });
#endif
	
}

void UServerGameinstance::Shutdown()
{
	// Handle session destruction if we are a dedicated server
	if (IsDedicatedServerInstance())
	{
		SessionsSubsystem->DestroySession();
	}
	Super::Shutdown();
	
}

void UServerGameinstance::TravelToSession(APlayerController* Controller, FString Address)
{
	if (!Controller) return;
	Controller->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
}

void UServerGameinstance::HandleNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	bIsGameError = true;
	GameErrorMsg = ErrorString;
	SessionsSubsystem->DestroySession();
}

void UServerGameinstance::HandleTravelFailure(UWorld* InWorld, ETravelFailure::Type FailureType,
	const FString& ErrorString)
{
	bIsGameError = true;
	GameErrorMsg = ErrorString;
	SessionsSubsystem->DestroySession();
}
