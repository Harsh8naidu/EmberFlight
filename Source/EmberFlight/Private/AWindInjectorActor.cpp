// Fill out your copyright notice in the Description page of Project Settings.


#include "AWindInjectorActor.h"

AWindInjectorActor::AWindInjectorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    SetActorTickEnabled(true);
    
    FieldOrigin = GetActorLocation();
}

void AWindInjectorActor::BeginPlay()
{
    Super::BeginPlay();
    FieldOrigin = GetActorLocation();
}

void AWindInjectorActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    WindField = nullptr;
}

void AWindInjectorActor::ShowDebugSphere()
{
    if (bShowDebugSphere && GetWorld())
    {
        DrawDebugSphere(GetWorld(), FieldOrigin, Radius, 16, FColor::Red, false, 1.0f, 0, 2.0f);
        UE_LOG(LogTemp, Warning, TEXT("Debug Sphere Drawn At: %s | Radius: %f"), *GetActorLocation().ToString(), Radius);
    }
}

void AWindInjectorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableInjection && WindField)
    {
        WindField->InjectWindAtPosition(GetActorLocation(), VelocityToInject, FieldOrigin, Radius);
    }
}

#if WITH_EDITOR
void AWindInjectorActor::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    FieldOrigin = GetActorLocation();
    //UE_LOG(LogTemp, Warning, TEXT("FieldOrigin updated after move to: %s"), FieldOrigin.ToString());
}
#endif