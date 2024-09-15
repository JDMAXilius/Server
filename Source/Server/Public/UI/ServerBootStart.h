// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerBootStart.generated.h"

/**
 * 
 */
UCLASS()
class SERVER_API UServerBootStart : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	//virtual bool Initialize() override;
	
	UFUNCTION(BlueprintCallable)
	void OnLogin(bool bWasSuccessful, const FString User);

protected:
	
private:
	// The subsystem designed to handle all online session functionality
	class UServerUserSubsystem* ServerUserSubsystem;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Menu",  meta = (AllowPrivateAccess = "true"))
	FName PathToMainMenu{TEXT("MainMenu")};

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Server Menu",  meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> LevelToOpen;
};
