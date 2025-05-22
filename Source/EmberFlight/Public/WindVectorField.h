// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DrawDebugHelpers.h"
#include "WindVectorField.generated.h"
UCLASS(Blueprintable)
class EMBERFLIGHT_API UWindVectorField : public UObject
{
    GENERATED_BODY()

public:
	UWindVectorField();

    UFUNCTION(BlueprintCallable, Category="Wind Field")
    void Initialize(int InSizeX, int InSizeY, int InSizeZ, float InCellSize);
    UFUNCTION(BlueprintCallable, Category="Wind Field")
    void Update(float DeltaTime);
    UFUNCTION(BlueprintCallable, Category = "Wind Field")
    void InjectWindAtPosition(const FVector& WorldPos, const FVector& VelocityToInject, float Radius);
    UFUNCTION(BlueprintCallable, Category="Wind Field")
    FVector SampleWindAtPosition(const FVector& WorldPos) const;
    UFUNCTION(BlueprintCallable, Category="Wind Field")
    void DebugDraw(float Scale = 100.0f) const;
	
private:
    int SizeX, SizeY, SizeZ;
    float CellSize;

    TArray<FVector> VelocityGrid;

    int GetIndex(int X, int Y, int Z) const;
    bool IsValidIndex(int X, int Y, int Z) const;
    void Advect(float DeltaTime);
    void DecayVelocity(float DeltaTime);
    FVector const SampleVelocityAtGridPosition(const FVector& GridPos) const;
};