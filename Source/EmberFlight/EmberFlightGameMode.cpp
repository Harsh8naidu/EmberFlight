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

        // Injecting some wind in the center of the field
        FVector InjectPosition(0.0f, 0.0f, 0.0f); // World space
        FVector WindVelocity(10.0f, 0.0, 0.0f); // Wind
        float Radius = 20000.0f;

        WindFieldInstance->InjectWindAtPosition(InjectPosition, WindVelocity, Radius);
    }
}