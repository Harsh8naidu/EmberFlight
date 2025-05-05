// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmberFlightGameMode.h"
#include "EmberFlightCharacter.h"
#include "UObject/ConstructorHelpers.h"

AEmberFlightGameMode::AEmberFlightGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
