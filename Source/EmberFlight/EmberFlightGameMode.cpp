// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmberFlightGameMode.h"
#include "EmberFlight.h"
#include "UObject/ConstructorHelpers.h"

AEmberFlightGameMode::AEmberFlightGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("Blueprint'/Game/Blueprints/BP_PhoenixCharacter.BP_PhoenixCharacter'"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void AEmberFlightGameMode::BeginPlay()
{
    Super::BeginPlay();
    //ListAllNiagaraInterfaces(); // Uncomment this line to list all the niagara interfaces (including the custom ones)

     //Initialize Wind Field
    if (WindFieldInstance)
    {
        WindFieldInstance->Initialize();
    }
}