// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ServerUserSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoginUpdate, bool, IsSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FServerLoginCompleteDelegate, bool, bWasSuccessful, const FString, User);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FServerLogoutCompleteDelegate, bool, bWasSuccessful);

UCLASS()
class SERVER_API UServerUserSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	
	FDelegateHandle LoginDelegateHandle;

	UPROPERTY()
	FTimerHandle SaveAvatarDataTimerDelegate;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetPlayerName, meta=(AllowPrivateAccess))
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetAuthenticationSource, meta=(AllowPrivateAccess))
	FString AuthenticationSource;
	
	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetEpicAccountID, meta=(AllowPrivateAccess))
	FString EpicAccountID;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetEpicProductUserID, meta=(AllowPrivateAccess))
	FString EpicProductUserID;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetEOSConnectIDToken, meta=(AllowPrivateAccess))
	FString EOSConnectIDToken;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetPlayfabAccountID, meta=(AllowPrivateAccess))
	FString PlayfabAccountID;

	UPROPERTY(BlueprintReadOnly, BlueprintGetter=GetNetID, meta=(AllowPrivateAccess))
	FUniqueNetIdRepl NetID;

public:

	UPROPERTY(BlueprintReadOnly, BlueprintAssignable, Category = "Server Subsystems|User|Callback")
	FOnLoginUpdate OnEpicOnlineServicesLoginStep;
	
	
	UFUNCTION(BlueprintCallable, Category = "Server User Subsystem|Online")
	void Login();
	
	UFUNCTION(BlueprintCallable, Category = "Shib Subsystems|User")
	void Logout();
	
	FUniqueNetIdRepl GetControllerUniqueNetId(APlayerController* InPlayerController) const;
	
private:
	
	void HandleEOSLogin( int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId &UserId, const FString &Error);
	void HandleLogoutComplete(int LocalUserNum, bool bWasSuccessful);
	
public:

	UFUNCTION(BlueprintPure, Category = "Server User Subsystem|Online")
	bool IsPlayerLoggedIn();

	UFUNCTION(BlueprintGetter)
	inline FString GetPlayerName() const
	{
		return PlayerName;
	};
	
	UFUNCTION(BlueprintGetter)
	inline FString GetAuthenticationSource() const
	{
		return AuthenticationSource;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetEpicAccountID() const
	{
		return EpicAccountID;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetEpicProductUserID() const
	{
		return EpicProductUserID;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetEOSConnectIDToken() const
	{
		return EOSConnectIDToken;
	};

	UFUNCTION(BlueprintGetter)
	inline FString GetPlayfabAccountID() const
	{
		return PlayfabAccountID;
	};

	UFUNCTION(BlueprintGetter)
	inline FUniqueNetIdRepl GetNetID() const
	{
		return NetID;
	};

	UPROPERTY(BlueprintAssignable, Category = "Server Subsystems|User|Callback")
	FServerLoginCompleteDelegate OnLoginComplete;

	UPROPERTY(BlueprintAssignable, Category = "Server Subsystems|User|Callback")
	FServerLogoutCompleteDelegate OnLogoutComplete;

private:
	
	FDelegateHandle LoginCompleteDelegateHandle;
	FDelegateHandle LogoutCompleteDelegateHandle;

	/*void HandleLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void HandleLogoutComplete(int LocalUserNum, bool bWasSuccessful);*/
};
