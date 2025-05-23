// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmberFlightGameMode.h"
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

    // Initialize Wind Field
    WindFieldInstance = NewObject<UWindVectorField>(this);
    if (WindFieldInstance)
    {
        WindFieldInstance->Initialize(30, 30, 30, 100.0f);
    }
}