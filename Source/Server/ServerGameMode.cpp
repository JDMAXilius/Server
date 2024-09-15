// Copyright Epic Games, Inc. All Rights Reserved.

#include "ServerGameMode.h"
#include "ServerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AServerGameMode::AServerGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
