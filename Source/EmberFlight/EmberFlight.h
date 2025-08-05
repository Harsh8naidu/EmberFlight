// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
* Custom game module for EmberFlight.
*/
class FEmberFlightModule : public FDefaultGameModuleImpl
{
public:
#if WITH_EDITOR
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
#endif
};

/**
* Utility function to list all Niagara Data Interfaces in the system
*/
void ListAllNiagaraInterfaces();