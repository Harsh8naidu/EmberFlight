// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WindVectorField.h"
#include "EmberFlightGameMode.generated.h"
UCLASS(minimalapi)
class AEmberFlightGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AEmberFlightGameMode();

    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadWrite, Category="Wind Field")
    UWindVectorField* WindFieldInstance;
};