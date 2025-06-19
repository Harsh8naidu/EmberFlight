// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "WindVectorFieldFactory.generated.h"

UCLASS()
class EMBERFLIGHT_API UWindVectorFieldFactory : public UFactory
{
    GENERATED_BODY()

public:
	UWindVectorFieldFactory();
	
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
        EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;

    virtual bool ShouldShowInNewMenu() const override { return true; }
};
