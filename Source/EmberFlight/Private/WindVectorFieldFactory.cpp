// Fill out your copyright notice in the Description page of Project Settings.

#include "WindVectorFieldFactory.h"
#include "WindVectorField.h"
#include <Exporters/Exporter.h>

UWindVectorFieldFactory::UWindVectorFieldFactory()
{
    SupportedClass = UWindVectorField::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

UObject* UWindVectorFieldFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
    EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UWindVectorField>(InParent, Class, Name, Flags);
}