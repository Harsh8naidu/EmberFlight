// Fill out your copyright notice in the Description page of Project Settings.


#include "AWindInjectorActor.h"

AWindInjectorActor::AWindInjectorActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    SetActorTickEnabled(true);
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

void AWindInjectorActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableInjection && WindField)
    {
        WindField->InjectWindAtPosition(GetActorLocation(), VelocityToInject, FieldOrigin, Radius);
    }

#if WITH_EDITOR
    if (IsInEditorMode() && bShowDebugSphereAlwaysInEditor)
    {
        ShowDebugSphere(true, 0.0f);
    }
#endif

    if (!IsInEditorMode() && bShowDebugSphereInPlayMode)
    {
        ShowDebugSphere(false, 0.1f);
    }
}

void AWindInjectorActor::ShowDebugSphere(bool persistentSphere, float lifetime)
{
    if (!GetWorld()) return;
    
    DrawDebugSphere(GetWorld(), FieldOrigin, Radius * 2.6, 16, FColor::Red, persistentSphere, lifetime, 0, 2.0f);
}

#if WITH_EDITOR
void AWindInjectorActor::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    FlushPersistentDebugLines(GetWorld());
    FieldOrigin = GetActorLocation();
    DrawTemporaryDebugSphere();
}

void AWindInjectorActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    FlushPersistentDebugLines(GetWorld());
    FieldOrigin = GetActorLocation();
    DrawTemporaryDebugSphere();
}
#endif

void AWindInjectorActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    FieldOrigin = GetActorLocation();
    //DrawTemporaryDebugSphere();
}

void AWindInjectorActor::DrawTemporaryDebugSphere()
{
    ShowDebugSphere(false, 0.1f);
}

bool AWindInjectorActor::IsInEditorMode() const
{
#if WITH_EDITOR
    return GIsEditor && !GetWorld()->IsGameWorld();
#else
    return false;
#endif
}

#if WITH_EDITOR
bool AWindInjectorActor::ShouldTickIfViewportsOnly() const
{
    return true;
}
#endif