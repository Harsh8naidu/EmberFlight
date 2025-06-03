// Copyright Epic Games, Inc. All Rights Reserved.

#include "EmberFlight.h"
#include "NiagaraDataInterface.h"
#include "NiagaraDataInterfaceWindField.h"
#include "NiagaraDataInterface.h"
#include "UObject/UObjectIterator.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, EmberFlight, "EmberFlight" );

void FEmberFlightModule::StartupModule()
{
    
}

void FEmberFlightModule::ShutdownModule()
{
    
}

void ListAllNiagaraInterfaces()
{
    static const FName TempClassName = UNiagaraDataInterfaceWindField::StaticClass()->GetFName();

    for (TObjectIterator<UClass> It; It; ++It)
    {
        if (It->IsChildOf(UNiagaraDataInterface::StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract))
        {
            UE_LOG(LogTemp, Warning, TEXT("Found DI: %s"), *It->GetName());
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Wind DI class is: %s"), *UNiagaraDataInterfaceWindField::StaticClass()->GetName());
}